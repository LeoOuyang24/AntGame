#include "animation.h"

AnimationWrapper defaultAnime;
AnimationWrapper shardAnime;
AnimationWrapper resourceAnime;
AnimationWrapper basicSoldierAnime;
AnimationWrapper turretSprite;
AnimationWrapper basicEnemyAnime;
AnimationWrapper portalAnime;

void initSprites()
{
    defaultAnime.init(new BaseAnimation("sprites/oldMan.png",.001,6,1));
    shardAnime.init(new BaseAnimation("sprites/orb.png", .01, 12,8,{0,0,6,4}));
    resourceAnime.init(new BaseAnimation("sprites/orb.png",.01,12,8,{.5,.5,6,4}));

    basicSoldierAnime.init(new BaseAnimation("sprites/astronaut.png", .01, 3,1));
    turretSprite.init(new BaseAnimation("sprites/turret.png",0,1,1));
    basicEnemyAnime.init(new BaseAnimation("sprites/evilMoon.png", .01,3,1));

    portalAnime.init(new BaseAnimation("sprites/portal.png",.01,3,2));
}
