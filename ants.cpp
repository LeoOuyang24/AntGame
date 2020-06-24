#define _USE_MATH_DEFINES
#include <cmath>

#include "render.h"
#include "vanilla.h"

#include "ants.h"
#include "world.h"
#include "game.h"

const short Ant::dimen = 10;
Ant::AntMoveComponent::AntMoveComponent(Anthill* hill, double speed, const glm::vec4& rect, Entity& entity) : PathComponent(speed,rect,entity), ComponentContainer<Ant::AntMoveComponent>(entity), home(hill)
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
    else if (move && move->getVelocity() == 0 && velocity == 0)
    {
        glm::vec2 otherPos = move->getPos();
        glm::vec2 center = getCenter();
        if (rect.x == otherPos.x && rect.y == otherPos.y)
        {
            otherPos.x += rand()%3 - 1;
            otherPos.y += rand()%3 - 1;
        }

        glm::vec2 targetPoint = {center.x + convertTo1(rect.x-otherPos.x)*.05, center.y + convertTo1(rect.y-otherPos.y)*.05};
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
    PathComponent::update();

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
    radius = fmod(radius + 1,500 );
    glm::vec4 rect = entity->getComponent<RectComponent>()->getRect();
   // GameWindow::requestNGon(10,{rect.x + rect.z/2, rect.y + rect.a/2},20,{.5,.5,.5,1},0,true,0);
    render({rect});
    ResourceComponent* counter = entity->getComponent<ResourceComponent>();
    //int width = counter->getResource()/((float)(counter->getMaxResource()))*rect.z, height = 10;
    counter->render({rect.x,rect.y + rect.a+ 10,rect.z},0);
    GameWindow::requestNGon(50,{rect.x + rect.z/2, rect.y + rect.a/2},radius/(tan(81*M_PI/180)),{.5,.5,0,1}, 0,false, 0);
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

Anthill::CreateAnt::CreateAnt(Anthill& hill) : Button({0,0,64, 16},nullptr,nullptr,{"Create Ant"},&Font::tnr,{0,1,0,1}), hill(&hill)
{

}

void Anthill::CreateAnt::press()
{
    if (hill)
    {
        hill->createAnt();
    }
}

Anthill::StartSignal::StartSignal(Anthill& hill) : Button({0,0,64,16},nullptr,nullptr,{"Start Signal"}, &Font::tnr, {0,1,0,1}), hill(&hill)
{

}

void Anthill::StartSignal::press()
{
    if(hill)
    {
        Manager* manager = (hill->getManager());
        if (manager)
        {
            manager->setSignalling(*hill);
        }
        else
        {
            throw std::logic_error("Manager is null");
        }
    }
    else
    {
        throw std::logic_error("Hill is null");
    }
}

Anthill::Anthill(const glm::vec2& pos) : Structure(*(new ClickableComponent("Anthill",*this)), *(new RectComponent({pos.x,pos.y,64,64}, *this)),
                                                *(new AntHillRender(*this)), *(new HealthComponent(*this, 100)))
{
    getClickable().addButton(*(new CreateAnt(*this)));
    getClickable().addButton(*(new StartSignal(*this)));
    addComponent(*(new ResourceComponent(1000,*this)));

}

void Anthill::createAnt()
{
    ResourceComponent* counter = getComponent<ResourceComponent>();
    if (counter->getResource() >= 1)
    {
        auto vec4 = getRect().getRect();
        glm::vec2 center = {vec4.x + vec4.z + 10, vec4.y + vec4.a + 10};
        counter->setResource(-1);
        /*std::shared_ptr<Ant> ptr = ;
        std::cout << ptr.use_count() << std::endl;
        std::weak_ptr<Ant> weak = ptr;*/
        (GameWindow::getLevel().addUnit( *(new Ant({center.x +5*cos(rand()%360/180.0*M_PI),center.y +5*sin(rand()%360/180.0*M_PI),10,10},*this))));
        //std::cout << ants.size() << std::endl;
    }
}

Anthill::~Anthill()
{
    std::cout << "Anthill deletion" << std::endl;
}

