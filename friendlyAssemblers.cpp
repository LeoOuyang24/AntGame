#include "friendlyAssemblers.h"
#include "animation.h"
#include "ants.h"
#include "game.h"
#include "effects.h"

ObjectAssembler::ObjectAssembler( std::string name_, const glm::vec2& rect_,const UnitAnimSet& anime, bool mov, bool friendly_, int goldCost_) :
    dimen(rect_), name(name_), sprites(anime), movable(mov), friendly(friendly_), goldCost(goldCost_)
{

}

Object* ObjectAssembler::assemble()
{
    Object* obj= new Object(name,{0,0,dimen.x,dimen.y},sprites.walking,movable);
    obj->setFriendly(friendly);
    return obj;
}


UnitAssembler::UnitAssembler( std::string name_,const glm::vec2& rect_, const UnitAnimSet& wrap, bool mov, double maxHealth_, float speed, double prodTime_, int prodCost, bool friendly_, int goldCost) :
     ObjectAssembler( name_,rect_, wrap, mov,friendly_,goldCost), maxHealth(maxHealth_), speed(speed), prodTime(prodTime_), prodCost(prodCost)
{
        if (movable) //if is a unit, add to units
        {
            addUnitToBucket(*this,allUnits);
        }
        else //add to structures
        {
            addUnitToBucket(*this,allStructures);
        }
}

Object* UnitAssembler::assemble()
{

    return new Unit(name,{0,0,dimen.x,dimen.y}, sprites.walking, movable, maxHealth);
}

Unit* UnitAssembler::commandUnitAssemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new Ant::AntMoveComponent(nullptr,speed,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new Ant::AntClickable(name,*ent)));
    ent->addRender((new UnitAnimationComponent(sprites,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->addComponent(*(new CommandableComponent(*ent)));
    ent->addComponent(*(new ForcesComponent(*ent)));
    ent->setFriendly(friendly);
    return ent;
}

ProjectileAssembler::ProjectileAssembler(double damage_, std::string name_,const glm::vec2& rect_, AnimationWrapper* anime, double maxHealth_, float speed, double prodTime_, int prodCost, bool friendly_, int goldCost) :
                                        damage(damage_),UnitAssembler(name_,rect_,{anime},true,maxHealth_,speed, prodTime_,prodCost,friendly_,goldCost)
{

}

Object* ProjectileAssembler::assemble(Object& shooter, const glm::vec2& point, const glm::vec2& target)
{
    Object* obj = assemble();
    obj->getRect().setPos(point);
    obj->getComponent<MoveComponent>()->setTarget(target);
    obj->getComponent<ProjectileComponent>()->setShooter(shooter);
    return obj;
}

CreateEnergyComponent::CreateEnergyComponent(Player& player_, int frames, Entity& entity) : player(&player_), waitTime(frames), Component(entity), ComponentContainer<CreateEnergyComponent>(entity)
{

}

void CreateEnergyComponent::update()
{
    if (player && alarm.framesPassed(waitTime))
    {
        GameWindow::staticAddPanel(*(new Ticker(1000,{entity->getComponent<RectComponent>()->getRect()},&resourceAnime,{""},&Font::tnr,{0,1,0,1},nullptr,GameWindow::interfaceZ)),false);
        player->addResource(1);
        alarm.set();
    }
    else if (!alarm.isSet())
    {
        alarm.set();
    }
}



AntAssembler::AntAssembler() : UnitAssembler("Ant",{20,20},{&basicSoldierAnime,&basicShootingAnime},true,10,.1, 0000,0,true,10)
{
}

Object* AntAssembler::assemble()
{
    Object* ent = commandUnitAssemble();
    ent->addComponent(*(new UnitAttackComponent(1,100,100,100,false,*ent)));
    return ent;
}


BlasterAssembler::BlasterRocket::ExplodingRocketComponent::ExplodingRocketComponent(const glm::vec2& target, const glm::vec2& pos, Unit& entity) :
                                                        ProjectileComponent(10,true,target,speed,{pos.x,pos.y,16,8},entity), ComponentContainer<ExplodingRocketComponent>(entity)
{

}

void BlasterAssembler::BlasterRocket::ExplodingRocketComponent::onCollide(Unit& other)
{
    auto vec = GameWindow::getLevel()->getTree()->getNearest(other.getComponent<RectComponent>()->getCenter(),100);
    int size = vec.size();
   // std::cout << size << std::endl;
    for (int i = 0; i < size; ++i)
    {
        ProjectileComponent::onCollide(*convertPosToUnit(vec[i]));
    }
    glm::vec4 rect = entity->getComponent<RectComponent>()->getRect();
     //   glm::vec4 (RenderCamera::*transform) (const glm::vec4&) const = &(GameWindow::getCamera().toScreen);
    explosionAnime.request({{rect.x - 50,rect.y - 50,100,100}},{-1,.01,1,&(GameWindow::getCamera().toScreen),&GameWindow::getCamera()});
}

BlasterAssembler::BlasterRocket::BlasterRocket() : ProjectileAssembler(1,"BlasterTankRocket",{8,16},{&tankRocketAnime},1,.1,1,1,true)
{

}

Object* BlasterAssembler::BlasterRocket::assemble()
{
    Unit* rocket = new Unit(movable);
    rocket->addRect((new BlasterRocket::ExplodingRocketComponent({0,0},{0,0},*rocket)));
    rocket->addClickable((new ClickableComponent(name,*rocket)));
    rocket->addRender((new UnitAnimationComponent(sprites,*rocket)));
    rocket->addHealth((new HealthComponent(*rocket,maxHealth)));
    return rocket;
}

BlasterAssembler::BlasterAssembler() : UnitAssembler("Blaster",{20,20},{&blasterAnime},true,5,1000,10,true,20)
{
    addUnitToBucket(*this,allShopItems);
}

Object* BlasterAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new Ant::AntMoveComponent(nullptr,.3,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new Ant::AntClickable(name,*ent)));
    ent->addRender((new UnitAnimationComponent(sprites,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    //ent->addComponent(*(new UnitAttackComponent(1,1,300,300,!friendly,*ent)));
    ent->addComponent(*(new ProjectileAttackComponent(rocket,1000,300,300,!friendly,*ent)));
    ent->addComponent(*(new CommandableComponent(*ent)));
    ent->addComponent(*(new ForcesComponent(*ent)));

    return ent;
}

IncineratorAssembler::IncineratorAttackComponent::IncineratorAttackComponent(Entity& unit) : UnitAttackComponent(1,500,200,200,false,unit), ComponentContainer<IncineratorAttackComponent>(unit)
{

}

void IncineratorAssembler::IncineratorAttackComponent::attack(HealthComponent* health)
{
    health->addEffect(StatusEffect(modData.damage,1000,[](StatusEffect& effect){
    effect.unit->getHealth().takeDamage(effect.value/1000.0*DeltaTime::deltaTime,*effect.source);},
    fireIcon,*static_cast<Unit*>(&health->getEntity()),*static_cast<Unit*>(&this->getEntity())));
}

IncineratorAssembler::IncineratorAssembler() : UnitAssembler("Incinerator",{30,30},{&incineratorAnime},true,100,.3,0,10,true,10)
{
    addUnitToBucket(*this,allShopItems);
    addUnitToBucket(*this,allUnits);
}

Object* IncineratorAssembler::assemble()
{
    Object* ent = UnitAssembler::commandUnitAssemble();
    ent->addComponent(*(new IncineratorAttackComponent(*ent)));
    return ent;
}

FreezerAssembler::FreezerAttackComponent::FreezerAttackComponent(Entity& unit) : UnitAttackComponent(10,200,100,100,false,unit), ComponentContainer<FreezerAttackComponent>(unit)
{

}

void FreezerAssembler::FreezerAttackComponent::attack(HealthComponent* health)
{
    UnitAttackComponent::attack(health);
    health->addEffect(chillEffect.getEffect(slowAmount,1000,*static_cast<Unit*>(&health->getEntity()),*static_cast<Unit*>(&this->getEntity())));
}

FreezerAssembler::FreezerAssembler() : UnitAssembler("Cryogenics Unit",{30,30},{&freezeUnitAnime},true,100,.1,0)
{
    addUnitToBucket(*this,allShopItems);
    addUnitToBucket(*this,allUnits);
}

Object* FreezerAssembler::assemble()
{
     Object* ent = UnitAssembler::commandUnitAssemble();
    //ent->addComponent(*(new UnitAttackComponent(1,1,300,300,!friendly,*ent)));
    ent->addComponent(*(new FreezerAttackComponent(*ent)));

    return ent;
}

MercenaryAssembler::MercenaryAttackComponent::MercenaryAttackComponent(Entity& unit) : UnitAttackComponent(100,100,100,100,false,unit)
{

}

bool MercenaryAssembler::MercenaryAttackComponent::canAttack(Object* ptr)
{
    return UnitAttackComponent::canAttack(ptr);
}

void MercenaryAssembler::MercenaryAttackComponent::attack(HealthComponent* health)
{
    if (GameWindow::getPlayer().getResource() >= 10)
    {

        GameWindow::getPlayer().addResource(-10);
        UnitAttackComponent::attack(health);
    }
}

MercenaryAssembler::MercenaryAssembler() : UnitAssembler("Mercenary",{20,20},{&mercenaryAnime},true,100,.1,1000,10,true,50)
{
    addUnitToBucket(*this,allShopItems);
    addUnitToBucket(*this,allUnits);
}

Object* MercenaryAssembler::assemble()
{
    Unit* ent = commandUnitAssemble();
    ent->addComponent(*(new MercenaryAttackComponent(*ent)));
    return ent;
}

MinigunUserAssembler::MinigunHypeComponent::MinigunHypeComponent(Entity& unit) : Component(unit)
{

}

void MinigunUserAssembler::MinigunHypeComponent::update()
{
    Unit* owner = static_cast<Unit*>(entity);
    auto vec = GameWindow::getLevel()->getTree()->getNearest(owner->getRect().getCenter(),100);
    auto end = vec.end();
    for (auto it = vec.begin(); it != end; ++it)
    {
        Unit* unit = convertPosToUnit(*it);
        if (unit->getFriendly() == owner->getFriendly() && unit != owner && unit->getMovable())
        {
            owner->getHealth().addEffect(StatusEffect(.1,1,[](StatusEffect& effect){
                                                      auto attack = effect.unit->getComponent<AttackComponent>();
                                                      attack->setAttackSpeed(.1);
                                                      },tankRocketAnime,*owner,*owner));
        }
    }

}

MinigunUserAssembler::MinigunUserAssembler() : UnitAssembler("Minigun Enthusiast",{20,20},{&minigunEnthAnime},true,20,.1,1000)
{
    addUnitToBucket(*this,allShopItems);
}

Object* MinigunUserAssembler::assemble()
{
    Unit* ent = commandUnitAssemble();
    ent->addComponent(*(new MinigunHypeComponent(*ent)));
    ent->addComponent(*(new UnitAttackComponent(10,100,100,100,false,*ent)));
    return ent;
}

ShrimpSuitOperator::ShrimpSuitOperator() : UnitAssembler("Shrimp Suit Operator", {25,25}, {&shrimpSuitAnime},true,300,.05,1000,50,true,50)
{
    addUnitToBucket(*this, allShopItems);
}

Object* ShrimpSuitOperator::assemble()
{
    Unit* unit = commandUnitAssemble();
    unit->addComponent(*(new UnitAttackComponent(1,1,1,100,false,*unit)));
    return unit;
}

SuperMegaTank::NukeComponent::NukeComponent(const glm::vec2& target, const glm::vec4& rect, Unit& entity) : ProjectileComponent(1000,true,target,0,rect,entity)
{
    timer.set();
}

void SuperMegaTank::NukeComponent::update()
{
    if (!tangible && timer.timePassed(1000))
    {
        tangible = true;
        timer.set();
    }
    else if (tangible && timer.timePassed(500))
    {
        static_cast<Unit*>(entity)->setDead(true);
    }
}

void SuperMegaTank::NukeComponent::collide(Entity& other)
{
    if (tangible)
    {
        ProjectileComponent::collide(other);
    }
}

SuperMegaTank::NukeAssembler::NukeAssembler() : ProjectileAssembler(0,"nuke",{100,100},{&radiation},1,0,0)
{

}

Object* SuperMegaTank::NukeAssembler::assemble()
{
    Unit* rocket = new Unit(movable);
    rocket->addRect((new SuperMegaTank::NukeComponent({0,0},{0,0,dimen.x,dimen.y},*rocket)));
    rocket->addClickable((new ClickableComponent(name,*rocket)));
    rocket->addRender((new UnitAnimationComponent(sprites,*rocket)));
    rocket->addHealth((new HealthComponent(*rocket,maxHealth)));
    return rocket;
}

SuperMegaTank::SMTankComponent::SMTankComponent(Unit& entity) : UnitAttackComponent(110,200,300,300,false,entity)
{

}

void SuperMegaTank::SMTankComponent::attack(HealthComponent* health)
{
    AttackComponent::attack(health);
    counter ++;
    if (counter > 3)
    {
        glm::vec2 center = static_cast<Object*>(&health->getEntity())->getRect().getCenter();
        counter = 0;
        GameWindow::getLevel()->addUnit(*nukeAssembler.assemble(),center.x,center.y,true);
    }
}

SuperMegaTank::SuperMegaTank() : UnitAssembler("Super Mega Tank",{50,50},{&tankAnime},true,1000,.05,10,10,true,10)
{
    addUnitToBucket(*this,allShopItems);
}

Object* SuperMegaTank::assemble()
{
    Unit* unit = commandUnitAssemble();
    unit->addComponent(*(new SMTankComponent(*unit)));
    return unit;
}

CommanderAssembler::CommanderComponent::CommanderComponent(Unit& unit) : Component(unit), ComponentContainer<CommanderComponent>(unit)
{

}

void CommanderAssembler::CommanderComponent::update()
{
    const int radius = 100;
    glm::vec2 center = entity->getComponent<RectComponent>()->getCenter();
    auto vec = GameWindow::getLevel()->getTree()->getNearest(center,radius);
    auto end = vec.end();

    for (auto it = vec.begin(); it != end; ++it)
    {
        Unit* unit = convertPosToUnit(*it);
        if (unit->getFriendly() == static_cast<Unit*>(entity)->getFriendly() && unit->getMovable() && !unit->getComponent<CommanderComponent>())
        {
            unit->getHealth().addArmor(5);
        }
    }
    //std::cout << "Commander\n";
    GameWindow::requestNGon(100,center,2*radius*cos(98 * 180/ 100*M_PI/180/2),{.1,.5,1,1},0,false,0);
}

CommanderAssembler::CommanderAssembler() : UnitAssembler("Commander",{20,20},{&commanderAnime},true,100,.1,0)
{
    addUnitToBucket(*this, allShopItems);
}

Object* CommanderAssembler::assemble()
{
    Unit* obj = commandUnitAssemble();
    obj->addComponent(* (new CommanderComponent(*obj)));
    obj->addComponent(*(new UnitAttackComponent(1,100,100,100,false,*obj)));
    return obj;
}

IceBlasterAssembler::IceBlasterAttackComponent::IceBlasterAttackComponent(Unit& unit) : UnitAttackComponent(1,1,100,100,false,unit)
{

}

void IceBlasterAssembler::IceBlasterAttackComponent::attack(HealthComponent* health)
{
    if (health)
    {
        Unit* unit = static_cast<Unit*>(entity);
        health->takeDamage(modData.damage,*unit);
        health->addEffect(chillEffect.getEffect(.1f,2000,static_cast<Unit&>(health->getEntity()),*unit));
    }
}

IceBlasterAssembler::IceBlasterAssembler() : UnitAssembler("Ice Laser",{30,30},{&iceTurretAnime},false,100,0,1000,5,true,10)
{
    addUnitToBucket(*this,allShopItems);
}

Object* IceBlasterAssembler::assemble()
{
    Unit* ent = commandUnitAssemble();
    ent->addComponent(*(new IceBlasterAttackComponent(*ent)));
    return ent;
}

FactoryAssembler::FactoryAssembler() : UnitAssembler("Factory",{30,30}, {&defaultAnime}, false, 100,0,1000,10,true,10)
{
    addUnitToBucket(*this,allShopItems);
}

Object* FactoryAssembler::assemble()
{
    Unit* stru = static_cast<Unit*>(UnitAssembler::assemble());
    stru->setFriendly(true);
    stru->addComponent(*(new CreateEnergyComponent(GameWindow::getPlayer(),1000,*stru)));

    return stru;
}

TurretAssembler::TurretAssembler() : UnitAssembler("Turret", {30,30},{&turretSprite},false,100,0,1000,5,true,10)
{
    addUnitToBucket(*this,allShopItems);
}

Object* TurretAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new MoveComponent(0,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new ClickableComponent(name,*ent)));
    ent->addRender((new UnitAnimationComponent(sprites,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->setFriendly(true);
    ent->addComponent(*(new UnitAttackComponent(1,.1,100,100,false,*ent)));
    return ent;
}

HealUnitComponent::HealUnitComponent(double rate, double radius, Entity& ent) : Component(ent),HPps(rate), radius(radius)
{
    healthTimer.set();
}

void HealUnitComponent::update()
{
    if (entity && entity->getComponent<RectComponent>())
    {
        Unit* owner = static_cast<Unit*>(entity);
        glm::vec4 rect = owner->getRect().getRect();
        glm::vec2 center = {rect.x + rect.z/2, rect.y + rect.a/2};
        PolyRender::requestCircle({0,1,0,1},GameWindow::getCamera().toScreen(center),radius,1);
       //PolyRender::requestNGon(8,GameWindow::getCamera().toScreen(center),10,{0,1,0,1},0,false,0);
        if (healthTimer.timePassed(1000))
        {
            auto nearby = GameWindow::getLevel()->getTree()->getNearest(center,radius);
            for (int i = 0; i < nearby.size(); ++i)
            {
                Unit* ptr = convertPosToUnit(nearby[i]);
                if (ptr->getFriendly() && !ptr->getComponent<HealUnitComponent>())
                {
                    ptr->getHealth().takeDamage(-1*HPps,*owner);
                }
            }

        }
    }
}

HealBuildingAssembler::HealBuildingAssembler() : UnitAssembler("HealBuilding",{30,30},{&greenCross},false,100,0,1000,true,10)
{
    //addComponent(*(new HealUnitComponent(1,*this)));
    addUnitToBucket(*this,allShopItems);
    addUnitToBucket(*this,allStructures);
    addUnitToBucket(*this, allUnits);
}

Object* HealBuildingAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new MoveComponent(0,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new ClickableComponent(name,*ent)));
    ent->addRender((new UnitAnimationComponent(sprites,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->setFriendly(true);
    ent->addComponent(*(new HealUnitComponent(1,100,*ent)));
    return ent;
}

UnitBucket allUnits; //vector of all units and structures
UnitBucket allStructures;
UnitBucket allShopItems; //list of purchasable units

FactoryAssembler factAssembler;
AntAssembler antAssembler;
TurretAssembler turretAssembler;
HealBuildingAssembler healBuildingAssembler;
BlasterAssembler blastAssembler;
IncineratorAssembler incineratorAssembler;
FreezerAssembler freezerAssembler;
MercenaryAssembler mercenaryAssembler;
MinigunUserAssembler minigunUserAssembler;
ShrimpSuitOperator shrimpSuitOperator;
SuperMegaTank superMegaTank;
CommanderAssembler commanderAssembler;
IceBlasterAssembler iceBlasterAssembler;

UnitAssembler* getRandomAssembler(UnitBucket& bucket)
{
    int it = rand()%(bucket.size());
//    std::cout << it << std::endl;
    return bucket[it];
}

void addUnitToBucket(UnitAssembler& ass, UnitBucket& bucket)
{
    bucket.push_back(&ass);
}
