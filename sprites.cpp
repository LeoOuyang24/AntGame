#include "animation.h"

SpriteWrapper coinIcon;
SpriteWrapper redX;

AnimationWrapper defaultAnime;
AnimationWrapper shardAnime;
AnimationWrapper resourceAnime;
AnimationWrapper basicSoldierAnime;
AnimationWrapper turretSprite;
AnimationWrapper greenCross;
AnimationWrapper basicEnemyAnime;
AnimationWrapper blasterAnime;
AnimationWrapper tankRocketAnime;

AnimationWrapper portalAnime;


void initSprites()
{
    coinIcon.init("sprites/gold_icon.png");
    redX.init("sprites/redX.png");

    defaultAnime.init(new BaseAnimation("sprites/oldMan.png",.001,6,1));
    shardAnime.init(new BaseAnimation("sprites/orb.png", .01, 12,8,{0,0,6,4}));
    resourceAnime.init(new BaseAnimation("sprites/orb.png",.01,12,8,{.5,.5,6,4}));

    basicSoldierAnime.init(new BaseAnimation("sprites/astronaut.png", .01, 3,1));
    turretSprite.init(new BaseAnimation("sprites/turret.png",0,1,1));
    basicEnemyAnime.init(new BaseAnimation("sprites/evilMoon.png", .01,3,1));
    greenCross.init(new BaseAnimation("sprites/greencross.png",0,1,1));
    blasterAnime.init(new BaseAnimation("sprites/blaster.png",0.3,3,1));
    tankRocketAnime.init(new BaseAnimation("sprites/blastRocket.png",0,1,1));


    portalAnime.init(new BaseAnimation("sprites/portal.png",.01,3,2));
}
