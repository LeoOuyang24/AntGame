#include "animation.h"

AnimationWrapper defaultAnime;
AnimationWrapper shardAnime;
AnimationWrapper basicSoldierAnime;
AnimationWrapper basicEnemyAnime;
AnimationWrapper portalAnime;

void initSprites()
{
    defaultAnime.init(new BaseAnimation("sprites/oldMan.png",.001,6,1));
    shardAnime.init(new BaseAnimation("sprites/orb.png", .01, 12,8,{0,0,6,4}));

    basicSoldierAnime.init(new BaseAnimation("sprites/astronaut.png", .01, 3,1));
    basicEnemyAnime.init(new BaseAnimation("sprites/evilMoon.png", .01,3,1));

    portalAnime.init(new BaseAnimation("sprites/portal.png",.01,3,2));
}
