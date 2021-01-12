#ifndef EFFECTS_H_INCLUDED
#define EFFECTS_H_INCLUDED

#include "render.h"

class Unit;
class StatusEffect;
typedef void (*effectFunc) (StatusEffect& thisEffect); //represents a statusEffects' effect. 1st param is the extent of the effect. 2nd, is the effect applying it.

struct StatusEffect
{
    const double value = 0; //Value is a multi-purpose value that can be used to represent many things, such as damage or slow amount
    const int duration = 0; //duration in milliseconds
    SpriteWrapper* const icon = nullptr; //an icon that represents the effect
    Unit* const unit = nullptr;
    Unit* const source = nullptr; //the unit that applied the status effect.
    StatusEffect(double value_, int duration_, effectFunc func_, SpriteWrapper& icon_, Unit& unit_, Unit& source_); //need to use a constructor to set startTime
    bool isDone();
    virtual void update();
private:
    const effectFunc func = nullptr;
    unsigned int startTime = 0;
};

#endif // EFFECTS_H_INCLUDED
