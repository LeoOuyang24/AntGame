#ifndef ENEMYASSEMBLERS_H_INCLUDED
#define ENEMYASSEMBLERS_H_INCLUDED

#include "animation.h"
#include "friendlyAssemblers.h"


struct SequencerData
{
    Uint8 frameStart;

};

class EvilMoonAssembler : public UnitAssembler
{
public:
    EvilMoonAssembler();
    Object* assemble();
};


class TurtFrog : public UnitAssembler
{
    class TurtFrogAttack : public Attack
    {
        static AnimationSequencer turtFrogAttackSequencer;

        constexpr static float lungeRange = 10;
        constexpr static int duration = 2000;
        void doAttack(Object* attacker, const glm::vec2& pos);
    public:
        TurtFrogAttack();
        //ImgParameter attack(Object* attacker, const glm::vec2& pos);
        bool canAttack(Object* attacker, Object* target);
    };
public:
    TurtFrog();
    Object* assemble();
};

class AttackAnt : public UnitAssembler
{
    class AntProjectile : public ProjectileAssembler
    {
    public:
        AntProjectile();
    };
    class AttackAntAttack : public ProjectileAttack
    {
        static AnimationSequencer attackAntAttackSequencer;
        AntProjectile antProjectile;
        void doAttack(Object* attacker, const glm::vec2& pos);
    public:
        AttackAntAttack();
        //ImgParameter attack(Object* attacker, const glm::vec2& pos);
    };
public:
    AttackAnt();
    Object* assemble();
};

class Dinosaur : public UnitAssembler
{
    class DinosaurAttackHitboxAssembler : public HitboxAssembler
    {

        public:
            DinosaurAttackHitboxAssembler();
    };
    class DinosaurAttack : public HitboxAttack
    {
        static DinosaurAttackHitboxAssembler hitbox;
        static AnimationSequencer dinosaurAttackSequencer;
       /* AnimationSequencer dinosaurAttackSequencer = AnimationSequencer({
                                                                                        {1000,5}
                                                                                          });*/
       // void doAttack(Object* attacker, const glm::vec2& pos);
    public:
        DinosaurAttack();
    };
public:
    Dinosaur();
    Object* assemble();
};

extern EvilMoonAssembler evilMoonAssembler;
extern TurtFrog turtFrog;
extern AttackAnt attackAnt;
extern Dinosaur dinosaur;
#endif // ENEMYASSEMBLERS_H_INCLUDED
