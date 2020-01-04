#define _USE_MATH_DEFINES
#include <cmath>

#include "render.h"
#include "vanilla.h"

#include "game.h"
#include "ants.h"

const short Ant::dimen = 10;
Ant::AntMoveComponent::AntMoveComponent(Anthill* hill, double speed, const glm::vec4& rect, Entity& entity) : MoveComponent(speed,rect,entity), ComponentContainer<Ant::AntMoveComponent>(entity), home(hill)
{

}

void Ant::AntMoveComponent::collide(Entity& entity)
{
    if (&entity == home && carrying > 0)
    {
        home->getComponent<ResourceComponent>()->setResource(carrying);
        carrying = 0;
    }
}

/*void Ant::AntMoveComponent::setCarrying(int amount)
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
}*/

Ant::AntRenderComponent::AntRenderComponent(const glm::vec4& color, Entity& entity) : RenderComponent(entity), ComponentContainer<AntRenderComponent>(entity), color(color)
{

}

void Ant::AntRenderComponent::update()
{
    if (angle == goalAngle)
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
    }
    const glm::vec4* rect = &(entity->getComponent<RectComponent>()->getRect());
    GameWindow::requestNGon(4,{rect->x+rect->z/2,rect->y+rect->a/2},rect->z,{0,0,0,1},angle,true,0);
    if (((Ant*)(entity))->getCarrying() > 0)
    {
        GameWindow::requestNGon(4,{rect->x + rect->z/2, rect->y - 3}, 10, {1,.8,.7,1},0,true,0);
    }
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

Ant::Ant(const glm::vec4& rect, Anthill& home) : Unit(*(new ClickableComponent("Ant",*this)), *(new AntMoveComponent(&home,.5,rect,*this)),*(new AntRenderComponent({0,0,0,1},*this)),
                                                      *(new HealthComponent(*this, 1)))
{
    health->setVisible(false);
    addComponent(*(new AttackComponent(1,10,*this)));
    addComponent(*(new ApproachComponent(*this)));
}

void Ant::setTarget(const glm::vec2& target, std::shared_ptr<Unit>* unit)
{
    getComponent<ApproachComponent>()->setTarget(target, unit);
}

void Ant::setTarget(const glm::vec2& target)
{
    setTarget(target,nullptr);
}

void Ant::setTarget(std::shared_ptr<Unit>& unit)
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

AntHillRender::AntHillRender(Entity& entity) : RenderComponent(entity), ComponentContainer<AntHillRender>(entity),color({0,0,0,1})
{

}

void AntHillRender::update()
{
    glm::vec4 rect = entity->getComponent<RectComponent>()->getRect();
    GameWindow::requestNGon(10,{rect.x + rect.z/2, rect.y + rect.a/2},20,{.5,.5,.5,1},0,true,-.1);

    ResourceComponent* counter = entity->getComponent<ResourceComponent>();
    int width = counter->getResource()/((float)(counter->getMaxResource()))*rect.z, height = 10;
    GameWindow::requestRect({rect.x, rect.y + rect.a + 10, width, height},{0,1,0,1},true,0,0);
    GameWindow::requestRect({rect.x + width, rect.y + rect.a + 10, rect.z - width, height}, {1,0,0,1}, true, 0, 0);
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
    if (counter->getResource() >= 10)
    {
        Manager* manager = &(getManager());
        glm::vec2 center = getCenter();
        counter->setResource(-10);
        /*std::shared_ptr<Ant> ptr = ;
        std::cout << ptr.use_count() << std::endl;
        std::weak_ptr<Ant> weak = ptr;*/
        ants.emplace_back(manager->addAnt( *(new Ant({center.x - 5,center.y - 5,10,10},*this))));
    }
}


