#include <assert.h>

#include "effects.h"
#include "entities.h"
#include "animation.h"

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

StatusEffectAssembler::StatusEffectAssembler(effectFunc func_, SpriteWrapper& icon_) : func(func_), icon(&icon_)
{

}

StatusEffect StatusEffectAssembler::getEffect(double value, int duration, Unit& unit, Unit& source)
{
    return StatusEffect(value,duration,func,*icon,unit,source);
}

StatusEffectAssembler chillEffect = StatusEffectAssembler([](StatusEffect& effect){
                                   MoveComponent* move = effect.unit->getComponent<MoveComponent>();
                                   if (move)
                                   {
                                       move->setSpeed(move->getCurSpeed()*(1-effect.value));
                                   }
                                   AnimationComponent* anime = effect.unit->getComponent<AnimationComponent>();
                                   if (anime)
                                   {
                                       anime->setTint({0,.5,0,1});
                                   }
                                   },freezeIcon);
