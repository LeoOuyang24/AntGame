#include "friendlyAssemblers.h"
#include "animation.h"
#include "ants.h"
#include "game.h"
#include "effects.h"

ObjectAssembler::ObjectAssembler( std::string name_, const glm::vec2& rect_,AnimationWrapper* anime, bool mov, bool friendly_, int goldCost_) :
    dimen(rect_), name(name_), sprite(anime), movable(mov), friendly(friendly_), goldCost(goldCost_)
{

}

Object* ObjectAssembler::assemble()
{
    Object* obj= new Object(name,{0,0,dimen.x,dimen.y},sprite,movable);
    obj->setFriendly(friendly);
    return obj;
}


UnitAssembler::UnitAssembler( std::string name_,const glm::vec2& rect_, AnimationWrapper* wrap, bool mov, double maxHealth_, double prodTime_, int prodCost, bool friendly_, int goldCost) :
     ObjectAssembler( name_,rect_, wrap, mov,friendly_,goldCost), maxHealth(maxHealth_), prodTime(prodTime_), prodCost(prodCost)
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
    return new Unit(name,{0,0,dimen.x,dimen.y}, sprite, movable, maxHealth);
}

ProjectileAssembler::ProjectileAssembler(double damage_, std::string name_,const glm::vec2& rect_, AnimationWrapper* anime, double maxHealth_, double prodTime_, int prodCost, bool friendly_, int goldCost) :
                                        damage(damage_),UnitAssembler(name_,rect_,anime,true,maxHealth_,prodTime_,prodCost,friendly_,goldCost)
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
        player->addResource(1);
        alarm.set();
    }
    else if (!alarm.isSet())
    {
        alarm.set();
    }
}



AntAssembler::AntAssembler() : UnitAssembler("Ant",{20,20},&basicSoldierAnime,true,10, 0000,10,true,10)
{
}

Object* AntAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new Ant::AntMoveComponent(nullptr,.1,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new Ant::AntClickable(name,*ent)));
    ent->addRender((new AnimationComponent(*sprite,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->addComponent(*(new UnitAttackComponent(1,100,100,100,false,*ent)));
    ent->addComponent(*(new CommandableComponent(*ent)));
    return ent;
}


BlasterAssembler::BlasterRocket::ExplodingRocketComponent::ExplodingRocketComponent(const glm::vec2& target, const glm::vec2& pos, Unit& entity) :
                                                        ProjectileComponent(10,true,target,1,{pos.x,pos.y,16,8},entity), ComponentContainer<ExplodingRocketComponent>(entity)
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

BlasterAssembler::BlasterRocket::BlasterRocket() : ProjectileAssembler(1,"BlasterTankRocket",{8,16},&tankRocketAnime,1,1,1,true)
{

}

Object* BlasterAssembler::BlasterRocket::assemble()
{
    Unit* rocket = new Unit(movable);
    rocket->addRect((new BlasterRocket::ExplodingRocketComponent({0,0},{0,0},*rocket)));
    rocket->addClickable((new ClickableComponent(name,*rocket)));
    rocket->addRender((new AnimationComponent(*sprite,*rocket)));
    rocket->addHealth((new HealthComponent(*rocket,maxHealth)));
    return rocket;
}

BlasterAssembler::BlasterAssembler() : UnitAssembler("Blaster",{20,20},&blasterAnime,true,5,1000,10,true,20)
{
    addUnitToBucket(*this,allShopItems);
}

Object* BlasterAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new Ant::AntMoveComponent(nullptr,.3,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new Ant::AntClickable(name,*ent)));
    ent->addRender((new AnimationComponent(*sprite,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    //ent->addComponent(*(new UnitAttackComponent(1,1,300,300,!friendly,*ent)));
    ent->addComponent(*(new ProjectileAttackComponent(rocket,1000,300,300,!friendly,*ent)));
    ent->addComponent(*(new CommandableComponent(*ent)));
    return ent;
}

IncineratorAssembler::IncineratorAttackComponent::IncineratorAttackComponent(Entity& unit) : UnitAttackComponent(1,500,200,200,false,unit), ComponentContainer<IncineratorAttackComponent>(unit)
{

}

void IncineratorAssembler::IncineratorAttackComponent::attack(HealthComponent* health)
{
    health->addEffect(StatusEffect(damage,1000,[](StatusEffect& effect){
    effect.unit->getHealth().takeDamage(effect.value/1000.0*DeltaTime::deltaTime,*effect.source);},
    fireIcon,*static_cast<Unit*>(&health->getEntity()),*static_cast<Unit*>(&this->getEntity())));
}

IncineratorAssembler::IncineratorAssembler() : UnitAssembler("Incinerator",{30,30},&incineratorAnime,true,100,0,10,true,10)
{
    addUnitToBucket(*this,allShopItems);
    addUnitToBucket(*this,allUnits);
}

Object* IncineratorAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new Ant::AntMoveComponent(nullptr,.3,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new Ant::AntClickable(name,*ent)));
    ent->addRender((new AnimationComponent(*sprite,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    //ent->addComponent(*(new UnitAttackComponent(1,1,300,300,!friendly,*ent)));
    ent->addComponent(*(new IncineratorAttackComponent(*ent)));
    ent->addComponent(*(new CommandableComponent(*ent)));
    return ent;
}

FactoryAssembler::FactoryAssembler() : UnitAssembler("Factory",{30,30}, &defaultAnime, false, 100,1000,10,true,10)
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

TurretAssembler::TurretAssembler() : UnitAssembler("Turret", {30,30},&turretSprite,false,100,1000,5,true,10)
{
    addUnitToBucket(*this,allShopItems);
}

Object* TurretAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new MoveComponent(0,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new ClickableComponent(name,*ent)));
    ent->addRender((new AnimationComponent(*sprite,*ent)));
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

HealBuildingAssembler::HealBuildingAssembler() : UnitAssembler("HealBuilding",{30,30},&greenCross,false,100,1000,true,10)
{
    //addComponent(*(new HealUnitComponent(1,*this)));
}

Object* HealBuildingAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new MoveComponent(0,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new ClickableComponent(name,*ent)));
    ent->addRender((new AnimationComponent(*sprite,*ent)));
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
