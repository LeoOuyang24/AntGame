#include <assert.h>

#include "effects.h"
#include "entities.h"

StatusEffect::StatusEffect(double value_, int duration_, effectFunc func_, SpriteWrapper& icon_, Unit& unit_, Unit& source_) : value(value_),
                                                                duration(duration_), func(func_), icon(&icon_), unit(&unit_),  source(&source_)
{
    assert (func != nullptr);
    startTime = SDL_GetTicks();
}
bool StatusEffect::isDone()
{
    return SDL_GetTicks() - startTime >= duration;
}
void StatusEffect::update()
{
    func(*this);
}
