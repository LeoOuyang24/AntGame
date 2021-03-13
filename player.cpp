
#include "vanilla.h"
#include "FreeTypeHelper.h"

#include "player.h"
#include "game.h"
#include "navigation.h"
#include "animation.h"
#include "enemyAssemblers.h"
#include "weaponAssemblers.h"

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
    player = static_cast<Unit*>(playerAssembler.assemble());
    addResource(100);
}

Unit* Player::getPlayer()
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

PlayerAssembler::PlayerControls::PlayerControls(float speed, const glm::vec4& rect, Unit& player) : MoveComponent(speed,rect , player)
{
    pistol = pistolAssembler.assemble(&player);
}

void PlayerAssembler::PlayerControls::update()
{
    pistol->update();
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
            glm::vec4 wall;
            if ((wall = GameWindow::getLevel()->getMesh().getWallRect(rect + glm::vec4(move.x,0,0,0) )) != glm::vec4(0))
            {
                if (rect.x < wall.x)
                {
                    move.x = wall.x - rect.z - rect.x - 1;
                }
                else
                {
                    move.x = (wall.x + wall.z + 1) - rect.x;
                }
            }
            if ((wall = GameWindow::getLevel()->getMesh().getWallRect(rect + glm::vec4(0,move.y,0,0))) != glm::vec4(0))
            {
                if (rect.y < wall.y)
                {
                    move.y = wall.y - rect.a - rect.y - 1;
                }
                else
                {
                    move.y = wall.y + wall.a + 1 - rect.y;
                }
            }
            setTarget(getCenter() + move);
        }
        else
        {
            setTarget(getCenter());
        }
    MoveComponent::update();
}

PlayerAssembler::PlayerAssembler() : UnitAssembler("Player",{30,30},{&basicSoldierAnime,&basicShootingAnime},true,100,.5,true)
{

}

Object* PlayerAssembler::assemble()
{
    Unit* player = new Unit(movable);
    player->addHealth(new HealthComponent(*player,maxHealth));
    player->addRect(new PlayerControls(speed,{0,0,dimen.x,dimen.y},*player));
    player->addRender(new UnitAnimationComponent(sprites,*player));
    player->addClickable(new ClickableComponent(name,*player));
    player->setFriendly(friendly);
    return player;
}

PlayerAssembler::~PlayerAssembler()
{

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

