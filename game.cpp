#include "SDLHelper.h"
#include "vanilla.h"

#include "game.h"
#include "ants.h"

SpriteWrapper frame;


void Map::init(const glm::vec4& region)
{
    rect = region;
    for (int i = region.x; i < rect.x + region.z; i += 100)
    {
        for (int j = region.y; j < rect.y + region.a; j += 100)
        {
            chunks.push_back({i,j,100,100});
        }
    }
}

void Map::render()
{
    for (int i = 0; i < rect.z/100; i++)
    {
        for (int j = 0; j < rect.a/100; j ++)
        {
            float index = i*rect.a/100.0 + j;
            GameWindow::requestRect(chunks[index],{1,index/((float)chunks.size()),j*100.0/rect.z,1},true,0,-.5);
        }
    }
}

const int AntManager::spacing = 2;

void AntManager::update()
{
    int justClicked = MouseManager::getJustClicked();
    std::pair<int,int> mousePos = MouseManager::getMousePos();
    std::vector<Positional*> nearest;
    glm::vec4 mouseClick = {GameWindow::getCamera().toWorld({mousePos.first,mousePos.second}),1,1};
    //RectPositional post(mouseClick);
    //tree.getNearest(nearest,post);
    manager->tree->getNearest(nearest,mouseClick);
    int chosen = selected.size();
    glm::vec2 clumpDimen = {0,0}; //how many ants horizontally and vertically
    glm::vec2 space; //space between ants horizontally and vertically
    glm::vec2 target;
    std::shared_ptr<Unit>* newTarget = nullptr;
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
                        newTarget = &(manager->entities[&((Unit&)ptr->getEntity())]);
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
            target = {mouseClick.x - clumpDimen.x*(Ant::dimen + space.x)/2 + Ant::dimen/2, mouseClick.y - clumpDimen.y*(Ant::dimen + space.y)/2 + Ant::dimen/2};
            targetPoint = target;
            currentTask = MOVE;
        }
    }
    Unit* unitPtr = targetUnit.lock().get(); //if we have a target
    const glm::vec4* targetRect  = nullptr;
    HealthComponent* health = nullptr;
    ResourceComponent* resource = nullptr;
    if (unitPtr) //if we clicked on a unit
    {
        targetRect = &(unitPtr->getRect().getRect());
        target = {targetRect->x + Ant::dimen/2, targetRect->y + Ant::dimen/2};
        clumpDimen.y = sqrt(chosen/(targetRect->z/targetRect->a)); //#of ants per dimension
        clumpDimen.x = chosen/clumpDimen.y;
        space = {std::min(spacing, (int)((targetRect->z - clumpDimen.x*Ant::dimen)/clumpDimen.x)),
                std::min(spacing, (int)((targetRect->a - clumpDimen.y*Ant::dimen)/clumpDimen.y))};
        health = unitPtr->getComponent<HealthComponent>();
        resource = unitPtr->getComponent<ResourceComponent>();

        if (resource )
        {
            currentTask = COLLECT;
        }
        else if (health && health->getHealth() > 0)
        {
            currentTask = ATTACK;
        }
        else
        {
            currentTask = MOVE;
        }
    }
    else if (currentTask != MOVE && currentTask != COLLECT && justClicked != SDL_BUTTON_RIGHT) //if the current task is move or collect, we set it to IDLE after processing all ants.
    {
        currentTask = IDLE;
    }
    bool atTarget = true; //used if the current Task is Move. Used to keep track of whether or not there are still units moving.
    bool collected = true; //used if the current Task is COLLECT. Used to keep track of whether or not there are still units collecting.
    if (currentTask != IDLE)
    {
        for (int i = 0; i < chosen; ++ i)
        {
            if (selected[i]->getCarrying() == 0)
            {
                if (targetRect && vecIntersect(selected[i]->getRect().getRect(),*targetRect))
                {
                    if (currentTask == ATTACK)
                    {
                        selected[i]->getComponent<AttackComponent>()->attack(health);
                    }
                    else if (currentTask == COLLECT)
                    {
                        if (resource)
                        {
                            resource->collect(*selected[i]);
                             collected = false;
                        }
                    }
                }
                else if (clumpDimen.x != 0)
                {
                    glm::vec2 moveTo = {target.x + i%((int)clumpDimen.x)*(Ant::dimen + space.x), i/((int)(clumpDimen.x))};
                    if (unitPtr)
                    {
                        moveTo.y = fmod(moveTo.y,clumpDimen.y);
                        moveTo.y *= (Ant::dimen + space.y);
                        moveTo.y += target.y;
                        selected[i]->setTarget(moveTo, &manager->entities[unitPtr]);
                    }
                    else
                    {
                        moveTo.y *= (Ant::dimen + space.y);
                        moveTo.y += target.y;
                        selected[i]->setTarget(moveTo,nullptr);
                    }
                }
                if (currentTask == MOVE && atTarget && !selected[i]->getComponent<MoveComponent>()->atTarget())
                {
                    atTarget = false;
                }
            }
            else
            {
                selected[i]->setTarget(manager->entities[(selected[i]->getComponent<Ant::AntMoveComponent>()->getHome())]);
            }
        }
        if (currentTask == MOVE && atTarget)
        {
            currentTask = IDLE;
        }
        else if (currentTask == COLLECT && collected)
        {
            currentTask = IDLE;
        }
    }


    if (justClicked == SDL_BUTTON_LEFT && !unitPtr)
    {
        selected.clear();
    }

}

void AntManager::remove(Unit& unit)
{
    if (targetUnit.lock().get() == &unit)
    {
        targetUnit.reset();
    }
    if (unit.getComponent<Ant::AntMoveComponent>() != nullptr) //if the unit is an ant and is clicked
    {
        int size = selected.size();
        for (int i = 0; i < size; ++i)
        {
            if (selected[i].get() == &unit)
            {
                selected.erase(selected.begin() + i);
            }
        }
    }

}


void Manager::init(const glm::vec4& region)
{
    tree.reset(new RawQuadTree(region));
}

std::shared_ptr<Unit> Manager::addEntity(Unit& entity)
{
    entity.setManager(*this);
    auto ptr = std::shared_ptr<Unit>(&entity);
    entities[&entity] = ptr;
    tree->add(entity.getRect());
    return ptr;
}

std::shared_ptr<Ant> Manager::addAnt(Ant& ant)
{
    ant.setManager(*this);
    auto ptr = std::shared_ptr<Ant>(&ant);
    ants[&ant] = ptr;
    tree->add(ant.getRect());
    return ptr;
}

void Manager::spawnCreatures()
{
    int random = rand()%3;
    Unit* toSpawn = nullptr;
    switch(random)
    {
    case 0:
        toSpawn = new Mushroom(0,0);
        break;
    case 1:
        toSpawn = new Mushroom(0,0);
        break;
    case 2:
        toSpawn = new Mushroom(0,0);
    default:
        toSpawn = new Bug(0,0);
    }

    const glm::vec4* mapSize = &(GameWindow::getLevel().getRect());
    const glm::vec4* camera = &(GameWindow::getCamera().getRect());
    const glm::vec4* entityRect = &(toSpawn->getRect().getRect());
    int x = rand()%((int)(mapSize->z -  entityRect->z)) + mapSize->x; //we want to make sure our object spawns outside of the camera's view and doesn't spawn partially out of the map
    bool cameraInTheWay = (x >= camera->x && x <= camera->x + camera->z); //if we chose an x coordinate that may overlap with the camera's rect, we need to adjust our y coordinate
    int y = rand() % ((int)(mapSize->a - entityRect->a - camera->a*cameraInTheWay)) + mapSize->y;
    //modify coordinates so our object doesn't spawn where in the player's view
    y += camera->a*(y > camera->y)*cameraInTheWay;
    toSpawn->getRect().setRect({x,y,entityRect->z,entityRect->a});
    addEntity(*toSpawn);

}

void Manager::update()
{
    //std::vector<Unit*> selected;
    int size = ants.size();
    int released = MouseManager::getJustReleased();
    if (released == SDL_BUTTON_LEFT)
    {
        antManager.clear();
        selectedUnits.clear();
    }

    int index = 0;
    auto it = ants.begin();
    for (auto i = ants.begin(); i != ants.end(); i = it)
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
                if (ptr == i->second->getComponent<ApproachComponent>()->getTargetUnit() && vec[j]->collides(*rect))
                {
                    //std::cout << "ASDF" << std::endl;
                    i->second->collide(*ptr);
                    ptr->collide(*(i->second.get()));
                }
            }
            if (released == SDL_BUTTON_LEFT && i->second->clicked())
            {
                antManager.addAnt(i->second);
                selectedUnits.emplace_back(ants[i->first]);
            }
        }
        else
        {
            remove(static_cast<Unit&>(*((i)->first)));
        }
        index ++;
    }
    antManager.update();
    auto it2 = entities.begin();
    auto end = entities.end(); //we do this in case we added an object while looping. This ensures we still iterate as if the object wasn't added.
    for (auto i = entities.begin(); i !=  end; i = it2)
    {
        ++it2;
        Unit* current = i->first;
        RectPositional* rectPos = &(current->getRect());
        RawQuadTree* oldTree = tree->find(*rectPos);
        if (current->getComponent<HealthComponent>()->getHealth() > 0 && !current->getDead())
        {
            current->update();
            tree->update(*rectPos,*oldTree);
            if (released == SDL_BUTTON_LEFT && current->clicked())
            {
                selectedUnits.emplace_back(entities[current]);
            }
        }
        else
        {
            current->onDeath();
            remove(*(current));
        }
    }
    size = entities.size();
    if (size < 10)
    {
        spawnCreatures();
    }
    //glm::vec2 disp = {GameWindow::getCamera().getRect().x,GameWindow::getCamera().getRect().y};
    //tree->render(disp);
}

class AntClickable;
void Manager::remove(Unit& unit)
{
    tree->remove(unit.getRect());
    antManager.remove(unit);
    if (unit.getComponent<Ant::AntMoveComponent>())
    {
        ants.erase(static_cast<Ant*>(&unit));
    }
    else
    {
        entities.erase(&unit);
    }
}

void Camera::init(int w, int h)
{
    rect = {0,0,w,h};
}

void Camera::update()
{
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
    glm::vec4 mapRect = GameWindow::getLevel().getRect();
    rect.x = std::max(mapRect.x,std::min(mapRect.x + mapRect.z - rect.z,rect.x));
    rect.y = std::max(mapRect.y, std::min(mapRect.y + mapRect.a - rect.a, rect.y));
}

glm::vec4 Camera::toScreen(const glm::vec4& change) const
{
    return {change.x - (rect.x), change.y - (rect.y), change.z, change.a};
}

glm::vec2 Camera::toScreen(const glm::vec2& point) const
{
    return {point.x - rect.x, point.y - rect.y};
}

glm::vec4 Camera::toWorld(const glm::vec4& change) const
{
    return {change.x + rect.x, change.y + rect.y, change.z, change.a};
}

glm::vec2 Camera::toWorld(const glm::vec2& point) const
{
    return {point.x + rect.x, point.y + rect.y};
}

glm::vec2 GameWindow::origin = {0,0};
glm::vec4 GameWindow::selection = {0,0,0,0};
const glm::vec4 GameWindow::selectColor = {0,1,0,.5};
Camera GameWindow::camera;
Manager GameWindow::manager;
Map GameWindow::level;
Window* GameWindow::gameOver = nullptr;

GameWindow::GameWindow() : Window({0,0},nullptr,{0,0,0,0})
{
    level.init({-1000,-1000,2000,2000});
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    rect.z = screenDimen.x;
    rect.a = screenDimen.y;
    camera.init(screenDimen.x,screenDimen.y);
    gameOver = new Window({screenDimen.x/10, screenDimen.y/10},nullptr, {1,0,0,1});
    gameOver->addButton(*(new QuitButton(*this)));
    manager.init(level.getRect());
    Anthill* hill = (new Anthill({320,320}));
    anthill = (manager.addEntity(*hill));
    //manager.addEntity(*(new Bug(160,160)));
    for (int i = 0; i < 10; i ++)
    {
        hill->createAnt();
    }
    hill->getComponent<ResourceComponent>()->setResource(1000);
   // manager.clear();
}

bool GameWindow::updateSelect()
{
    if (MouseManager::isPressed(SDL_BUTTON_LEFT))
    {
        std::pair<int,int> mouse_Pos = MouseManager::getMousePos();
        glm::vec2 mousePos = camera.toWorld({mouse_Pos.first, mouse_Pos.second});
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
    Anthill* hill = static_cast<Anthill*>(anthill.lock().get());
    if (!hill || manager.ants.size() <= 0)
    {
        gameOver->update(x,y,clicked);
        level.render();
    }
    else
    {
        bool select = updateSelect();
        if (select)
        {
            requestRect(selection,selectColor,true,0,-.1);
        }
        //requestRect(camera.getRect(),{1,0,1,1},false,0,0);
        manager.update();
        camera.update();
        level.render();
        renderSelectedUnits();
    }
}

void GameWindow::renderSelectedUnits()
{
    double height = .1*rect.a;//height of the selection part;
    PolyRender::requestRect({0,rect.a - height,rect.z, height},{0,1,0,1},true,0,.1);
    std::vector<std::shared_ptr<Ant>>* selectedUnits = &(manager.getAntManager().getAnts());
    int size = selectedUnits->size();
    for (int i = 0; i < size; ++i)
    {
        Ant* current = selectedUnits->at(i).get();
        glm::vec4 outlineRect = {.1*rect.z + i%10*(.06*rect.z),rect.a - height + .2*height + i/10*.3*height,.03*rect.z,.03*rect.z};
        HealthComponent* health = current->getComponent<HealthComponent>();
        double healthRatio = health->getHealth()/health->getMaxHealth();//how much health this ant currently has;
        current->getComponent<RenderComponent>()->render({{outlineRect.x + 2, outlineRect.y + 2, outlineRect.z - 4, outlineRect.a - 4},0,NONE,{1-healthRatio,0,0,1},&RenderProgram::basicProgram,.2});
        PolyRender::requestRect(outlineRect,{1,0,1,1},false,0,.2);
    }
}

void GameWindow::requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z)
{
    PolyRender::requestNGon(n, camera.toScreen(center),side,color,angle,filled,z);
}

void GameWindow::requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z)
{
    PolyRender::requestRect(camera.toScreen(rect),color,filled,angle,z);
}
