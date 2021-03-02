#include "worldMap.h"
#include "game.h"
#include "animation.h"

ShopButton::ShopButton(Player& player, UnitAssembler* obj,const glm::vec4& rect) : Button(rect,nullptr,nullptr,{},nullptr,{0,1,0,0}),  assembler(obj),
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
    else
    {
        soldOut = true;
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
        buttons[i] = new ShopButton(GameWindow::getPlayer(),nullptr,{
                                    shopRect.x + buttDimen.x*(i%numButtons),
                                    shopRect.y + buttDimen.y*(i/numButtons),
                                     buttDimen.x - margin,buttDimen.y - margin});
        addPanel(*buttons[i]);
    }
  //  onSwitch(*this);
}

void ShopWindow::onSwitch(Window& previous)
{
    std::set<UnitAssembler*> set;
    for (int i = 0; i < shopItems; ++i)
    {
        UnitAssembler* newAss = nullptr;//getRandomAssembler(allShopItems);
        if (allShopItems.size() == set.size())
        {
            break;
        }
        while (set.find(newAss) != set.end() || newAss == nullptr)
        {
            newAss = getRandomAssembler(allShopItems);
        }
        set.insert(newAss);
        buttons[i]->changeAssembler(newAss);
    }
}

WorldMapWindow::LevelButton::LevelButton(LevelButton* prev_, LevelButton* next_,WorldMapWindow& window, Map& level, const glm::vec4& rect) :
                                                                                            Button(rect,nullptr,nullptr,{},nullptr,{1,1,0,1}),
                                                                                            window(&window), level(&level), prev(prev_),next(next_)
{

}

void WorldMapWindow::LevelButton::setNext(LevelButton* next_)
{
    this->next = next_;
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
    if (next)
    {
        int dotNumber = 4;
        double minDist = window->getPlanetPlanetDistance()/dotNumber;
        glm::vec2 center = {rect.x + rect.z/2, rect.y + rect.a/2};
        float angle = atan2(next->getRect().y - rect.y,next->getRect().x - rect.x);
        int framesPerDot = 120;
        auto frame = SDL_GetTicks();
        for (int i = 0; i < dotNumber; ++i)
        {
            PolyRender::requestNGon(20,window->getCamera().toScreen({rect.x + (i+1)*minDist*cos(angle),rect.y + (i+1)*minDist*sin(angle)}),2,
                                    {1,1,1,((frame%(dotNumber*framesPerDot))/framesPerDot == i)*((frame%framesPerDot) + 1)*(1.0/framesPerDot)},0,true,.01);
        }
    }
    //glm::vec2 point = window->getCamera().toScreen({x,y});
    Button::update(x,y,z,blit);
    PolyRender::requestNGon(10,{x,y},10,{1,0,0,1},0,true,1);
}


void WorldMapWindow::setCurrentLevel(Map& level)
{
    currentLevel = &level;
}

WorldMapWindow::LevelButton* WorldMapWindow::addLevel(Map& level,LevelButton* prev, LevelButton* next)
{
    int width = 64;
    int height = 64;
    levels[&level] = std::shared_ptr<Map>(&level);
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    glm::vec2 pos;
    if (prev)
    {
        glm::vec4 prevRect = prev->getRect();
        float angle = (rand()%90)*M_PI/180;
        pos = {prevRect.x + prevRect.z/2 + cos(angle)*planetPlanetDistance,
                prevRect.y + prevRect.a/2 + sin(angle)*planetPlanetDistance};
    }
    else
    {
        pos = {10,10};
    }
    auto butt = (new LevelButton(prev,next,*this,level,{pos.x,pos.y,width,height}));
    addPanel(*butt);
    return butt;
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


WorldMapWindow::WorldMapWindow() : Window({0,0,10000,10000},nullptr,{0,0,1,1})
{
    auto screenDimen = RenderProgram::getScreenDimen();
    camera.init(screenDimen.x,screenDimen.y);
    camera.setBounds(rect);
    setCamera(&camera);
    planetPlanetDistance = rect.z/20;
  //  addPanel(*(new OnOffButton(OnOffMode::DYNAMIC,*shopWindow,{100,100,100,100},nullptr,{"Shop"},&Font::tnr,{1,1,1,1})));
}

WorldMapWindow::LevelButton* WorldMapWindow::generate(int count, LevelButton* start, LevelButton* end)
{
    if (count < 0)
    {
        return generate(rand()%6 + 3,start,end);
    }
    else if (count == 0)
    {
        return start;
    }
    else
    {
        Map* level = Map::generateLevel();
        auto butt = addLevel(*level,start,nullptr);
        auto next = generate(count - 1,butt,end);
        if (next != butt) //happens if count = 1
        {
            butt->setNext(next);
        }
        return butt;
    }
}

void WorldMapWindow::update(float x, float y, float z, const glm::vec4& blit)
{
    camera.update();
    Window::update(x,y,z,blit);
}

Map* WorldMapWindow::getCurrentLevel()
{
    return currentLevel;
}

int WorldMapWindow::getPlanetPlanetDistance()
{
    return planetPlanetDistance;
}

const Camera& WorldMapWindow::getCamera()
{
    return camera;
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
