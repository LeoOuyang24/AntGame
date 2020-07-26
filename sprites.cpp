#include "animation.h"

AnimationWrapper defaultAnime;
AnimationWrapper shardAnime;

void initSprites()
{
    defaultAnime.init(new BaseAnimation("sprites/oldMan.png",.001,6,1));
    shardAnime.init(new BaseAnimation("sprites/orb.png", .01, 12,8,{0,0,6,4}));
}
