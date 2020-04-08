#include "SDLHelper.h"
#include "vanilla.h"

#include "game.h"
#include "ants.h"
#include "sequence.h"

SpriteWrapper frame;



const int AntManager::spacing = 2;

AntManager::AntManager(Manager& newManager) : manager(&newManager)
{

}
AntManager::~AntManager()
{
    clear();
}
const glm::vec2& AntManager::getCenter()
{
    return antsCenter;
}
std::vector<std::weak_ptr<Ant>>& AntManager::getAnts()
{
    return selected;
}
void AntManager::clear()
{
    selected.clear();
    targetUnit.reset();
}
const Object* AntManager::getTargetUnit()
{
    return targetUnit.lock().get();
}

void AntManager::getInput()
{
    int justClicked = MouseManager::getJustClicked();
    std::pair<int,int> mousePos = MouseManager::getMousePos();
    std::vector<Positional*> nearest;
    glm::vec4 mouseClick = {GameWindow::getCamera().toWorld({mousePos.first,mousePos.second}),1,1};
    //RectPositional post(mouseClick);
    //tree.getNearest(nearest,post);
    Map* map = &(GameWindow::getLevel());
    map->getTree(map->getCurrentChunk())->getNearest(nearest,mouseClick);
    int chosen = selected.size();
    std::shared_ptr<Object>* newTarget = nullptr;
    if (justClicked == SDL_BUTTON_RIGHT)
    {
        int nearSize = nearest.size();
        if (nearSize > 0)
        {
            for (int i = 0; i < nearSize; ++i)
            {
                RectComponent* ptr = static_cast<RectComponent*>(nearest[i]);
                if (ptr->getEntity().getComponent<Ant::AntMoveComponent>() == nullptr && ptr->collides(mouseClick))
                    {
                        newTarget = &(map->getUnit((Object*)(&(ptr->getEntity()))));
                        break;
                    }
            }
        }
        if (newTarget)
        {
            targetUnit = *newTarget;
        }
        else
        {
            targetUnit.reset();
        }
        if (targetUnit.expired())
        {
             clumpDimen = {sqrt(chosen),sqrt(chosen)};
             space = {spacing,spacing};
            targetPoint = {mouseClick.x - clumpDimen.x*(Ant::dimen + space.x)/2 + Ant::dimen/2, mouseClick.y - clumpDimen.y*(Ant::dimen + space.y)/2 + Ant::dimen/2};
            currentTask = MOVE;
        }
    }
    for (int i = 0; i < chosen; ++i)
    {
        Ant* current = selected[i].lock().get();
        if (current)
        {
            current->getComponent<ClickableComponent>()->click(true);
        }
        else
        {
            selected.erase(selected.begin() + i);
            i--;
        }
    }
}

void AntManager::updateAnts()
{
    Object* unitPtr = targetUnit.lock().get(); //if we have a target
    const glm::vec4* targetRect  = nullptr;
    HealthComponent* health = nullptr;
    InteractionComponent* interact = nullptr;
    ResourceComponent* resource = nullptr;
    int chosen = selected.size();
    Map* map = &(GameWindow::getLevel());
    if (unitPtr) //if we clicked on a unit
    {
        targetRect = &(unitPtr->getRect().getRect());
        targetPoint = {targetRect->x + Ant::dimen/2, targetRect->y + Ant::dimen/2}; //the starting point at which we can place an ant
        clumpDimen.y = sqrt(chosen/(targetRect->z/targetRect->a)); //#of ants per dimension
        clumpDimen.x = chosen/clumpDimen.y;
        space = {std::min(spacing, (int)((targetRect->z - clumpDimen.x*Ant::dimen)/clumpDimen.x)),
                std::min(spacing, (int)((targetRect->a - clumpDimen.y*Ant::dimen)/clumpDimen.y))};
        health = unitPtr->getComponent<HealthComponent>();
        resource = unitPtr->getComponent<ResourceComponent>();
        interact = unitPtr->getComponent<InteractionComponent>();
        if (resource )
        {
            currentTask = COLLECT;
        }
        else if (health && health->getHealth() > 0)
        {
            currentTask = ATTACK;
        }
        else if (interact)
        {
            currentTask = INTERACT;
        }
        else
        {
            currentTask = MOVE;
        }
    }
    else if (currentTask != MOVE && currentTask != COLLECT && MouseManager::getJustClicked() != SDL_BUTTON_RIGHT) //if the current task is move or collect, we set it to IDLE after processing all ants.
    {
        currentTask = IDLE;
    }
    bool atTarget = true; //used if the current Task is Move. Used to keep track of whether or not there are still units moving.
    bool collected = true; //used if the current Task is COLLECT. Used to keep track of whether or not there are still units collecting.
    if (currentTask != IDLE)
    {
            antsCenter = {0,0};

        for (int i = 0; i < chosen; ++ i)
        {
            Ant* current = selected[i].lock().get();

            if (!current)
            {
                selected.erase(selected.begin() + i);
                i--;
                chosen--;
            }
            else
            {
                glm::vec4 currentRect = current->getRect().getRect();
                antsCenter.x += currentRect.x;
                antsCenter.y += currentRect.y;
                if (current->getCarrying() == 0)
                {
                    if (targetRect && vecIntersect(currentRect,*targetRect))
                    {
                        if (currentTask == ATTACK)
                        {
                            current->getComponent<AttackComponent>()->attack(health);
                        }
                        else if (currentTask == COLLECT)
                        {
                            if (resource)
                            {
                                resource->collect(*current);
                                 collected = false;
                            }
                        }
                        else if (currentTask == INTERACT)
                        {
                            interact->interact(*current);
                            currentTask = IDLE;
                            targetUnit.reset();
                            break;
                        }
                    }
                    else if (clumpDimen.x != 0)
                    {
                        glm::vec2 moveTo = {targetPoint.x + i%((int)clumpDimen.x)*(Ant::dimen + space.x), i/((int)(clumpDimen.x))};
                        if (unitPtr)
                        {
                            //std::cout << currentTask << std::endl;
                            moveTo.y = fmod(moveTo.y,clumpDimen.y);
                        }
                        moveTo.y *= (Ant::dimen + space.y);
                        moveTo.y += targetPoint.y;
                        if (current->getCenter() != moveTo)
                        {
                            //atTarget = false;
                            if (unitPtr)
                            {
                                current->setTarget(moveTo,&(map->getUnit(unitPtr)));
                            }
                            else
                            {
                                current->setTarget(moveTo,nullptr);
                            }
                        }
                    }
                    if (currentTask == MOVE && atTarget && !current->getComponent<MoveComponent>()->atTarget())
                    {
                        atTarget = false;
                    }
                }
                else
                {
                    current->setTarget(map->getUnit(current->getComponent<Ant::AntMoveComponent>()->getHome()));
                    collected = false;
                }
            }
        }
        if (currentTask == MOVE && atTarget)
        {
            currentTask = IDLE;
        }
        if (currentTask == COLLECT && collected && !unitPtr)
        {
            currentTask = IDLE;
        }
        antsCenter/=chosen;
    }
}

void AntManager::addAnt(std::shared_ptr<Ant>& ant)
{
    AntManager* oldTask = ant->getCurrentTask();
    if (oldTask)
    {
        oldTask->remove(*(ant.get()));
    }
    selected.emplace_back(ant);
    ant->setCurrentTask(*this);

}

void AntManager::remove(Unit& unit)
{
    if (targetUnit.lock().get() == &unit)
    {
        targetUnit.reset();
        return;
    }
    if (unit.getComponent<Ant::AntMoveComponent>() != nullptr) //if the unit is an ant
    {
        int size = selected.size();
        for (int i = 0; i < size; ++i)
        {
            if (selected[i].lock().get() == &unit)
            {
                selected.erase(selected.begin() + i);
                break;
            }
        }
    }
}

void AntManager::render(const glm::vec4& rect, int i)
{
    glm::vec4 taskColor = {currentTask == ATTACK || currentTask == IDLE,std::max((double)(currentTask == MOVE || currentTask == IDLE),.5*(currentTask == COLLECT)),currentTask == COLLECT || currentTask == IDLE,1};
    GameWindow::requestRect(rect,taskColor,true,0,GameWindow::interfaceZ, true);
    Font::alef.write(Font::wordProgram,{convert(i),GameWindow::getCamera().toAbsolute(rect),0,{0,0,0},GameWindow::interfaceZ});
}

Manager::Manager()
{

}
ObjPtr Manager::getSelectedUnit()
{
    return selectedUnit;
}
AntManager* Manager::getCurrentTask()
{
    return currentTask;
}

void Manager::init(const glm::vec4& region)
{
   // tree.reset(new RawQuadTree(region));
    tasks.reserve(maxTasks);
}

void Manager::spawnCreatures()
{
    int random = rand()%5;
    Unit* toSpawn = nullptr;
    switch(random)
    {
    case 0:
        toSpawn = new Bug(0,0);
        break;
    case 1:
        toSpawn = new Beetle(0,0);
        break;
    case 2:
    case 3:
    case 4:
        toSpawn = new Mushroom(0,0);
        break;
    default:
        toSpawn = new Bug(0,0);
    }

    Map* level = &(GameWindow::getLevel());
    const glm::vec4* mapSize = &(level->getRect(level->getCurrentChunk()));
    const glm::vec4* camera = &(GameWindow::getCamera().getRect());
    const glm::vec4* entityRect = &(toSpawn->getRect().getRect());
    int x = rand()%((int)(mapSize->z -  entityRect->z)) + mapSize->x; //we want to make sure our object spawns outside of the camera's view and doesn't spawn partially out of the map
    bool cameraInTheWay = (x >= camera->x && x <= camera->x + camera->z); //if we chose an x coordinate that may overlap with the camera's rect, we need to adjust our y coordinate
    int y = rand() % ((int)(mapSize->a - entityRect->a - camera->a*cameraInTheWay)) + mapSize->y;
    //modify coordinates so our object doesn't spawn in the player's view
    y += camera->a*(y > camera->y)*cameraInTheWay;
    toSpawn->getRect().setRect({x,y,entityRect->z,entityRect->a});
    level->addUnit(*toSpawn);

}


void Manager::updateAntManagers()
{
    int size = tasks.size();
    for (int i = size - 1; i >= 0; --i)
    {
        if (tasks[i]->getAnts().size() != 0)
        {
            tasks[i]->updateAnts();
            tasks[i]->render({10,i*25,30,20},i+1);
        }
        else
        {
            if (currentTask == tasks[i].get())
            {
                currentTask = nullptr;
            }
            tasks.erase(tasks.begin() + i);
        }
    }
    int pressed = KeyManager::getJustPressed();
    if (pressed != -1)
    {
        int index = (pressed - SDLK_1) - floor((pressed-SDLK_1)/static_cast<float>(maxTasks))*maxTasks; //this is is the same as (pressed-SDLK1%maxTasks) except it always returns a positive number
        if (index < tasks.size() && index >= 0) //we have to use .size() rather than the size variable here because size mostl
        {
            currentTask = tasks[index].get();
            Camera* cam = &(GameWindow::getCamera());
            glm::vec2 center = currentTask->getCenter();
            if (!pointInVec(cam->getRect(),center.x,center.y,0))
            {
                cam->center(center);
            }
        }
    }


}

void Manager::updateAnts()
{
    int released = MouseManager::getJustReleased();
    if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT)
    {
       // antManager.clear();
       currentTask = nullptr;
        //selectedUnit = nullptr;
    }
    int index = 0;
    Map* level = &(GameWindow::getLevel());
    AntStorage* ants = &(level->getAnts(level->getCurrentChunk()));
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
            std::vector<Positional*> vec;
            tree->getNearest(vec,*(rectPos));
            for (int j = vec.size() - 1; j >= 0; j --)
            {
                Entity* ptr = &(((RectComponent*)vec[j])->getEntity());
                if ( ptr != i->first && vec[j]->collides(*rect))
                {
                    i->second->collide(*ptr);
                    ptr->collide(*(i->second.get()));
                }
            }
            if (released == SDL_BUTTON_LEFT && i->second->getRect().collides(*selectRect))
            {
                if (!newTask)
                {
                    newTask = new AntManager(*this);
                    tasks.emplace_back(newTask);
                    currentTask = newTask;
                }
                newTask->addAnt(i->second);
                //selectedUnits.emplace_back(ants[i->first]);
            }
        }
        else
        {
            i->second->onDeath();
            level->remove(static_cast<Unit&>(*((i)->first)));
        }
        index ++;
    }
}

void Manager::updateEntities()
{
    Map* level = &(GameWindow::getLevel());
    ObjectStorage* entities = &(level->getEntities(level->getCurrentChunk()));
    RawQuadTree* tree = level->getTree(level->getCurrentChunk());

    auto it2 = entities->begin();
    auto end = entities->end(); //we do this in case we added an object while looping. This ensures we still iterate as if the object wasn't added.
   // const glm::vec4* selectRect = &(GameWindow::getSelection());
    bool clicked = MouseManager::getJustClicked() == SDL_BUTTON_LEFT;
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
            if (current->clicked())
               {
                   //current->getClickable().click(true);
                   //selectedUnits.emplace_back(entities[current]);
                   newSelected = current;
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

void Manager::update()
{
    //std::vector<Unit*> selected;

    //std::cout << released << std::endl;


    updateAnts();

    updateEntities();

    updateAntManagers();

    if (currentTask)
    {
        currentTask->getInput();
    }
   /* else if (selectedUnit)
    {
        selectedUnit->getClickable().click(true);
    }*/
    int size = GameWindow::getLevel().getEntities(GameWindow::getLevel().getCurrentChunk()).size();
    if (size < 10)
    {
        //spawnCreatures();
    }
    //glm::vec2 disp = {GameWindow::getCamera().getRect().x,GameWindow::getCamera().getRect().y};
    //tree->render(disp);
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
            rect.x += absMin((mousePos.first - 1), (mousePos.first - screenDimen.x +2 ));
        }
        if (mousePos.second >= screenDimen.y - 1 || mousePos.second <= 0)
        {
            rect.y +=  absMin((mousePos.second - 1), (mousePos.second - screenDimen.y + 2 ));
        }
        if (bounds)
        {
            rect.x = std::max(bounds->x,std::min(bounds->x + bounds->z - rect.z,rect.x));
            rect.y = std::max(bounds->y, std::min(bounds->y + bounds->a + GameWindow::getMenuHeight() - rect.a , rect.y));
        }
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
    zoomAmount = std::max(minZoom,std::min(maxZoom,zoomAmount+ amount));
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

float GameWindow::menuHeight = 1;
glm::vec2 GameWindow::origin = {0,0};
glm::vec4 GameWindow::selection = {0,0,0,0};
const glm::vec4 GameWindow::selectColor = {0,1,0,.5};
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
    //level.remove(*hill);
   // level.addUnit(*(new Beetle(320,320)));
    level.addAnt(*(new Ant({400,400,10,10},*hill)));
    hill->getComponent<ResourceComponent>()->setResource(-1000);
    auto mushPtr = level.addUnit(*(new Mushroom(480,480)));
    labels.emplace_back(new Label({400,400,256,32},"Click on the mushroom",{1,1,1,1},{480,480},
                                  *(new LambdaTrigger([](){return !GameWindow::getCamera().isZooming();})),
                                  *(new ObjectTrigger([](Object* obj){
                                return obj && obj->clicked();},mushPtr)),1));
    //camera.getComponent<MoveComponent>()->setTarget({-100,-100});
    camera.center({480,480});
    camera.setZoomTarget(.5);
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
        if (select)
        {
            requestRect(selection,selectColor,true,0,-.1);
        }

        camera.update();
        manager.update();
        level.render();

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

    AntManager* antManager = manager.getCurrentTask();
    if (antManager)
    {
        std::vector<std::weak_ptr<Ant>>* selectedUnits = &(antManager->getAnts());
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

GameWindow::QuitButton::QuitButton(GameWindow& window_) : Button({10,50,32,32},nullptr,nullptr, {"Quit"},&Font::alef,{0,1,0,1}), window(&window_)
{

}
void GameWindow::QuitButton::press()
{
    window->quit = true;
}
