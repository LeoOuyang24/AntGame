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

WeaponComponent::WeaponComponent(Entity& ent, Unit* owner_, AttackComponent* at1, AttackComponent* at2) : Component(ent),ComponentContainer<WeaponComponent>(ent),owner(owner_), attack1(at1), attack2(at2)
{

}

Unit* WeaponComponent::getOwner()
{
    return owner;
}

void WeaponComponent::update()
{
    RectComponent* rect = entity->getComponent<RectComponent>();
    if (rect)
    {
        glm::vec4 ownerRect = owner->getRect().getRect();
        rect->setPos(glm::vec2(ownerRect.x + ownerRect.z - (1*ownerRect.z + rect->getRect().z/2),ownerRect.y + ownerRect.a/2 - rect->getRect().a/2));
        AnimationComponent* animation = entity->getComponent<AnimationComponent>();
        if (animation)
        {
            SpriteParameter param;
           // param.effect = MIRROR;
            glm::vec2 mousePos = GameWindow::getCamera().toWorld(pairtoVec(MouseManager::getMousePos()));
            param.radians = atan2(mousePos.y - rect->getCenter().y, mousePos.x - rect->getCenter().x);
            animation->setParam(param,AnimationParameter());
        }
    }
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

PistolAssembler::PistolBulletAssembler::PistolBulletAssembler() : ProjectileAssembler(1,"Pistol Bullet",{50,50},tankRocketAnime,1,true)
{

}
PistolAssembler::PistolAssembler() : WeaponAssembler({45,15},&pistolAnime,1)
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
