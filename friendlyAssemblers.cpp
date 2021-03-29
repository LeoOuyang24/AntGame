#include "friendlyAssemblers.h"
#include "animation.h"
#include "ants.h"
#include "game.h"
#include "effects.h"

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

Unit* UnitAssembler::commandUnitAssemble()
{
    Unit* ent = new Unit(movable);
   // ent->addRect((new Ant::AntMoveComponent(nullptr,speed,{0,0,dimen.x,dimen.y},*ent)));
   ent->addRect(((new MoveComponent(speed,{0,0,dimen.x,dimen.y},*ent))));
    //ent->addClickable((new Ant::AntClickable(name,*ent)));
    ent->addRender((new UnitAnimationComponent(*sprite,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
   // ent->addComponent(*(new CommandableComponent(*ent)));
    ent->addComponent(*(new ForcesComponent(*ent)));
    ent->setFriendly(friendly);
    return ent;
}

ProjectileAssembler::ProjectileAssembler(double damage_, std::string name_,const glm::vec2& rect_, AnimationWrapper& anime, float speed, bool friendly_) :
                                        damage(damage_),speed(speed),ObjectAssembler(name_,rect_,anime,true,friendly_)
{

}

Object* ProjectileAssembler::assemble(Object& shooter, const glm::vec2& point, const glm::vec2& target)
{
    Object* obj = new Object(movable);
    obj->addRect(new ProjectileComponent(damage,friendly,target,speed,{point.x-dimen.x/2,point.y-dimen.y/2,dimen.x,dimen.y},*obj));
    obj->addRender(new AnimationComponent(*sprite,*obj));
    obj->getComponent<ProjectileComponent>()->setShooter(shooter);
    obj->setFriendly(friendly);
    return obj;
}

HitboxAssembler::HitboxComponent::HitboxComponent(float damage_, Positional& pos, Entity& entity) : Component(entity), ComponentContainer<HitboxComponent>(entity),
                                                                                                    damage(damage_),pos(&pos)
{

}

HitboxAssembler::HitboxComponent::~HitboxComponent()
{
    delete pos;
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
