#include "enemyAssemblers.h"
#include "animation.h"

EvilMoonAssembler::EvilMoonAssembler() : UnitAssembler("evilMoon", {20,20}, {&basicEnemyAnime},true,100,.1,100,0)
{

}

Object* EvilMoonAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new PathComponent(speed,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new ClickableComponent(name,*ent)));
    ent->addRender((new UnitAnimationComponent(sprites,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->addComponent(*(new UnitAttackComponent(0,100,1,100,true,*ent)));
    return ent;
}

EvilMoonAssembler evilMoonAssembler;
