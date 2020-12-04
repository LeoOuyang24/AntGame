#include "worldMap.h"
#include "game.h"
#include "animation.h"

ShopButton::ShopButton(bool isStructure, Player& player, UnitAssembler& obj,const glm::vec4& rect) : Button(rect,nullptr,nullptr,{},nullptr,{0,1,0,0}), isStructure(isStructure), assembler(&obj),
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
            if (isStructure)
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
        assembler->getSprite()->request({renderRect,0, NONE, {1,1,1,1},&RenderProgram::basicProgram,finalZ},{0,0});
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

WorldMapWindow::LevelButton::LevelButton(WorldMapWindow& window, Map& level, const glm::vec4& rect) : Button(rect,nullptr,nullptr,{},nullptr,{1,1,0,1}), window(&window), level(&level)
{

}

void WorldMapWindow::LevelButton::press()
{
    if (window && level)
    {
        window->setCurrentLevel(*level);
    }
}

void WorldMapWindow::LevelButton::render(bool hover,float x, float y, float z, float xScale, float yScale)
{
    glm::vec4 renderRect = scale({x,y,xScale,yScale});
    glm::vec4 color = {0,1,1,1};
    if (hover )
    {
        color *= .5;
        color.a = 1;
    }
    if (level->getChangeLevel())
    {
        color.g = 0;
        color.b = 0;
    }
    PolyRender::requestNGon(20,{renderRect.x + renderRect.z/2, renderRect.y + renderRect.a/2},10,color,0,true,z);
    if (window && level && window->getCurrentLevel() == level)
    {
        PolyRender::requestNGon(20,{renderRect.x + renderRect.z/2, renderRect.y + renderRect.a/2},11,{1,1,0,1},0,true,z);
    }
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

WorldMapWindow::WorldMapWindow() : Window({0,0,0,0},nullptr,{0,0,1,1})
{
    shopWindow = new Window({.1*rect.z,.1*rect.a,rect.z*.8,rect.a*.8},nullptr,{0,0,0,1},1);
    addPanel(*(shopWindow));
    shopWindow->setDoUpdate(false);
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

void WorldMapWindow::switchTo(Window& swapTo)
{
    GameWindow::setLevel(levels[currentLevel]);
    currentLevel = nullptr;
}

WorldMapWindow::WorldSwitchToGame::WorldSwitchToGame(const glm::vec4& box, Interface& interface, Window& to, WorldMapWindow& worldMap) :
                                                CondSwitchButton(box,nullptr,interface,to,{"Switch"},&Font::tnr,{1,0,1,1},nullptr), worldMap(&worldMap)
{

}

bool WorldMapWindow::WorldSwitchToGame::doSwitch()
{
    return worldMap && worldMap->getCurrentLevel();
}
