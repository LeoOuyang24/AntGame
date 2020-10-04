#include "friendlyAssemblers.h"
#include "animation.h"
#include "ants.h"
#include "game.h"

CreateEnergyComponent::CreateEnergyComponent(Player& player_, int frames, Entity& entity) : player(&player_), waitTime(frames), Component(entity), ComponentContainer<CreateEnergyComponent>(entity)
{

}

void CreateEnergyComponent::update()
{
    if (player && alarm.framesPassed(waitTime))
    {
        player->addResource(1);
        alarm.set();
    }
    else if (!alarm.isSet())
    {
        alarm.set();
    }
}



AntAssembler::AntAssembler() : UnitAssembler("Ant",{20,20},&basicSoldierAnime,true,10, 0000)
{
    prodCost = 10;
}

Object* AntAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new Ant::AntMoveComponent(nullptr,.1,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new Ant::AntClickable(name,*ent)));
    ent->addRender((new AnimationComponent(sprite,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->addComponent(*(new UnitAttackComponent(1,100,100,100,false,*ent)));
    ent->addComponent(*(new CommandableComponent(*ent)));
    return ent;
}


FactoryAssembler::FactoryAssembler() : UnitAssembler("Factory",{30,30}, &defaultAnime, false, 100,1000)
{
    prodCost = 10;
}

Object* FactoryAssembler::assemble()
{
    Unit* stru = static_cast<Unit*>(UnitAssembler::assemble());
    stru->setFriendly(true);
    stru->addComponent(*(new CreateEnergyComponent(GameWindow::getPlayer(),1000,*stru)));

    return stru;
}

TurretAssembler::TurretAssembler() : UnitAssembler("Turret", {30,30},&turretSprite,false,100,1000)
{
    prodCost = 5;
}

Object* TurretAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new MoveComponent(0,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new ClickableComponent(name,*ent)));
    ent->addRender((new AnimationComponent(sprite,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->setFriendly(true);
    ent->addComponent(*(new UnitAttackComponent(1,.1,100,100,false,*ent)));
    return ent;
}

FactoryAssembler factAssembler;
AntAssembler antAssembler;
TurretAssembler turretAssembler;
