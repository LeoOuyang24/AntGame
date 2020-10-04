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

void initAssemblers();

class AntAssembler : public UnitAssembler
{
public:
    AntAssembler();
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

extern AntAssembler antAssembler;
extern FactoryAssembler factAssembler;
extern TurretAssembler turretAssembler;

#endif // FRIENDLYASSEMBLERS_H_INCLUDED
