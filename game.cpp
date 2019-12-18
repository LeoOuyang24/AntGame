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
    if (justClicked == SDL_BUTTON_RIGHT)
    {
        std::shared_ptr<Unit>* newTarget = nullptr;
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
    bool fit = true; //whether or not the ants fit perfectly on the targetUnit. Vacuously true if there is no targetUnit
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
        if ( ((space.x + Ant::dimen)*clumpDimen.x > targetRect->z || (space.y + Ant::dimen)*clumpDimen.y > targetRect->a))
        {
            fit = false;
        }
        if (health && health->getHealth() > 0)
        {
            currentTask = ATTACK;
        }
        else
        {
            currentTask = COLLECT;
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
                        if (health)
                        {
                            health->addHealth(-1);
                        }
                    }
                    else if (currentTask == COLLECT)
                    {
                        if (resource)
                        {
                            resource->collect(*selected[i]);
                             collected = false;
                            selected[i]->setTarget(*(selected[i]->getComponent<Ant::AntMoveComponent>()->getHome()));
                        }
                    }
                }
                else if (clumpDimen.x != 0)
                {
                    glm::vec2 moveTo = {target.x + i%((int)clumpDimen.x)*(Ant::dimen + space.x), target.y + (i/((int)clumpDimen.y))%((int)clumpDimen.y)*(Ant::dimen+space.y)};
                    selected[i]->setTarget(moveTo,nullptr);
                }
                if (currentTask == MOVE && atTarget && !selected[i]->getComponent<MoveComponent>()->atTarget())
                {
                    atTarget = false;
                }
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
    if (unit.getComponent<Ant::AntClickable>() != nullptr && unit.clicked()) //if the unit is an ant and is clicked
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
    ants.push_back(ptr);
    tree->add(ant.getRect());
    return ptr;
}

void Manager::spawnCreatures()
{
    const glm::vec4* mapSize = &(GameWindow::getLevel().getRect());
    const glm::vec4* camera = &(GameWindow::getCamera().getRect());
    int x = rand()%((int)(mapSize->z - camera->z)) + mapSize->x;
    int y = rand() % ((int)(mapSize->a - camera->a)) + mapSize->y;
    if (pointInVec(*camera,x,y,0))
    {
        x += camera->z;
        y += camera->a;
    }
    addEntity(*(new Bug( x,y )));
}

void Manager::update()
{
    //std::vector<Unit*> selected;
    int size = ants.size();
    int released = MouseManager::getJustReleased();
    if (released == SDL_BUTTON_LEFT)
    {
        antManager.clear();
    }

    for (int i = 0; i < size; ++ i)
    {
        RectPositional* rectPos = &(ants[i]->getRect());
        const glm::vec4* rect = &(rectPos->getRect());
        RawQuadTree* oldTree = tree->find(*rectPos);
        ants[i]->update();
        RawQuadTree* newTree = tree->update(*rectPos,*oldTree);
        std::vector<Positional*> vec;
        tree->getNearest(vec,*(rectPos));
        for (int j = vec.size() - 1; j >= 0; j --)
        {
            Entity* ptr = &(((RectComponent*)vec[j])->getEntity());
            if (ptr == ants[i]->getComponent<Ant::AntMoveComponent>()->getTargetUnit() && vec[j]->collides(*rect))
            {
                //std::cout << "ASDF" << std::endl;
                ants[i]->collide(*ptr);
                ptr->collide(*(ants[i].get()));
            }
        }
        if (released == SDL_BUTTON_LEFT && ants[i]->clicked())
        {
            antManager.addAnt(ants[i]);
        }
    }
    antManager.update();
    int c = 0;
    for (auto i = entities.begin(); i !=  entities.end(); ++i)
    {
        Unit* current = i->first;
        RectPositional* rectPos = &(current->getRect());
        RawQuadTree* oldTree = tree->find(*rectPos);
        if (current->getComponent<HealthComponent>()->getHealth() > 0)
        {
            current->update();
        }
        else
        {
            CorpseComponent* corpse = current->getComponent<CorpseComponent>();
            if (corpse)
            {
                corpse->update();
            }
            else
            {
                current->setDead(true);
            }
        }
        tree->update(*rectPos,*oldTree);
        if (current->getDead())
        {
            remove(*(current));
        }
    }
    size = entities.size();
    if (size < 3)
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
    entities.erase(&unit);
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

GameWindow::GameWindow() : Window({0,0},nullptr)
{
    level.init({-1000,-1000,2000,2000});
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    camera.init(screenDimen.x,screenDimen.y);
    gameOver = new Window({screenDimen.x/10, screenDimen.y/10},nullptr);
    gameOver->addButton(*(new QuitButton(*this)));
    manager.init(level.getRect());
    Anthill* hill = (new Anthill({320,320}));
    anthill = (manager.addEntity(*hill));
    //manager.addEntity(*(new Resource({160,160,100})));
    for (int i = 0; i < 100; i ++)
    {
        hill->createAnt();
    }
    hill->getComponent<ResourceCountComponent>()->setResource(1000);
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
    if (anthill.expired())
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
