#define _USE_MATH_DEFINES
#include <cmath>

#include "render.h"
#include "vanilla.h"

#include "ants.h"
#include "world.h"
#include "game.h"

const short Ant::dimen = 10;
Ant::AntMoveComponent::AntMoveComponent(Anthill* hill, double speed, const glm::vec4& rect, Entity& entity) : MoveComponent(speed,rect,entity), ComponentContainer<Ant::AntMoveComponent>(entity), home(hill)
{

}

void Ant::AntMoveComponent::collide(Entity& other)
{
    auto move = other.getComponent<Ant::AntMoveComponent>();
    if (&other == home && carrying > 0)
    {
        home->getComponent<ResourceComponent>()->setResource(carrying);
        carrying = 0;
    }
    else if (move && move->atTarget() && atTarget())
    {
        glm::vec2 otherPos = move->getPos();
        glm::vec2 center = getCenter();
        if (rect.x == otherPos.x && rect.y == otherPos.y)
        {
            otherPos.x += rand()%3 - 1;
            otherPos.y += rand()%3 - 1;
        }

        glm::vec2 targetPoint = {center.x + convertTo1(rect.x-otherPos.x)*.05, center.y + convertTo1(rect.y-otherPos.y)*.05};
        //std::cout << targetPoint.x << " " << targetPoint.y << std::endl;
        //std::cout << "ASDF" << std::endl;
       teleport(targetPoint);
        //setTarget(targetPoint);//, rect.z, rect.a});
    }
}

void Ant::AntMoveComponent::setCarrying(int amount)
{
    carrying = amount;
}

int Ant::AntMoveComponent::getCarrying()
{
    return carrying;
}

Anthill* Ant::AntMoveComponent::getHome()
{
    return home;
}

void Ant::AntMoveComponent::update()
{
    if (!atTarget())
    {
        /*Object* owner = static_cast<Object*>(entity);
        RawQuadTree* tree = GameWindow::getLevel().getTreeOf(*owner);
        auto nearby = tree->getNearest(getCenter(), 100);
        int size = nearby.size();
        glm::vec2 modified = {0,0};
        glm::vec2 avgTarg = {0,0};
        int ants = 0;
        glm::vec2 ownerCenter = owner->getCenter();
        for (int i = 0; i < size; ++i)
        {
            Unit* current = static_cast<Unit*>(&((static_cast<RectComponent*>(nearby[i]))->getEntity()));
            AntMoveComponent* move = current->getComponent<AntMoveComponent>();
            if (move)
            {
                ants ++;
                glm::vec2 center =  current->getCenter();
                double distance = glm::distance(center,ownerCenter);
                bool repulse =  false;
                modified.x += center.x - ownerCenter.x + repulse*(-.1*distance);
                modified.y += center.y - ownerCenter.y + repulse*(-.1*distance);
                avgTarg.x += move->getTarget().x;
                avgTarg.y += move->getTarget().y;
            }
        }
        modified.x /= ants;
        modified.y /= ants;
      //  target.x = (avgTarg.x + target.x)/(ants+1);
     //   target.y = (avgTarg.y + target.y)/(ants+1);
        target.x += .1*modified.x;
        target.y += .1*modified.y;*/
        MoveComponent::update();
    }

}

Ant::AntMoveComponent::~AntMoveComponent()
{

}

Ant::AntRenderComponent::AntRenderComponent(const glm::vec4& color, Entity& entity) : RenderComponent(entity), ComponentContainer<AntRenderComponent>(entity), color(color)
{

}

void Ant::AntRenderComponent::update()
{
    /*if (angle == goalAngle)
    {
        if (rand()%1000 == 0)
        {
            goalAngle = rand()%360*M_PI/180;
        }
    }
    else
    {
        float speed = .001;
        if (goalAngle < angle)
        {
            speed *= -1;
        }
        angle += absMin((goalAngle-angle),speed);
        //std::cout << angle << " " << goalAngle << std::endl;
    }*/
    const glm::vec4* rect = &(entity->getComponent<RectComponent>()->getRect());
    GameWindow::requestNGon(4,{rect->x+rect->z/2,rect->y+rect->a/2},rect->z,{0,0,0,1},angle,true,0);
    if (((Ant*)(entity))->getCarrying() > 0)
    {
        GameWindow::requestNGon(4,{rect->x + rect->z/2, rect->y - 3}, 10, {1,.8,.7,1},0,true,0);
    }
}

void Ant::AntRenderComponent::render(const SpriteParameter& param)
{
    GameWindow::requestRect(param.rect,param.tint,true,param.radians,param.z);
}

Ant::AntRenderComponent::~AntRenderComponent()
{

}

Ant::AntClickable::AntClickable(std::string name, Unit& entity) : ClickableComponent(name, entity), ComponentContainer<AntClickable>(entity)
{

}

void Ant::AntClickable::clicked()
{
    AntRenderComponent* ptr = (entity->getComponent<AntRenderComponent>());
    if (ptr)
    {

    }
}

Ant::AntClickable::~AntClickable()
{

}

Ant::Ant(const glm::vec4& rect, Anthill& home) : Unit(*(new ClickableComponent("Ant",*this)), *(new AntMoveComponent(&home,.1,rect,*this)),*(new AntRenderComponent({0,0,0,1},*this)),
                                                      *(new HealthComponent(*this, 10)))
{
    health->setVisible(false);
    addComponent(*(new AttackComponent(1,100,*this)));
    addComponent(*(new ApproachComponent(*this)));
}

void Ant::setTarget(const glm::vec2& target, std::shared_ptr<Object>* unit)
{
    getComponent<ApproachComponent>()->setTarget(target, unit);
}

void Ant::setTarget(const glm::vec2& target)
{
    setTarget(target,nullptr);
}

void Ant::setTarget(std::shared_ptr<Object>& unit)
{
    setTarget(unit->getCenter(), &unit);
}

void Ant::setCarrying(int amount)
{
    getComponent<AntMoveComponent>()->setCarrying(amount);
}

int Ant::getCarrying()
{
    return ((AntMoveComponent*)rect)->getCarrying();
}

AntManager* Ant::getCurrentTask()
{
    return currentTask;
}
void Ant::setCurrentTask(AntManager& newTask)
{
    currentTask = &newTask;
}

AntHillRender::AntHillRender(Entity& entity) : RenderComponent(entity), ComponentContainer<AntHillRender>(entity),color({0,0,0,1})
{

}

void AntHillRender::update()
{
    glm::vec4 rect = entity->getComponent<RectComponent>()->getRect();
   // GameWindow::requestNGon(10,{rect.x + rect.z/2, rect.y + rect.a/2},20,{.5,.5,.5,1},0,true,0);
    render({rect});
    ResourceComponent* counter = entity->getComponent<ResourceComponent>();
    //int width = counter->getResource()/((float)(counter->getMaxResource()))*rect.z, height = 10;
    counter->render({rect.x,rect.y + rect.a+ 10,rect.z},0);
   // GameWindow::requestRect({rect.x, rect.y + rect.a + 10, width, height},{0,1,0,1},true,0,0);
    //GameWindow::requestRect({rect.x + width, rect.y + rect.a + 10, rect.z - width, height}, {1,0,0,1}, true, 0, 0);
}

void AntHillRender::render(const SpriteParameter& param)
{
    GameWindow::requestNGon(10,{param.rect.x + param.rect.z/2, param.rect.y + param.rect.a/2},20,{.5,.5,.5,1},param.radians,true,param.z);
}

AntHillRender::~AntHillRender()
{

}

Anthill::CreateAnt::CreateAnt(Anthill& hill) : Button({0,0,64, 16},nullptr,nullptr,{"Create Ant"},&Font::alef,{0,1,0,1}), hill(&hill)
{

}

void Anthill::CreateAnt::press()
{
    if (hill)
    {
        hill->createAnt();
    }
}

Anthill::Anthill(const glm::vec2& pos) : Unit(*(new ClickableComponent("Anthill",*this)), *(new RectComponent({pos.x,pos.y,64,64}, *this)), *(new AntHillRender(*this)), *(new HealthComponent(*this, 100)))
{
    getClickable().addButton(*(new CreateAnt(*this)));
    addComponent(*(new ResourceComponent(1000,*this)));
}

void Anthill::createAnt()
{
    ResourceComponent* counter = getComponent<ResourceComponent>();
    if (counter->getResource() >= 1)
    {
        glm::vec2 center = getCenter();
        counter->setResource(-1);
        /*std::shared_ptr<Ant> ptr = ;
        std::cout << ptr.use_count() << std::endl;
        std::weak_ptr<Ant> weak = ptr;*/
        ants.emplace_back(GameWindow::getLevel().addAnt( *(new Ant({center.x +5*cos(rand()%360/180.0*M_PI),center.y +5*sin(rand()%360/180.0*M_PI),10,10},*this))));
        //std::cout << ants.size() << std::endl;
    }
}

Anthill::~Anthill()
{
    std::cout << "Anthill deletion" << std::endl;
}

