#include "friendlyAssemblers.h"
#include "animation.h"
#include "ants.h"
#include "game.h"
#include "effects.h"
#include "debug.h"

ObjectAssembler::ObjectAssembler( std::string name_, const glm::vec2& rect_,AnimationWrapper& anime, bool mov, bool friendly_) :
    dimen(rect_), name(name_), sprite(&anime), movable(mov), friendly(friendly_)
{

}

ObjectAssembler::~ObjectAssembler()
{
    std::cout << "Deleted: " << name <<"\n";
}

Object* ObjectAssembler::assemble()
{

    Object* obj= new Object(name,{0,0,dimen.x,dimen.y},sprite,movable);
    obj->setFriendly(friendly);
    return obj;
}


UnitAssembler::UnitAssembler( std::string name_,const glm::vec2& rect_, AnimationWrapper& wrap, bool mov, double maxHealth_, float speed,bool friendly_) :
     ObjectAssembler( name_,rect_, wrap, mov,friendly_), maxHealth(maxHealth_), speed(speed)
{
        if (movable) //if is a unit, add to units
        {
            addUnitToBucket(*this,allUnits);
        }
        else //add to structures
        {
            addUnitToBucket(*this,allStructures);
        }
}

Object* UnitAssembler::assemble()
{
    return new Unit(name,{0,0,dimen.x,dimen.y}, sprite, movable, maxHealth);
}

Unit* UnitAssembler::NPCUnitAssemble()
{
    Unit* ent = new Unit();
   // ent->addRect((new Ant::AntMoveComponent(nullptr,speed,{0,0,dimen.x,dimen.y},*ent)));
   ent->addRect(((new PathComponent(speed,{0,0,dimen.x,dimen.y},*ent))));
    ent->addRender((new UnitAnimationComponent(*sprite,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->addObject(new ObjectComponent(false,movable,friendly,*ent));
   // ent->addComponent(*(new CommandableComponent(*ent)));
    ent->addComponent(*(new ForcesComponent(*ent)));
    return ent;
}

ProjectileAssembler::ProjectileAssembler(double damage_, std::string name_,const glm::vec2& rect_, AnimationWrapper& anime, float speed, bool friendly_) :
                                        damage(damage_),speed(speed),ObjectAssembler(name_,rect_,anime,true,friendly_)
{

}

Object* ProjectileAssembler::assemble(Object& shooter, const glm::vec2& point, const glm::vec2& target)
{
    Object* obj = new Object();
    obj->addRect(new ProjectileComponent(damage,friendly,target,speed,{point.x-dimen.x/2,point.y-dimen.y/2,dimen.x,dimen.y},*obj));
    obj->addRender(new AnimationComponent(*sprite,*obj));
    obj->getComponent<ProjectileComponent>()->setShooter(shooter);
    obj->addObject(new ObjectComponent(false,movable,friendly,*obj));
    return obj;
}


HitboxAssembler::HitboxRender::HitboxRender(Entity& entity) : AnimationComponent(hitboxAnime,entity),ComponentContainer<HitboxRender>(entity)
{

}

void HitboxAssembler::HitboxRender::collide(Entity& other)
{
    ObjectComponent* otherObj = other.getComponent<ObjectComponent>();
    ObjectComponent* obj = entity->getComponent<ObjectComponent>();
    if (obj && otherObj && obj->getFriendly() != otherObj->getFriendly())
    {
        collided = true;
    }
}

void HitboxAssembler::HitboxRender::update()
{
    ObjectComponent* obj = entity->getComponent<ObjectComponent>();
    if (Debug::getRenderHitboxes() && obj && !obj->getInactive())
    {
        RectComponent* rect = entity->getComponent<RectComponent>();
        if (camera && rect)
        {
            sprite = collided ? &hitboxAnimeRed : &hitboxAnime;
            render({camera->toScreen(rect->getRect())});
        }
    }
    collided = false;
}

HitboxAssembler::HitboxAssembler(int duration, float damage, std::string name, bool friendly, const glm::vec2& dimen ,ProjectileComponent::ProjCollideFunc func) : duration(duration),
                                                                                                                                                                    projCollideFunc(func),
                                                                                                                                                                    ProjectileAssembler(damage,name,dimen,hitboxAnime,0,friendly)
{

}

Object* HitboxAssembler::assemble(Object& shooter, const glm::vec2& point, const glm::vec2& target)
{
    Object* obj = new Object();
    obj->addObject(new ObjectComponent(false,movable,friendly,*obj));
    obj->addRect(new HitboxComponent(duration,damage,friendly,target,{point.x-dimen.x/2,point.y-dimen.y/2,dimen.x,dimen.y},*obj,projCollideFunc));
    obj->addRender(new HitboxRender(*obj));
    obj->getComponent<ProjectileComponent>()->setShooter(shooter);
    return obj;
}


void HitboxAttack::doAttack(Object* attacker, const glm::vec2& pos)
{
    RectComponent* rect = nullptr;
    if (assembler && attacker && (rect = attacker->getComponent<RectComponent>()))
    {

        glm::vec2 center = rect->getCenter();
        float angle = atan2( pos.y - center.y, pos.x - center.x);
        glm::vec2 newPos = center + glm::vec2(cos(angle)*(modData.range + rect->getRect().z/2), sin(angle)*(modData.range+ rect->getRect().a/2));
        if (!hitbox)
        {
            hitbox = GameWindow::getRoom()->addUnit(*assembler->assemble(*attacker,newPos,newPos),assembler->friendly);
        }
        HitboxAssembler::HitboxComponent* hit = hitbox->getComponent<HitboxAssembler::HitboxComponent>();
       // if (hit && !hit->getActive())
        //{
            hit->activate();
         //   std::cout << "ACGTIVATE\n";
        //}
       // std::cout << "POinter: " << hitbox->getComponent<RectComponent>() <<"\n";
        hitbox->getComponent<RectComponent>()->setCenter(newPos);
        //std::cout << "ATTACKING\n";
    }
}

HitboxAttack::HitboxAttack(HitboxAssembler& ass, int endLag, double range, AnimationWrapper* attackAnime_, AnimationSequencer* sequencer_) : ProjectileAttack (ass,endLag,range,attackAnime_,sequencer_)
{

}

HitboxAttack::~HitboxAttack()
{
    if (hitbox)
    {
        hitbox->setDead(true);
    }
}

HitboxAssembler::HitboxComponent::HitboxComponent( int duration, float damage, bool friendly, const glm::vec2& target,
                                                  const glm::vec4& rect,Object& entity, ProjectileComponent::ProjCollideFunc func) :
                                                                                ProjectileComponent(damage,friendly,target,0,rect,entity,func),
                                                                                    ComponentContainer<HitboxComponent>(entity),
                                                                                    duration(duration)
{

}

bool HitboxAssembler::HitboxComponent::getActive()
{
    return active;
}

void HitboxAssembler::HitboxComponent::collide(Entity& other) //almost identical to ProjectileComponent::collide except we don't have to destroy our hitbox
{
   ProjectileComponent::collide(other);
   if (ObjectComponent* obj = entity->getComponent<ObjectComponent>())
   {
       obj->setDead(false);
   }
}

void HitboxAssembler::HitboxComponent::activate()
{
    /*timer.set();
    active = true;
    if (ObjectComponent* obj = entity->getComponent<ObjectComponent>())
    {
        obj->setInactive(!active);
    }*/
    active = true;
}

void HitboxAssembler::HitboxComponent::deactivate()
{
    active = false;
}

void HitboxAssembler::HitboxComponent::update()
{
        if (ObjectComponent* obj = entity->getComponent<ObjectComponent>())
        {
            obj->setInactive(!active);
        }
        active = false;
}


UnitBucket allUnits; //vector of all units and structures
UnitBucket allStructures;
UnitBucket allShopItems; //list of purchasable units

UnitAssembler* getRandomAssembler(UnitBucket& bucket)
{
    int it = rand()%(bucket.size());
//    std::cout << it << std::endl;
    return bucket[it];
}

void addUnitToBucket(UnitAssembler& ass, UnitBucket& bucket)
{
    bucket.push_back(&ass);
}
