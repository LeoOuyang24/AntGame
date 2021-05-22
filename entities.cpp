#include <math.h>

#include "SDLHelper.h"
#include "render.h"

#include "entities.h"
#include "game.h"
#include "ants.h"
#include "navigation.h"
#include "animation.h"
#include "friendlyAssemblers.h"
#include "weaponAssemblers.h"
#include "effects.h"
#include "debug.h"

const int barHeight = 10;
void renderMeter(const glm::vec3& xyWidth, const glm::vec4& color, double current, double maximum, float z)
{
    glm::vec4 renderRect = {xyWidth.x,xyWidth.y,xyWidth.z*current/maximum,barHeight};
    GameWindow::requestRect(renderRect,color,true, 0, z);
    GameWindow::requestRect(renderRect,{0,0,0,1},false,0,z);
}

void renderTimeMeter(const glm::vec4& rect, const glm::vec4& color, DeltaTime& alarm, double duration, float z)
{
    double time = std::min((double)(SDL_GetTicks() - alarm.getTime()),duration); //amount of time passed
    PolyRender::requestRect({rect.x,rect.y,rect.z*(time)/duration,rect.a},color,true,0,z);
    Font::tnr.requestWrite({convert ((duration - time)/1000.0),rect,0,{0,0,0,1},z+.01f});
}

Unit* convertPosToUnit(Positional* pos)
{
    return static_cast<Unit*>(&static_cast<RectComponent*>(pos)->getEntity());
}

TangibleComponent::TangibleComponent(TangibleFunction isTangible_, Entity& entity) : Component(entity), ComponentContainer<TangibleComponent>(&entity), isTangible(isTangible_)
{

}

bool TangibleComponent::getTangible()
{
    return isTangible(entity);
}

ClickableComponent::ClickableComponent(std::string name, Entity& entity) : Component(entity), ComponentContainer<ClickableComponent>(&entity), name(name)
{

}

std::string ClickableComponent::getName()
{
    return name;
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
    const glm::vec4* camRect = &(GameWindow::getCamera().getRect());

    glm::vec2 mousePos = GameWindow::getCamera().toWorld(pairtoVec(MouseManager::getMousePos()));
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

             //   GameWindow::requestRect(buttonRect,{0,1,0,1},true,0,GameWindow::interfaceZ);
          //  buttons[i]->changeRect(GameWindow::getCamera().toScreen(buttonRect));
            //glm::vec4 disp = GameWindow::getCamera().getRect();
            buttons[i]->updateBlit(GameWindow::interfaceZ,GameWindow::getCamera(),false,buttonRect);
            offset += rect->a + spacing.y;
            if (pointInVec(buttonRect,mousePos.x,mousePos.y,0))
            {
                if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT)
                {
                    stillClicked = true;
                 //   buttons[i]->press();
                    break;
                }
            }
            else
            {
              /*  std::cout << mousePos2.x << " " << mousePos2.y << " ";
                printRect(buttonRect);*/
            }

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

AnimationComponent::AnimationComponent(AnimationWrapper& anime, Entity& entity, RenderCamera* camera) :  RenderComponent(entity,camera), ComponentContainer<AnimationComponent>(entity),
                                                                                                            sprite(&anime)
{

}

AnimationComponent::AnimationComponent(AnimationWrapper& anime, Entity& entity) : AnimationComponent(anime,entity,&GameWindow::getCamera())
{

}

void AnimationComponent::render(const SpriteParameter& param)
{
    if (sprite)
    {
        sprite->request(param,animeParam);
    }
}

void AnimationComponent::setParam(const SpriteParameter& param, const AnimationParameter& animeParam)
{
    this->param = param;
    this->animeParam = animeParam;
}

void AnimationComponent::update()
{
    auto rect = entity->getComponent<RectComponent>();
    if (rect && vecContains(rect->getRect(),camera->getRect()))
    {
        double angle = 0;
        MoveComponent* move = entity->getComponent<MoveComponent>();
        if (move)
        {
            glm::vec2 target = move->getCenter();
            if (!entity->getComponent<UnitAnimationComponent>())
            {
                angle = move->getAngle() + M_PI/2;
            }

            //GameWindow::requestLine(glm::vec4(target,move->getCenter()),{1,0,0,1},1,false);

        }
        else
        {
            angle = param.radians;
        }
        if (entity->getComponent<WeaponComponent>())
        {
            param.z += .1;
//param.z -= .1;
        }
        glm::vec4 renderRect = rect->getRect();
        if (camera)
        {
            renderRect = camera->toScreen(renderRect);
        }
        render({renderRect,angle,param.effect,param.tint,param.program,param.z});
        param = SpriteParameter();
        animeParam = AnimationParameter();
    }
}

UnitAnimationComponent::UnitAnimationComponent(AnimationWrapper& set, Unit& entity) : AnimationComponent(set,entity),
                                                                                    ComponentContainer<UnitAnimationComponent>(entity)
{

}

void UnitAnimationComponent::request(AnimationWrapper& sprite_, const SpriteParameter& param, const AnimationParameter& animeParam)
{
    tempSprite = &sprite_;
    setParam(param,animeParam);
}

bool UnitAnimationComponent::doMirror()
{
    MoveComponent* move = entity->getComponent<MoveComponent>();
    if (move)
    {
        return abs(round(move->getAngle())) < M_PI/2;
    }
    return false;
}

void UnitAnimationComponent::update()
{
    AnimationWrapper* base = sprite; //record the old value in case sprite changes
    if (tempSprite)
    {
        sprite = tempSprite;
        tempSprite = nullptr;
    }

    MoveComponent* move = entity->getComponent<MoveComponent>();
    if (doMirror())
    {
        param.effect = MIRROR;
    }
    if (move->getVelocity() == 0 && animeParam.start == -1 && sprite == base) //we check if anything else has modified animeParam to be anything else before setting to 0
    {
        animeParam.start = 0;//SDL_GetTicks();
    }
    AnimationComponent::update();
    sprite = base;
}

RectRenderComponent::RectRenderComponent(const glm::vec4& color, Entity& unit, RenderCamera* cam) : RenderComponent(unit,cam), ComponentContainer<RectRenderComponent>(&unit), color(color)
{

}

RectRenderComponent::RectRenderComponent(const glm::vec4& color, Entity& unit) : RectRenderComponent(color,unit, &GameWindow::getCamera())
{

}

void RectRenderComponent::update()
{
    glm::vec4 renderRect = ((Object*)entity)->getRect().getRect();
    if (camera)
    {
       renderRect = camera->toScreen(renderRect);
    }
    render({renderRect,0,NONE,color,&RenderProgram::basicProgram,0});
}

void RectRenderComponent::render(const SpriteParameter& param)
{
    PolyRender::requestRect(param.rect,color*param.tint,true,param.radians,param.z);
}

RectRenderComponent::~RectRenderComponent()
{

}

ObjectComponent::ObjectComponent(std::string name_, bool dead_, bool movable_, bool friendly_, Entity& entity) : Component(entity),
                                                                                                ComponentContainer<ObjectComponent>(entity),
                                                                                                name(name_), dead(dead_),movable(movable_),friendly(friendly_)
{

}

bool ObjectComponent::getDead()
{
    return dead;
}
bool ObjectComponent::getMovable()
{
    return movable;
}
bool ObjectComponent::getFriendly()
{
    return friendly;
}
bool ObjectComponent::getInactive()
{
    return inactive;
}
void ObjectComponent::setMovable(bool m)
{
    movable = m;
}
void ObjectComponent::setFriendly(bool f)
{
    friendly = f;
}
void ObjectComponent::setDead(bool d)
{
    dead = d;
}
void ObjectComponent::setInactive(bool i)
{
    inactive = i;
}
Object::Object(ClickableComponent& click, RectComponent& rect_, RenderComponent& render_, bool mov) : Entity(),
 clickable(&click), rect(&rect_), render(&render_),    object(new ObjectComponent(click.getName(),false,mov,false,*this))
{
    addComponent(click);
    addComponent(rect_);
    addComponent(render_);
    addComponent(*object);

}

Object::Object(std::string name, const glm::vec4& vec, AnimationWrapper* rapper, bool mov) : Entity(),
    clickable(new ClickableComponent(name, *this)), rect(new RectComponent(vec, *this)), render(new AnimationComponent(*rapper, *this)),
    object(new ObjectComponent(name,false,mov,false,*this))
{
    addComponent(*(clickable));
    addComponent(*(rect));
    addComponent(*(render));
    addComponent(*object);
}

Object::Object()
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

void Object::addObject(ObjectComponent* obj)
{
    addComponent(*obj);
    object = obj;
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
    return object && object->getDead();
}

bool Object::getMovable()
{
    return object && object->getMovable();
}

bool Object::getFriendly()
{
    return object && object->getFriendly();
}

void Object::setFriendly(bool val)
{
    if (object)
    {
        object->setFriendly(val);
    }
}

void Object::setDead(bool isDead)
{
    if (object)
    {
        object->setDead(isDead);
    }
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
    health = std::max(0.0,std::min(health + amount, maxHealth));
}


bool HealthComponent::isDamaging(double amount)
{
    return amount > 0 && isInvincible() <= 0;
}

HealthComponent::HealthComponent(Entity& entity, double health_,  int displacement_) : Component(entity), ComponentContainer<HealthComponent>(&entity), health(health_), maxHealth(health_), displacement(displacement_)//height defaults to 10 and displacement defaults to 20
{

}

void HealthComponent::addArmor(int val)
{
    tempArmor = std::min(tempArmor + val,100);
}

void HealthComponent::takeDamage(double amount, Object& attacker)
{
    //lastAttacker = GameWindow::getLevel()->getUnit(&attacker);
    if (isDamaging(amount))
    {
        if (amount > 0 )
        {
            amount *= 1 - (armor/100.0);
        }
        addHealth(-1*amount);
        invincible.set();
    }

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

float HealthComponent::isInvincible()
{
    if (!invincible.isSet() || invulTime == 0)
    {
        return 0;
    }
    return (invulTime - invincible.getTimePassed())/invulTime;
}

void HealthComponent::setVisible(bool value)
{
    visible = value;
}


void HealthComponent::update()
{
    armor = tempArmor;
    tempArmor = 0;

    RectComponent* rectComp = entity->getComponent<RectComponent>();
    const glm::vec4* rect = &(rectComp->getRect());
    glm::vec2 mousePos = GameWindow::getCamera().toWorld(pairtoVec(MouseManager::getMousePos()));
    auto end = effects.end();
    int disp = 0; //keeps track of where in the effect icon rendering we are in
    for (auto i = effects.begin(); i != end; ++i)
    {
        auto lstEnd = i->second.end();
        glm::vec4 iconRect;
        int size = i->second.size();
        if (size > 0)
        {
            glm::vec4 iconRect = {rect->x + disp,rect->y - displacement*2,displacement,displacement};
            i->second.begin()->icon->request({GameWindow::getCamera().toScreen(iconRect)});

            disp += displacement;
        }
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
        size = i->second.size();
        if (size > 1)
        {
            Font::tnr.requestWrite({"x" + convert(size),GameWindow::getCamera().toScreen(iconRect + glm::vec4(displacement/2,0,0,0))
                                   ,0,{0,0,0,1},1});
            //PolyRender::requestRect(GameWindow::getCamera().toScreen(iconRect + glm::vec4(displacement/2,displacement/2,0,0)),{1,0,0,1},true,0,1);

        }


    }

    if (visible && !entity->getComponent<ProjectileComponent>() && pointInVec(*rect,mousePos.x,mousePos.y))
    {
        //GameWindow::requestRect({rect->x ,rect->y - displacement, rect->z, 0},{1,0,0,1},true,0,0);
        render({rect->x,rect->y - displacement,rect->z}, 0);
    }
    if (armor > 0)
    {
        shieldIcon.request({GameWindow::getCamera().toScreen({rect->x + rect->z + 10, rect->y, barHeight, 2*barHeight}),0,NONE,{1,1,1,1},&RenderProgram::basicProgram,1});
        Font::tnr.requestWrite({convert(armor),GameWindow::getCamera().toScreen({rect->x + rect->z + 10, rect->y, barHeight, 2*barHeight}),0,{0,0,0,1},1});
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

Unit::Unit() : Object()
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

EntityForces::EntityForces(Entity& entity) : ForcesComponent(entity), ComponentContainer<EntityForces>(entity)
{

}

void EntityForces::update()
{
    if (getBeingPushed())
    {
        applyAllForces();
        glm::vec2 adjPos = move->getPos() + finalForce; //adjusted position in case we are being pushed into a wall
        glm::vec4 rect = move->getRect();
        NavMesh* mesh = &GameWindow::getLevel()->getCurrentRoom()->getMesh();
        bool inNode = !pointInVec(curNodeRect,adjPos,0) ; //if
        if ((!vecContains({adjPos.x,adjPos.y,rect.z,rect.a},curNodeRect))) //if we are completely contained in the same node as last frame,no real reason to do anything
            {
                if (!pointInVec(curNodeRect,adjPos)) //if we have moved to a different node, update the curNodeRect
                {
                    curNodeRect = mesh->getNearestNodeRect(adjPos);
                }

                glm::vec4 newRect = mesh->validMove(rect,finalForce);
                if (newRect.x != adjPos.x || newRect.y != adjPos.y) //if there was a wall in our way
                {
                    forces.clear(); //clear our forces
                }
                adjPos = {newRect.x,newRect.y};

            }
        move->setPos(adjPos);
        finalForce = glm::vec2(0);
    }
}

Structure::Structure(ClickableComponent& click, RectComponent& rect, RenderComponent& render, HealthComponent& health) : Unit(click,rect,render,health,false)
{

}



PathComponent::PathComponent(double speed, const glm::vec4& rect, Entity& unit) : MoveComponent(speed,rect,unit), ComponentContainer<PathComponent> (unit)
{

}

bool PathComponent::atPoint(const glm::vec2& point)
{
    return pointDistance(point,this->getCenter()) <= std::min(rect.z,rect.a)/2;
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
        /*if (MouseManager::getJustClicked() != SDL_BUTTON_RIGHT)
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
            NavMesh* mesh = &(GameWindow::getLevel()->getCurrentRoom()->getMesh());
            auto time = SDL_GetTicks();
            path = mesh->getPath(getCenter(),point, std::max(entity->getComponent<RectComponent>()->getRect().z,
                                                             entity->getComponent<RectComponent>()->getRect().a)/2*sqrt(2));
           // std::cout << SDL_GetTicks() - time << std::endl;
            if (path.size() > 0)
            {
                target = path.front().point;
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
    return path.back().point;
}

bool PathComponent::atFinalTarget()
{
   // std::cout << path.size() << std::endl;
    return pointDistance(getTarget(),getCenter()) <= .001;
}

glm::vec2 PathComponent::getNextTarget()
{
    if (path.size() == 0)
    {
        return target;
    }
    return path.front().point;
}

void PathComponent::addPoint(PathPoint& point)
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
            target = path.front().point;
           // std::cout << "New: " << target.x << " " << target.y << std::endl;
        } //otherwise, we're done
    }
    MoveComponent::update();
    GameWindow::requestNGon(10,target,1,{1,1,1,1},0,true,1);


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
        Room* level = (GameWindow::getLevel()->getCurrentRoom());
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
            glm::vec4 bounds = GameWindow::getLevel()->getCurrentRoom()->getRect() + glm::vec4(move->getRect().z/2,move->getRect().a/2,-1*(move->getRect().z),-1*(move->getRect().a));
            move->setTarget({std::min(std::max(bounds.x,target.x),bounds.x + bounds.z), std::min(std::max(bounds.y,target.y),bounds.y + bounds.a)});
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
            GameWindow::requestLine(glm::vec4(ptr->getCenter(),entity->getComponent<RectComponent>()->getCenter()),{1,0,0,1},1,false);
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

void Attack::doAttack(Object* attacker, const glm::vec2& pos)
{

}

ImgParameter Attack::getParam(Object* attacker, const glm::vec2& pos)
{
    if (sequencer && !sequencer->isDone(startAttack))
    {
        return {SpriteParameter(),sequencer->process(startAttack)};
    }
     return {SpriteParameter(),AnimationParameter()};
}

Attack::Attack( float damage_, int endLag_, float range_,AnimationWrapper* anime_, AnimationSequencer* sequencer_) : attackAnime(anime_),
                                                                                    sequencer(sequencer_),
                                                                                    baseData({range_,damage_,endLag_}),
                                                                                    modData({range_,damage_,endLag_})
{

}

bool Attack::canAttack(Object* owner, Object* ptr)
{
    //if (owner && ptr)
    return  (ptr && owner && ptr->getComponent<HealthComponent>() && offCooldown() && vecDistance(ptr->getRect().getRect(),owner->getRect().getRect()) <= modData.range)
            ||
            (sequencer != nullptr && (!sequencer->isDone(startAttack) || startAttack != -1)); //get one last frame in. Only applies to attack animations
}

bool Attack::offCooldown()
{
    return coolDownTimer.timePassed(modData.endLag) || !coolDownTimer.isSet();
}

int Attack::getCooldownRemaining()
{
    return offCooldown() ? 0 : modData.endLag - coolDownTimer.getTimePassed();
}

ImgParameter Attack::attack(Object* attacker, const glm::vec2& pos)
{
    if (offCooldown() && (startAttack == -1 || !sequencer))
    {
        startAttack = SDL_GetTicks();
        coolDownTimer.set();
    }
    doAttack(attacker,pos);
    if (sequencer && sequencer->isDone(startAttack) && startAttack != -1) //get one more frame of attacking in. Some attacks trigger after the sequence is done
    {
        startAttack = -1;
    }
    modData = baseData;
    return getParam(attacker,pos);
}

AnimationWrapper* Attack::getAnimation()
{
    return attackAnime;
}

float Attack::getRange()
{
    return modData.range;
}
float Attack::getDamage()
{
    return modData.damage;
}
int Attack::getEndLag()
{
    return modData.endLag;
}

void Attack::setRange(float range)
{
    modData.range = range;
}
void Attack::setDamage(float damage)
{
    modData.damage = damage;
}
void Attack::setAttackSpeed(float increase)
{
    modData.endLag /= std::max(1 + increase,.001f); //do this to prevent dividing by 0
}



Attack::~Attack()
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

ProjectileComponent::ProjectileComponent(double damage, bool friendly,const glm::vec2& target, double speed, const glm::vec4& rect, Object& entity,ProjCollideFunc collideFun_) : MoveComponent(speed, rect, entity),
                        ComponentContainer<ProjectileComponent>(entity), damage(damage), friendly(friendly), collideFunc(collideFun_)
{
    setTarget(target);
}

ProjectileComponent::ProjectileComponent(double damage, bool friendly,const glm::vec2& target, double xspeed, double yspeed, const glm::vec4& rect, Object& entity,ProjCollideFunc collideFun_) :
                        ProjectileComponent(damage,friendly,target,sqrt(xspeed*xspeed + yspeed*yspeed),rect,entity,collideFunc)
{

}

void ProjectileComponent::setShooter(Object& obj)
{
    shooter = &obj;
}

Object* ProjectileComponent::getShooter()
{
    return shooter;
}

void ProjectileComponent::collide(Entity& other)
{
    if (other.getComponent<HealthComponent>())
    {
        Object* obj = static_cast<Object*>(entity);
        Unit* otherUnit = static_cast<Unit*>(&other);
        if (obj && obj->getFriendly() != otherUnit->getFriendly() ) //if we collided with an enemy (or friendly unit if enemy projectile)
        {
            obj->setDead(true);
            if (shooter)
            {
                onCollide(*otherUnit);
            }
            else
            {
                otherUnit->getHealth().takeDamage(damage,*obj);
            }
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

void UnitAttackComponent::processAttack(Attack& attack)
{
    Object* obj = static_cast<Object*>(entity);
    Object* target = targetUnit.lock().get();
    if (attack.canAttack(obj,target))
    {
        ImgParameter param = attack.attack(obj,target->getCenter());

        UnitAnimationComponent* anime = entity->getComponent<UnitAnimationComponent>();
        if (anime && attack.getAnimation())
        {
            anime->request(*attack.getAnimation(),param.first,param.second);
        }
    }
}

void UnitAttackComponent::doPassive()
{
    if (move && (move->atTarget() || move->getTarget() == (glm::vec2(move->getRect().z/2,move->getRect().a/2))))
    {
        const int radius = std::max(move->getRect().z,move->getRect().a); //wander radius;
        float radians = rand()%360/180.0*M_PI;
        ApproachComponent::setTarget(move->getCenter() + glm::vec2(cos(radians)*radius,sin(radians)*radius),nullptr);
    }
    //GameWindow::getLevel()->getCurrentRoom()->getMesh().get
}

UnitAttackComponent::UnitAttackComponent(double damage_, int endLag_, double range_,double searchRange_, bool f, Entity& entity) : ApproachComponent(entity),
ComponentContainer<UnitAttackComponent>(entity), damage(damage_),notFriendly(f), searchRange(searchRange_), activated(false)//coincidentally, activated should always be the same value as f
{

}

void UnitAttackComponent::addAttack(Attack& attack)
{
    attacks.emplace_back(&attack);
}

void UnitAttackComponent::update()
{

    Object* ent = targetUnit.lock().get();
    Room* level = GameWindow::getLevel()->getCurrentRoom();
    if (!ent && level) //if we aren't already fighting something, find something nearby
    {
       /* Object* nearby = findNearestUnit<HealthComponent>(std::max(RenderProgram::getScreenDimen().x, RenderProgram::getScreenDimen().y)/2,notFriendly,*(level->getTree()));
        if (nearby)
        {
            ApproachComponent::setTarget(level->getUnit(nearby));
        }*/
        ApproachComponent::setTarget(level->getUnit(GameWindow::getPlayer().getPlayer()));
       // doPassive();
    }
    if (ent && level) //don't use else if because there's a chance that both ifs can trigger
    {
        for (auto it = attacks.begin();it != attacks.end(); ++it)
        {
            processAttack((*(*it).get()));
        }

    }
    else if (level) //if ent is still null, wander around
    {
        doPassive();
    }

    ApproachComponent::update();
}

void UnitAttackComponent::collide(Entity& other)
{
    Object* obj = static_cast<Object*>(entity);
    if (obj->getFriendly() == false && static_cast<Object*>(&other)->getFriendly())
    {
        HealthComponent* health = other.getComponent<HealthComponent>();
        if (health)
        {
            health->takeDamage(damage,*obj);
        }
    }
}

UnitAttackComponent::~UnitAttackComponent()
{

}


ProjectileAttack::ProjectileAttack(ProjectileAssembler& ass, int endLag, double range,
                                   AnimationWrapper* attackAnime_, AnimationSequencer* sequencer_) :
        Attack(ass.damage,endLag,range,attackAnime_,sequencer_),assembler(&ass)
{


}

void ProjectileAttack::doAttack(Object* attacker, const glm::vec2& mousePos)
{
    if (attacker->getComponent<RectComponent>())
    {
        glm::vec2 center = attacker->getComponent<RectComponent>()->getCenter();
        float angle = atan2(center.y - mousePos.y, center.x - mousePos.x);
        glm::vec4 levelRect = GameWindow::getLevel()->getCurrentRoom()->getRect(); //we want projectile to travel to the end of the map
        GameWindow::getLevel()->getCurrentRoom()->addUnit(*assembler->assemble(*attacker,center,mousePos - glm::vec2(cos(angle)*levelRect.z, sin(angle)*levelRect.a)),assembler->friendly);
    }
}


