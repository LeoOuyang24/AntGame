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
    float speed = .1;
    UnitAssembler( std::string name_,const glm::vec2& rect_, AnimationWrapper* anime, bool mov, double maxHealth_, float speed, double prodTime_, int prodCost = 10, bool friendly_ = false, int goldCost = 10);
    virtual Object* assemble();
    Unit* commandUnitAssemble(); //creates a unit with a bunch of components they will need to be commanded, including CommandableComponent and ForceComponent. Does not add unitAttackComponent since that may be unique to each assembler
};

struct ProjectileAssembler : public UnitAssembler
{
    const double damage = 0;
    ProjectileAssembler(double damage_, std::string name_,const glm::vec2& rect_, AnimationWrapper* anime, double maxHealth_, float speed, double prodTime_, int prodCost = 10, bool friendly_ = false, int goldCost = 10);
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
            ExplodingRocketComponent(const glm::vec2& target, const glm::vec2& rect, Unit& entity);
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

class MercenaryAssembler : public UnitAssembler
{
    class MercenaryAttackComponent: public UnitAttackComponent
    {
    public:
        MercenaryAttackComponent(Entity& unit);
        virtual bool canAttack(Object* ptr);
        void attack(HealthComponent* health);
    };
    public:
    MercenaryAssembler();
    Object* assemble();
};

class MinigunUserAssembler : public UnitAssembler
{
    class MinigunHypeComponent : public Component
    {
    public:
        MinigunHypeComponent(Entity& unit);
        void update();
    };
public:
    MinigunUserAssembler();
    Object* assemble();
};

class ShrimpSuitOperator : public UnitAssembler
{
public:
    ShrimpSuitOperator();
    Object* assemble();
};

class SuperMegaTank : public UnitAssembler
{
    class NukeComponent : public ProjectileComponent
    {
        DeltaTime timer;
        bool tangible = false;
    public:
        NukeComponent(const glm::vec2& target, const glm::vec4& rect, Unit& entity);
        void update();
        void collide(Entity& other);
    };
    class NukeAssembler : public ProjectileAssembler
    {
    public:
        NukeAssembler();
        Object* assemble();
    };
    class SMTankComponent : public UnitAttackComponent
    {
        NukeAssembler nukeAssembler;
        unsigned int counter = 0;
    public:
        SMTankComponent(Unit& entity);
        void attack(HealthComponent* health);
    };
public:
    SuperMegaTank();
    Object* assemble();
};

class CommanderAssembler : public UnitAssembler
{
    class CommanderComponent : public Component, public ComponentContainer<CommanderComponent>
    {
    public:
        CommanderComponent(Unit& unit);
        void update();
    };
public:
    CommanderAssembler();
    Object* assemble();

};

class IceBlasterAssembler :public UnitAssembler
{
    class IceBlasterAttackComponent : public UnitAttackComponent
    {
    public:
        IceBlasterAttackComponent(Unit& unit);
        void attack(HealthComponent* health);
    };
public:
    IceBlasterAssembler();
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
