#include "enemyAssemblers.h"

EvilMoonAssembler::EvilMoonAssembler() : UnitAssembler("evilMoon", {20,20}, basicEnemyAnime,true,100,.1)
{

}

Object* EvilMoonAssembler::assemble()
{
    Unit* ent = new Unit();
    ent->addRect((new PathComponent(speed,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new ClickableComponent(name,*ent)));
    ent->addRender((new AnimationComponent(*sprite,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->addComponent(*(new UnitAttackComponent(1,1000,1,100,true,*ent)));
    return ent;
}

AnimationSequencer TurtFrog::TurtFrogAttack::turtFrogAttackSequencer({
                                                                                                                               {500,2},
                                                                                                                               {1000,1},
                                                                                                                               {500,2}
                                                                                                                               });

void TurtFrog::TurtFrogAttack::doAttack(Object* attacker, const glm::vec2& pos)
{
    attacker->getComponent<MoveComponent>()->setSpeed(0);
    if (sequencer->getStateIndex(startAttack) == 2)
    {
        ForcesComponent* forces = attacker->getComponent<ForcesComponent>();
        if (forces)
        {
           // std::cout << atan2( attacker->getCenter().y - pos.y, attacker->getCenter().x - pos.x) << "\n";
            forces->addForce({atan2( attacker->getCenter().y - pos.y, attacker->getCenter().x - pos.x),DeltaTime::deltaTime/500.0*modData.range*-1});
        }
    }
}

TurtFrog::TurtFrogAttack::TurtFrogAttack() : Attack(10,3000,lungeRange,&turtFrogAttack, &turtFrogAttackSequencer)
{

}

bool TurtFrog::TurtFrogAttack::canAttack(Object* owner, Object* ptr)
{
    return Attack::canAttack(owner,ptr);
}

TurtFrog::TurtFrog() : UnitAssembler("TurtFrog",{100,100},turtFrogWalk,true,100,.05,false)
{

}

Object* TurtFrog::assemble()
{
    Unit* ent = NPCUnitAssemble();
    ent->addComponent(*(new UnitAttackComponent(1,100,1,100,true,*ent)));
    ent->getComponent<UnitAttackComponent>()->addAttack(*(new TurtFrogAttack));
//    ent->addComponent();
    return ent;
}

AnimationSequencer AttackAnt::AttackAntAttack::attackAntAttackSequencer = AnimationSequencer({
                                                                                             {1000,7},
                                                                                             {100,0}
                                                                                             });

AttackAnt::AntProjectile::AntProjectile() : ProjectileAssembler(1,"Ant Projectile",{10,10},attackAntProjectile,1,false)
{

}

void AttackAnt::AttackAntAttack::doAttack(Object* attacker, const glm::vec2& pos)
{
    attacker->getComponent<MoveComponent>()->setSpeed(0);
    if (sequencer && sequencer->getStateIndex(startAttack) == -1)
    {
            ProjectileAttack::doAttack(attacker,pos);
    }
}

AttackAnt::AttackAntAttack::AttackAntAttack() : ProjectileAttack(antProjectile,3000,300,&attackAntAttack,&attackAntAttackSequencer)
{

}

AttackAnt::AttackAnt() : UnitAssembler("Attack Ant",{60,30},attackAntAnime,true,20,.1,false)
{

}

Object* AttackAnt::assemble()
{
    Unit* ent = NPCUnitAssemble();
    ent->addComponent(*(new UnitAttackComponent(1,100,1,100,true,*ent)));
    ent->getComponent<UnitAttackComponent>()->addAttack(*(new AttackAnt::AttackAntAttack));
    return ent;
}

/*void Dinosaur::DinosaurAttackHitbox::onCollide(Unit& other)
{
    ForcesComponent* forces = other.getComponent<ForcesComponent>();
    if (forces)
    {
        forces->addForce({atan2(shooter->getCenter().y - other.getCenter().y, shooter->getCenter().x - other.getCenter().x),10});
    }
}

Dinosaur::DinosaurAttackHitbox::DinosaurAttackHitbox() : ProjectileComponent(20,false)
{

}*/

Dinosaur::DinosaurAttackHitboxAssembler::DinosaurAttackHitboxAssembler() : HitboxAssembler(1000,20,"dinosaur attack hitbox", false,{64,64},
                                                                                  [](Unit& other, ProjectileComponent& thisProjectile){
                ForcesComponent* forces = other.getComponent<ForcesComponent>();
                if (forces)
                {
                    forces->addForce({atan2( other.getCenter().y - thisProjectile.getShooter()->getCenter().y, other.getCenter().x - thisProjectile.getShooter()->getCenter().x ),
                                            1,
                                            25,
                                            1
                                            });
                }
                                                                                  }
                                                                                  )
{

}
Dinosaur::DinosaurAttackHitboxAssembler Dinosaur::DinosaurAttack::hitbox;

AnimationSequencer Dinosaur::DinosaurAttack::dinosaurAttackSequencer = AnimationSequencer({
                                                                                          {400,1},
                                                                                        {600,4}
                                                                                          });

void Dinosaur::DinosaurAttack::doAttack(Object* attacker, const glm::vec2& pos)
{
    if (sequencer && sequencer->getStateIndex(startAttack) == 1)
    {
        HitboxAttack::doAttack(attacker,pos);
    }
}

Dinosaur::DinosaurAttack::DinosaurAttack() : HitboxAttack(hitbox,1000,0,&dinosaurAttackAnime,&dinosaurAttackSequencer)
{
    //sequencer = new AnimationSequencer({{1000,5}});
}

Dinosaur::Dinosaur() : UnitAssembler("Dinosaur",{87,58},dinosaurAnime,false,100,.2,false)
{

}
Object* Dinosaur::assemble()
{
    Unit* ent = NPCUnitAssemble();
    ent->addComponent(*(new UnitAttackComponent(1,100,1,100,true,*ent)));
    ent->getComponent<UnitAttackComponent>()->addAttack(*(new Dinosaur::DinosaurAttack));
    return ent;
}

EvilMoonAssembler evilMoonAssembler;
TurtFrog turtFrog;
AttackAnt attackAnt;
Dinosaur dinosaur;
//AttackAnt::AntProjectile antProjectile;
