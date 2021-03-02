#include "animation.h"

SpriteWrapper fireIcon;
SpriteWrapper freezeIcon;
SpriteWrapper coinIcon;
SpriteWrapper redX;
SpriteWrapper shieldIcon;

AnimationWrapper defaultAnime;
AnimationWrapper shardAnime;
AnimationWrapper resourceAnime;
AnimationWrapper basicSoldierAnime;
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


AnimationWrapper explosionAnime;
AnimationWrapper fireBlastSprite;
AnimationWrapper radiation;


AnimationWrapper portalAnime;


void initSprites()
{
    fireIcon.init("sprites/fire-icon.png");
    freezeIcon.init("sprites/freeze-icon.png");
    coinIcon.init("sprites/gold_icon.png");
    redX.init("sprites/redX.png");
    shieldIcon.init("sprites/shield.png");

    defaultAnime.init(new BaseAnimation("sprites/oldMan.png",.001,6,1));
    shardAnime.init(new BaseAnimation("sprites/orb.png", .01, 12,8,{0,0,6,4}));
    resourceAnime.init(new BaseAnimation("sprites/orb.png",.01,12,8,{.5,.5,6,4}));

    basicSoldierAnime.init(new BaseAnimation("sprites/astronaut.png", .01, 3,1));
    turretSprite.init(new BaseAnimation("sprites/turret.png",0,1,1));
    basicEnemyAnime.init(new BaseAnimation("sprites/evilMoon.png", .01,3,1));
    greenCross.init(new BaseAnimation("sprites/greencross.png",0,1,1));
    blasterAnime.init(new BaseAnimation("sprites/blaster.png",0.01,3,1));
    tankRocketAnime.init(new BaseAnimation("sprites/blastRocket.png",0,1,1));
    incineratorAnime.init(new BaseAnimation("sprites/incinerator.png",.01,3,1));
    freezeUnitAnime.init(new BaseAnimation("sprites/freezeUnit.png",.01,3,1) );
    mercenaryAnime.init(new BaseAnimation("sprites/mercenary.png",.01,3,1));
    minigunEnthAnime.init(new BaseAnimation("sprites/minigunEnth.png",.01,3,1));
    shrimpSuitAnime.init(new BaseAnimation("sprites/shrimp.png",.01,3,1));
    tankAnime.init(new BaseAnimation("sprites/tank.png",0,1,1));
    commanderAnime.init(new BaseAnimation("sprites/commander.png",.01,3,1));
    iceTurretAnime.init(new BaseAnimation("sprites/iceTurret.png",0,1,1));

    explosionAnime.init(new BaseAnimation("sprites/explosion.png",.1,3,4));
    fireBlastSprite.init(new BaseAnimation("sprites/fireBlast.png",0,1,1));
    radiation.init(new BaseAnimation("sprites/radiation.png",0,1,1));

    portalAnime.init(new BaseAnimation("sprites/portal.png",.01,3,2));
}
