#include "SDLHelper.h"
#include "vanilla.h"

#include "debug.h"
#include "game.h"
#include "ants.h"
#include "sequence.h"
#include "navigation.h"
#include "enemyAssemblers.h"

SpriteWrapper frame;

Manager::TaskNode::TaskNode(Manager& t) : task(t, Player::selectColor)
{

}

Manager::TaskNode::TaskNode(AntManager&& t) : task(std::move(t))
{

}

bool Manager::TaskNode::hasChildren()
{
    for (int i = 0; i < AntManager::maxChildren; ++i)
    {
        if (child[i].get())
        {
            return true;
        }
    }
    return false;
}

Manager::TaskNode::~TaskNode()
{
    for (int i = 0; i < AntManager::maxChildren; ++i)
    {
        child[i].reset();
    }
}

Manager::Manager()
{

}
const ObjPtr Manager::getSelectedUnit() const
{
    return selectedUnit;
}
const AntManager* Manager::getCurrentTask() const
{
    auto task = currentTask.lock().get();
    if (task)
    {
        return &(task->task);
    }
    return nullptr;
}

void Manager::init(const glm::vec4& region)
{
   // tree.reset(new RawQuadTree(region));
    tasks.reserve(maxTasks);
}

Unit* Manager::generateCreature()
{
    int random = rand()%5;

    return static_cast<Unit*>(evilMoonAssembler.assemble());
}

/*void Manager::spawnCreatures()
{

    Unit* toSpawn = generateCreature();
    Map* level = &(GameWindow::getLevel());
    const glm::vec4* mapSize = &(level->getRect(level->getCurrentChunk()));
    const glm::vec4* camera = &(GameWindow::getCamera().getRect());
    const glm::vec4* entityRect = &(toSpawn->getRect().getRect());
    auto area = level->getMesh().getRandomArea(level->getRect());
    if (area.z - entityRect->z > 0 && area.a - entityRect->a > 0) //if we have enough space
    {
        int x = rand()%((int)(area.z -  entityRect->z)) + area.x; //we want to make sure our object spawns outside of the camera's view and doesn't spawn partially out of the map
        bool cameraInTheWay = (x >= camera->x && x <= camera->x + camera->z); //if we chose an x coordinate that may overlap with the camera's rect, we need to adjust our y coordinate
        int y = rand() % ((int)(area.a - entityRect->a - camera->a*cameraInTheWay)) + area.y;
        //modify coordinates so our object doesn't spawn in the player's view
        y += camera->a*(y > camera->y)*cameraInTheWay;
        toSpawn->getRect().setPos({x,y});
        level->addUnit(*toSpawn);
    }

}*/

void Manager::spawnCreatures(Anthill& hill, double minR, double maxR) //spawn creatures near an anthill at a certain radius
{
    Unit* toSpawn = generateCreature();
    Map* level = (GameWindow::getLevel());
    const glm::vec4* mapSize = &(level->getRect());
  //  const glm::vec4* camera = &(GameWindow::getCamera().getRect());
    const glm::vec4* entityRect = &(toSpawn->getRect().getRect());
    auto area = level->getMesh().getRandomArea(hill.getCenter(), minR, maxR);
    //printRect(area);
    if (area.z - entityRect->z > 0 && area.a - entityRect->a > 0) //if we have enough space
    {
        int x = rand()%((int)(area.z -  entityRect->z)) + area.x; //we want to make sure our object spawns outside of the camera's view and doesn't spawn partially out of the map
       // bool cameraInTheWay = (x >= camera->x && x <= camera->x + camera->z); //if we chose an x coordinate that may overlap with the camera's rect, we need to adjust our y coordinate
        int y = rand() % ((int)(area.a - entityRect->a)) + area.y;
        //modify coordinates so our object doesn't spawn in the player's view
        //y += camera->a*(y > camera->y)*cameraInTheWay;
        toSpawn->getRect().setPos({x,y});
        UnitAttackComponent* unitAttack = toSpawn->getComponent<UnitAttackComponent>();
        AttackComponent* attack = toSpawn->getComponent<AttackComponent>();
        if (unitAttack)
        {
            unitAttack->setLongTarget(closestPointOnVec(hill.getRect().getRect(),{x,y}),&level->getUnit(&hill),false);
        }
        else if (attack)
        {
            attack->setTarget(level->getUnit(&hill));
        }
        level->addUnit(*toSpawn);
    }

   // std::cout << point.x << " " << point.y << std::endl;

}

int Manager::processAntManagers(std::shared_ptr<TaskNode>& node,int index, int y, int x)
{
    TaskNode* nodePtr = node.get();
    if (nodePtr)
    {
        y++;
        std::string key = "";
        TaskNode* parent = parentTask.lock().get();
        node->task.updateAnts();
        if (nodePtr == parent)
        {
            key = "~";
        }
        else if(index > -1)
        {
            key = convert(index + 1);
        }

        else
        {
            if ( parent != nullptr)
            {
                if (nodePtr == parent->child[0].get())
                {
                    key = "a";
                }
                else if (nodePtr == parent->child[1].get())
                {
                    key = "s";
                }
                else if (nodePtr == parent->child[2].get())
                {
                    key = "d";
                }
                else if (nodePtr == parent->child[3].get())
                {
                    key = "f";
                }
            }
        }
        node->task.render({10*x + 1,y*25,30,20},key);
        if (nodePtr->task.getAnts().size() > 0)
        {
            for (int i = 0; i < AntManager::maxChildren; ++i)
            {
               y = processAntManagers(node->child[i],-1,y , x + 1);
            }
        }
    }
    return y;
}

void Manager::updateAntManagers()
{

    int size = tasks.size();
    int previous = 0;
    for (int i = 0; i < size; ++i)
    {
        previous = processAntManagers(tasks[i],i,previous,0);
    }
    int pressed = KeyManager::getJustPressed();
    auto taskNode = currentTask.lock().get();
    if (pressed != -1)
    {
        int index = (pressed - SDLK_1); //this is is the same as (pressed-SDLK1%maxTasks) except it always returns a positive number
        if (index < (int)tasks.size() && index >= 0) //we have to use .size() rather than the size variable here because size might change
        {
            currentTask = tasks[index];
            if (tasks[index]->hasChildren())
            {
                parentTask = currentTask;
            }
            else
            {
                parentTask.reset();
            }
            Camera* cam = &(GameWindow::getCamera());
            glm::vec2 center = currentTask.lock().get()->task.getCenter();
            if (!pointInVec(cam->getRect(),center.x,center.y,0))
            {
                cam->center(center);
            }
        }
        else if (taskNode)
        {
            auto task = &(taskNode->task);
            switch (pressed)
                {
                    case SDLK_TAB:
                        split();
                        parentTask = currentTask;
                        break;
                    case SDLK_BACKQUOTE:
                        currentTask = parentTask;
                        break;
                    case SDLK_F1:
                        parentTask = currentTask;
                        break;
                    default:
                        auto parent = parentTask.lock().get();
                        if (parent)
                        {
                            auto oldTask = currentTask;
                            switch (pressed)
                            {
                            case SDLK_a:
                                currentTask = parent->child[0];
                                break;
                            case SDLK_s:
                                currentTask = parent->child[1];
                                break;
                            case SDLK_d:
                                currentTask = parent->child[2];
                                break;
                            case SDLK_f:
                                currentTask = parent->child[3];
                                break;
                            }
                            if (!currentTask.lock().get())
                            {
                                currentTask = oldTask;
                            }
                        }
                }
        }
    }
    if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT)
    {
        currentTask.reset();
        parentTask.reset();
    }

}

void Manager::updateEntities()
{
    Map* level = (GameWindow::getLevel());

    ObjectStorage* entities = &(level->getEntities());
    const glm::vec4* selectRect = &(GameWindow::getSelection());
    RawQuadTree* tree = level->getTree();
    AntManager* newTask = nullptr;
    auto it2 = entities->begin();
    auto end = entities->end(); //we do this in case we added an object while looping. This ensures we still iterate as if the object wasn't added.
   // const glm::vec4* selectRect = &(GameWindow::getSelection());
    bool clicked = MouseManager::getJustClicked() == SDL_BUTTON_LEFT;
    int released = MouseManager::getJustReleased();
   // chunk->update();
   // std::cout << mousePos.x << " " << mousePos.y << std::endl;
    Object* newSelected = nullptr;
    int index = 0;
    for (auto i = entities->begin(); i !=  end; i = it2)
    {
        ++it2;
        Object* current = i->first;
        RectPositional* rectPos = &(current->getRect());
      //  std::cout << rectPos->getRect().x << std::endl;
        RawQuadTree* oldTree = tree->find(*rectPos);
        HealthComponent* health = current->getComponent<HealthComponent>();
        if (!current->getDead() && (!health || health->getHealth() > 0))
        {
            InactiveComponent* inactive = current->getComponent<InactiveComponent>();
            if (!inactive || inactive->done())
            {
                current->update();
                tree->update(*rectPos,*oldTree);
                positionalVec vec = tree->getNearest(*(rectPos));
                for (int j = vec.size() - 1; j >= 0; j --)
                {
                    Entity* ptr = &(((RectComponent*)vec[j])->getEntity());
                    if ( ptr != i->first && (!static_cast<Object*>(ptr)->getDead()) && vec[j]->collides(rectPos->getRect()))
                    {
                        i->second->collide(*ptr);
                        ptr->collide(*(i->second.get()));
                    }
                }
                if (current->getFriendly())
                {
                    //std::cout << current << std::endl;
                    GameWindow::getFogMaker().requestPolyFog(rectPos->getCenter(),100,10);
                }
                if (current->getComponent<Ant::AntMoveComponent>())
                {
                    if (released == SDL_BUTTON_LEFT && i->second->getRect().collides(*selectRect))
                    {
                        if (!newTask)
                        {
                            tasks.emplace_back(new TaskNode(*this));
                            auto shared = tasks[tasks.size() - 1];
                            currentTask = shared;
                            parentTask.reset();
                            newTask = &(shared->task);
                        }
                        newTask->addAnt((i->second));
                        //std::cout << newTask->getAnts().size() << " "<< tasks[0]->task.getAnts().size() << std::endl;
                        //selectedUnits.emplace_back(ants[i->first]);
                    }
                }
                else
                {
                    if (current->clicked())
                   {
                       //current->getClickable().click(true);
                       //selectedUnits.emplace_back(entities[current]);
                       newSelected = current;
                   }
                }
            }
            else
            {
                inactive->render();
            }
        }
        else
        {
            current->onDeath();
            level->remove(*(current));
        }
        index ++;
    }
    if (clicked)
    {
        if (!newSelected)
        {
            selectedUnit.reset();
        }
        else
        {
            selectedUnit = entities->at(newSelected);
        }
    }
}

void Manager::split()
{
    auto current = currentTask.lock().get();
    if (current)
    {
        auto arr = current->task.split(AntManager::maxChildren);
        for (int i = 0; i < AntManager::maxChildren; ++i)
        {
           // if (i < pieces)
            {
                current->child[i].reset(new TaskNode(std::move(arr[i])));

            }
        }
    }
}

void Manager::update()
{
    //std::vector<Unit*> selected;

    //std::cout << released << std::endl;



    updateEntities();


    auto curTask = currentTask.lock().get();
    if (curTask)
    {
        curTask->task.getInput();
        //std::cout << currentTask->getAnts().size() << std::endl;
    }
    Map* level = GameWindow::getLevel();
    if (Debug::getSpawnCreatures() && level && level->getAnthill() && (!spawner.isSet() || spawner.timePassed(std::min(180000 - SDL_GetTicks(),(Uint32)1000))))
    {
        spawnCreatures(*level->getAnthill() , level->getRect().z/6, level->getRect().z/6);
        spawner.reset();
        spawner.set();
    }
   /* else if (selectedUnit)
    {
        selectedUnit->getClickable().click(true);
    }*/
    //glm::vec2 disp = {GameWindow::getCamera().getRect().x,GameWindow::getCamera().getRect().y};
    //tree->render(disp);
}

void Manager::reset()
{
    tasks.clear();
}

class AntClickable;

double Camera::minZoom = .1, Camera::maxZoom = 2;
Camera::Camera()
{

}

void Camera::init(int w, int h)
{
    RenderCamera::init(w,h);
    baseDimen = {w,h};

    move = new MoveComponent(1,rect,*this);
    addComponent(*(move));
}

void Camera::update()
{
    if ( (zoomAmount != zoomGoal && zoomGoal != -1))
    {
        if (!move->atTarget())
        {
            move->update();
            rect = move->getRect();
        }
        if (zoomAmount != zoomGoal)
        {
            zoom(absMin(zoomGoal - zoomAmount,convertTo1(zoomGoal - zoomAmount)*zoomSpeed*DeltaTime::deltaTime));
        }
    }
    else
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
        zoomGoal = -1;

        auto mouseWheel = MouseManager::getMouseWheel();
        if (mouseWheel.second > 0)
        {

            zoom(-.1);//,toWorld({mousePos.first, mousePos.second}));
        }
        if (mouseWheel.second < 0)
        {
            zoom(.1);//,toWorld({mousePos.first, mousePos.second}));
        }
        if (KeyManager::getJustPressed() == SDLK_SPACE)
        {
            resetZoom();
        }
        //    GameWindow::requestRect({screenDimen.x/2 - 5, screenDimen.y/2 - 5,10,10},{0,0,0,1},true,0,0,true);

    }
      //  GameWindow::requestNGon(4,{screenDimen.x/2, screenDimen.y/2},10,{0,0,0,1},0,true,0,true ); //renders a small square at the camera's center

}

void Camera::setBounds(const glm::vec4& newBounds)
{
    bounds = newBounds;
}


void Camera::center(const glm::vec2& point)
{
    rect.x = point.x - rect.z/2;
    rect.y = point.y - rect.a/2;
}

void Camera::zoom(float amount)
{
    zoomAmount = std::max(minZoom,std::min(maxZoom,zoomAmount+ amount));
    glm::vec2 zoomDimen = {zoomAmount*baseDimen.x,zoomAmount*baseDimen.y};

    //if (vecContains(glm::vec4((rectCenter - glm::vec2(rect.z/2, rect.a/2)),rect.z,rect.a), bounds))
    rect.x = std::min(std::max(bounds.x,rect.x - (zoomDimen.x - rect.z)/2),bounds.x + bounds.z - zoomDimen.x);
    rect.y = std::min(std::max(bounds.y,rect.y - (zoomDimen.y - rect.a)/2),bounds.y + bounds.a - zoomDimen.y);
    rect.z = zoomDimen.x;
    rect.a = zoomDimen.y;
    RenderProgram::setXRange(0,rect.z);
    RenderProgram::setYRange(0,rect.a);
}

void Camera::zoom(float amount, const glm::vec2& point)
{
    center(point);
    zoom(amount);
}

void Camera::setZoomTarget(double goal)
{
    zoomGoal = (goal);
}

void Camera::setZoomTarget(double goal, double speed )
{
    setZoomTarget(goal);
    zoomSpeed = speed;
}

void Camera::resetZoom()
{
    zoom(1- zoomAmount);
}

bool Camera::isZooming()
{
    return zoomGoal != -1;
}

void Camera::close()
{
    removeComponent<MoveComponent>();
}

Camera::~Camera()
{

}

float GameWindow::menuHeight = 1; //is set in the GameWindow constructor
Camera GameWindow::camera;
Manager GameWindow::manager;
std::weak_ptr<Map> GameWindow::level;
Debug GameWindow::debug;
Window* GameWindow::gameOver = nullptr;
Player GameWindow::player;
FogMaker GameWindow::fogMaker;
float GameWindow::interfaceZ = 3;
float GameWindow::fontZ = GameWindow::interfaceZ + 1;
bool GameWindow::renderAbsolute = false;

GameWindow::GameWindow() : Window({0,0},nullptr,{0,0,0,0})
{
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    menuHeight = .2*screenDimen.y;
    rect.z = screenDimen.x;
    rect.a = screenDimen.y;
    camera.init(screenDimen.x,screenDimen.y);
    player.init();
  /*  auto ptr = evilMoonAssembler.assemble();
    ptr->getComponent<UnitAttackComponent>()->setLongTarget({0,0},&level.getUnit(level.getAnthill()));
    level.addUnit(*ptr,0,0,false);*/

   // level.addUnit(*(new Dummy(levelRect.z/2 - 100,levelRect.a/2)));

   // camera.setZoomTarget(.5);
   // manager.clear();
}

void GameWindow::onSwitch(Window& from)
{
    camera.resetZoom();
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    Map* levelPtr = level.lock().get();
    if (levelPtr)
    {
        glm::vec4 levelRect = levelPtr->getRect();
        camera.setBounds({levelRect.x,levelRect.y,levelRect.z,levelRect.a + menuHeight});
        camera.center({levelRect.z/2,levelRect.a/2});
        gameOver = new Window({screenDimen.x/10, screenDimen.y/10},nullptr, {1,0,0,1});
        gameOver->addPanel(*(new QuitButton(*this)));
        manager.init(levelPtr->getRect());
    }
    debug.init();
   // player.init();
}

void GameWindow::updateTop(float z)
{
   // PolyRender::requestPolygon({{0,0,0},{0,400,0},{100,100,0},{100,400,0}},{0,0,0,1});
    Anthill* hill = static_cast<Anthill*>(anthill.lock().get());
   /* if (!hill)
    {
        gameOver->update(x,y,clicked);
        level.render();
    }
    else */if (KeyManager::getJustPressed() == SDLK_n)
    {
        //level.nextLevel();
        level.lock().get()->setChangeLevel(true);
    }
    else
    {
        renderAbsolute = false;

        camera.update();

        level.lock().get()->render();

        int size = labels.size();
        for (int i = 0; i < size; ++i)
        {
            if (!labels[i]->isDead())
            {
                labels[i]->update();
            }
        }
        player.update();

      //  GameWindow::requestNGon(10,camera.toWorld(pairtoVec(MouseManager::getMousePos())),30,{0,0,0,0},0,true,3);

       // requestRect(camera.getRect(),{0,0,0,.5},true,0,2,0);

        debug.update();
        manager.updateAntManagers();

        //everything rendered before this is rendered as if it were under fog (unless it has a z higher than fog obviously)
      /* fogMaker.renderFog(); //anything rendered after this is only shown if it were in a sight bubble.

        glDisable(GL_DEPTH_TEST);
        glStencilFunc(GL_NOTEQUAL,1,0xFF);
        glStencilMask(0x00);*/

        manager.update();
        PolyRender::render();
        SpriteManager::render();

      /* glDisable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS,1,0xFF);
        glStencilMask(0xFF);
        glEnable(GL_DEPTH_TEST); //anything rendered after this will be discarded if they are under the fog.*/


        renderAbsolute = true;
    // std::cout << camera.getRect().x << " " << camera.getRect().x + camera.getRect().z << std::endl;
       // camera.reserveZoom();
        renderTopBar();
        renderSelectedUnits();

        if (level.lock().get()->getChangeLevel())
        {
            if (switchToMap)
            {
                switchToMap->press();
            }
        }

        //camera.goBack();
        //requestRect({0,0,320,320},{1,0,1,1},true,0,1,1);

    }
    //printRect(getCamera().getRect());
    //std::cout << ComponentContainer<RenderComponent>::components.size() << std::endl;
}

void GameWindow::renderTopBar()
{
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    glm::vec4 menuRect = camera.toAbsolute({0,0,screenDimen.x,menuHeight});
    //PolyRender::requestRect(menuRect,{1,0,0,1},true,0,interfaceZ);
    Font::tnr.requestWrite({"Resources: " + convert(player.getResource()),camera.toAbsolute({screenDimen.x - .2*screenDimen.x, .01*screenDimen.y
                                                                                    , -1,.6}),0,{1,1,1,1},GameWindow::fontZ});
    Font::tnr.requestWrite({"Shards: " + convert(level.lock().get()->getFoundShards()),camera.toAbsolute({screenDimen.x - .2*screenDimen.x, .03*screenDimen.y
                                                                                    , -1,.6}),0,{1,1,1,1},GameWindow::fontZ});
    Font::tnr.requestWrite({"Gold: " + convert(player.getGold()),camera.toAbsolute({screenDimen.x - .2*screenDimen.x, .05*screenDimen.y
                                                                                    , -1,.6}),0,{1,1,1,1},GameWindow::fontZ});

}

void GameWindow::renderSelectedUnits()
{
    const glm::vec4* cameraRect = &(camera.getRect());
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    glm::vec4 wholeRect = {0,rect.a - menuHeight,rect.z, menuHeight};
   // printRect(wholeRect);
    player.render({wholeRect.x + wholeRect.z - 1.1*wholeRect.a, wholeRect.y - wholeRect.a, wholeRect.a,wholeRect.a});
    glm::vec4 selectedAntsRect = {0,wholeRect.y, .7*wholeRect.z, wholeRect.a};
    glm::vec4 selectedUnitRect = {selectedAntsRect.x + selectedAntsRect.z, wholeRect.y, wholeRect.z - selectedAntsRect.z, wholeRect.a};
    glm::vec2 margin = {.05*wholeRect.z,.2*menuHeight}; //the horizontal and vertical margins to render the selected ants and selected unit
    int antsPerRow = 10;
    float antRectWidth = .03*rect.z; //width and height of each of the outlineRects. This is a bit of a magic number and was chosen just because it looks good
    glm::vec2 spacing = {(selectedAntsRect.z - margin.x*2 - antsPerRow*antRectWidth)/antsPerRow,.3*menuHeight}; //horizontal and vertical spacing between ants and unit and health bar

    GameWindow::requestLine({selectedUnitRect.x*cameraRect->z/screenDimen.x,
                            selectedUnitRect.y*cameraRect->a/screenDimen.y,
                            selectedUnitRect.x*cameraRect->z/screenDimen.x,
                             (selectedUnitRect.y + selectedUnitRect.a)*cameraRect->a/screenDimen.y},{0,0,0,1},interfaceZ,true);

    const AntManager* antManager = manager.getCurrentTask();
    Object* selectedUnit = manager.getSelectedUnit().lock().get();
    if (selectedUnit)
    {
        glm::vec4 selectedRect = {selectedUnitRect.x + margin.x, selectedUnitRect.y + margin.y ,selectedUnitRect.a - margin.y*2, selectedUnitRect.a - margin.y*2};
        selectedUnit->getRender().render({camera.toAbsolute(selectedRect),0,NONE,{1,1,1,1},&RenderProgram::basicProgram,fontZ});
        //printRect(camera.toAbsolute(selectedRect));

       // GameWindow::requestRect(selectedRect,{0,0,0,1},false,0,interfaceZ);
        HealthComponent* health = selectedUnit->getComponent<HealthComponent>();
        if (health)
        {
            health->render({selectedRect.x + selectedRect.z + spacing.x, selectedRect.y , selectedRect.z}, interfaceZ);
        }
        ResourceComponent* resource = selectedUnit->getComponent<ResourceComponent>();
        if (resource)
        {
            resource->render({selectedRect.x + selectedRect.z + spacing.x, selectedRect.y + spacing.y, selectedRect.z},interfaceZ);
        }
        selectedUnit->getClickable().click(true);
        selectedUnit->getClickable().display(selectedAntsRect);
    }
    else if (antManager)
    {
        auto selectedUnits = &(antManager->getAnts());
        int size = selectedUnits->size();
        for (int i = 0; i < size; ++i)
        {
            Entity* current = selectedUnits->at(i).lock().get();
            if (current)
            {
                glm::vec4 outlineRect = { selectedAntsRect.x + margin.x + i%antsPerRow*(spacing.x + antRectWidth),selectedAntsRect.y + margin.y + i/antsPerRow*spacing.y,antRectWidth,antRectWidth};
                HealthComponent* health = current->getComponent<HealthComponent>();
                double healthRatio = health->getHealth()/health->getMaxHealth();//how much health this ant currently has;
                current->getComponent<RenderComponent>()->render({camera.toAbsolute({outlineRect.x + 2, outlineRect.y + 2, outlineRect.z - 4, outlineRect.a - 4})
                                                                 ,0,NONE,{1-healthRatio,0,0,1},&RenderProgram::basicProgram,fontZ});
                GameWindow::requestRect(outlineRect,{1,0,1,1},false,0,interfaceZ,true);
            }
        }
    }
    GameWindow::requestRect(wholeRect,{0,1,0,1},true,0,interfaceZ,true);
}

float GameWindow::getMenuHeight()
{
    return menuHeight;
}

const glm::vec4& GameWindow::getSelection()
{
    return player.getSelection();
}
Camera& GameWindow::getCamera()
{
    return camera;
}
const Manager& GameWindow::getManager()
{
    return manager;
}
Map* GameWindow::getLevel()
{
    return level.lock().get();
}

void GameWindow::setLevel(std::shared_ptr<Map>& map)
{
    level = map;
}

Player& GameWindow::getPlayer()
{
    return player;
}

FogMaker& GameWindow::getFogMaker()
{
    return fogMaker;
}

void GameWindow::setWorldMap(WindowSwitchButton& butt)
{
    switchToMap = &butt;
}

void GameWindow::requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z, bool absolute)
{
    bool oldAbs = renderAbsolute;
    renderAbsolute = renderAbsolute || absolute;
    glm::vec2 c = camera.toScreen(center);
    const glm::vec4* cameraRect = &(camera.getRect());
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    if (renderAbsolute)
    {
        c = {(center.x )/screenDimen.x*cameraRect->z, (center.y)/screenDimen.y*cameraRect->a};
        side = side/screenDimen.x*cameraRect->z;
    }
    PolyRender::requestNGon(n,c ,side,color,angle,filled,z);
    if (absolute)
    {
        renderAbsolute = oldAbs;
    }
}

void GameWindow::requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z, bool absolute)
{
    bool oldAbs = renderAbsolute;
    renderAbsolute = renderAbsolute || absolute;
    glm::vec4 renderRect = rect;
    if (renderAbsolute)
    {

        renderRect = camera.toAbsolute(renderRect);
    }
    else
    {
        renderRect = camera.toScreen(rect);
    }
    PolyRender::requestRect(renderRect,color,filled,angle,z);
    if (absolute)
    {
        renderAbsolute = oldAbs;
    }
}

void GameWindow::requestLine(const glm::vec4& line, const glm::vec4& color, float z, bool absolute)
{
    glm::vec4 lineCopy = line;
    if (absolute)
    {
        lineCopy = glm::vec4(
                            getCamera().toAbsolute(glm::vec2(line.x,line.y)),
                            getCamera().toAbsolute(glm::vec2(line.z,line.a))
                             );
    }
    else
    {
        lineCopy =      glm::vec4(
                        getCamera().toScreen(glm::vec2(line.x,line.y)),
                        getCamera().toScreen(glm::vec2(line.z,line.a))
                         );
    }
    PolyRender::requestLine(
                             lineCopy,
                            color,
                            z);
}

void GameWindow::close()
{
    camera.close();
    level.reset();
}

GameWindow::QuitButton::QuitButton(GameWindow& window_) : Button({10,50,32,32},nullptr,nullptr, {"Quit"},&Font::tnr,{0,1,0,1}), window(&window_)
{

}
void GameWindow::QuitButton::press()
{
    window->quit = true;
}
