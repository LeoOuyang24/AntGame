#include "weaponAssemblers.h"
#include "animation.h"
#include "game.h"
#include "entities.h"

WeaponAssembler::WeaponAssembler(AnimationWrapper* spr, float damage_) : sprite(spr), damage(damage_)
{

}

Entity* WeaponAssembler::assemble(Unit* unit)
{
    Entity* ent = new Entity();
    ent->addComponent(*(new AnimationComponent(*sprite,*ent)));
    return ent;
}

WeaponComponent::WeaponComponent(Entity& ent, Unit* owner_, AttackComponent* at1, AttackComponent* at2) : Component(ent),ComponentContainer<WeaponComponent>(ent),owner(owner_), attack1(at1), attack2(at2)
{

}

Unit* WeaponComponent::getOwner()
{
    return owner;
}

void WeaponComponent::update()
{
    if (MouseManager::isPressed(SDL_BUTTON_LEFT))
    {
        attack1->update();
    }
    if (MouseManager::isPressed(SDL_BUTTON_RIGHT))
    {
        attack2->update();
    }
    if (KeyManager::findNumber(SDLK_r) != -1)
    {
      //  attack3->update();
    }
}

PistolAssembler::PistolBulletAssembler::PistolBulletAssembler() : ProjectileAssembler(1,"Pistol Bullet",{64,64},&tankRocketAnime,1,1,true)
{

}
PistolAssembler::PistolAssembler() : WeaponAssembler(&pistolAnime,1)
{

}

Entity* PistolAssembler::assemble(Unit* user)
{
    Entity* pistol = WeaponAssembler::assemble(user);
    pistol->addComponent(*(new WeaponComponent(*pistol,user,
                                                new ProjectileAttackComponent(bullet,1000,0,0,false,*pistol),
                                               new ProjectileAttackComponent(bullet,3,0,0,false,*pistol))));
    return pistol;
}

PistolAssembler pistolAssembler;
