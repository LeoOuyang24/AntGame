#include "friendlyAssemblers.h"
#include "animation.h"
#include "ants.h"

AntAssembler antAssembler;

AntAssembler::AntAssembler() : UnitAssembler("Ant",{20,20},&basicSoldierAnime,true,10, 0000)
{
    prodCost = 10;
}

Object* AntAssembler::assemble()
{
    Unit* ent = new Unit();
    ent->addRect((new Ant::AntMoveComponent(nullptr,.1,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new Ant::AntClickable(name,*ent)));
    ent->addRender((new AnimationComponent(sprite,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->addComponent(*(new UnitAttackComponent(1,100,100,*ent)));
    return ent;
}
