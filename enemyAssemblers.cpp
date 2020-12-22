#include "enemyAssemblers.h"
#include "animation.h"
#include "ants.h"

EvilMoonAssembler::EvilMoonAssembler() : UnitAssembler("evilMoon", {20,20}, &basicEnemyAnime,true,100,100)
{

}

Object* EvilMoonAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new PathComponent(.1,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new ClickableComponent(name,*ent)));
    ent->addRender((new AnimationComponent(*sprite,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->addComponent(*(new UnitAttackComponent(1,100,1,100,true,*ent)));
    return ent;
}

EvilMoonAssembler evilMoonAssembler;
