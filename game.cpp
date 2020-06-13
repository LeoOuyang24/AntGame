#include "SDLHelper.h"
#include "vanilla.h"

#include "game.h"
#include "ants.h"
#include "sequence.h"

SpriteWrapper frame;

Manager::TaskNode::TaskNode(Manager& t) : task(t, GameWindow::selectColor)
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
    Unit* toSpawn = nullptr;
    switch(random)
    {
    case 0:
        toSpawn = new Bug(0,0);
        break;
    case 1:
        toSpawn = new Bug(0,0);
        break;
    case 2:
    case 3:
    case 4:
        toSpawn = new Bug(0,0);
        break;
    default:
        toSpawn = new Bug(0,0);
    }
    return toSpawn;
}

void Manager::spawnCreatures()
{

    Unit* toSpawn = generateCreature();
    Map* level = &(GameWindow::getLevel());
    const glm::vec4* mapSize = &(level->getRect(level->getCurrentChunk()));
    const glm::vec4* camera = &(GameWindow::getCamera().getRect());
    const glm::vec4* entityRect = &(toSpawn->getRect().getRect());
    int x = rand()%((int)(mapSize->z -  entityRect->z)) + mapSize->x; //we want to make sure our object spawns outside of the camera's view and doesn't spawn partially out of the map
    bool cameraInTheWay = (x >= camera->x && x <= camera->x + camera->z); //if we chose an x coordinate that may overlap with the camera's rect, we need to adjust our y coordinate
    int y = rand() % ((int)(mapSize->a - entityRect->a - camera->a*cameraInTheWay)) + mapSize->y;
    //modify coordinates so our object doesn't spawn in the player's view
    y += camera->a*(y > camera->y)*cameraInTheWay;
    toSpawn->getRect().setPos({x,y});
    level->addUnit(*toSpawn);

}

void Manager::spawnCreatures(Anthill& hill, double minR, double maxR) //spawn creatures near an anthill at a certain radius
{
    const glm::vec4* rect = &(hill.getRect().getRect());
    double diag = rect->z*sqrt(2); //largest distance from the center that intersects with the anthill
    minR = std::max(diag,minR);
    maxR = std::max(minR, maxR);
    double r = fmod(rand(),(maxR - minR)) + minR;
    double theta = rand()%360*M_PI/180;
    Unit* toSpawn = generateCreature();
    glm::vec2 point = {rect->x + rect->z/2 + cos(theta)*r,rect->y + rect->a/2 + sin(theta)*r};
    GameWindow::getLevel().moveObject(*toSpawn,point.x,point.y);
    SeigeComponent* seige = new SeigeComponent(*toSpawn,hill);
    toSpawn->addComponent(*seige);
    GameWindow::getLevel().addUnit(*toSpawn);

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



/*void Manager::updateAnts()
{
    int released = MouseManager::getJustReleased();
    if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT)
    {
       // antManager.clear();
       currentTask.reset();
        //selectedUnit = nullptr;
    }
    int index = 0;
    Map* level = &(GameWindow::getLevel());
    AntStorage* ants = &(level->getUnit(level->getCurrentChunk()));
    auto it = ants->begin();
    AntManager* newTask = nullptr; //if we select any ants, this pointer will point to the new task
    const glm::vec4* selectRect = &(GameWindow::getSelection());
    RawQuadTree* tree = level->getTree(level->getCurrentChunk());
    for (auto i = ants->begin(); i != ants->end(); i = it)
    {
        ++it;
        if (i->second->getComponent<HealthComponent>()->getHealth() > 0)
        {
            RectPositional* rectPos = &(i->second->getRect());
            const glm::vec4* rect = &(rectPos->getRect());
            RawQuadTree* oldTree = tree->find(*rectPos);
            i->second->update();
            tree->update(*rectPos,*oldTree);


        }
        else
        {
            i->second->onDeath();
            level->remove(static_cast<Unit&>(*((i)->first)));
        }
        index ++;
    }
}*/

void Manager::updateEntities()
{
    Map* level = &(GameWindow::getLevel());
    ObjectStorage* entities = &(level->getEntities(level->getCurrentChunk()));
    const glm::vec4* selectRect = &(GameWindow::getSelection());
    RawQuadTree* tree = level->getTree(level->getCurrentChunk());
    AntManager* newTask = nullptr;
    auto it2 = entities->begin();
    auto end = entities->end(); //we do this in case we added an object while looping. This ensures we still iterate as if the object wasn't added.
   // const glm::vec4* selectRect = &(GameWindow::getSelection());
    bool clicked = MouseManager::getJustClicked() == SDL_BUTTON_LEFT;
    int released = MouseManager::getJustReleased();
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
        if (!health || (health->getHealth() > 0 && !current->getDead()))
        {
            current->update();
            tree->update(*rectPos,*oldTree);
            positionalVec vec = tree->getNearest(*(rectPos));
            for (int j = vec.size() - 1; j >= 0; j --)
            {
                Entity* ptr = &(((RectComponent*)vec[j])->getEntity());
                if ( ptr != i->first && vec[j]->collides(rectPos->getRect()))
                {
                    i->second->collide(*ptr);
                    ptr->collide(*(i->second.get()));
                }
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
                    newTask->addAnt(std::dynamic_pointer_cast<Ant>(i->second));
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

    updateAntManagers();

    auto curTask = currentTask.lock().get();
    if (curTask)
    {
        curTask->task.getInput();
        //std::cout << currentTask->getAnts().size() << std::endl;
    }
    if (signalling && spawner.framesPassed(100))
    {
        Map* map = &(GameWindow::getLevel());
        spawnCreatures(*signalling, 500, map->getRect(map->getCurrentChunk()).z);
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

void Manager::setSignalling(Anthill& hill)
{
    signalling = &hill;
    spawner.set();
}

class AntClickable;


double Camera::minZoom = .1, Camera::maxZoom = 2;
Camera::Camera()
{

}

void Camera::init(int w, int h)
{
    baseDimen = {w,h};
    rect = {0,0,w,h};

    move = new MoveComponent(.1,rect,*this);
    addComponent(*(move));
}

void Camera::update()
{
    if (!move->atTarget() || (zoomAmount != zoomGoal && zoomGoal != -1))
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
        zoomGoal = -1;
        std::pair<int,int> mousePos = MouseManager::getMousePos();
        glm::vec2 screenDimen = RenderProgram::getScreenDimen();
        if (mousePos.first >= screenDimen.x - 1 || mousePos.first <= 0)
        {
            rect.x += absMin((mousePos.first - 1), (mousePos.first - screenDimen.x +2 ))*DeltaTime::deltaTime;
        }
        if (mousePos.second >= screenDimen.y - 1 || mousePos.second <= 0)
        {
            rect.y +=  absMin((mousePos.second - 1), (mousePos.second - screenDimen.y + 2 ))*DeltaTime::deltaTime;
        }
      /*  if (bounds)
        {
            rect.x = std::max(bounds->x,std::min(bounds->x + bounds->z - rect.z,rect.x));
            rect.y = std::max(bounds->y, std::min(bounds->y + bounds->a + GameWindow::getMenuHeight() - rect.a , rect.y));
        }*/
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

const glm::vec4& Camera::getRect() const
{
    return rect;
}
void Camera::setBounds(const glm::vec4* newBounds)
{
    bounds = newBounds;
}

glm::vec4 Camera::toScreen(const glm::vec4& change) const
{
    glm::vec2 point = toScreen({change.x,change.y});
    return {point.x,point.y,change.z, change.a};
}

glm::vec2 Camera::toScreen(const glm::vec2& point) const
{

    return {(point.x - rect.x), (point.y - rect.y)};
}

glm::vec4 Camera::toWorld(const glm::vec4& change) const
{

    glm::vec2 point = toWorld({change.x,change.y});
    return {point.x,point.y , change.z, change.a};
}

glm::vec2 Camera::toWorld(const glm::vec2& point) const
{
     glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    return {(point.x/screenDimen.x*rect.z + rect.x), (point.y/screenDimen.y*rect.a + rect.y)};
}

glm::vec4 Camera::toAbsolute(const glm::vec4& rect) const
{
    glm::vec2 dimen = toAbsolute({rect.z,rect.a});
    glm::vec2 converted = toAbsolute({rect.x,rect.y});
    return {converted.x,converted.y, dimen.x,dimen.y};
}
glm::vec2 Camera::toAbsolute(const glm::vec2& point) const
{
     glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    return {point.x*this->rect.z/screenDimen.x,point.y*this->rect.a/screenDimen.y};
}

void Camera::center(const glm::vec2& point)
{
    rect.x = point.x - rect.z/2;
    rect.y = point.y - rect.a/2;
}

void Camera::zoom(float amount)
{
    zoomAmount += amount ;//std::max(minZoom,std::min(maxZoom,zoomAmount+ amount));
    glm::vec2 rectCenter = {rect.x + rect.z/2, rect.y + rect.a/2};
    rect.z = zoomAmount*baseDimen.x;
    rect.a = zoomAmount*baseDimen.y;
    center(rectCenter);
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

float GameWindow::menuHeight = 1;
glm::vec2 GameWindow::origin = {0,0};
glm::vec4 GameWindow::selection = {0,0,0,0};
const glm::vec4 GameWindow::selectColor = {1,1,1,.5};
Camera GameWindow::camera;
Manager GameWindow::manager;
Map GameWindow::level;
Window* GameWindow::gameOver = nullptr;
float GameWindow::interfaceZ = .3;
bool GameWindow::renderAbsolute = false;

GameWindow::GameWindow() : Window({0,0},nullptr,{0,0,0,0})
{
    level.init({-1000,-1000,2000,2000});
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    menuHeight = .1*screenDimen.y;
    rect.z = screenDimen.x;
    rect.a = screenDimen.y;
    camera.init(screenDimen.x,screenDimen.y);
    gameOver = new Window({screenDimen.x/10, screenDimen.y/10},nullptr, {1,0,0,1});
    gameOver->addButton(*(new QuitButton(*this)));
    manager.init(level.getRect(level.getCurrentChunk()));
    Anthill* hill = (new Anthill({320,320}));
    anthill = level.addUnit(*hill);
    hill->setManager(manager);
    //level.remove(*hill);
   // level.addUnit(*(new Beetle(320,320)));

    auto antPtr = level.addUnit(*(new Ant({380,380,10,10},*hill)));
 //   hill->getComponent<ResourceComponent>()->setResource(-1000);
    auto mushPtr = level.addUnit(*(new Mushroom(480,480)));
    std::shared_ptr<Label> antLabel = std::shared_ptr<Label>(new Label({400,300,128,32},"Click on the ant",{1,1,1,1},{385,385},
                                  *(new LambdaTrigger([](){return !GameWindow::getCamera().isZooming();})),
                                  *(new ObjectTrigger([](Object* obj){
                                return obj && obj->clicked();},antPtr)),1));

    auto mushLabel = std::shared_ptr<Label>(new Label({500,400,128,32}, "Right click on the mushroom", {1,1,1,1},{485,485},
                                                        *(new ChainTrigger(antLabel)),
                                                        *(new ObjectTrigger([](Object* obj){
                                                            auto task = GameWindow::getManager().getCurrentTask();
                                                        return obj &&  task && task->getTargetUnit() == obj;},mushPtr)),1));
    auto duringMush = std::shared_ptr<Label>(new Label({500,400,250,32},"Wait for the ant to collect the mushroom", {1,1,1,1},{485,485},
                                                       *(new ChainTrigger(mushLabel)),
                                                       *(new ObjectTrigger([](Object* obj){
                                                            return obj == nullptr;},mushPtr))));
    auto firstEnemy = std::shared_ptr<EZSequenceUnit>(new EZSequenceUnit(nullptr,
                                                                         {[](){MoveComponent* comp = GameWindow::getCamera().getComponent<MoveComponent>();
                                                                         if (comp)
                                                                         {
                                                                             comp->setTarget({800,800});
                                                                             comp->setSpeed(1);
                                                                         }   }},
                                                                         {[](){
                                                                         GameWindow::getLevel().addUnit(*(new Bug(810,810)));
                                                                         }},
                                                                         *(new ChainTrigger(duringMush,5000)),
                                                                         *(new LambdaTrigger([](){MoveComponent* comp = GameWindow::getCamera().getComponent<MoveComponent>();
                                                                             return comp && comp->atTarget();
                                                                          }))
                                                                         ));
    auto zoomOut = std::shared_ptr<Label>(new Label
                                           ({750,750,100,32},"Right click to kill the bug", {1,1,1,1},{842, 842},
                                                     *(new ChainTrigger(firstEnemy,0)),
                                                      *(new LambdaTrigger([](){return false;}))));
    /*labels.push_back(antLabel);
    labels.push_back(mushLabel);
    labels.push_back(firstEnemy);
    labels.push_back(duringMush);
    labels.push_back(zoomOut);*/
    //camera.getComponent<MoveComponent>()->setTarget({-100,-100});
    camera.center({480,480});
   // camera.setZoomTarget(.5);
   // manager.clear();
}

bool GameWindow::updateSelect()
{
    if (MouseManager::isPressed(SDL_BUTTON_LEFT))
    {
        std::pair<int,int> mouse_Pos = MouseManager::getMousePos();
        glm::vec2 mousePos = camera.toWorld({mouse_Pos.first, mouse_Pos.second});
      //  std::cout << mousePos.x << " " << mousePos.y << std::endl;
        if (MouseManager::getJustClicked() != SDL_BUTTON_LEFT)
        {

            double width = mousePos.x - origin.x;
            double height = mousePos.y - origin.y;
            selection = {origin.x,origin.y, width, height};
        }
        else
        {
            origin.x = mousePos.x;
            origin.y = mousePos.y;
        }
        return true;
    }
    return false;
}

void GameWindow::update(int x, int y, bool clicked)
{
   // PolyRender::requestPolygon({{0,0,0},{0,400,0},{100,100,0},{100,400,0}},{0,0,0,1});
    Anthill* hill = static_cast<Anthill*>(anthill.lock().get());
    if (!hill)
    {
        gameOver->update(x,y,clicked);
        level.render();
    }
    else
    {
        renderAbsolute = false;

        bool select = updateSelect();


        camera.update();
        level.render();
        if (select)
        {
            requestRect(selection,selectColor,true,0,-.1);
        }

        manager.update();

        int size = labels.size();
        for (int i = 0; i < size; ++i)
        {
            if (!labels[i]->isDead())
            {
                labels[i]->update();
            }
        }

        renderAbsolute = true;
    // std::cout << camera.getRect().x << " " << camera.getRect().x + camera.getRect().z << std::endl;
        renderSelectedUnits();


    }
    //std::cout << ComponentContainer<RenderComponent>::components.size() << std::endl;
}

void GameWindow::renderSelectedUnits()
{
    const glm::vec4* cameraRect = &(camera.getRect());
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    glm::vec4 wholeRect = {0,rect.a - menuHeight,rect.z, menuHeight};
    glm::vec4 selectedAntsRect = {0,wholeRect.y, .7*wholeRect.z, wholeRect.a};
    glm::vec4 selectedUnitRect = {selectedAntsRect.x + selectedAntsRect.z, wholeRect.y, wholeRect.z - selectedAntsRect.z, wholeRect.a};
    glm::vec2 margin = {.05*wholeRect.z,.2*menuHeight}; //the horizontal and vertical margins to render the selected ants and selected unit
    int antsPerRow = 10;
    float antRectWidth = .03*rect.z; //width and height of each of the outlineRects. This is a bit of a magic number and was chosen just because it looks good
    glm::vec2 spacing = {(selectedAntsRect.z - margin.x*2 - antsPerRow*antRectWidth)/antsPerRow,.3*menuHeight}; //horizontal and vertical spacing between ants and unit and health bar

    PolyRender::requestLine({selectedUnitRect.x*cameraRect->z/screenDimen.x,selectedUnitRect.y*cameraRect->a/screenDimen.y, selectedUnitRect.x*cameraRect->z/screenDimen.x,
                             (selectedUnitRect.y + selectedUnitRect.a)*cameraRect->a/screenDimen.y},{0,0,0,1},interfaceZ);

    const AntManager* antManager = manager.getCurrentTask();
    if (antManager)
    {
        const std::vector<std::weak_ptr<Ant>>* selectedUnits = &(antManager->getAnts());
        int size = selectedUnits->size();
        for (int i = 0; i < size; ++i)
        {
            Ant* current = selectedUnits->at(i).lock().get();
            glm::vec4 outlineRect = { selectedAntsRect.x + margin.x + i%antsPerRow*(spacing.x + antRectWidth),selectedAntsRect.y + margin.y + i/antsPerRow*spacing.y,antRectWidth,antRectWidth};
            HealthComponent* health = current->getComponent<HealthComponent>();
            double healthRatio = health->getHealth()/health->getMaxHealth();//how much health this ant currently has;
            current->getComponent<RenderComponent>()->render({{outlineRect.x + 2, outlineRect.y + 2, outlineRect.z - 4, outlineRect.a - 4}
                                                             ,0,NONE,{1-healthRatio,0,0,1},&RenderProgram::basicProgram,interfaceZ});
            GameWindow::requestRect(outlineRect,{1,0,1,1},false,0,interfaceZ,true);
        }
    }
    Object* selectedUnit = manager.getSelectedUnit().lock().get();
    if (selectedUnit)
    {
        glm::vec4 selectedRect = {selectedUnitRect.x + margin.x, selectedUnitRect.y + margin.y ,selectedUnitRect.a - margin.y*2, selectedUnitRect.a - margin.y*2};
        selectedUnit->getRender().render({selectedRect,0,NONE,{1,1,1,1},&RenderProgram::basicProgram,interfaceZ});
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
    }
        GameWindow::requestRect(wholeRect,{0,1,0,1},true,0,interfaceZ,true);
}

float GameWindow::getMenuHeight()
{
    return menuHeight;
}

const glm::vec4& GameWindow::getSelection()
{
    return selection;
}
Camera& GameWindow::getCamera()
{
    return camera;
}
const Manager& GameWindow::getManager()
{
    return manager;
}
Map& GameWindow::getLevel()
{
    return level;
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
