#ifndef FRIENDLYASSEMBLERS_H_INCLUDED
#define FRIENDLYASSEMBLERS_H_INCLUDED

#include "entities.h"

struct ObjectAssembler
{
    const glm::vec2 dimen;
    const std::string name;
    AnimationWrapper* const sprite = nullptr;
    const bool movable = false;
    const bool friendly = false;
    const int goldCost = 0;
    ObjectAssembler( std::string name_, const glm::vec2& rect_,AnimationWrapper* anime, bool mov = false, bool friendly_ = true, int goldCost_ = 10);
    virtual Object* assemble();

};

struct UnitAssembler : public ObjectAssembler
{
    const double prodTime = 0; //milliseconds it takes to produce this unit
    const double maxHealth = 0;
    const int prodCost = 0;
    UnitAssembler( std::string name_,const glm::vec2& rect_, AnimationWrapper* anime, bool mov, double maxHealth_, double prodTime_, int prodCost = 10, bool friendly_ = false, int goldCost = 10);
    virtual Object* assemble();
};

struct ProjectileAssembler : public UnitAssembler
{
    const double damage = 0;
    ProjectileAssembler(double damage_, std::string name_,const glm::vec2& rect_, AnimationWrapper* anime, double maxHealth_, double prodTime_, int prodCost = 10, bool friendly_ = false, int goldCost = 10);
    using UnitAssembler::assemble;
    Object* assemble(Object& shooter, const glm::vec2& point, const glm::vec2& target);
};

class Player;
class CreateEnergyComponent : public Component, public ComponentContainer<CreateEnergyComponent>
{
    DeltaTime alarm; //the timer for when to generate energy.
    int waitTime = 0; //the number of frames before an energy is generated
    Player* player;
public:
    CreateEnergyComponent(Player& player_, int frames, Entity& entity);
    void update();
};

class AntAssembler : public UnitAssembler
{
public:
    AntAssembler();
    Object* assemble();
};

class BlasterAssembler : public UnitAssembler
{
    class BlasterRocket : public ProjectileAssembler
    {
        class ExplodingRocketComponent : public ProjectileComponent, public ComponentContainer<ExplodingRocketComponent>
        {
        public:
            ExplodingRocketComponent(const glm::vec2& target, const glm::vec2& pos, Unit& entity);
            void onCollide(Unit& other);
        } ;
    public:
        BlasterRocket();
        Object* assemble();
    };
    BlasterRocket rocket;
public:
    BlasterAssembler();
    Object* assemble();
};

class IncineratorAssembler : public UnitAssembler
{
    class IncineratorAttackComponent : public UnitAttackComponent, public ComponentContainer<IncineratorAttackComponent>
    {
    public:
        IncineratorAttackComponent(Entity& unit);
        virtual void attack(HealthComponent* health);
    };
public:
    IncineratorAssembler();
    Object* assemble();
};

class FreezerAssembler : public UnitAssembler
{
    class FreezerAttackComponent : public UnitAttackComponent, public ComponentContainer<FreezerAttackComponent>
    {
        const float slowAmount = .1f; //speed decrease amount
    public:
        FreezerAttackComponent(Entity& unit);
        virtual void attack(HealthComponent* health);
    };
public:
    FreezerAssembler();
    Object* assemble();
};

class FactoryAssembler : public UnitAssembler
{
public:
    FactoryAssembler();
    Object* assemble();
};

class TurretAssembler : public UnitAssembler
{
public:
    TurretAssembler();
    Object* assemble();
};

class HealUnitComponent : public Component
{
    double HPps  = 0;//HP per second
    double radius = 0;
    DeltaTime healthTimer; //keeps track of when we radiate another burst of health
public:
    HealUnitComponent(double rate, double radius, Entity& ent);
    void update();
};

class HealBuildingAssembler : public UnitAssembler
{

public:
    HealBuildingAssembler();
    Object* assemble();
};

typedef std::vector<UnitAssembler*> UnitBucket;

extern UnitBucket allUnits; //vector of all units and structures
extern UnitBucket allStructures;
extern UnitBucket allShopItems;

extern AntAssembler antAssembler;
extern FactoryAssembler factAssembler;
extern TurretAssembler turretAssembler;
extern HealBuildingAssembler healBuildingAssembler;
extern BlasterAssembler blastAssembler;
extern IncineratorAssembler incineratorAssembler;
extern FreezerAssembler freezerAssembler;

void initAssemblers();

UnitAssembler* getRandomAssembler(UnitBucket& bucket);
void addUnitToBucket(UnitAssembler& ass, UnitBucket& bucket);

#endif // FRIENDLYASSEMBLERS_H_INCLUDED
