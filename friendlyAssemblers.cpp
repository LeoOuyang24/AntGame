#include "friendlyAssemblers.h"
#include "animation.h"
#include "ants.h"
#include "game.h"
#include "effects.h"

ObjectAssembler::ObjectAssembler( std::string name_, const glm::vec2& rect_,const UnitAnimSet& anime, bool mov, bool friendly_, int goldCost_) :
    dimen(rect_), name(name_), sprites(anime), movable(mov), friendly(friendly_), goldCost(goldCost_)
{

}

Object* ObjectAssembler::assemble()
{
    Object* obj= new Object(name,{0,0,dimen.x,dimen.y},sprites.walking,movable);
    obj->setFriendly(friendly);
    return obj;
}


UnitAssembler::UnitAssembler( std::string name_,const glm::vec2& rect_, const UnitAnimSet& wrap, bool mov, double maxHealth_, float speed, double prodTime_, int prodCost, bool friendly_, int goldCost) :
     ObjectAssembler( name_,rect_, wrap, mov,friendly_,goldCost), maxHealth(maxHealth_), speed(speed), prodTime(prodTime_), prodCost(prodCost)
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

    return new Unit(name,{0,0,dimen.x,dimen.y}, sprites.walking, movable, maxHealth);
}

Unit* UnitAssembler::commandUnitAssemble()
{
    Unit* ent = new Unit(movable);
   // ent->addRect((new Ant::AntMoveComponent(nullptr,speed,{0,0,dimen.x,dimen.y},*ent)));
   ent->addRect(((new MoveComponent(speed,{0,0,dimen.x,dimen.y},*ent))));
    //ent->addClickable((new Ant::AntClickable(name,*ent)));
    ent->addRender((new UnitAnimationComponent(sprites,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
   // ent->addComponent(*(new CommandableComponent(*ent)));
    ent->addComponent(*(new ForcesComponent(*ent)));
    ent->setFriendly(friendly);
    return ent;
}

ProjectileAssembler::ProjectileAssembler(double damage_, std::string name_,const glm::vec2& rect_, AnimationWrapper* anime, double maxHealth_, float speed, double prodTime_, int prodCost, bool friendly_, int goldCost) :
                                        damage(damage_),UnitAssembler(name_,rect_,{anime},true,maxHealth_,speed, prodTime_,prodCost,friendly_,goldCost)
{

}

Object* ProjectileAssembler::assemble(Object& shooter, const glm::vec2& point, const glm::vec2& target)
{
    Object* obj = assemble();
    obj->getRect().setPos(point);
    obj->getComponent<MoveComponent>()->setTarget(target);
    obj->getComponent<ProjectileComponent>()->setShooter(shooter);
    return obj;
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
