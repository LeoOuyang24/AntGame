#include "SDLHelper.h"
#include "render.h"

#include "entities.h"
#include "game.h"
#include "ants.h"

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
                }
                break;
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
    Font::alef.write(Font::wordProgram,{name,rect});
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

RectRenderComponent::~RectRenderComponent()
{

}

Object::Object(ClickableComponent& click, RectComponent& rect_, RenderComponent& render_) : Entity(), clickable(&click), rect(&rect_), render(&render_)
{
    addComponent(click);
    addComponent(rect_);
    addComponent(render_);

}
RectComponent& Object::getRect()
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

HealthComponent::HealthComponent(Entity& entity, double health_,  int displacement_) : Component(entity), ComponentContainer<HealthComponent>(&entity), health(health_), maxHealth(health_), displacement(displacement_)//height defaults to 10 and displacement defaults to 20
{

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

HealthComponent::~HealthComponent()
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


Unit::Unit(ClickableComponent& click, RectComponent& rect, RenderComponent& render, HealthComponent& health) : Object(click,rect,render), health(&health)
{
    addComponent(health);
}

void Unit::setManager(Manager& manager)
{
    this->manager = &manager;
}

HealthComponent& Unit::getHealth()
{
    return *health;
}

void Unit::interact(Ant& ant)
{

}

Manager& Unit::getManager()
{
    return *manager;
}

AttackComponent::AttackComponent(float damage_, int endLag_, Unit& unit) : Component(unit), ComponentContainer<AttackComponent>(&unit),  damage(damage_), endLag(endLag_)
{

}

void AttackComponent::attack(HealthComponent* health)
{
    if (health && (attackTimer.framesPassed(endLag) || !attackTimer.isSet()))
    {
        health->addHealth(-1*damage);
        attackTimer.set();
    }
}

AttackComponent::~AttackComponent()
{

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

void ApproachComponent::setTarget(const std::shared_ptr<Object>& unit)
{
    setTarget(unit->getCenter(),&unit);
}

void ApproachComponent::update()
{
    if (move)
    {
        Object* ptr = targetUnit.lock().get();
        if (ptr)
        {
            glm::vec2 center = ptr->getRect().getCenter() + displacement;
            if (move->collides(ptr->getRect().getRect()))
            {
                move->setTarget(entity->getComponent<RectComponent>()->getCenter());
            }
            else
            {
                move->setTarget(center);
            }
        }
       // move->update();
    }
}

ApproachComponent::~ApproachComponent()
{

}

Mushroom::Mushroom(int x, int y) : Unit(*(new ClickableComponent("Mushroom", *this)), *(new RectComponent({x,y,10,10},*this)),*(new RectRenderComponent({0,.5,1,1},*this)),*(new HealthComponent(*this,1)))
{
    addComponent(*(new ResourceComponent(rand()%5,*this)));
    health->setVisible(false);
}

Bug::Bug(int x, int y) : Unit(*(new ClickableComponent("Bug", *this)), *(new WanderMove(.02,{x,y,64,64},*this)), *(new RectRenderComponent({1,.5,1,1},*this)),*(new HealthComponent(*this, 100)))
{
    //getComponent<MoveComponent>()->setTarget({0,0});
    addComponent(*(new CorpseComponent(*this, 100)));
    addComponent(*(new ResourceEatComponent(*this)));
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
            setTarget(GameWindow::getLevel().getAnt((static_cast<Ant*>(nearest))));
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
                targetPtr->getComponent<ResourceComponent>()->collect(*targetPtr);
            }
        }
    }
    ApproachComponent::update();
}

ResourceEatComponent::~ResourceEatComponent()
{

}
