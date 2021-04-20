#ifndef WEAPONASSEMBLERS_H_INCLUDED
#define WEAPONASSEMBLERS_H_INCLUDED

#include "friendlyAssemblers.h"

struct WeaponAttack //represents attack and icon
{
    SpriteWrapper* const icon = 0;
    Attack* const attack = 0;
};

struct WeaponAssembler
{
    const glm::vec2 dimen;
    AnimationWrapper* const sprite = nullptr;
    const float damage = 0;
    WeaponAssembler(const glm::vec2& dimen_, AnimationWrapper* spr, float damage_);
    Entity* assemble(Unit* user);
};

enum AttackIndex
{
    ATTACK1,
    ATTACK2,
    ATTACK3
};

const unsigned int numAttacks = 3; //number of attacks per weapon
typedef std::array<WeaponAttack,numAttacks> AttackStorage;
class WeaponComponent : public Component, public ComponentContainer<WeaponComponent>
{
    Unit* owner = nullptr; //owner that's using the weapon
    AttackStorage attacks;
public:
    WeaponComponent(Entity& ent, Unit* owner_, WeaponAttack a1, WeaponAttack a2, WeaponAttack a3);
    Unit* getOwner();
    AttackStorage& getAttacks();
    void processAttack(void (*weaponFunc)(int i, WeaponAttack& attack)); //function to apply a function to each attack. i is the index of the attack
    virtual void update();
};

struct PistolAssembler : public WeaponAssembler
{
    class PistolSpreadAttack : public ProjectileAttack
    {
        void doAttack(Object* attacker, const glm::vec2& pos);
    public:
        PistolSpreadAttack();
    };
    class PistolSpecialAttack : public ProjectileAttack
    {
        void doAttack(Object* attacker, const glm::vec2& pos);
    public:
        PistolSpecialAttack();
    };
    struct PistolBulletAssembler : public ProjectileAssembler
    {
        PistolBulletAssembler();
    };
    static PistolBulletAssembler bullet;
public:
    PistolAssembler();
    Entity* assemble(Unit* user);
};

extern PistolAssembler pistolAssembler;

#endif // WEAPONASSEMBLERS_H_INCLUDED
