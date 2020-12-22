#include "worldMap.h"
#include "game.h"
#include "animation.h"

ShopButton::ShopButton(Player& player, UnitAssembler& obj,const glm::vec4& rect) : Button(rect,nullptr,nullptr,{},nullptr,{0,1,0,0}),  assembler(&obj),
                                                                                                    player(&player)
{

}

void ShopButton::press()
{
    if (player && assembler && !soldOut)
    {
        if (assembler->goldCost <= player->getGold())
        {
            soldOut = true;
            player->addGold(-1*assembler->goldCost);
            if (!assembler->movable)
            {
                player->addBuilding(*assembler);
            }
            else
            {
                player->addUnit(*assembler);
            }
        }
    }
}

void ShopButton::update(float x, float y, float z, const glm::vec4& scale)
{
    glm::vec4 renderRect = this->scale(scale);
    float finalZ = z + baseZ + .01;
    if(assembler)
    {
        assembler->sprite->request({renderRect,0, NONE, {1,1,1,1},&RenderProgram::basicProgram,finalZ},{0,0});
        Font::tnr.requestWrite({convert(assembler->goldCost),{renderRect.x,renderRect.y + renderRect.a*1.1, renderRect.z*.8,renderRect.z*.2},
                               0, {0,0,0,1},finalZ});
    }
    PolyRender::requestRect(renderRect,{0,0,1,1},false,0,z + baseZ);
    coinIcon.request({{renderRect.x + renderRect.z*.8,renderRect.y + renderRect.a*1.1, renderRect.z*.2, renderRect.z*.2}, 0, NONE, {1,1,1,1},
                     &RenderProgram::basicProgram, finalZ});
    if (soldOut)
    {
      //  std::cout << "ASDF" << std::endl;
        redX.request({renderRect,0, NONE, {1,1,1,1}, &RenderProgram::basicProgram, finalZ + .01});
    }
   Button::update(x,y,z,scale);
}

void ShopButton::changeAssembler(UnitAssembler* assembler)
{
    this->assembler = assembler;
}

ShopWindow::ShopWindow() : Window({0,0},nullptr,{.5,.5,1,1},0)
{
    // buttons = new ShopButton*[shopItems];
    int margin = .05*rect.z;
    glm::vec4 shopRect = {.2*rect.z,.2*rect.a,.6*rect.z,.6*rect.a}; //rect where all the buttons will go
    int numButtons = sqrt(shopItems); //number of buttons per row and column
    glm::vec2 buttDimen = {shopRect.z/numButtons,shopRect.a/numButtons}; //dimen of the space of buttons and margins, not the dimensions of the buttons
    for (int i = 0; i < shopItems; ++i)
    {
        buttons[i] = new ShopButton(GameWindow::getPlayer(),antAssembler,{
                                    shopRect.x + buttDimen.x*(i%numButtons),
                                    shopRect.y + buttDimen.y*(i/numButtons),
                                     buttDimen.x - margin,buttDimen.y - margin});
        addPanel(*buttons[i]);
    }
  //  onSwitch(*this);
}

void ShopWindow::onSwitch(Window& previous)
{
    for (int i = 0; i < shopItems; ++i)
    {
        UnitAssembler* newAss = getRandomAssembler(allShopItems);
        buttons[i]->changeAssembler(newAss);
    }
}

WorldMapWindow::LevelButton::LevelButton(WorldMapWindow& window, Map& level, const glm::vec4& rect) : Button(rect,nullptr,nullptr,{},nullptr,{1,1,0,1}), window(&window), level(&level)
{

}

void WorldMapWindow::LevelButton::press()
{
    if (window && level && !level->getChangeLevel()) //if we haven't already beaten this level
    {
        window->setCurrentLevel(*level);
    }
}

void WorldMapWindow::LevelButton::update(float x, float y, float z, const glm::vec4& blit)
{
    if (level->getChangeLevel())
    {
        backgroundColor.g = 0;
        backgroundColor.b = 0;
    }
    Button::update(x,y,z,blit);
}


void WorldMapWindow::setCurrentLevel(Map& level)
{
    currentLevel = &level;
}

void WorldMapWindow::addLevel(Map& level)
{
    int width = 64;
    int height = 64;
    levels[&level] = std::shared_ptr<Map>(&level);
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    addPanel(*(new LevelButton(*this,level,{10 + (width+10)*(levels.size()%((int)(screenDimen.x/width))),levels.size()/(screenDimen.x/width),width,height})));
}

void WorldMapWindow::switchToGame()
{
    GameWindow::setLevel(levels[currentLevel]);
    if (currentLevel)
    {
        currentLevel->getAnthill()->setButtons();
    }
    currentLevel = nullptr;
}


WorldMapWindow::WorldMapWindow() : Window({0,0,0,0},nullptr,{0,0,1,1})
{
  //  addPanel(*(new OnOffButton(OnOffMode::DYNAMIC,*shopWindow,{100,100,100,100},nullptr,{"Shop"},&Font::tnr,{1,1,1,1})));
}

void WorldMapWindow::generate()
{
    int numberOfLevels = rand()%6 + 3;
    for (int i = 0; i < numberOfLevels; ++i)
    {
        Map* level = Map::generateLevel();
        addLevel(*level);
    }
}

Map* WorldMapWindow::getCurrentLevel()
{
    return currentLevel;
}


WorldMapWindow::WorldSwitchToGame::WorldSwitchToGame(const glm::vec4& box, Interface& interface, Window& to, WorldMapWindow& worldMap) :
                                                CondSwitchButton(box,nullptr,interface,to,{"Switch"},&Font::tnr,{1,0,1,1},nullptr), worldMap(&worldMap)
{

}

bool WorldMapWindow::WorldSwitchToGame::doSwitch()
{
    return worldMap && worldMap->getCurrentLevel();
}

void WorldMapWindow::WorldSwitchToGame::press()
{
    if (worldMap)
    {
        worldMap->switchToGame();
    }
    WindowSwitchButton::press();
}
