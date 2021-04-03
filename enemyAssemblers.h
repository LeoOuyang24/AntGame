#ifndef ENEMYASSEMBLERS_H_INCLUDED
#define ENEMYASSEMBLERS_H_INCLUDED

#include "friendlyAssemblers.h"


struct SequencerData
{
    Uint8 frameStart;

};

class AnimationSequencer
{
    int totalFrames = 0;
    int fullDuration = 0;
    int infoSize = 0;
    glm::vec3* info = nullptr; //x value is the duration of the sequence (ms), y is the number of frames should pass during that time, z is the frame that starts at each sequence
    int getStateIndex(int frameStart, int* timeSince); //gets frameStart AND the left over time, which is placed into timeSince
public:
    AnimationSequencer(const std::vector<glm::vec2>& info_);
    AnimationParameter process(int frameStart);
    int getStateIndex(int frameStart);
    ~AnimationSequencer();
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
        AnimationSequencer sequencer;
        constexpr static float lungeRange = 10;
        constexpr static int duration = 2000;
        float delay; //can't make this static because it depends on the animation
        DeltaTime attackTimer;
        ImgParameter getParam(Object* attacker, const glm::vec2& pos);
        void doAttack(Object* attacker, const glm::vec2& pos);
    public:
        TurtFrogAttack(Entity& unit);
        virtual ImgParameter attack(Object* attacker, const glm::vec2& pos);
        bool canAttack(Object* attacker, Object* target);
    };
public:
    TurtFrog();
    Object* assemble();
};

extern EvilMoonAssembler evilMoonAssembler;
extern TurtFrog turtFrog;

#endif // ENEMYASSEMBLERS_H_INCLUDED
