#include "antManager.h"
#include "game.h"

const int AntManager::spacing = 2;

void AntManager::addChildAnt(const std::shared_ptr<Entity>& ant)
{
    selected.emplace_back(ant);
}

glm::vec4 AntManager::getChildColor(int index)
{
    double a = 1;//GameWindow::selectColor.a;
    switch (index)
    {
        case 0: return {1,0,0,a}; break;
        case 1: return {0,1,0,a}; break;
        case 2: return {0,.5,.8,a}; break;
        case 3: return {1,1,0,a}; break;
        default: return {1,0,0,a}; break;
    }
}

AntManager::AntManager(Manager& newManager, const glm::vec4& selectColor) : manager(&newManager),
 selectColor(selectColor)
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
const std::vector<std::weak_ptr<Entity>>& AntManager::getAnts() const
{
    return selected;
}
void AntManager::clear()
{
    selected.clear();
    targetUnit.reset();
}
const Object* AntManager::getTargetUnit() const
{
    return targetUnit.lock().get();
}

void AntManager::setShortTarget(std::shared_ptr<Object>& obj)
{
    int size = selected.size();
    for (int i = 0; i < size; ++i)
    {
        Entity* ant = selected[i].lock().get();
        if (ant)
        {
            UnitAttackComponent* attack = ant->getComponent<UnitAttackComponent>();
            if (attack)
            {
                attack->setShortTarget(obj);
            }
        }
    }
}

void AntManager::getInput()
{


    std::pair<int,int> mousePos = MouseManager::getMousePos();
    glm::vec4 mouseClick = {GameWindow::getCamera().toWorld({mousePos.first,mousePos.second}),1,1};
    int chosen = selected.size();
    if (chosen > 0)
    {
        attackMove = attackMove || (KeyManager::getJustPressed() == SDLK_a);

        bool justClicked = MouseManager::getJustClicked() == SDL_BUTTON_RIGHT;

        std::vector<Positional*> nearest;
        //RectPositional post(mouseClick);
        //tree.getNearest(nearest,post);
        Map* map = (GameWindow::getLevel());
        nearest = map->getTree()->getNearest(mouseClick);
        std::shared_ptr<Object>* newTarget = nullptr;
        if (justClicked)
        {
            clickTimer.set();
            int nearSize = nearest.size();
            if (nearSize > 0)
            {
                for (int i = 0; i < nearSize; ++i)
                {
                    RectComponent* ptr = static_cast<RectComponent*>(nearest[i]);
                    if (ptr->getEntity().getComponent<Ant::AntMoveComponent>() == nullptr && ptr->collides(mouseClick) &&
                        !static_cast<Object*>(&(ptr->getEntity()))->getFriendly())
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

        }
        if (newTarget) //if we clicked on a unit
        {
            const glm::vec4* targetRect = &(newTarget->get()->getRect().getRect());
            clumpDimen.y = sqrt(chosen/(targetRect->z/targetRect->a)); //#of ants per dimension
            clumpDimen.x = std::max(chosen/clumpDimen.y,1.0f);
            space = {std::min(spacing, (int)((targetRect->z - clumpDimen.x*Ant::dimen)/clumpDimen.x)),
                    std::min(spacing, (int)((targetRect->a - clumpDimen.y*Ant::dimen)/clumpDimen.y))};
            HealthComponent* health = newTarget->get()->getComponent<HealthComponent>();

            if (newTarget->get()->getComponent<ResourceComponent>() )
            {
                setTask(MOVE);
            }
            else if (health && health->getHealth() > 0)
            {
                setTask(ATTACK);
            }
            else if (newTarget->get()->getComponent<InteractionComponent>())
            {
                setTask(INTERACT);
            }
            else
            {
                setTask(MOVE);
            }
        }
        else if (justClicked)
        {
             clumpDimen.x = floor(sqrt(chosen));
             clumpDimen.y = chosen/clumpDimen.x;
             space = {spacing,spacing};
            setTask(MOVE);
        }

        targetPoint = {mouseClick.x, mouseClick.y - clumpDimen.y};
       /* if (KeyManager::getJustPressed() == SDLK_TAB && !parent.lock().get())
        {
            split(AntManager::maxChildren);
        }*/

        for (int i = 0; i < chosen; ++i)
        {
            Entity* ptr = selected[i].lock().get();
            if (ptr)
            {
                ClickableComponent* click = ptr->getComponent<ClickableComponent>();
                if (click)
                {
                    click->click(true);
                   // GameWindow::requestRect(ptr->getRect().getRect(),selectColor,true,0,0);
                    if (justClicked) //if we've recieved a new task
                    {
                        CommandableComponent* command = ptr->getComponent<CommandableComponent>();
                         if( command && command->getCurrentTask() != this)
                         {
                            command->setCurrentTask(this);
                         }
                       // std::cout << this << " " << currentTask << std::endl;
                        if (clumpDimen.x >= 1) //moving to target. Decimals can still be rounded to 0
                        {
                            glm::vec2 moveTo;
                            /*if (repel)
                            {
                                double angle = atan2(targetPoint.y - currentRect.y - currentRect.a/2,targetPoint.x-currentRect.x - currentRect.z/2);
                                moveTo = {currentRect.x + currentRect.z/2 + -1*(cos(angle)), currentRect.y + currentRect.a/2 + -1*sin(angle)};
                            }
                            else*/
                            {
                               // std::cout << scale << std::endl;
                                moveTo = {targetPoint.x + ((i%((int)clumpDimen.x)) - (clumpDimen.x-1)/2)*(Ant::dimen + space.x),
                                i/((int)(clumpDimen.x))};
                                if (newTarget)
                                {
                                    moveTo.y = fmod(moveTo.y,clumpDimen.y);
                                }
                              //  std::cout << scale << std::endl;
                                moveTo.y = (moveTo.y-  (clumpDimen.y-1)/2.0)*(Ant::dimen + space.y) + targetPoint.y;
                            }
                            auto otherTarg = ptr->getComponent<MoveComponent>()->getTarget();
                            if (otherTarg.x != moveTo.x || otherTarg.y != moveTo.y)
                            {
                                //atTarget = false;
                              //  std::cout << "Move: " << i << " " << moveTo.x << " " << moveTo.y<< "\n";
                                // current->getComponent<MoveComponent>()->getTarget().x << " " << current->getComponent<MoveComponent>()->getTarget().y << std::endl;
                                UnitAttackComponent* attack = ptr->getComponent<UnitAttackComponent>();
                                if (attack)
                                {
                                    attack->setLongTarget(moveTo,newTarget,false);
                                }
                                else
                                {
                                    ptr->getComponent<ApproachComponent>()->setTarget(moveTo,newTarget);
                                }
                            }
                        }
                    }
                }
              /*  else if ()
                {
                    chosen--;
                    i--;
                    selected.erase(selected.begin() + i);
                }*/
            }
        }
    }
    if (clickTimer.isSet())
    {
        PolyRender::requestNGon(100,GameWindow::getCamera().toScreen({mouseClick.x,mouseClick.y}),(SDL_GetTicks()-clickTimer.getTime())/500*5,{0,1,0,1},0,false,GameWindow::fontZ);
        if (clickTimer.timePassed(500))
        {
            clickTimer.reset();
        }
    }
}

void AntManager::updateAnts()
{
    int chosen = selected.size();
    if (chosen > 0)
    {
        Object* unitPtr = targetUnit.lock().get(); //if we have a target
        const glm::vec4* targetRect  = nullptr;
        HealthComponent* health = nullptr;
        InteractionComponent* interact = nullptr;
        ResourceComponent* resource = nullptr;
        if (unitPtr && unitPtr->getCenter() != targetPoint)
        {
            targetRect = &(unitPtr->getRect().getRect());
           // targetPoint = {targetRect->x + targetRect->z/2, targetRect->y + targetRect->a/2};
            health = unitPtr->getComponent<HealthComponent>();
            resource = unitPtr->getComponent<ResourceComponent>();
            interact = unitPtr->getComponent<InteractionComponent>();
        }
        Map* map = (GameWindow::getLevel());
       // bool lastRepel = repel && MouseManager::getJustClicked() != SDL_BUTTON_RIGHT; //whether or not we were repelling last frame.

        bool atTarget = true; //used if the current Task is Move. Used to keep track of whether or not there are still units moving.
        bool collected = true; //used if the current Task is COLLECT. Used to keep track of whether or not there are still units collecting.
        antsCenter = {0,0};
        for (int i = 0; i < chosen; ++ i)
        {
            Entity* current = selected[i].lock().get();
            if (!current)
            {
                selected.erase(selected.begin() + i);
                i--;
                chosen--;
            }
            else
            {
                glm::vec4 currentRect = current->getComponent<RectComponent>()->getRect();
                antsCenter.x += currentRect.x + currentRect.z/2;
                antsCenter.y += currentRect.y + currentRect.a/2;
                if (currentTask != IDLE)
                {
                   /* if (current->getCarrying() == 0)
                    {
                        if (targetRect && vecIntersect(currentRect,*targetRect)) //at target
                        {
                            if (currentTask == ATTACK)
                            {
                               // current->getComponent<AttackComponent>()->attack(health);
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
                                setTask(IDLE);
                                targetUnit.reset();
                                break;
                            }
                        }

                    }
                    else
                    {
                        current->setTarget(map->getUnit(current->getComponent<Ant::AntMoveComponent>()->getHome()));
                        collected = false;
                    }*/
                    if (currentTask == MOVE && atTarget && !current->getComponent<MoveComponent>()->atTarget())
                    {
                        atTarget = false;
                    }
                }
            }
        }
        if (currentTask == MOVE && atTarget)
        {
           setTask(IDLE);
        }
        if (currentTask == COLLECT && collected && !unitPtr)
        {
            setTask(IDLE);
        }
        antsCenter/=chosen;
                  //  std::cout << antsCenter.x << " " << antsCenter.y << std::endl;

        }
       // std::cout << "END" << std::endl;
   /* for (int i = 0; i < maxChildren; ++i)
    {
        if (child[i].get())
        {
            child[i]->updateAnts();
        }
    }*/
   // GameWindow::requestNGon(10,antsCenter,1,{.3,.4,.5,1},0,1,1);

}

void AntManager::addAnt(const std::shared_ptr<Entity>& ant)
{

    selected.emplace_back(ant);
  //  ant->setCurrentTask(*this);

}

void AntManager::remove(Unit& unit)
{
    if (targetUnit.lock().get() == &unit)
    {
        targetUnit.reset();
        return;
    }
    else if (unit.getComponent<Ant::AntMoveComponent>() != nullptr) //if the unit is an ant
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

void AntManager::render(const glm::vec4& rect, std::string c )
{
    glm::vec4 taskColor;
    if (selected.size() == 0)
    {
        taskColor = {.5,.5,.5,1};
    }
    else
    {
        taskColor =   {currentTask == ATTACK || currentTask == IDLE,
                        std::max((double)(currentTask == MOVE || currentTask == IDLE),
                        .5*(currentTask == COLLECT)),currentTask == COLLECT || currentTask == IDLE,1};
    }
    if (this == GameWindow::getManager().getCurrentTask())
    {
        GameWindow::requestNGon(3,{rect.x + rect.z/2, rect.y + rect.a/2},10,{1,0,0,1},M_PI/3,true,GameWindow::interfaceZ,true);
    }
    GameWindow::requestRect(rect,taskColor,true,0,GameWindow::interfaceZ, true);
    Font::tnr.requestWrite({c,GameWindow::getCamera().toAbsolute(rect),0,{0,0,0,1},GameWindow::interfaceZ});
}

std::vector<AntManager> AntManager::split(int pieces )
{
    int size = selected.size();
    std::vector<AntManager> arr;
    for (int i = 0; i < pieces; ++i)
    {
        arr.push_back(AntManager(*manager,getChildColor(i)));
    }
    for (int i = 0; i < size; ++i)
    {
        int index = 0;
        Entity* ant = selected[i].lock().get();
        if (ant)
        {
            glm::vec2 center = ant->getComponent<RectComponent>()->getCenter();
            if (center.x <= antsCenter.x)
            {
                if (center.y > antsCenter.y)
                {
                    index = 2;
                }
                else
                {
                    index = 0;
                }
            }
            else
            {
                if (center.y > antsCenter.y)
                {
                    index = 3;
                }
                else
                {
                    index = 1;
                }
            }
            arr[index].addChildAnt(selected[i].lock());
         //   std::cout << index << std::endl;
        }
    }
    return arr;
}


void AntManager::setTask(Task t)
{
    currentTask = t;
}
