#include "animation.h"

SpriteWrapper fireIcon;
SpriteWrapper freezeIcon;
SpriteWrapper coinIcon;
SpriteWrapper redX;
SpriteWrapper shieldIcon;

AnimationWrapper defaultAnime;
AnimationWrapper shardAnime;
AnimationWrapper resourceAnime;

AnimationWrapper playerAnime;
AnimationWrapper playerArm;
AnimationWrapper playerHurt;
AnimationWrapper basicSoldierAnime;
AnimationWrapper basicShootingAnime;
AnimationWrapper turretSprite;
AnimationWrapper greenCross;
AnimationWrapper basicEnemyAnime;
AnimationWrapper blasterAnime;
AnimationWrapper tankRocketAnime;
AnimationWrapper incineratorAnime;
AnimationWrapper freezeUnitAnime;
AnimationWrapper mercenaryAnime;
AnimationWrapper minigunEnthAnime;
AnimationWrapper shrimpSuitAnime;
AnimationWrapper tankAnime;
AnimationWrapper commanderAnime;
AnimationWrapper iceTurretAnime;

AnimationWrapper pistolAnime;

AnimationWrapper explosionAnime;
AnimationWrapper fireBlastSprite;
AnimationWrapper radiation;


AnimationWrapper portalAnime;


void initSprites()
{

    playerAnime.init(new BaseAnimation("sprites/astro/astro2.png",4,4,1,{0,0,4,1}));
    playerArm.init(new BaseAnimation("sprites/astro/astroArm.png",1,1,1));
    playerHurt.init(new BaseAnimation("sprites/astro/astrohurt.png",0,1,1));
    pistolAnime.init(new BaseAnimation("sprites/gun2.png",.01,1,1));



    basicEnemyAnime.init(new BaseAnimation("sprites/evilMoon.png", .01,3,1));

    fireIcon.init("sprites/fire-icon.png");
    freezeIcon.init("sprites/freeze-icon.png");
    coinIcon.init("sprites/gold_icon.png");
    redX.init("sprites/redX.png");
    shieldIcon.init("sprites/shield.png");

    defaultAnime.init(new BaseAnimation("sprites/oldMan.png",.001,6,1));
    shardAnime.init(new BaseAnimation("sprites/orb.png", .01, 12,8,{0,0,6,4}));
    resourceAnime.init(new BaseAnimation("sprites/orb.png",.01,12,8,{.5,.5,6,4}));

    tankRocketAnime.init(new BaseAnimation("sprites/blastRocket.png",0,1,1));


    explosionAnime.init(new BaseAnimation("sprites/explosion.png",.1,3,4));
    fireBlastSprite.init(new BaseAnimation("sprites/fireBlast.png",0,1,1));
    radiation.init(new BaseAnimation("sprites/radiation.png",0,1,1));

    portalAnime.init(new BaseAnimation("sprites/portal.png",.01,3,2));
}
