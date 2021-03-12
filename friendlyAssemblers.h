#ifndef FRIENDLYASSEMBLERS_H_INCLUDED
#define FRIENDLYASSEMBLERS_H_INCLUDED

#include "entities.h"

struct ObjectAssembler
{
    const glm::vec2 dimen;
    const std::string name;
    UnitAnimSet const sprites;
    const bool movable = false;
    const bool friendly = false;
    ObjectAssembler( std::string name_, const glm::vec2& rect_,const UnitAnimSet& anime, bool mov = false, bool friendly_ = true);
    ~ObjectAssembler();
    virtual Object* assemble();
};

struct UnitAssembler : public ObjectAssembler
{
    const double maxHealth = 0;
    float speed = .1;
    UnitAssembler( std::string name_,const glm::vec2& rect_, const UnitAnimSet& anime, bool mov, double maxHealth_, float speed,bool friendly_ = false);
    virtual Object* assemble();
    Unit* commandUnitAssemble(); //creates a unit with a bunch of components they will need to be commanded, including CommandableComponent and ForceComponent. Does not add unitAttackComponent since that may be unique to each assembler
};

struct ProjectileAssembler : public UnitAssembler
{
    const double damage = 0;
    ProjectileAssembler(double damage_, std::string name_,const glm::vec2& rect_, AnimationWrapper* anime, double maxHealth_, float speed, bool friendly_ = false);
    using UnitAssembler::assemble;
    Object* assemble(Object& shooter, const glm::vec2& point, const glm::vec2& target);
};



typedef std::vector<UnitAssembler*> UnitBucket;

extern UnitBucket allUnits; //vector of all units and structures
extern UnitBucket allStructures;
extern UnitBucket allShopItems;

void initAssemblers();

UnitAssembler* getRandomAssembler(UnitBucket& bucket);
void addUnitToBucket(UnitAssembler& ass, UnitBucket& bucket);

#endif // FRIENDLYASSEMBLERS_H_INCLUDED
