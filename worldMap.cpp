#include "worldMap.h"
#include "game.h"

ShopButton::ShopButton(bool isStructure, Player& player, UnitAssembler& obj,const glm::vec4& rect) : Button(rect,nullptr,nullptr,{},nullptr,{0,1,0,1}), isStructure(isStructure), assembler(&obj), player(&player)
{

}

void ShopButton::press()
{
    if (player && assembler)
    {

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

void ShopButton::render(bool hover, float x, float y, float z, float xScale, float yScale)
{
    glm::vec4 renderRect = scale({x,y,xScale,yScale});
    if(assembler)
    {
        assembler->getSprite()->request({renderRect},{0,0});
    }
    PolyRender::requestRect({renderRect.x*.8,renderRect.y*.8,1.4*renderRect.z,1.4*renderRect.a},{0,1,0,1},true,0,z);
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
    shopWindow = new Window({0,0,rect.z*.9,rect.a*.9},nullptr,{0,1,1,1});
    addPanel(*(new OnOffButton(OnOffMode::DYNAMIC,*shopWindow,{100,100,100,100},nullptr,{"Shop"},&Font::tnr,{1,1,1,1})));
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
