#ifndef FRIENDLYASSEMBLERS_H_INCLUDED
#define FRIENDLYASSEMBLERS_H_INCLUDED

#include "entities.h"
#include "player.h"


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
public:
    BlasterAssembler();
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

extern std::vector<UnitAssembler*> allUnits; //vector of all units and structures
extern std::vector<UnitAssembler*> allStructures;

extern AntAssembler antAssembler;
extern FactoryAssembler factAssembler;
extern TurretAssembler turretAssembler;
extern HealBuildingAssembler healBuildingAssembler;
extern BlasterAssembler blastAssembler;

void initAssemblers();

UnitAssembler* getRandomAnyAssembler();

#endif // FRIENDLYASSEMBLERS_H_INCLUDED
