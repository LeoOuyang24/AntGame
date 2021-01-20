#define _USE_MATH_DEFINES
#include <cmath>

#include "render.h"
#include "vanilla.h"

#include "ants.h"
#include "world.h"
#include "game.h"
#include "animation.h"


CommandableComponent::CommandableComponent(Entity& entity) : Component(entity), ComponentContainer<CommandableComponent>(entity)
{

}

void CommandableComponent::setCurrentTask(AntManager* task)
{
    currentTask = task;
}

AntManager* CommandableComponent::getCurrentTask()
{
    return currentTask;
}

const short Ant::dimen = 20;
Ant::AntMoveComponent::AntMoveComponent(Anthill* hill, double speed, const glm::vec4& rect, Entity& entity) : PathComponent(speed,rect,entity), ComponentContainer<Ant::AntMoveComponent>(entity), home(hill)
{

}

void Ant::AntMoveComponent::collide(Entity& other)
{
    auto force = other.getComponent<ForcesComponent>();
   // std::cout << entity << " " << &other << std::endl;
    /*if (&other == home && carrying > 0)
    {
        home->getComponent<ResourceComponent>()->setResource(carrying);
        carrying = 0;
    }*/
    if (force)
    {
        glm::vec2 otherCenter = other.getComponent<RectComponent>()->getCenter();
        glm::vec2 center = getCenter();
        float angle = atan2(otherCenter.y - center.y, otherCenter.x - center.x) + (M_PI/4*(center.x < otherCenter.x));
        force->addForce({angle - (M_PI/8),1});
        /*PolyRender::requestLine(glm::vec4(
                                          GameWindow::getCamera().toScreen(getCenter()),
                                          GameWindow::getCamera().toScreen(getCenter() + glm::vec2(cos(-M_PI/4)*100.0,sin(-M_PI/4)*100.0))
                                          ),
                                {1,1,1,1},10
                                );*/
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
  /*  if (KeyManager::getJustPressed() == SDLK_1)
    {
        entity->getComponent<ForcesComponent>()->addForce({-M_PI/4,.1});

    }*/
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
    PolyRender::requestRect(param.rect,param.tint,true,param.radians,param.z);
}

Ant::AntRenderComponent::~AntRenderComponent()
{

}

Ant::AntClickable::AntClickable(std::string name, Entity& entity) : ClickableComponent(name, entity), ComponentContainer<AntClickable>(entity)
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

Ant::Ant(const glm::vec4& rect, Anthill& home) : Unit(*(new ClickableComponent("Ant",*this)), *(new AntMoveComponent(&home,.5,rect,*this)),
                                                      *(new AnimationComponent((basicSoldierAnime),*this)), *(new HealthComponent(*this, 10)))
{
    health->setVisible(false);
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
    render({GameWindow::getCamera().toScreen(rect)});

}

void AntHillRender::render(const SpriteParameter& param)
{
    PolyRender::requestNGon(8,{param.rect.x + param.rect.z/2, param.rect.y + param.rect.a/2},(std::min(param.rect.z,param.rect.a)*sqrt(2))/(2 + sqrt(2)),
                            {.5,.5,.5,1},param.radians,true,param.z);
}

AntHillRender::~AntHillRender()
{

}

ProduceUnitButton::ProduceUnitButton(UnitAssembler& obj, ProduceUnitComponent* own) : assembler(&obj),owner(own), Button({0,0,64,16},nullptr,nullptr,
                                                                                                           {"Create " + obj.name,{0,0,0,0},0,{0,0,0,1},GameWindow::fontZ},&Font::tnr,
                                                                                                           {0,1,0,1})
{

}

void ProduceUnitButton::press()
{
    if (owner && assembler)
    {
        owner->produceUnit(*assembler);
    }
}

ProduceUnitComponent::ProduceUnitComponent(std::string name, Unit& entity) : ClickableComponent(name, entity), ComponentContainer<ProduceUnitComponent>(entity)
{
    auto units = &(GameWindow::getPlayer().getUnits());
}

void ProduceUnitComponent::produceUnit(UnitAssembler& assembler)
{
    if (toProduce.size() < 10 && GameWindow::getPlayer().getResource() >= assembler.prodCost)
    {
        toProduce.push_back(&assembler);
    }
}

void ProduceUnitComponent::update()
{
    ClickableComponent::update();
    if (beingProduced && produceTimer.timePassed(beingProduced->prodTime) && GameWindow::getPlayer().getResource() >= beingProduced->prodCost)  //if something is in production and is done, put it into the real world
    {
        auto vec4 = entity->getComponent<RectComponent>()->getRect();
        glm::vec2 center = {vec4.x + vec4.z + beingProduced->dimen.x/2, vec4.y + vec4.a + beingProduced->dimen.y/2};

        Map* level = GameWindow::getLevel();
        if (level)
        {
            level->addUnit(*(beingProduced->assemble()),center.x +5*cos(rand()%360/180.0*M_PI),center.y +5*sin(rand()%360/180.0*M_PI), true);
            GameWindow::getPlayer().addResource(-1*beingProduced->prodCost);
        }

        beingProduced = nullptr;
        toProduce.pop_front();
    }
    if (toProduce.size() > 0 && !beingProduced) //if nothing is in production and we have units to produce, set the next unit as the one to produce.
    {
        beingProduced = toProduce.front();
        produceTimer.set();
    }
    if (beingProduced)
    {
        glm::vec4 rect = entity->getComponent<RectComponent>()->getRect();
        PolyRender::requestRect(GameWindow::getCamera().toScreen({rect.x,rect.y + rect.a + 10,
                                                                 (SDL_GetTicks() - produceTimer.getTime())/beingProduced->prodTime*rect.z, 13}),
                                                                {.5,.5,.5,1},true,0,0);
    }

}

void ProduceUnitComponent::display(const glm::vec4& rect)
{
    int size = toProduce.size();
    for (int it = 0; it < size; ++it)
    {
        glm::vec4 renderRect = GameWindow::getCamera().toAbsolute({rect.x + .1*rect.z,rect.y + .1*rect.a*(it),.1*rect.a,.1*rect.a});
        toProduce[it]->sprite->request({renderRect,0,NONE,
                                            {1,1,1,1},&RenderProgram::basicProgram,GameWindow::fontZ} ,{});
        if (it == 0)
        {
              renderTimeMeter({renderRect.x+renderRect.z+.1*rect.a, renderRect.y,
                    rect.z - (renderRect.x + renderRect.z + .1*rect.a), .1*rect.a},
                    {.5,.5,.5,1},produceTimer,beingProduced->prodTime,GameWindow::interfaceZ);
        }
    }
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
    std::cout << " This function has been removed!" << std::endl;
}

Anthill::Anthill(const glm::vec2& pos) : Structure(*(new ProduceUnitComponent("Anthill",*this)), *(new RectComponent({pos.x,pos.y,64,64}, *this)),
                                                *(new AntHillRender(*this)), *(new HealthComponent(*this, 100)))
{
    this->setFriendly(true);
 //   getClickable().addButton(*(new CreateAnt(*this)));
  //  getClickable().addButton(*(new StartSignal(*this)));
    //addComponent(*(new ResourceComponent(1000,*this)));

}

void Anthill::createAnt()
{
    ResourceComponent* counter = getComponent<ResourceComponent>();
    if (counter->getResource() >= 1)
    {
        auto vec4 = getRect().getRect();
        glm::vec2 center = {vec4.x + vec4.z + 20, vec4.y + vec4.a + 20};
        counter->setResource(-1);
        /*std::shared_ptr<Ant> ptr = ;
        std::cout << ptr.use_count() << std::endl;
        std::weak_ptr<Ant> weak = ptr;*/
        GameWindow::getLevel()->addUnit( *(new Ant({center.x +5*cos(rand()%360/180.0*M_PI),center.y +5*sin(rand()%360/180.0*M_PI),20,20},*this)), true);
        //std::cout << ants.size() << std::endl;
    }
}

void Anthill::setButtons()
{

    auto units = &(GameWindow::getPlayer().getUnits());
    //std::cout << units->size() << std::endl;
    ProduceUnitComponent* produce = getComponent<ProduceUnitComponent>();
    if (produce)
    {
        for (auto i = units->begin(); i != units->end(); ++i)
        {
            produce->addButton(*(new ProduceUnitButton(**i,produce)));
        }
    }
    else
    {
        std::cout << "No ProduceUnitComponent in Anthill!" << std::endl;
    }

}

Anthill::~Anthill()
{
    std::cout << "Anthill deletion" << std::endl;
}

