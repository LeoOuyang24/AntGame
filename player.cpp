
#include "vanilla.h"
#include "FreeTypeHelper.h"

#include "weaponAssemblers.h"
#include "player.h"
#include "game.h"
#include "navigation.h"
#include "animation.h"
#include "enemyAssemblers.h"
#include "weaponAssemblers.h"

InventoryComponent::InventoryComponent(Unit& player) : Component(player), ComponentContainer<InventoryComponent>(player)
{
    pistol = pistolAssembler.assemble(&player);
}

Entity* InventoryComponent::getWeapon()
{
    return pistol;
}

void InventoryComponent::update()
{
    if (pistol)
    {
        pistol->getComponent<WeaponComponent>()->update();
      //  pistol->update();
    }
}

WeaponAnimationComponent::WeaponAnimationComponent(AnimationWrapper* arm_, const glm::vec2& armOffset_, AnimationWrapper& sprite, Unit& owner) : UnitAnimationComponent(sprite,owner),
                                                                                                                                        ComponentContainer<WeaponAnimationComponent>(owner),
                                                                                                                                         arm(arm_), armOffset(armOffset_)
{

}

bool WeaponAnimationComponent::doMirror()
{
    if (entity && entity->getComponent<RectComponent>())
    {
    glm::vec2 mousePos = GameWindow::getCamera().toWorld(pairtoVec(MouseManager::getMousePos()));
        glm::vec4 rect = entity->getComponent<RectComponent>()->getRect();
    float angle = atan2(mousePos.y - (rect.y + rect.a/2), mousePos.x - (rect.x + rect.a/2));
    return abs(round(angle)) < M_PI/2;
    }
    return false;
}

void WeaponAnimationComponent::update()
{
    if (arm && sprite)
    {
        SpriteParameter armParam;
        glm::vec2 dimen = sprite->getDimen();
        glm::vec2 armDimen = arm->getDimen();
        glm::vec4 rect = entity->getComponent<RectComponent>()->getRect();
        bool mirror = doMirror();
        if (!mirror)
        {
         armParam.rect = {rect.x - armOffset.x/dimen.x*rect.z,rect.y + armOffset.y/dimen.y*rect.a,armDimen.x/dimen.x*rect.z,armDimen.y/dimen.y*rect.a};

        }
        else
        {
            armParam.rect = {rect.x + rect.z + armOffset.x/dimen.x*rect.z,rect.y + armOffset.y/dimen.y*rect.a,-1*armDimen.x/dimen.x*rect.z,armDimen.y/dimen.y*rect.a};
        }
        armParam.rect = GameWindow::getCamera().toScreen(armParam.rect);
        armParam.z = param.z + 1;
        arm->request(armParam);
        if (entity->getComponent<InventoryComponent>())
        {
            Entity* weapon = entity->getComponent<InventoryComponent>()->getWeapon();
            if (weapon)
            {
                AnimationComponent* weaponAnime = weapon->getComponent<AnimationComponent>();
                RectComponent* weaponRectComp = weapon->getComponent<RectComponent>();
                if (weaponAnime && weaponRectComp)
                {
                    glm::vec4 weaponRect = weaponRectComp->getRect();
                    glm::vec2 mousePos = GameWindow::getCamera().toWorld(pairtoVec(MouseManager::getMousePos()));
                    armParam.radians = atan2(mousePos.y - (rect.y + rect.a/2), mousePos.x - (rect.x + rect.a/2));
                    if (!mirror)
                    {
                        armParam.radians += M_PI;
                        armParam.rect = {rect.x - weaponRect.z/2,rect.y + rect.a/2 - weaponRect.a/2, weaponRect.z,weaponRect.a};//repurpose armParam for the weapon
                    }
                    else
                    {
                        armParam.rect = {rect.x + rect.z + weaponRect.z/2, rect.y + rect.a/2 - weaponRect.a/2, -1*weaponRect.z, weaponRect.a};
                    }
                    //armParam.effect = MIRROR;
                    armParam.rect = GameWindow::getCamera().toScreen(armParam.rect);
                    armParam.z = param.z + .5;
                    weaponAnime->render(armParam);
                }
            }
        }
    }
    UnitAnimationComponent::update();
}

PlayerAssembler::PlayerControls::PlayerControls(float speed, const glm::vec4& rect, Unit& player) : MoveComponent(speed,rect , player)
{

}

void PlayerAssembler::PlayerControls::update()
{

        glm::vec2 move = {0,0};
        if (KeyManager::findNumber(SDLK_d) != -1)
        {
            move.x += 1;
        }
        if (KeyManager::findNumber(SDLK_a) != -1)
        {
            move.x -= 1;
        }
        if (KeyManager::findNumber(SDLK_w) != -1)
        {
            move.y -= 1;
        }
        if (KeyManager::findNumber(SDLK_s) != -1)
        {
            move.y += 1;
        }
        if (move.x != 0 || move.y != 0)
        {
            float angle = atan2(move.y,move.x);
            move = {cos(angle)*speed*DeltaTime::deltaTime,sin(angle)*speed*DeltaTime::deltaTime};
            glm::vec4 target = GameWindow::getLevel()->getCurrentRoom()->getMesh().validMove(rect,move);
           // GameWindow::requestRect(target,{1,0,1,1},true,0,2);
            setTarget({target.x + target.z/2, target.y + target.a/2});
            //printRect(target);
        }
        else
        {
            setTarget(getCenter());
        }
    MoveComponent::update();
}

PlayerAssembler::PlayerHealth::PlayerHealth(Entity& entity, float health_) : HealthComponent(entity, health_)
{
    invulTime = 250;
}

void PlayerAssembler::PlayerHealth::takeDamage(double amount, Object& attacker)
{
    if (isDamaging(amount))
    {
        ForcesComponent* forces = entity->getComponent<ForcesComponent>();
        RectComponent* rect = entity->getComponent<RectComponent>();
        if (forces && rect)
        {
            glm::vec2 center = rect->getCenter();
            glm::vec2 otherCenter = attacker.getCenter();
            float angle = atan2(center.y - otherCenter.y, center.x - otherCenter.x);
            forces->addForce({angle,5});
        }

    }
    HealthComponent::takeDamage(amount,attacker);
    if (getHealth() <= 0)
    {
        ProjectileComponent* proj = entity->getComponent<ProjectileComponent>();
        if (proj)
        {
            assembler  = proj->getAssembler(); //get the assembler's attacker in case this kills us
        }
        else
        {
            ObjectComponent* obj = attacker.getComponent<ObjectComponent>();
            if (obj)
            {
                assembler = obj->assembler;
            }
        }
    }
}

ObjectAssembler* PlayerAssembler::PlayerHealth::getAssembler()
{
    return assembler;
}

PlayerAssembler::PlayerRender::PlayerRender(AnimationWrapper* arm_, const glm::vec2& offset, AnimationWrapper& wrap, Unit& owner) : WeaponAnimationComponent(arm_,offset,wrap,owner)
{

}

void PlayerAssembler::PlayerRender::update()
{
    HealthComponent* health = entity->getComponent<HealthComponent>();
    if (health && health->isInvincible() > 0)
    {
        param.tint.a = DeltaTime::getCurrentFrame() % 10 < 5;
        if (health->isInvincible() > .75)
        {
            SpriteParameter param = {GameWindow::getCamera().toScreen(entity->getComponent<RectComponent>()->getRect())};
            param.effect = doMirror() ? MIRROR : NONE;
            playerHurt.request(param);
        }
        else
        {
                WeaponAnimationComponent::update();
        }

    }
    else
    {
        WeaponAnimationComponent::update();
    }
}

PlayerAssembler::PlayerAssembler() : UnitAssembler("Player",{25,47},playerAnime,true,10,.5,true)
{

}

Object* PlayerAssembler::assemble()
{
    Unit* player = new Unit();
    player->addObject(new ObjectComponent(name,false,movable,friendly,this,*player));
    player->addHealth(new PlayerHealth(*player,maxHealth));
    player->addRect(new PlayerControls(speed,{0,0,dimen.x,dimen.y},*player));
    player->addRender(new PlayerRender(&playerArm,{3,49},*sprite,*player));
    player->addClickable(new ClickableComponent(name,*player));
    player->addComponent(*(new InventoryComponent(*player)));
    player->addComponent(*(new EntityForces(*player)));
    player->setFriendly(friendly);
    return player;
}

PlayerAssembler::~PlayerAssembler()
{

}

const glm::vec4 Player::selectColor = {1,1,1,.5};

/*bool Player::updateSelect()
{
    if (MouseManager::isPressed(SDL_BUTTON_LEFT))
    {
        std::pair<int,int> mouse_Pos = MouseManager::getMousePos();
        glm::vec2 mousePos = GameWindow::getCamera().toWorld({mouse_Pos.first, mouse_Pos.second});
      //  std::cout << mousePos.x << " " << mousePos.y << std::endl;
        if (MouseManager::getJustClicked() != SDL_BUTTON_LEFT)
        {
            double width = mousePos.x - origin.x;
            double height = mousePos.y - origin.y;
            selection = absoluteValueRect({origin.x,origin.y, width, height});
            return true;
        }
        else
        {
            origin.x = mousePos.x;
            origin.y = mousePos.y;
            selection.z = 0;
            selection.a = 0;
            return false;
        }
    }
    return false;
}*/

Player::Player()
{
    //currentBuilding = &factAssembler;
}

void Player::init()
{
   // addBuilding(factAssembler);
   // addBuilding(turretAssembler);
   // reset();
}

void Player::reset()
{
    player.reset(static_cast<Unit*>(playerAssembler.assemble()));
    addResource(100);
}

Unit* Player::getPlayer()
{
    return static_cast<Unit*>(player.get());
}

std::shared_ptr<Unit>& Player::getPlayerPtr()
{
    return player;
}


int Player::getResource()
{
    return resource;
}

void Player::addResource(int r)
{
    resource = std::max(resource + r,0);
}

int Player::getGold()
{
    return gold;
}

void Player::addGold(int g)
{
    gold = std::max(gold + g, 0);
}

void Player::renderUI()
{
    if (player)
    {
      glm::vec2 screenDimen = RenderProgram::getScreenDimen();
        const int space = screenDimen.x/20;
        const float iconDimen = screenDimen.x/10;
        if (InventoryComponent* inv = player->getComponent<InventoryComponent>())
        {
            if (Entity* weaponEnt = inv->getWeapon())
            {
                if (WeaponComponent* weapon = weaponEnt->getComponent<WeaponComponent>() )
                {
                    AttackStorage arr = weapon->getAttacks();
                    for (int i = 0; i < numAttacks; ++i)
                    {
                        if (arr[i].icon)
                        {
                            SpriteParameter param = {{screenDimen.x/2 - iconDimen/2-space + (iconDimen + space)*i,
                                                    screenDimen.y*.8,
                                                    iconDimen,
                                                    iconDimen
                                                  }};
                            if (arr[i].attack && !arr[i].attack->offCooldown())
                            {
                                auto cooldownLeft = arr[i].attack->getCooldownRemaining();
                                auto cooldown = arr[i].attack->getEndLag();
                                float ratio = 1-(float)cooldownLeft/cooldown;
                                param.tint = {ratio,ratio,ratio,1};
                                Font::tnr.requestWrite({convert(ceil(cooldownLeft/1000.0)),param.rect,0,{1,1,1,1},GameWindow::interfaceZ + .1});
                            }
                            param.z = GameWindow::interfaceZ;
                            arr[i].icon->request(param);
                        }
                    }
                }
            }
        }
    }
}

Player::~Player()
{
  //  std::cout << player.get()->getComponent<ObjectComponent>()->name;
  //  player.get()->removeComponent<ObjectComponent>();
}

InactiveComponent::InactiveComponent(double duration, Entity& entity) : waitTime(duration), Component(entity), ComponentContainer<InactiveComponent>(entity)
{

}

void InactiveComponent::init()
{
    timeLeft.set();
}

bool InactiveComponent::done()
{
    return timeLeft.timePassed(waitTime);
}

void InactiveComponent::render()
{
    RenderComponent* render = entity->getComponent<RenderComponent>();
    RectComponent* rect = entity->getComponent<RectComponent>();
    if (render && rect)
    {
        glm::vec4 renderRect = GameWindow::getCamera().toScreen(rect->getRect());
        render->render({renderRect,0,NONE,{.5,.5,.5,1}});
        renderTimeMeter({renderRect.x,renderRect.y + 1.1*renderRect.a, renderRect.z,20 },
                        {.5,.5,.5,1},timeLeft,waitTime,GameWindow::interfaceZ);
      //  Font::tnr.requestWrite({convert((waitTime - (SDL_GetTicks() - timeLeft.getTime()))/1000.0),renderRect,0,{1,1,1,1},GameWindow::fontZ});
    }
}

