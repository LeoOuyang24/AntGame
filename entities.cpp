#include "SDLHelper.h"
#include "render.h"

#include "entities.h"
#include "game.h"
#include "ants.h"
#include "navigation.h"
#include "animation.h"

void renderMeter(const glm::vec3& xyWidth, const glm::vec4& color, double current, double maximum, float z)
{
    const int height = 10;
    glm::vec4 renderRect = {xyWidth.x,xyWidth.y,xyWidth.z*current/maximum,height};
    GameWindow::requestRect(renderRect,color,true, 0, z);
    GameWindow::requestRect(renderRect,{0,0,0,1},false,0,z);
}

void renderTimeMeter(const glm::vec4& rect, const glm::vec4& color, DeltaTime& alarm, double duration, float z)
{
    double time = std::min(SDL_GetTicks() - alarm.getTime(),duration); //amount of time passed
    PolyRender::requestRect({rect.x,rect.y,rect.z*(time)/duration,rect.a},color,true,0,z);
    Font::tnr.requestWrite({convert ((duration - time)/1000.0),rect,0,{0,0,0,1},z+.01f});
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
        PolyRender::requestNGon(10,GameWindow::getCamera().toScreen({unitRect->x + unitRect->z/2,unitRect->y + unitRect->a/2}),unitRect->z/2*sqrt(2),{0,.5,1,1},0,false,1);
        //GameWindow::requestRect(*unitRect,Player::selectColor,true,0,0);
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
   // Font::tnr.requestWrite({name,rect});
}

void ClickableComponent::addButton(Button& button)
{
    buttons.emplace_back(&button);
}

ClickableComponent::~ClickableComponent()
{

}

AnimationComponent::AnimationComponent(AnimationWrapper* anime, Entity& entity ) :  RenderComponent(entity), ComponentContainer<AnimationComponent>(entity), sprite(anime)
{

}

void AnimationComponent::render(const SpriteParameter& param)
{
    if (sprite)
    {
        sprite->request(param,animeParam);
    }
}

void AnimationComponent::update()
{
    auto rect = entity->getComponent<RectComponent>();
    if (rect && vecContains(rect->getRect(),GameWindow::getCamera().getRect()))
    {
        double angle = 0;
        MoveComponent* move = entity->getComponent<MoveComponent>();
        if (move)
        {
            if (!move->atTarget() )
            {
                angle = atan2(move->getCenter().y - move->getTarget().y,move->getCenter().x - move->getTarget().x) + M_PI/2;
            }
            else
            {
                ApproachComponent* approach = entity->getComponent<ApproachComponent>();
                Object* target = approach->getTargetUnit();
                if ( approach && target)
                {
                    angle = atan2( move->getCenter().y - target->getCenter().y , move->getCenter().x -  target->getCenter().x ) + M_PI/2;
                }
            }
        }
        render({GameWindow::getCamera().toScreen(rect->getRect()),angle,NONE});
    }
}

RectRenderComponent::RectRenderComponent(const glm::vec4& color, Entity& unit) : RenderComponent(unit), ComponentContainer<RectRenderComponent>(&unit), color(color)
{

}

void RectRenderComponent::update()
{
    render({GameWindow::getCamera().toScreen(((Object*)entity)->getRect().getRect()),0,NONE,color,&RenderProgram::basicProgram,0});
}

void RectRenderComponent::render(const SpriteParameter& param)
{
    PolyRender::requestRect(param.rect,color*param.tint,true,param.radians,param.z);
}

RectRenderComponent::~RectRenderComponent()
{

}

Object::Object(ClickableComponent& click, RectComponent& rect_, RenderComponent& render_, bool mov) : Entity(),movable(mov),
 clickable(&click), rect(&rect_), render(&render_)
{
    addComponent(click);
    addComponent(rect_);
    addComponent(render_);


}

Object::Object(std::string name, const glm::vec4& vec, AnimationWrapper* rapper, bool mov) : Entity(), movable(mov),
    clickable(new ClickableComponent(name, *this)), rect(new RectComponent(vec, *this)), render(new AnimationComponent(rapper, *this))
{
    addComponent(*(clickable));
    addComponent(*(rect));
    addComponent(*(render));
}

Object::Object(bool mov) : movable(mov)
{

}
void Object::addRect(RectComponent* r)
{
    addComponent(*r);
    rect = r;
}
void Object::addClickable(ClickableComponent* c)
{
    addComponent(*c);
    clickable = c;
}
void Object::addRender(RenderComponent* rend)
{
    addComponent(*rend);
    render = rend;
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

bool Object::getFriendly()
{
    return friendly;
}

void Object::setFriendly(bool val)
{
    friendly = val;
}

void Object::setDead(bool isDead)
{
    dead = isDead;
}
Object::~Object()
{
    //std::cout << "Object Destructor" << std::endl;
}

ObjectAssembler::ObjectAssembler( std::string name_, const glm::vec2& rect_,AnimationWrapper* anime, bool mov) : dimen(rect_), name(name_), sprite(anime), movable(mov)
{

}

const glm::vec2& ObjectAssembler::getDimen()
{
    return dimen;
}

std::string ObjectAssembler::getName()
{
    return name;
}

AnimationWrapper* ObjectAssembler::getSprite()
{
    return sprite;
}

bool ObjectAssembler::getMovable()
{
    return movable;
}

Object* ObjectAssembler::assemble()
{
    return new Object(name,{0,0,dimen.x,dimen.y},sprite,movable);
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
        if (invincible.framesPassed(0) || !invincible.isSet())
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
        render({rect->x,rect->y - displacement,rect->z}, 0);
    }
}

void HealthComponent::render(const glm::vec3& rect, float z)
{
    //PolyRender::requestRect({rect.x,rect.y,health/maxHealth*rect.z,height},{1,0,0,1},true,0,rect.a);
    glm::vec4 color = {1,0,0,1};
    if (static_cast<Object*>(entity)->getFriendly()) //if the entity is friendly, render a green healthbar
    {
        color.r = 0;
        color.g = 1;
    }
    renderMeter({rect.x,rect.y,rect.z},color,health,maxHealth,z);
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

Unit::Unit(std::string name, const glm::vec4& rect, AnimationWrapper* anime, bool mov, double maxHealth) : Object(name, rect, anime, mov),
        health(new HealthComponent(*this, maxHealth, .001*RenderProgram::getScreenDimen().y))
{
    addComponent(*(health));
}

Unit::Unit(bool mov) : Object(mov)
{

}

void Unit::addHealth(HealthComponent* h)
{
    health = h;
    addComponent(*h);
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

Manager* Unit::getManager()
{
    return manager;
}

UnitAssembler::UnitAssembler( std::string name_,const glm::vec2& rect_, AnimationWrapper* wrap, bool mov, double maxHealth_, double prodTime_) :
     ObjectAssembler( name_,rect_, wrap, mov), maxHealth(maxHealth_), prodTime(prodTime_)
{

}

double UnitAssembler::getMaxHealth()
{
    return maxHealth;
}

double UnitAssembler::getProdTime()
{
    return prodTime;
}

int UnitAssembler::getProdCost()
{
    return prodCost;
}

Object* UnitAssembler::assemble()
{
    return new Unit(name,{0,0,dimen.x,dimen.y}, sprite, movable, maxHealth);
}

RepelComponent::RepelComponent(Object& unit) : Component(unit), ComponentContainer<RepelComponent>(unit)
{

}

void RepelComponent::collide(Entity& unit)
{
    Object* ptr = static_cast<Object*>(&unit);
    if (vecContains(ptr->getRect().getRect(), entity->getComponent<RectComponent>()->getRect()))
    {
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
        GameWindow::getLevel().addUnit(*(resource), false);
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

void PathComponent::setPos(const glm::vec2& pos)
{
    rect.x = pos.x - rect.z/2;
    rect.y = pos.y - rect.a/2;
    setTarget(pos);
}

void PathComponent::setTarget(const glm::vec2& point)
{
    if (getTarget() != point)
    {
       /* if (MouseManager::getJustClicked() != SDL_BUTTON_RIGHT)
        {
            std::cout << point.x << " " << point.y << std::endl;
        }*/
        path.clear();
        NavMesh* mesh = &(GameWindow::getLevel().getMesh());
        path = mesh->getPath(getCenter(),point, entity->getComponent<RectComponent>()->getRect().z/2*sqrt(2));
        if (path.size() > 0)
        {
            target = path.front();
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
    //Debug::DebugNavMesh::showPath(path);
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
        glm::vec2 point = {rect.x + rect.z/2 + cos(angle)*radius, rect.y + rect.a/2 + sin(angle)*radius}; //target point, currenlty randomly generated
        Map* level = &(GameWindow::getLevel());
        const glm::vec4* levelRect = &(level->getCurrentChunk().getRect());
        point.x = std::max(levelRect->x, std::min(point.x, levelRect->x + levelRect->z - rect.z)); //clamp point to levelREct
        point.y = std::max(levelRect->y, std::min(point.y, levelRect->y + levelRect->a - rect.a));
        setTarget(point);
    }
    else
    {
        MoveComponent::update();
    }
}

ApproachComponent::ApproachComponent(Entity& entity) : Component(entity), ComponentContainer<ApproachComponent>(entity), move(entity.getComponent<MoveComponent>())
{

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
    if (move)
    {
        setTarget(closestPointOnVec(unit->getRect().getRect(),move->getCenter()),&unit);
    }
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
                move->setSpeed(0);
            }
            else
            {
                glm::vec2 target = move->getTarget();
                glm::vec2 newTarget = closestPointOnVec(ptr->getRect().getRect(),target);
                if (target != newTarget && !pointInVec(ptr->getRect().getRect(),target.x,target.y,0)) //otherwise, approach
                {
                    move->setTarget(newTarget);
                    //displacement = move->getTarget() - ptr->getRect().getCenter(); //sometimes, the point can't be reached. Set the target to the point returned by getPath.
                }
            }
        }
    }
}

ApproachComponent::~ApproachComponent()
{

}


bool AttackComponent::canAttack(Object* ptr)
{
    if (ptr)
    {
        RectComponent* otherRect = &ptr->getRect();
        return move && ptr->getComponent<HealthComponent>() && vecDistance(otherRect->getRect(),move->getRect()) <= range
            && GameWindow::getLevel().getMesh().straightLine(glm::vec4(otherRect->getCenter(), move->getCenter())) ;
    }
    return false;
}

AttackComponent::AttackComponent(float damage_, int endLag_, double range_, Entity& unit) : ApproachComponent(unit), ComponentContainer<AttackComponent>(&unit),
                                                                                            damage(damage_), range(range_), endLag(endLag_)
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
    Object* ptr = targetUnit.lock().get();
  //  std::cout << ptr << std::endl;
    if (canAttack(ptr)) //attack if we are able to.
    {
        attack(ptr->getComponent<HealthComponent>());
        if (move)
        {
            move->setSpeed(0);
        }
    }
    else //otherwise, move
    {
        ApproachComponent::update();
    }
}

void AttackComponent::setTarget(const glm::vec2& target, const std::shared_ptr<Object>* unit) //unit is a pointer so you can move to a point rather than a unit
{
    if (move)
    {
        if (unit)
        {
            if (targetUnit.lock().get() != unit->get())
            {
                targetUnit = *unit;
            }
        }
        else
        {
            targetUnit.reset();
        }
        move->setTarget(target);
    }
}

AttackComponent::~AttackComponent()
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

Bug::Bug(int x, int y) : Unit(*(new ClickableComponent("Bug", *this)), *(new PathComponent(.02,{x,y,20,20},*this)), *(new AnimationComponent(&basicEnemyAnime,*this)),*(new HealthComponent(*this, 100)))
{
    //getComponent<MoveComponent>()->setTarget({0,0});
    addComponent(*(new AttackComponent(1, 50, 0,*this)));
   // addComponent(*(new ResourceEatComponent(*this)));
}

Bug::~Bug()
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
            Object* nearest = findNearestUnit<ResourceComponent>(500,false,*GameWindow::getLevel().getTree());
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
