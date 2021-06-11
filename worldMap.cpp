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
        //if (assembler->goldCost <= player->getGold())
       /* {
            soldOut = true;
            player->addGold(-1*assembler->goldCost);*/
            /*if (!assembler->movable)
            {
                player->addBuilding(*assembler);
            }
            else
            {
                player->addUnit(*assembler);
            }*/
        //}
    }
}

void ShopButton::update(float x, float y, float z, const glm::vec4& scale)
{
   /* glm::vec4 renderRect = this->scale(scale);
    float finalZ = z + baseZ + .01;
    if(assembler)
    {
        assembler->sprites.walking->request({renderRect,0, NONE, {1,1,1,1},&RenderProgram::basicProgram,finalZ},{0,0});
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
   Button::update(x,y,z,scale);*/
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

void MouseCamera::init(int w, int h, const glm::vec4& bounds_)
{
    bounds = bounds_;
    RenderCamera::init(w,h);
}

void MouseCamera::update()
{
    {
        auto mousePos = MouseManager::getMousePos();
        auto screenDimen = RenderProgram::getScreenDimen();
        int speed = 1;
        if (mousePos.first >= screenDimen.x - 1 || mousePos.first <= 0)
        {
            rect.x += absMin((mousePos.first - speed), (mousePos.first - screenDimen.x +1 + speed ))*DeltaTime::deltaTime;
            //std::cout << absMin((mousePos.first - speed), (mousePos.first - screenDimen.x +1 + speed ))*DeltaTime::deltaTime << "\n";
        }
        if (mousePos.second >= screenDimen.y - 1 || mousePos.second <= 0)
        {
            rect.y +=  absMin((mousePos.second - speed), (mousePos.second - screenDimen.y + 1+ speed ))*DeltaTime::deltaTime;
        }
        rect.x = std::max(bounds.x,std::min(bounds.x + bounds.z - rect.z,rect.x));
        rect.y = std::max(bounds.y, std::min(bounds.y + bounds.a  - rect.a , rect.y));
        //    GameWindow::requestRect({screenDimen.x/2 - 5, screenDimen.y/2 - 5,10,10},{0,0,0,1},true,0,0,true);

    }
}

/*The way Levelbutton renders the planet sprite is by rendering a portion of the sprites/planets.png (planetsAnime)
that corresponds to the right planet. spriteNum tells us which planet to render. Currently, which spriteNum corresponds
to which portion is hard-coded. */

void WorldMapWindow::LevelButton::setSprite()
{
    float width = 463.0;
    float height = 375.0;
    switch (spriteNum)
    {
        case 0: //cat planet
            param.portion = {33/width,122/height,183/width,163/height};
            break;
        case 1: //top left blue planet
            param.portion = {38/width,17/height,103/width,95/height};
            break;
        case 2: //skinny brown planet
            param.portion = {169/width,16/height,83/width,97/height};
            break;
        case 3: //dark, green planet
            param.portion = {287/width,14/height,108/width,108/height};
            break;
        case 4: //pink planet
            param.portion = {226/width,125/height,90/width,90/height};
            break;
        case 5: //bottom left blue planet
            param.portion = {278/width,218/height,148/width,140/height};
            break;
    }
    rect.z = param.portion.z/param.portion.a*rect.a;
    rect.a = param.portion.a/param.portion.z*rect.z;
}

WorldMapWindow::LevelButton::LevelButton(int prevs_, LevelButton* prev_,WorldMapWindow& window, Level& level, const glm::vec4& rect) :
                                                                                            Button(rect,nullptr,&planetsAnime,{},nullptr,{1,1,0,1}),
                                                                                            window(&window), level(&level), prev(prev_), prevs(prevs_)
{
    spriteNum = rand()%6;

}

WorldMapWindow::LevelButton* WorldMapWindow::LevelButton::getLeft()
{
    return left;
}

WorldMapWindow::LevelButton* WorldMapWindow::LevelButton::getRight()
{
    return right;
}

Level* WorldMapWindow::LevelButton::getLevel() const
{
    return level;
}

void WorldMapWindow::LevelButton::addNext(LevelButton* next_)
{
    if (next_)
    {

        if (left) //if we already have a left, check if we got a new left or right
        {
            float y =next_->getRect().y;
            if (y > left->getRect().y) //lower than left
            {
                if ((right && y > right->getRect().y) || !right) //no right or more right than current right
                {
                    right = next_;
                }
            }
            else if (y == left->getRect().y)//as high if not higher than left
            {
                if (!right) //no right, so shift left to right. Note that if right is non-null, there is no way a value higher than left can be lower than right
                {
                    right = left;
                }
                left = next_;  //update left
            }
        }
        else //left should always be filled before right
        {
            left= next_;
        }
        next.insert(next_);
    }
}

void WorldMapWindow::LevelButton::press()
{
    if (window && level && !level->getCompleted() && (window->getCurrentLevel() && window->getCurrentLevel()->getNext().count(this) != 0)) //if we haven't already beaten this level
    {
        window->setSelectedLevel(*this);
    }
}

void WorldMapWindow::LevelButton::update(float x, float y, float z, const glm::vec4& blit)
{
    if (window)
    {
        if (next.size() > 0)
        {
            double dotDist = .05*window->getRect().z; //distance between each dot
            glm::vec2 center = {rect.x + rect.z/2, rect.y + rect.a/2};
            auto end = next.end();
            for (auto i = next.begin(); i != end; ++i)
            {
                LevelButton* button = *i;
                if (button)
                {
                    const glm::vec4* buttonRect = &button->getRect();
                    glm::vec2 buttonCenter={buttonRect->x + buttonRect->z/2, buttonRect->y + buttonRect->a/2};
                    int dotNumber = pointDistance({rect.x,rect.y},{buttonRect->x,buttonRect->y})/dotDist;
                    //window->requestLine(glm::vec4(center,glm::vec2(button->getRect().x,button->getRect().y)),{0,1,0,1},1);
                    float angle = atan2(buttonRect->y - rect.y,buttonRect->x - rect.x);
                    glm::vec2 lineCenter ={(center.x + buttonCenter.x)/2, (center.y + buttonCenter.y)/2};
                    glm::vec2 dimen = {pointDistance(buttonCenter,center), 10};
                    window->requestRect({lineCenter.x - dimen.x/2,lineCenter.y -dimen.y/2, dimen.x,dimen.y},{1,1,1,1},true,angle,z - .1);
                }
            }
        }
        if (this == window->getSelectedLevel())
        {
            SpriteParameter spriteParam = {window->getCamera().toScreen({rect.x + rect.z/2 - rect.z/4, rect.y + rect.a/2 - rect.a/4,rect.z/2,rect.a/2})};
            spriteParam.z = z + 1;
            rocketIcon.request(spriteParam);
        }
        if (level->getCompleted())
        {
            SpriteParameter spriteParam = {window->getCamera().toScreen(rect)};
            spriteParam.z = z + .5;
            redX.request(spriteParam);
        }
        else if (window->getCurrentLevel() && window->getCurrentLevel()->getNext().count(this) == 0)
        {
            param.tint = {.3,.3,.3,1};
        }
        param.radians = cos(SDL_GetTicks()/5000.0 + 1000*spriteNum);
        setSprite(); //param.portion gets wiped every time so we just reset it. <<< REFACTOR (inefficient?)
    }
    //glm::vec2 point = window->getCamera().toScreen({x,y});
    Button::update(x,y,z,blit);
    //PolyRender::requestNGon(10,{x,y},10,{1,0,0,1},0,true,1);
}

const std::unordered_set<WorldMapWindow::LevelButton*>& WorldMapWindow::LevelButton::getNext() const
{
    return next;
}

void WorldMapWindow::setSelectedLevel(LevelButton& level)
{
    selectedLevel = &level;
}

void WorldMapWindow::setCurrentLevel(LevelButton& level)
{
    currentLevel =&level;
}

WorldMapWindow::LevelButton* WorldMapWindow::addLevel(Level& level,LevelButton* prev)
{
    if (levels.find(&level) == levels.end())
    {
        int width = 64;
        int height = 64;
        levels[&level] = std::shared_ptr<Level>(&level);
        glm::vec2 screenDimen = RenderProgram::getScreenDimen();
        glm::vec2 pos = {10,10};
        auto butt = (new LevelButton(prev ? prev->prevs + 1: 0 //determine how many prevs based on prev88
                                     ,prev,*this,level,{pos.x,pos.y,width,height}));
        levelButtons[&level] = butt;
        while (butt->prevs >= levelLayers.size())
        {
            levelLayers.push_back({});
        }
        levelLayers[butt->prevs].push_back(butt);
        rerender = true;
        addPanel(*butt);
        if (!prev)
        {
            rootButton = butt;
        }
        return butt;
    }
    return nullptr;
}

void WorldMapWindow::switchToGame()
{
    GameWindow::setLevel(levels[selectedLevel->getLevel()]);
    currentLevel = selectedLevel;


}

void WorldMapWindow::buttonRerender()
{
    int layers = levelLayers.size();
    for (int i = 0; i < layers; ++i)
    {
        int siblings = levelLayers[i].size();
        for (int j = 0; j < siblings; ++j)
        {
            LevelButton* button = levelLayers[i][j];
            glm::vec4 newRect = button->getRect();
            int columnSpace = .1*rect.z; //space between each column
            int vertSpace = (rect.a - siblings*newRect.a)/(siblings + 1); //vertical spacing between each level
            int size = button->getNext().size();
            newRect.x = columnSpace*(i + 1) + newRect.z*i;
            newRect.y = rect.y + rect.a/2 + (j - siblings/2.0)*newRect.a + ((j + 1) - (siblings + 1)/2.0)*vertSpace;
            button->changeRect(newRect);
        }
    }
}

void WorldMapWindow::clearLevels()
{
    rootButton = nullptr;
    levels.clear();
    auto end = levelButtons.end();
    for (auto it = levelButtons.begin(); it != end; ++it)
    {
        removePanel(*(it->second));
    }
    levelButtons.clear();
    levelLayers.clear();
}

WorldMapWindow::WorldMapWindow() : Window({0,0,2000,1500},nullptr,{0,0,1,1})
{
    camera.init(RenderProgram::getScreenDimen().x,RenderProgram::getScreenDimen().y,rect);
    setCamera(&camera);
    planetPlanetDistance = rect.z/20;
  //  addPanel(*(new OnOffButton(OnOffMode::DYNAMIC,*shopWindow,{100,100,100,100},nullptr,{"Shop"},&Font::tnr,{1,1,1,1})));
}

WorldMapWindow::LevelButton* WorldMapWindow::generate(int count, LevelButton* start, LevelButton* end)
{
    if (count < 0)
    {
        return generate(rand()%2 + 3,start,end);
    }
    else
    {
        clearLevels();
        auto root = addLevel(*(new Level()),nullptr); //initialize root. Root is always the only level of the first layer
        for (int layer = 0; layer < count - 1; ++layer) //for each level layer. Only go up to count - 1 because we already made the first layer (root)
        {
            int levelAmount = levelLayers[layer].size(); //number of levels in this layer
            for (int level = 0; level < levelAmount; ++level) //for each level in each layer
            {
                int levelCount = (layer == 0) ?  5 : rand()%2 + 1 ; //generate 5 levels if start is null (level would therefore be the root)
                for (int next = 0; next < levelCount; ++next) //for each level after each level in the layer
                {
                    if (next == 0 && rand()%3 == 0 && level > 0)
                    {
                        LevelButton* right = levelLayers[layer][level - 1]->getRight();
                        levelLayers[layer][level]->addNext(right ? right : levelLayers[layer][level - 1]->getLeft());
                    }
                    else
                    {
                        Level* newLevel = new Level();
                        LevelButton* butt = addLevel(*newLevel,levelLayers[layer][level]);//new currently generated level
                        glm::vec4 newRect = butt->getRect();
                        newRect.y = levels.size(); //we want addNext() to correctly set left and right, so we have to change the y value
                                                    //this gets changed by rerender to a more visually appealing value later <<< REFACTOR (unintuitive)
                        butt->changeRect(newRect);
                        levelLayers[layer][level]->addNext(butt);
                    }

                }
            }
        }
        setCurrentLevel(*root);
        setSelectedLevel(*root);
        rerender = true;
        return root;
    }
}

void WorldMapWindow::update(float x, float y, float z, const glm::vec4& blit)
{
    camera.update();
    if (rerender)
    {
        buttonRerender();
        rerender = false;
    }
    requestLine({rect.x,rect.y + rect.a/2,rect.z + rect.x,rect.y + rect.a/2},{1,0,0,1},z);
    Window::update(x,y,z,blit);
}

const WorldMapWindow::LevelButton* WorldMapWindow::getCurrentLevel()
{
    return currentLevel;
}

const WorldMapWindow::LevelButton* WorldMapWindow::getSelectedLevel()
{
    return selectedLevel;
}

int WorldMapWindow::getPlanetPlanetDistance()
{
    return planetPlanetDistance;
}

const MouseCamera& WorldMapWindow::getCamera()
{
    return camera;
}

void WorldMapWindow::switchToRoot()
{
    if (!rootButton)
    {
        throw std::logic_error("WorldMapWindow::switchToRoot: rootButton is null!");
    }
    selectedLevel = rootButton;
    switchToGame();
}

WorldMapWindow::WorldSwitchToGame::WorldSwitchToGame(const glm::vec4& box, Interface& interface, Window& to, WorldMapWindow& worldRoom) :
                                                CondSwitchButton(box,nullptr,interface,to,{"Switch"},&Font::tnr,{1,0,1,1},nullptr), worldRoom(&worldRoom)
{

}

bool WorldMapWindow::WorldSwitchToGame::doSwitch()
{
    return worldRoom && worldRoom->getSelectedLevel() && worldRoom->getSelectedLevel()->getLevel();
}

void WorldMapWindow::WorldSwitchToGame::press()
{
    if (doSwitch())
    {
        worldRoom->switchToGame();
        WindowSwitchButton::press();
    }
}
