#ifndef WEAPONASSEMBLERS_H_INCLUDED
#define WEAPONASSEMBLERS_H_INCLUDED

#include "friendlyAssemblers.h"

struct WeaponAssembler
{
    AnimationWrapper* const sprite = nullptr;
    const float damage = 0;
    WeaponAssembler(AnimationWrapper* spr, float damage_);
    Entity* assemble(Unit* user);
};

class WeaponComponent : public Component, public ComponentContainer<WeaponComponent>
{
    Unit* owner = nullptr; //owner that's using the weapon
    AttackComponent* attack1 = nullptr, *attack2 = nullptr, *attack3 = nullptr;
public:
    WeaponComponent(Entity& ent, Unit* owner_, AttackComponent* at1, AttackComponent* at2);
    Unit* getOwner();
    virtual void update();
};

struct PistolAssembler : public WeaponAssembler
{
    struct PistolBulletAssembler : public ProjectileAssembler
    {
        PistolBulletAssembler();
    };
    PistolBulletAssembler bullet;
public:
    PistolAssembler();
    Entity* assemble(Unit* user);
};

extern PistolAssembler pistolAssembler;

#endif // WEAPONASSEMBLERS_H_INCLUDED
