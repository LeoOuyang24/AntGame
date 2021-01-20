#include "SDLHelper.h"
#include "render.h"

#include "entities.h"
#include "game.h"
#include "ants.h"
#include "navigation.h"
#include "animation.h"
#include "friendlyAssemblers.h"
#include "effects.h"

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

Unit* convertPosToUnit(Positional* pos)
{
    return static_cast<Unit*>(&static_cast<RectComponent*>(pos)->getEntity());
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
    glm::vec2 mousePos = pairtoVec({MouseManager::getMousePos().first, MouseManager::getMousePos().second});
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
            buttonRect = GameWindow::getCamera().toScreen(buttonRect);
          //  buttons[i]->changeRect(GameWindow::getCamera().toScreen(buttonRect));
            //glm::vec4 disp = GameWindow::getCamera().getRect();
            buttons[i]->updateBlit(GameWindow::interfaceZ,buttonRect);
             //   GameWindow::requestRect(buttonRect,{0,1,0,1},true,0,GameWindow::interfaceZ);
            if (pointInVec(buttonRect,mousePos.x,mousePos.y,0))
            {
                if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT)
                {
                    stillClicked = true;
                  //  std::cout << "Clicked " << std::endl;
                 //   buttons[i]->press();
                    break;
                }
            }
            else
            {
              /*  std::cout << mousePos2.x << " " << mousePos2.y << " ";
                printRect(buttonRect);*/
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
        clicked = stillClicked || (MouseManager::getJustClicked() == SDL_BUTTON_LEFT && pointInVec(GameWindow::getCamera().toScreen(*unitRect),mousePos.x,mousePos.y));
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

AnimationComponent::AnimationComponent(AnimationWrapper& anime, Entity& entity ) :  RenderComponent(entity), ComponentContainer<AnimationComponent>(entity), sprite(&anime)
{

}

void AnimationComponent::render(const SpriteParameter& param)
{
    if (sprite)
    {
        sprite->request(param,animeParam);
    }
}

void AnimationComponent::setTint(const glm::vec4& param)
{
    tint = param;
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
            glm::vec2 target = move->getCenter();
            if (!move->atTarget() )
            {
                {
                    PathComponent* path = entity->getComponent<PathComponent>();
                    if (path)
                    {
                        target = path->getNextTarget();
                    }
                    else
                    {

                        target = move->getTarget();

                    }
                }
               // GameWindow::requestNGon(10,target,1,{1,1,1,1},0,true,10);
            }
            else if (move->getVelocity() == 0) //typically happens if we are attacking a unit
            {
                AttackComponent* approach = entity->getComponent<AttackComponent>();
                if ( approach)
                {
                    Object* targetUnit = approach->getTargetUnit();
                    if (targetUnit) //this should be guaranteed to be true
                    {
                        target = targetUnit->getCenter();
                    }
                    else
                    {
                        target.y += 1;
                    }
                }
                else
                {
                    target.y += 1;
                }
            }
            else
            {
                target.y += 1; //we want our angle to be 0, so we set target to a value that would force it to be 0.
            }
            angle = atan2(move->getCenter().y - target.y,move->getCenter().x - target.x) + M_PI/2;

        }
        render({GameWindow::getCamera().toScreen(rect->getRect()),angle,NONE,tint});
        tint = glm::vec4(1);
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
    clickable(new ClickableComponent(name, *this)), rect(new RectComponent(vec, *this)), render(new AnimationComponent(*rapper, *this))
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
    //std::cout << "Object Destructor: " << this << std::endl;
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

void HealthComponent::addHealth(double amount)
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

void HealthComponent::takeDamage(double amount, Object& attacker)
{
    lastAttacker = GameWindow::getLevel()->getUnit(&attacker);
    addHealth(-1*amount);
}

void HealthComponent::addEffect(StatusEffect effect)
{
    effects[effect.icon].push_back(effect);
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
    const glm::vec4* rect = &(rectComp->getRect());
    glm::vec2 mousePos = GameWindow::getCamera().toWorld(pairtoVec(MouseManager::getMousePos()));
    if (visible && !entity->getComponent<ProjectileComponent>() && pointInVec(*rect,mousePos.x,mousePos.y))
    {
        //GameWindow::requestRect({rect->x ,rect->y - displacement, rect->z, 0},{1,0,0,1},true,0,0);
        render({rect->x,rect->y - displacement,rect->z}, 0);
    }
    //SpriteParameter
    auto end = effects.end();
    int disp = 0; //keeps track of where in the effect icon rendering we are in
    for (auto i = effects.begin(); i != end; ++i)
    {
        auto lstEnd = i->second.end();
        for (auto j = i->second.begin(); j != lstEnd;)
        {

            if (j->isDone())
            {
                j = i->second.erase(j);
            }
            else
            {
                j->update();
                ++j;
            }
        }
        int size = i->second.size();
        if (size > 0)
        {
            glm::vec4 iconRect = GameWindow::getCamera().toScreen({rect->x + disp,rect->y - displacement*2,displacement,displacement});
            i->second.begin()->icon->request({iconRect});
            if (size > 1)
            {
                Font::tnr.requestWrite({convert(size),iconRect + glm::vec4(displacement/2,displacement/2,0,0),0,{1,1,1,1},1});
            }
            disp += displacement;
        }
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
        GameWindow::getLevel()->addUnit(*(resource), false);
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
    glm::vec2 adjPos = pos; //adjusted position in case we are being pushed into a wall
    glm::vec4 nodeRect = GameWindow::getLevel()->getMesh().getNearestNodeRect(pos);
    if (nodeRect == glm::vec4(0))
    {
        std::cerr << "attempted to move entity outside of world boundaries\n";
        throw std::logic_error("attempted to move entity outside of world boundaries");
    }
    else
    {
        adjPos = closestPointOnVec(nodeRect,adjPos);
    }
    //MoveComponent::setPos(pos);
    if(velocity == 0)
    {
        changePos(adjPos + glm::vec2(rect.z/2,rect.a/2)); //changePos sets center rather than top-left corner. Here we have to adjust it.
    }
    else
    {
        MoveComponent::setPos(adjPos);
    }

}

void PathComponent::changePos(const glm::vec2& pos)
{
    rect.x = pos.x - rect.z/2;
    rect.y = pos.y - rect.a/2;
    ApproachComponent* app = entity->getComponent<ApproachComponent>();
    if (app)
    {
        app->setTarget(pos,nullptr);
    }
    setTarget(pos);
}

void PathComponent::setTarget(const glm::vec2& point)
{
    if (getTarget() != point)
    {
       /* if (MouseManager::getJustClicked() != SDL_BUTTON_RIGHT && point != getCenter() && !Debug::getSpawnCreatures())
        {
            std::cout << point.x << " " << point.y << std::endl;
        }*/
        path.clear();
        if (point == getCenter())
        {
            target = point;
        }
        else
        {
            NavMesh* mesh = &(GameWindow::getLevel()->getMesh());
            auto time = SDL_GetTicks();
            path = mesh->getPath(getCenter(),point, entity->getComponent<RectComponent>()->getRect().z/2*sqrt(2));
           // std::cout << SDL_GetTicks() - time << std::endl;
            if (path.size() > 0)
            {
                target = path.front();
            }
            else
            {
                std::cout << "No path!" << std::endl;
            }
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

glm::vec2 PathComponent::getNextTarget()
{
    if (path.size() == 0)
    {
        return target;
    }
    return path.front();
}

void PathComponent::addPoint(const glm::vec2& point)
{
    path.push_back(point);
}

void PathComponent::update()
{
    if (Debug::getRenderPath())
    {
        Debug::DebugNavMesh::showPath(path);
    }
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
        Map* level = (GameWindow::getLevel());
        const glm::vec4* levelRect = &(level->getRect());
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
void ApproachComponent::setTarget(const glm::vec2& target, std::shared_ptr<Object>* unit)
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
                move->setTarget(target);
        }
    }
}

void ApproachComponent::setTarget(std::shared_ptr<Object>& unit)
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
            && GameWindow::getLevel()->getMesh().straightLine(glm::vec4(otherRect->getCenter(), move->getCenter())) ;
    }
    return false;
}

AttackComponent::AttackComponent(double damage_, int endLag_, double range_, Entity& unit) : ApproachComponent(unit), ComponentContainer<AttackComponent>(&unit),
                                                                                            damage(damage_), range(range_), endLag(endLag_)
{

}

void AttackComponent::attack(HealthComponent* health)
{
    health->takeDamage(damage,*static_cast<Unit*>(entity));
}

void AttackComponent::update()
{
    Object* ptr = targetUnit.lock().get();
  //  std::cout << ptr << std::endl;
    if (canAttack(ptr)) //attack if we are able to.
    {
        if (attackTimer.timePassed(endLag) || !attackTimer.isSet())
        {
            attack(ptr->getComponent<HealthComponent>());
            attackTimer.set();
        }
        if (move)
        {
            move->setSpeed(0);
            if (!pointInVec(ptr->getRect().getRect(),move->getTarget().x, move->getTarget().y,0))
            {
                //std::cout << "ASDF" << std::endl;
                 /*there is a small chance that if an enemy enters our attack range at the same time that set it as a target,
                  we won't have our move->getTarget = to the enemy target. We set it here so that AnimationComponent renders our angle correctly.*/
              //  move->setTarget(ptr->getCenter()); //
            }
        }
    }
    else //otherwise, move
    {
        ApproachComponent::update();
    }
}

void AttackComponent::setTarget(const glm::vec2& target, std::shared_ptr<Object>* unit) //unit is a pointer so you can move to a point rather than a unit
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
            move->setTarget(target);
            targetUnit.reset();
        }
    }
}

AttackComponent::~AttackComponent()
{

}



SeigeComponent::SeigeComponent(Unit& entity, Anthill& hill) : ApproachComponent(entity), ComponentContainer<SeigeComponent>(entity),
 targetHill(std::dynamic_pointer_cast<Anthill>(GameWindow::getLevel()->getUnit(&hill)))
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
                setTarget(GameWindow::getLevel()->getUnit(u));
            }
            else
            {
                setTarget(GameWindow::getLevel()->getUnit(targetHill.lock().get()));
            }
        }
    }

}
SeigeComponent::~SeigeComponent()
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
            Object* nearest = findNearestUnit<ResourceComponent>(500,false,*GameWindow::getLevel()->getTree());
            if (nearest)
            {
                setTarget((GameWindow::getLevel()->getUnit((static_cast<Object*>(nearest)))));
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

void ProjectileComponent::onCollide(Unit& other)
{
    if (collideFunc)
    {
        collideFunc(other,*this);
    }
    else
    {
        other.getHealth().takeDamage(damage,*shooter);
    }
}

ProjectileComponent::ProjectileComponent(double damage, bool friendly,const glm::vec2& target, double speed, const glm::vec4& rect, Unit& entity,ProjCollideFunc collideFun_) : MoveComponent(speed, rect, entity),
                        ComponentContainer<ProjectileComponent>(entity), damage(damage), friendly(friendly), collideFunc(collideFun_)
{
    setTarget(target);
}

ProjectileComponent::ProjectileComponent(double damage, bool friendly,const glm::vec2& target, double xspeed, double yspeed, const glm::vec4& rect, Unit& entity,ProjCollideFunc collideFun_) :
                        ProjectileComponent(damage,friendly,target,sqrt(xspeed*xspeed + yspeed*yspeed),rect,entity,collideFunc)
{

}

void ProjectileComponent::setShooter(Object& obj)
{
    shooter = &obj;
}

void ProjectileComponent::collide(Entity& other)
{
    Unit* unit = static_cast<Unit*>(entity);
    Unit* otherUnit = static_cast<Unit*>(&other);
    if (unit && friendly != otherUnit->getFriendly() ) //if we collided with an enemy (or friendly unit if enemy projectile)
    {
        unit->setDead(true);
        if (shooter)
        {
            onCollide(*unit);
        }
        else
        {
            otherUnit->getHealth().takeDamage(damage,*unit);
        }
    }
}

void ProjectileComponent::update()
{
    MoveComponent::update();
    if (atTarget())
    {
        static_cast<Unit*>(entity)->setDead(true);
    }

}

UnitAttackComponent::UnitAttackComponent(double damage_, int endLag_, double range_,double searchRange_, bool f, Entity& entity) : AttackComponent(damage_,endLag_,range_,entity),
ComponentContainer<UnitAttackComponent>(entity), notFriendly(f), searchRange(searchRange_), activated(false) //coincidentally, activated should always be the same value as f
{

}

void UnitAttackComponent::update()
{

    Object* ent = shortTarget.lock().get();
    Map* level = GameWindow::getLevel();
    if (!ent && level && !ignore) //if we aren't already fighting something, find something nearby
    {
        Object* nearby = findNearestUnit<HealthComponent>(searchRange,notFriendly,*(level->getTree()));
        if (nearby && nearby != longTarget.first.lock().get() &&
            entity && GameWindow::getLevel()->getMesh().straightLine(glm::vec4(nearby->getRect().getCenter(),entity->getComponent<RectComponent>()->getCenter())))
        {
            setShortTarget(level->getUnit(nearby));
        }
        else if (activated && (longTarget.second != move->getTarget() || targetUnit.lock().get() != longTarget.first.lock().get())) //if there's nothing nearby to fight, set the target to the long target
        {
            if (longTarget.first.lock().get())
            {
            //    std::cout << longTarget.first.lock().get() << std::endl;
                AttackComponent::setTarget(longTarget.second,&level->getUnit(longTarget.first.lock().get()));
            }
            else
            {
               // std::cout << longTarget.second.x << " " << longTarget.second.y << std::endl;
                AttackComponent::setTarget(longTarget.second,nullptr);
            }
        }

    }
   /* if (shortTarget.lock().get())
    {
        glm::vec4 selfRect = entity->getComponent<RectComponent>()->getRect();
        glm::vec4 otherRect = shortTarget.lock().get()->getComponent<RectComponent>()->getRect();
        PolyRender::requestLine(glm::vec4(GameWindow::getCamera().toScreen(
                                {selfRect.x + selfRect.z/2, selfRect.y + selfRect.a/2}),
                                GameWindow::getCamera().toScreen(
                                {otherRect.x + otherRect.z/2, otherRect.y + otherRect.a/2})),
                                {1,0,1,1},3);
    }
    */
    if (move->getCenter() == longTarget.second) //if we are at the target, set our state back to ignore
    {
        ignore = false;
    }
    AttackComponent::update();
}

void UnitAttackComponent::setTarget(const glm::vec2& target,std::shared_ptr<Object>* unit)
{
    setLongTarget(target,unit,false);
    AttackComponent::setTarget(target,unit);
}

void UnitAttackComponent::setLongTarget(const glm::vec2& target, std::shared_ptr<Object>* unit, bool ignore)
{
    activated = true; //once moveComponent has set a target, we gets permission to affect moveComponent
    //AttackComponent::setTarget(target,unit);
    //std::cout << target.x << " " << target.y << " " << unit->get() << std::endl;
    this->ignore = ignore;
    if (unit)
    {
        longTarget.first = *unit;
        //std::cout << longTarget.first.lock().get() << std::endl;
    }
    else
    {
        longTarget.second = target;
        longTarget.first.reset();
    }
}

void UnitAttackComponent::setShortTarget(std::shared_ptr<Object>& unit)
{
    if (shortTarget.lock().get() != unit.get())
    {
        AttackComponent::setTarget(unit);
        if (unit.get())
        {
            shortTarget = unit;
           /* if (entity)
            {
                CommandableComponent* command = entity->getComponent<CommandableComponent>();
                if (command)
                {
                    AntManager* curTask = command->getCurrentTask();
                    if (curTask)
                    {
                        curTask->setShortTarget(*unit);
                    }
                }
            }*/
        }
        else
        {
            shortTarget.reset();
        }
    }

}


ProjectileAttackComponent::ProjectileAttackComponent(ProjectileAssembler& ass, int endLag, double range,double searchRange_,bool f,  Unit& entity) :
        UnitAttackComponent(0,endLag,range,searchRange_,f, entity), ComponentContainer<ProjectileAttackComponent>(entity),assembler(&ass)
{

}

void ProjectileAttackComponent::attack(HealthComponent* health)
{
    Object* unit = assembler->assemble(*static_cast<Object*>(entity),entity->getComponent<RectComponent>()->getCenter(),targetUnit.lock().get()->getRect().getCenter());
    GameWindow::getLevel()->addUnit(*unit,true);
}
