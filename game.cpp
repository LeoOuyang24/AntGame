#include "SDLHelper.h"
#include "vanilla.h"

#include "game.h"
#include "ants.h"

SpriteWrapper frame;


void Map::init(const glm::vec4& region)
{
    rect = region;
}

void Manager::AntManager::update(RawQuadTree& tree)
{
    int justClicked = MouseManager::getJustClicked();
    std::pair<int,int> mousePos = MouseManager::getMousePos();
    std::vector<Positional*> nearest;
    glm::vec4 mouseClick = {GameWindow::getCamera().toWorld({mousePos.first,mousePos.second}),1,1};
    //RectPositional post(mouseClick);
    //tree.getNearest(nearest,post);
    tree.getNearest(nearest,mouseClick);
    int chosen = selected.size();
    bool alreadyTargeted = targetUnit;
    glm::vec2 clumpDimen; //how many ants horizontally and vertically
    glm::vec2 space; //space between ants horizontally and vertically
    glm::vec2 target;
    if (justClicked == SDL_BUTTON_RIGHT)
    {
        int nearSize = nearest.size();
        Unit* newTarget = nullptr;
        if (nearSize > 0)
        {
            for (int i = 0; i < nearSize; ++i)
            {
                Unit* ptr = (Unit*)&(((RectComponent*)(nearest[i]))->getEntity());
                if (ptr->getComponent<Ant::AntMoveComponent>() == nullptr && nearest[i]->collides(mouseClick))
                    {
                        newTarget = ptr;
                        break;
                    }
            }
        }
        targetUnit = newTarget;
        if (!targetUnit)
        {
            targetUnit = nullptr;
             clumpDimen = {sqrt(chosen),sqrt(chosen)};
             space = {spacing,spacing};
            target = {mouseClick.x - clumpDimen.x*(Ant::dimen + space.x)/2 + Ant::dimen/2, mouseClick.y - clumpDimen.y*(Ant::dimen + space.y)/2 + Ant::dimen/2};
        }
    }
    if (targetUnit) //if we clicked on a unit
    {
        //std::cout << "ASDF" << std::endl;
        const glm::vec4* targetRect = &(targetUnit->getRect().getRect());
        target = {targetRect->x + Ant::dimen/2, targetRect->y + Ant::dimen/2};
        clumpDimen.y = sqrt(chosen/(targetRect->z/targetRect->a));
        clumpDimen.x = chosen/clumpDimen.y;
        space = {std::min(spacing, (int)((targetRect->z - clumpDimen.x*Ant::dimen)/clumpDimen.x)),
                std::min(spacing, (int)((targetRect->a - clumpDimen.y*Ant::dimen)/clumpDimen.y))};
    }
    if (targetUnit || justClicked == SDL_BUTTON_RIGHT)
    {
        for (int i = 0; i < chosen; ++ i)
        {
            if (selected[i]->getCarrying() == 0)
            {
                selected[i]->setTarget({target.x + i%((int)clumpDimen.x)*(Ant::dimen + space.x), target.y + i/((int)clumpDimen.x)*(Ant::dimen+space.y)},targetUnit);
            }
            else
            {
                selected[i]->setTarget(*(selected[i]->getComponent<Ant::AntMoveComponent>()->getHome()));
            }
        }
    }

    if (justClicked == SDL_BUTTON_LEFT && targetUnit == nullptr)
    {
        selected.clear();
    }

}

void Manager::AntManager::remove(Unit& unit)
{
    if (targetUnit == &unit)
    {
        targetUnit = nullptr;
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

const int Manager::spacing = 2;

Manager::Manager()
{

}

void Manager::init(const glm::vec4& region)
{
    tree.reset(new RawQuadTree(region));
}

void Manager::addEntity(Unit& entity)
{
    entity.setManager(*this);
    entities.emplace_back(&entity);
    tree->add(entity.getRect());
}

void Manager::addAnt(Ant& ant)
{
    ant.setManager(*this);
    ants.emplace_back(&ant);
    tree->add(ant.getRect());
}

void Manager::spawnCreatures()
{
    const glm::vec4* mapSize = &(GameWindow::getLevel().getRect());
    addEntity(*(new Bug(rand()%((int)(640)), rand() % ((int)(640)))));
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
    antManager.update(*(tree.get()));

    size = entities.size();
    for (int i = size - 1; i >= 0; i--) //we iterate backwards so deleting entities doesn't cause issues
    {
        RectPositional* rectPos = &(entities[i]->getRect());
        RawQuadTree* oldTree = tree->find(*rectPos);
        if (entities[i]->getComponent<HealthComponent>()->getHealth() > 0)
        {
            entities[i]->update();
        }
        else
        {
            CorpseComponent* corpse = entities[i]->getComponent<CorpseComponent>();
            if (corpse)
            {
                corpse->update();
            }
        }
        tree->update(*rectPos,*oldTree);
        if (entities[i]->getDead())
        {
            remove(*(entities[i].get()));
        }
    }
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
    int size = entities.size();
    for (int i = 0; i < size; ++i)
    {
        if (entities[i].get() == &unit)
        {
            entities.erase(entities.begin() + i);
            break;
        }
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
    rect.x = std::max(mapRect.x,std::min(mapRect.x + mapRect.z,rect.x));
    rect.y = std::max(mapRect.y, std::min(mapRect.y + mapRect.a, rect.y));
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

GameWindow::GameWindow() : Window({0,0},nullptr)
{
    level.init({-1000,-1000,2000,2000});
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    camera.init(screenDimen.x,screenDimen.y);
    manager.init(level.getRect());
    Anthill* hill = (new Anthill({320,320}));
    manager.addEntity(*hill);
    //manager.addEntity(*(new Resource({160,160,100})));
    manager.addEntity(*(new Bug(160,160)));
    for (int i = 0; i < 100; i ++)
    {
        manager.addAnt(*(new Ant({320,320,10,10},*hill)));
    }
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

    bool select = updateSelect();
    if (select)
    {
        requestRect(selection,selectColor,true,0,-.1);
    }
    //requestRect(camera.getRect(),{1,0,1,1},false,0,0);
    manager.update();
    camera.update();
}

void GameWindow::requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z)
{
    PolyRender::requestNGon(n, camera.toScreen(center),side,color,angle,filled,z);
}

void GameWindow::requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z)
{
    PolyRender::requestRect(camera.toScreen(rect),color,filled,angle,z);
}
