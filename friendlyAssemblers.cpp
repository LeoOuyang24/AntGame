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



AntAssembler::AntAssembler() : UnitAssembler("Ant",{20,20},&basicSoldierAnime,true,10, 0000,true,10)
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

BlasterAssembler::BlasterAssembler() : UnitAssembler("Blaster",{20,20},&blasterAnime,true,5,1000,true,20)
{

}

Object* BlasterAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new Ant::AntMoveComponent(nullptr,.3,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new Ant::AntClickable(name,*ent)));
    ent->addRender((new AnimationComponent(sprite,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->addComponent(*(new UnitAttackComponent(.5f,10,100,100,false,*ent)));
    ent->addComponent(*(new CommandableComponent(*ent)));
    return ent;
}

FactoryAssembler::FactoryAssembler() : UnitAssembler("Factory",{30,30}, &defaultAnime, false, 100,1000,true,10)
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

TurretAssembler::TurretAssembler() : UnitAssembler("Turret", {30,30},&turretSprite,false,100,1000,true,10)
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

HealUnitComponent::HealUnitComponent(double rate, double radius, Entity& ent) : Component(ent),HPps(rate), radius(radius)
{
    healthTimer.set();
}

void HealUnitComponent::update()
{
    if (entity && entity->getComponent<RectComponent>())
    {
        Unit* owner = static_cast<Unit*>(entity);
        glm::vec4 rect = owner->getRect().getRect();
        glm::vec2 center = {rect.x + rect.z/2, rect.y + rect.a/2};
        PolyRender::requestCircle({0,1,0,1},GameWindow::getCamera().toScreen(center),radius,1);
       //PolyRender::requestNGon(8,GameWindow::getCamera().toScreen(center),10,{0,1,0,1},0,false,0);
        if (healthTimer.timePassed(1000))
        {
            auto nearby = GameWindow::getLevel()->getTree()->getNearest(center,radius);
            for (int i = 0; i < nearby.size(); ++i)
            {
                Unit* ptr = convertPosToUnit(nearby[i]);
                if (ptr->getFriendly() && !ptr->getComponent<HealUnitComponent>())
                {
                    ptr->getHealth().takeDamage(-1*HPps,*owner);
                }
            }

        }
    }
}

HealBuildingAssembler::HealBuildingAssembler() : UnitAssembler("HealBuilding",{30,30},&greenCross,false,100,1000,true,10)
{
    //addComponent(*(new HealUnitComponent(1,*this)));
}

Object* HealBuildingAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new MoveComponent(0,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new ClickableComponent(name,*ent)));
    ent->addRender((new AnimationComponent(sprite,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->setFriendly(true);
    ent->addComponent(*(new HealUnitComponent(1,100,*ent)));
    return ent;
}

std::vector<UnitAssembler*> allUnits; //vector of all units and structures
std::vector<UnitAssembler*> allStructures;

FactoryAssembler factAssembler;
AntAssembler antAssembler;
TurretAssembler turretAssembler;
HealBuildingAssembler healBuildingAssembler;
BlasterAssembler blastAssembler;

UnitAssembler* getRandomAnyAssembler()
{
    int it = rand()%(allUnits.size() + allStructures.size());
//    std::cout << it << std::endl;
    if (it >= allUnits.size())
    {
        return allStructures[it - allUnits.size()];
    }
    else
    {
        return allUnits[it];
    }
}
