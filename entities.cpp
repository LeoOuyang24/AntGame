#include "SDLHelper.h"
#include "render.h"

#include "entities.h"
#include "game.h"
#include "ants.h"
#include "navigation.h"

void renderMeter(const glm::vec3& xyWidth, const glm::vec4& color, double current, double maximum, float z)
{
    const int height = 10;
    glm::vec4 renderRect = {xyWidth.x,xyWidth.y,xyWidth.z*current/maximum,height};
    GameWindow::requestRect(renderRect,color,true, 0, z);
    GameWindow::requestRect(renderRect,{0,0,0,1},false,0,z);
}

ClickableComponent::ClickableComponent(std::string name, Entity& entity) : Component(entity), ComponentContainer<ClickableComponent>(&entity), name(name)
{

}

const glm::vec2 ClickableComponent::spacing = {10,5};

void ClickableComponent::click(bool val)
{
    clicked = val;
}

bool ClickableComponent::getClicked()
{
    return clicked;
}

void ClickableComponent::update()
{
    const glm::vec4* unitRect = &(entity->getComponent<RectComponent>()->getRect());
    int size = buttons.size();
    bool stillClicked = false;
    glm::vec2 mousePos = GameWindow::getCamera().toWorld({MouseManager::getMousePos().first, MouseManager::getMousePos().second});
    const glm::vec4* camRect = &(GameWindow::getCamera().getRect());
    if (clicked)
    {
        int offset = 0;
        for (int i = 0; i < size; ++i) //render and update all buttons
        {
            const glm::vec4* rect = &(buttons[i]->getRect());
            bool rightSpace = (camRect->x + camRect->z - unitRect->x - unitRect->z > spacing.x); //true if we have enough space on the right
            bool leftSpace = (unitRect->x - camRect->x > spacing.x);
            bool upSpace = unitRect->y - camRect->y > spacing.y;
            bool downSpace =  camRect->y + camRect->a - unitRect->y - unitRect->a > spacing.y;
            glm::vec4 buttonRect = {0,0,rect->z,rect->a};
            if (rightSpace || leftSpace)
            {
                buttonRect.x = unitRect->x + (unitRect->z+spacing.x)*rightSpace - (rect->z +spacing.x)*(!rightSpace && leftSpace);
                buttonRect.y = unitRect->y + spacing.y + offset;
            }
            else
            {
                buttonRect.x = unitRect->x + spacing.x;
                buttonRect.y = unitRect->y + (unitRect->a+spacing.y)*downSpace - (rect->a +spacing.y)*(!downSpace && upSpace);
            }
            buttons[i]->changeRect(buttonRect);
            glm::vec4 disp = GameWindow::getCamera().getRect();
            buttons[i]->render(-disp.x,-disp.y);
            //GameWindow::requestRect(buttonRect,{0,1,0,1},true,0,0);
            if (pointInVec(buttonRect,mousePos.x,mousePos.y,0))
            {
                if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT)
                {
                    stillClicked = true;
                    buttons[i]->press();
                    break;
                }
            }
            offset += rect->a + spacing.y;
        }
        GameWindow::requestRect(*unitRect,GameWindow::selectColor,true,0,0.01);
    }
    /*bool becomeClicked = (MouseManager::isPressed(SDL_BUTTON_LEFT) && vecIntersect(GameWindow::getSelection(),*unitRect));
        if (entity->getComponent<Ant::AntMoveComponent>())
        {
           // std::cout << stillClicked << " " << (MouseManager::getJustReleased() != SDL_BUTTON_LEFT && clicked) << std::endl;
        }*/
        //stillClicked = stillClicked ;//|| (MouseManager::getJustClicked() != SDL_BUTTON_LEFT && clicked);//  ||  becomeClicked;*/
        clicked = stillClicked || (MouseManager::getJustClicked() == SDL_BUTTON_LEFT && pointInVec(*unitRect,mousePos.x,mousePos.y));
}
void ClickableComponent::display(const glm::vec4& rect)
{
    Font::tnr.write(Font::wordProgram,{name,rect});
}

void ClickableComponent::addButton(Button& button)
{
    buttons.emplace_back(&button);
}

ClickableComponent::~ClickableComponent()
{

}

RectRenderComponent::RectRenderComponent(const glm::vec4& color, Entity& unit) : RenderComponent(unit), ComponentContainer<RectRenderComponent>(&unit), color(color)
{

}

void RectRenderComponent::update()
{
    GameWindow::requestRect(((Object*)entity)->getRect().getRect(),color,true,0,0);
}

void RectRenderComponent::render(const SpriteParameter& param)
{
    GameWindow::requestRect(param.rect,color*param.tint,true,param.radians,param.z,true);
}

RectRenderComponent::~RectRenderComponent()
{

}

Object::Object(ClickableComponent& click, RectComponent& rect_, RenderComponent& render_, bool mov) : Entity(),movable(mov), clickable(&click), rect(&rect_), render(&render_)
{
    addComponent(click);
    addComponent(rect_);
    addComponent(render_);


}
RectComponent& Object::getRect() const
{
    return *rect;
}
glm::vec2 Object::getCenter()
{
    return rect->getCenter();
}
ClickableComponent& Object::getClickable()
{
    return *clickable;
}
RenderComponent& Object::getRender()
{
    return *render;
}
bool Object::clicked()
{
    return clickable->getClicked();
}
bool Object::getDead()
{
    return dead;
}

bool Object::getMovable()
{
    return movable;
}

void Object::setDead(bool isDead)
{
    dead = isDead;
}
Object::~Object()
{
    //std::cout << "Object Destructor" << std::endl;
}

InteractionComponent::InteractionComponent(Object& unit) : Component(unit), ComponentContainer<InteractionComponent>(&unit)
{

}

void InteractionComponent::interact(Object& actor)
{

}

InteractionComponent::~InteractionComponent()
{

}

void HealthComponent::addHealth(int amount)
{
    if (amount < 0)
    {
        if (invincible.framesPassed(10) || !invincible.isSet())
        {
            invincible.set();
            health = std::max(0.0,std::min(health + amount, maxHealth));
        }
    }
    else
    {
        health = std::max(0.0,std::min(health + amount, maxHealth));
    }
}

HealthComponent::HealthComponent(Entity& entity, double health_,  int displacement_) : Component(entity), ComponentContainer<HealthComponent>(&entity), health(health_), maxHealth(health_), displacement(displacement_)//height defaults to 10 and displacement defaults to 20
{

}

void HealthComponent::takeDamage(int amount, Object& attacker)
{
    lastAttacker = GameWindow::getLevel().getUnit(&attacker);
    addHealth(-1*amount);
}

double HealthComponent::getHealth()
{
    return health;
}

double HealthComponent::getMaxHealth()
{
    return maxHealth;
}

void HealthComponent::setVisible(bool value)
{
    visible = value;
}


void HealthComponent::update()
{
    RectComponent* rectComp = entity->getComponent<RectComponent>();
    if (rectComp && visible)
    {
        const glm::vec4* rect = &(rectComp->getRect());
        //GameWindow::requestRect({rect->x ,rect->y - displacement, rect->z, 0},{1,0,0,1},true,0,0);
        render({rect->x,rect->y - displacement, rect->z}, 0);
    }
}

void HealthComponent::render(const glm::vec3& rect, float z)
{
    //PolyRender::requestRect({rect.x,rect.y,health/maxHealth*rect.z,height},{1,0,0,1},true,0,rect.a);
    renderMeter({rect.x,rect.y,rect.z},{1,0,0,1},health,maxHealth,z);
}

Object* HealthComponent::getLastAttacker()
{
    return lastAttacker.lock().get();
}

HealthComponent::~HealthComponent()
{

}


Unit::Unit(ClickableComponent& click, RectComponent& rect, RenderComponent& render, HealthComponent& health, bool mov) : Object(click,rect,render, mov), health(&health)
{
    addComponent(health);
}

void Unit::setManager(Manager& manager)
{
    this->manager = &manager;
}

RepelComponent::RepelComponent(Object& unit) : Component(unit), ComponentContainer<RepelComponent>(unit)
{

}

void RepelComponent::collide(Entity& unit)
{
    Object* ptr = static_cast<Object*>(&unit);
    auto otherMove = ptr->getComponent<MoveComponent>();
    if (ptr->getMovable() && (!otherMove || otherMove->getVelocity() == 0)) //if the unit can be moved and isn't currently moving.
    {
        auto ourMove = entity->getComponent<RectComponent>();
        if (otherMove && ourMove)
        {
            const glm::vec4* otherRect = &otherMove->getRect();
            const glm::vec4* ourRect = &ourMove->getRect();
            otherMove->teleport({otherRect->x + otherRect->z/2 + convertTo1(otherRect->x - ourRect->x)*1, otherRect->y + otherRect->a/2 + convertTo1(otherRect->y - ourRect->y)*1});
        }

    }
}

HealthComponent& Unit::getHealth()
{
    return *health;
}

void Unit::interact(Ant& ant)
{

}

Manager* Unit::getManager()
{
    return manager;
}

Structure::Structure(ClickableComponent& click, RectComponent& rect, RenderComponent& render, HealthComponent& health) : Unit(click,rect,render,health,false)
{
    addComponent(*(new RepelComponent(*this)));
}

ResourceComponent::ResourceComponent(int amount, Entity& entity) : Component(entity), ComponentContainer<ResourceComponent>(entity), resource(amount), maxResource(amount)
{

}

int ResourceComponent::getResource()
{
    return resource;
}
int ResourceComponent::getMaxResource()
{
    return maxResource;
}
void ResourceComponent::setResource(double amount)
{
    resource = std::max(std::min(resource + amount, (double)maxResource),0.0);
}

void ResourceComponent::render(const glm::vec3& rect, float z)
{
    renderMeter(rect,{0,1,0,1},resource,maxResource,z);
}

void ResourceComponent::collect(Object& other)
{
    if (entity && !((Object*)entity)->getDead())
    {
          Ant::AntMoveComponent* antMove = other.getComponent<Ant::AntMoveComponent>();
        if (antMove)
        {
            antMove->setCarrying(1);
        }

        resource -=1;
        if (resource <= 0)
        {
            ((Object*)(entity))->setDead(true);
        }
    }

}

ResourceComponent::~ResourceComponent()
{

}

CorpseComponent::CorpseComponent(Unit& unit, int amount_) : Component(unit), ComponentContainer<CorpseComponent>(&unit), amount(amount_), render(unit.getComponent<RenderComponent>())
{

}

CorpseComponent::~CorpseComponent()
{

}

void CorpseComponent::onDeath()
{
    if (entity)
    {
        ResourceUnit* resource = new ResourceUnit(amount,entity->getComponent<RectComponent>()->getRect());
        GameWindow::getLevel().addUnit(*(resource));
    }
}

ResourceUnit::ResourceUnit(int resources, const glm::vec4& rect) : Unit(*(new ClickableComponent("Resource", *this)), *(new RectComponent(rect, *this)), *(new RectRenderComponent({1,1,1,1},*this)), *(new HealthComponent(*this,1,false)))
{
    addComponent(*(new ResourceComponent(resources,*this)));
    health->setVisible(false);
}

ResourceUnit::~ResourceUnit()
{

}

PathComponent::PathComponent(double speed, const glm::vec4& rect, Entity& unit) : MoveComponent(speed,rect,unit), ComponentContainer<PathComponent> (unit)
{

}

void PathComponent::setTarget(const glm::vec2& point)
{
    if (getTarget() != point)
    {
        path.clear();
        NavMesh* mesh = &(GameWindow::getLevel().getMesh());
        try
        {
            path = mesh->getPath(getCenter(),point);
            target = path.front();
        }
        catch (...)
        {
            target = point;
        }
    }
    //MoveComponent::setTarget(point);
}

const glm::vec2& PathComponent::getTarget()
{
    if (path.size() == 0)
    {
        return target;
    }
    return path.back();
}

void PathComponent::addPoint(const glm::vec2& point)
{
    path.push_back(point);
}

void PathComponent::update()
{
    Debug::DebugNavMesh::showPath(path);
        //std::cout << target.x << " " << target.y << std::endl;
    if (atTarget())
    {
        if (path.size() > 1) //if we haven't reached the end of the path, select the next point
        {
            path.pop_front();
            target = path.front();
           // std::cout << "New: " << target.x << " " << target.y << std::endl;
        } //otherwise, we're done
    }
    MoveComponent::update();

}

WanderMove::WanderMove(double speed, const glm::vec4& rect, Entity& unit) : MoveComponent(speed, rect, unit), ComponentContainer<WanderMove>(unit)
{

}

WanderMove::~WanderMove()
{

}

void WanderMove::update()
{
    if (atTarget())
    {
        double angle = rand()%360*M_PI/180.0;
        int maxDimen = std::max(rect.z,rect.a);
        double radius = rand()%(100 - 10) + maxDimen + 10;
        glm::vec2 point = {rect.x + rect.z/2 + cos(angle)*radius, rect.y + rect.a/2 + sin(angle)*radius};
        Map* level = &(GameWindow::getLevel());
        const glm::vec4* levelRect = &(level->getRect(level->getChunk(*(Unit*)entity)));
        point.x = std::max(levelRect->x, std::min(point.x, levelRect->x + levelRect->z - rect.z));
        point.y = std::max(levelRect->y, std::min(point.y, levelRect->y + levelRect->a - rect.a));
        setTarget(point);
    }
    else
    {
        MoveComponent::update();
    }
}

ApproachComponent::ApproachComponent(Unit& entity) : Component(entity), ComponentContainer<ApproachComponent>(entity), move(entity.getComponent<MoveComponent>())
{

}

template <typename T>
Object* ApproachComponent::findNearestUnit(double radius)
{
    Object* owner = ((Object*)entity);
    Entity* closest = nullptr;
    double minDistance = -1;
    if (owner)
    {
        RawQuadTree* tree = GameWindow::getLevel().getTreeOf(*owner);
        if (tree)
        {
            glm::vec2 center = owner->getRect().getCenter();
            std::vector<Positional*> nearby = tree->getNearest( center , radius);
            int size = nearby.size();
            for (int i = 0; i < size; ++i)
            {
                Entity* current = &(static_cast<RectComponent*>(nearby[i])->getEntity());
                if (current->getComponent<T>())
                {
                    double distance = current->getComponent<RectComponent>()->distance(center);
                    if ((distance < minDistance || minDistance == -1) && current != owner)
                    {
                        minDistance = distance;
                        closest = current;
                    }
                }
            }
        }
    }
    return static_cast<Object*>(closest);
}
void ApproachComponent::setMove(MoveComponent& move_)
{
    move = &move_;
}
Object* ApproachComponent::getTargetUnit()
{
    return targetUnit.lock().get();
}
void ApproachComponent::setTarget(const glm::vec2& target, const std::shared_ptr<Object>* unit)
{
    if (move)
    {
        if (unit)
        {
            const glm::vec4* tarRect = &((*unit)->getRect().getRect());
            displacement = {target.x - (tarRect->x + tarRect->z/2) , target.y - (tarRect->y + tarRect->a/2)};
            targetUnit = *unit;
        }
        else
        {
            targetUnit.reset();
        }
        move->setTarget(target);
    }
}

void ApproachComponent::setTarget(const std::shared_ptr<Object>& unit)
{
    setTarget(unit->getCenter(),&unit);
}

void ApproachComponent::update()
{
    if (move)
    {
        Object* ptr = targetUnit.lock().get();
        if (ptr) //if we have a target...
        {
            //glm::vec2 center = ptr->getRect().getCenter() + displacement;
            if (move->collides(ptr->getRect().getRect())) //if at the target, stop moving
            {
                move->setTarget(move->getCenter());
            }
            else if (!pointInVec(ptr->getRect().getRect(),move->getTarget().x,move->getTarget().y,0)) //otherwise, approach
            {
                move->setTarget(closestPointOnVec(ptr->getRect().getRect(),move->getTarget()));
                displacement = move->getTarget() - ptr->getRect().getCenter(); //sometimes, the point can't be reached. Set the target to the point returned by getPath.
            }
        }
       // move->update();
    }
}

ApproachComponent::~ApproachComponent()
{

}


AttackComponent::AttackComponent(float damage_, int endLag_, Unit& unit) : ApproachComponent(unit), ComponentContainer<AttackComponent>(&unit),  damage(damage_), endLag(endLag_)
{

}

void AttackComponent::attack(HealthComponent* health)
{
    if (entity && health && (attackTimer.timePassed(endLag) || !attackTimer.isSet()))
    {
        health->takeDamage(damage,*static_cast<Unit*>(entity));
        attackTimer.set();
    }
}

void AttackComponent::update()
{
    ApproachComponent::update();
    Object* ptr = targetUnit.lock().get();
    if (move && ptr && ptr->getComponent<HealthComponent>() && vecIntersect(move->getRect(),ptr->getRect().getRect()))
    {
        attack(ptr->getComponent<HealthComponent>());
    }
}

AttackComponent::~AttackComponent()
{

}

ShootComponent::ShootComponent(float damage_, int endLag_, double range_, Unit& unit) : range(range_), AttackComponent(damage_,endLag_, unit), ComponentContainer<ShootComponent>(unit)
{

}

void ShootComponent::attack(HealthComponent* health)
{

}


SeigeComponent::SeigeComponent(Unit& entity, Anthill& hill) : ApproachComponent(entity), ComponentContainer<SeigeComponent>(entity),
 targetHill(std::dynamic_pointer_cast<Anthill>(GameWindow::getLevel().getUnit(&hill)))
{

}

void SeigeComponent::update()
{
    if ( Object* owner = ((Object*)entity))
    {
        if (!targetUnit.lock().get())
        {
            if (Object* u = owner->getComponent<HealthComponent>()->getLastAttacker())
            {
                setTarget(GameWindow::getLevel().getUnit(u));
            }
            else
            {
                setTarget(GameWindow::getLevel().getUnit(targetHill.lock().get()));
            }
        }
    }
}

SeigeComponent::~SeigeComponent()
{

}

Mushroom::Mushroom(int x, int y) : Unit(*(new ClickableComponent("Mushroom", *this)), *(new RectComponent({x,y,10,10},*this)),*(new RectRenderComponent({0,.5,1,1},*this)),*(new HealthComponent(*this,1)))
{
    addComponent(*(new ResourceComponent(rand()%5,*this)));
    health->setVisible(false);
}

Dummy::Dummy(int x, int y) : Unit(*(new ClickableComponent("Dummy", *this)), *(new RectComponent({x,y,32,32},*this)), *(new RectRenderComponent({1,.5,1,1},*this)),
                                  *(new HealthComponent(*this, 50)))
{

}

Bug::Bug(int x, int y) : Unit(*(new ClickableComponent("Bug", *this)), *(new WanderMove(.02,{x,y,64,64},*this)), *(new RectRenderComponent({1,.5,1,1},*this)),*(new HealthComponent(*this, 100)))
{
    //getComponent<MoveComponent>()->setTarget({0,0});
    addComponent(*(new CorpseComponent(*this, 100)));
   // addComponent(*(new ResourceEatComponent(*this)));
}

Bug::~Bug()
{

}

Beetle::Beetle(int x, int y) : Unit(*(new ClickableComponent("Beetle", *this)), *(new WanderMove(.02,{x,y,64,64},*this)), *(new RectRenderComponent({1,.5,0,1},*this)),*(new HealthComponent(*this, 100)))
{
    addComponent(*(new AttackComponent(1,50,*this)));
    addComponent(*(new BeetleMove(*this)));
    addComponent(*(new CorpseComponent(*this,200)));
}

Beetle::BeetleMove::BeetleMove(Unit& unit) : ApproachComponent(unit), ComponentContainer<BeetleMove>(unit)
{

}

void Beetle::BeetleMove::update()
{
    Object* targetPtr = getTargetUnit();
    Object* owner = ((Object*)entity);
    if (owner)
    {
        Object* nearest = findNearestUnit<Ant::AntMoveComponent>(100);
        if (nearest)
        {
            setTarget(GameWindow::getLevel().getUnit(nearest));
        }
        if (targetPtr)
        {
            if ( targetPtr->getRect().collides(owner->getRect().getRect()))
            {
                AttackComponent* attack = owner->getComponent<AttackComponent>();
                if (attack)
                {
                    attack->attack(targetPtr->getComponent<HealthComponent>());
                }
            }
        }

    }
    ApproachComponent::update();
}

Beetle::BeetleMove::~BeetleMove()
{

}

Beetle::~Beetle()
{

}

ResourceEatComponent::ResourceEatComponent(Unit& unit) : ApproachComponent(unit), ComponentContainer<ResourceEatComponent>(unit)
{

}

void ResourceEatComponent::update()
{
    Object* targetPtr = getTargetUnit();
    Object* owner = ((Object*)entity);
    if (owner)
    {
        if (!targetPtr)
        {
                Object* nearest = findNearestUnit<ResourceComponent>(500);
                if (nearest)
                {
                    setTarget((GameWindow::getLevel().getUnit((static_cast<Object*>(nearest)))));
                }
        }
        else
        {
            if ( targetPtr->getRect().collides(owner->getRect().getRect()))
            {
                targetPtr->getComponent<ResourceComponent>()->collect(*owner);
            }
        }
    }
    ApproachComponent::update();
}

ResourceEatComponent::~ResourceEatComponent()
{

}
