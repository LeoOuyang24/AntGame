#include "weaponAssemblers.h"
#include "animation.h"
#include "game.h"
#include "entities.h"

WeaponAssembler::WeaponAssembler(const glm::vec2& dimen_, AnimationWrapper* spr, float damage_) : dimen(dimen_),sprite(spr), damage(damage_)
{

}

Entity* WeaponAssembler::assemble(Unit* unit)
{
    Entity* ent = new Entity();
    ent->addComponent(*(new AnimationComponent(*sprite,*ent)));
    ent->addComponent(*(new RectComponent({0,0,dimen.x,dimen.y},*ent)));
    return ent;
}

WeaponComponent::WeaponComponent(Entity& ent, Unit* owner_, WeaponAttack a1, WeaponAttack a2, WeaponAttack a3) : Component(ent),
                                                                                                                ComponentContainer<WeaponComponent>(ent),owner(owner_),
                                                                                                                attacks({a1,a2,a3})
{

}

Unit* WeaponComponent::getOwner()
{
    return owner;
}

AttackStorage& WeaponComponent::getAttacks()
{
    return attacks;
}

void WeaponComponent::processAttack(void (*weaponFunc)(int i,WeaponAttack& attack))
{
    if (weaponFunc)
    {
        for (int i = 0; i < numAttacks; ++i)
        {
            weaponFunc(i,attacks[i]);
        }
    }

}

void WeaponComponent::update()
{
    glm::vec2 mousePos = GameWindow::getCamera().toWorld(pairtoVec(MouseManager::getMousePos()));
    if (MouseManager::isPressed(SDL_BUTTON_LEFT) && attacks[ATTACK1].attack->offCooldown())
    {
        attacks[ATTACK1].attack->attack(owner,mousePos);
    }
    if (MouseManager::isPressed(SDL_BUTTON_RIGHT)&& attacks[ATTACK2].attack->offCooldown())
    {
        attacks[ATTACK2].attack->attack(owner,mousePos);
    }
    if (KeyManager::findNumber(SDLK_SPACE) != -1&& attacks[ATTACK3].attack->offCooldown())
    {
        attacks[ATTACK3].attack->attack(owner,mousePos);
    }
}

void PistolAssembler::PistolSpreadAttack::doAttack(Object* attacker, const glm::vec2& pos)
{
    if (attacker)
    {
        glm::vec2 center = attacker->getCenter();
        float dist = pointDistance(center,pos);
        float angle = atan2(pos.y - center.y, pos.x - center.x);
        float altAngle = M_PI/8;
        ProjectileAttack::doAttack(attacker,{center.x + dist*cos(angle + altAngle),center.y + dist*sin(angle + altAngle)});
        ProjectileAttack::doAttack(attacker,pos);
        ProjectileAttack::doAttack(attacker,{center.x + dist*cos(angle + -1*altAngle),center.y + dist*sin(angle + -1*altAngle)});
    }
}

PistolAssembler::PistolSpreadAttack::PistolSpreadAttack() : ProjectileAttack(bullet,3000,0)
{

}

void PistolAssembler::PistolSpecialAttack::doAttack(Object* attacker, const glm::vec2& pos)
{
    if (attacker)
    {
        glm::vec2 center = attacker->getCenter();
        float dist = pointDistance(center,pos);
        float angle = atan2(pos.y - center.y, pos.x - center.x);
        int amount = 8; //amount of bullets;
        float altAngle = M_PI/amount*2;
        for (int i = 0; i < amount; ++i)
        {
            ProjectileAttack::doAttack(attacker,{center.x + dist*cos(angle + i*altAngle),center.y + dist*sin(angle + i*altAngle)});
        }
        //ProjectileAttack::doAttack(attacker,pos);
        //ProjectileAttack::doAttack(attacker,{center.x + dist*cos(angle + -1*altAngle),center.y + dist*sin(angle + -1*altAngle)});
    }
}

PistolAssembler::PistolSpecialAttack::PistolSpecialAttack() : ProjectileAttack(bullet,5000,0)
{

}

PistolAssembler::PistolBulletAssembler::PistolBulletAssembler() : ProjectileAssembler(5,"Pistol Bullet",{50,50},tankRocketAnime,1,true)
{

}

PistolAssembler::PistolBulletAssembler PistolAssembler::bullet;

PistolAssembler::PistolAssembler() : WeaponAssembler({21,23},&pistolAnime,1)
{

}

Entity* PistolAssembler::assemble(Unit* user)
{
    Entity* pistol = WeaponAssembler::assemble(user);
    pistol->addComponent(*(new WeaponComponent(*pistol,user,
                                                {&pistolAttack1,new ProjectileAttack(bullet,500,0)},
                                               {&pistolAttack2,new PistolSpreadAttack()},
                                               {&pistolAttackSpecial,new PistolSpecialAttack()})));
    return pistol;
}

PistolAssembler pistolAssembler;
