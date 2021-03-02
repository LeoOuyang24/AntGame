
#include "vanilla.h"
#include "FreeTypeHelper.h"

#include "player.h"
#include "game.h"
#include "navigation.h"
#include "animation.h"
#include "enemyAssemblers.h"

const glm::vec4 Player::selectColor = {1,1,1,.5};

bool Player::updateSelect()
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
}

Player::BuildingButton::BuildingButton(const glm::vec4& rect, Player& player_,UnitAssembler& building_) :
    Button(rect,nullptr,building_.sprite,{},nullptr,{0,0,0,0}),player(&player_), building(&building_)
{

}

void Player::BuildingButton::press()
{
    if (player && building)
    {
        player->setCurrentBuilding(building);
    }
}

Player::Player()
{
    //currentBuilding = &factAssembler;
}

void Player::init()
{
    buildingWindow = new Window({.2*RenderProgram::getScreenDimen().y,.2*RenderProgram::getScreenDimen().y}
                          ,nullptr,{0,1,1,1});
   // addBuilding(factAssembler);
   // addBuilding(turretAssembler);
   addUnit(antAssembler);
   addBuilding(evilMoonAssembler);
    addResource(100);
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

void Player::update()
{
 //       PolyRender::requestRect(GameWindow::getCamera().toScreen(selection),{1,0,0,.5},true,0,1);

    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
   // Font::tnr.requestWrite({"Resources: " + convert(resource),GameWindow::getCamera().toAbsolute({screenDimen.x - 200, 50, 100,100}),0,{0,0,0,1},GameWindow::interfaceZ});

    switch(KeyManager::getJustPressed())
    {
    case SDLK_b:
        if (mode == BUILDING)
        {
            mode = SELECTING;
        }
        else
        {
            mode = BUILDING;
        }
        break;
    }

    switch (mode)
    {
        case SELECTING:
            if (updateSelect())
            {
                GameWindow::requestRect(selection,selectColor,true,0,-.1);
            }
            break;
        case BUILDING:
            auto mousePos = GameWindow::getCamera().toWorld(pairtoVec(MouseManager::getMousePos()));
            glm::vec4 color = selectColor;
            if(currentBuilding)
            {
                glm::vec2 dimen = currentBuilding->dimen;
                glm::vec4 structRect = {mousePos.x - dimen.x/2,mousePos.y - dimen.y/2,dimen.x, dimen.y};
                bool collides = !GameWindow::getLevel()->getMesh().notInWall(structRect);
                if (!collides)
                {
                    Map* level = GameWindow::getLevel();
                    RawQuadTree* tree = level->getTree();
                    auto vec = tree->getNearest(structRect);
                    int size = vec.size();
                    for (int i = 0; i < size; ++i)
                    {
                        if (vec[i]->collides(structRect))
                        {
                            collides = true;
                            break;
                        }
                    }
                }
                else
                {
                    color = {1,0,0,1};
                }
                GameWindow::requestRect(structRect,color,true,0,0,0);
                if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT && !collides && getResource() >= currentBuilding->prodCost)
                {
                    Object* ptr = (currentBuilding->assemble());
                    RectComponent* rect = &ptr->getRect();
                    rect->setPos({mousePos.x - dimen.x/2, mousePos.y - dimen.y/2});
                    InactiveComponent* inactive = new InactiveComponent(currentBuilding->prodTime,*ptr);
                    ptr->addComponent(*inactive);
                    GameWindow::getLevel()->addUnit(*(ptr), ptr->getFriendly());
                    inactive->init();
                    addResource(-1*currentBuilding->prodCost);
                }
            }
            break;
    }
    if (mode != SELECTING)
    {
        selection = {0,0,0,0};
    }

}

void Player::render(const glm::vec4& windowSize)
{
    buildingWindow->updateBlit(GameWindow::interfaceZ,GameWindow::getCamera(),true, windowSize);
   // GameWindow::requestRect(windowSize,{0,1,1,1},true,0,GameWindow::interfaceZ,true);
}

const glm::vec4& Player::getSelection()
{
    return selection;
}

void Player::setCurrentBuilding(UnitAssembler* building)
{
    currentBuilding = building;
}

void Player::addBuilding(UnitAssembler& building)
{
    if (buildings.insert(&building).second) //insert the element if it's not already in the set. If it's new, add the buildingButton
    {

        glm::vec4 rect = buildingWindow->getRect();
        int size = (buildingWindow->countPanels());
        double butWidth = .25*rect.z;
        double butHeight = .25*rect.a;
        buildingWindow->addPanel(*(new BuildingButton({butWidth*fmod(size,rect.z/butWidth),butHeight*(size/((int)(rect.z/butWidth))),butWidth,butHeight},*this,building)));
    }
}

void Player::addUnit(UnitAssembler& unit)
{
    units.insert(&unit);
}

std::set<UnitAssembler*>& Player::getUnits()
{
    return units;
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

