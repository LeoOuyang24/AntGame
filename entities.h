#ifndef ENTITIES_H_INCLUDED
#define ENTITIES_H_INCLUDED

#include "components.h"
#include "world.h"

void renderMeter(const glm::vec3& xyWidth, const glm::vec4& color, double current, double maximum, float z);

class Unit;


class HealthComponent : public Component, public ComponentContainer<HealthComponent>
{
    double health = 0;
    double maxHealth = 0;
    int displacement = 0; //height above the entity
    bool visible = true;
    DeltaTime invincible; //keeps track of how many frames of invincibility
public:
    HealthComponent(Entity& entity, double health_,  int displacement_ = 20) : Component(entity), ComponentContainer<HealthComponent>(entity), health(health_), maxHealth(health_), displacement(displacement_)//height defaults to 10 and displacement defaults to 20
    {

    }
    void addHealth(int amount); //increases health by amount. Health cannot exceed maxHealth nor go below 0
    double getHealth()
    {
        return health;
    }
    double getMaxHealth()
    {
        return maxHealth;
    }
    void update(); //render health bar and reset invincibility frames
    void render(const glm::vec3& rect, float z); //renders the healthbar at the given location with the given width. The height will always be height so rect.a is the z value to render at.
    void inline setVisible(bool value)
    {
        visible = value;
    }
};

class Ant;
class Manager;
class Unit : public Object //Units can be clicked on and seen and have health
{
protected:
    HealthComponent* health = nullptr;
    Manager* manager = nullptr;
public:
    Unit(ClickableComponent& click, RectComponent& rect, RenderComponent& render, HealthComponent& health);

    HealthComponent& getHealth()
    {
        return *health;
    }
    bool clicked();
    virtual void interact(Ant& ant);
    Manager& getManager();
    void setManager(Manager& manager);

};


class AttackComponent : public Component, public ComponentContainer<AttackComponent>
{
    float damage = 0;
    int endLag = 0; //how much time must pass before the attack can be reused.
    DeltaTime attackTimer;
public:
    AttackComponent(float damage_, int endLag_, Unit& unit) : Component(unit), ComponentContainer<AttackComponent>(unit),  damage(damage_), endLag(endLag_)
    {

    }
    void attack(HealthComponent* health); //this is a pointer so you can legally pass in a null pointer. This function will make sure it's not null
};

class ResourceComponent : public Component, public ComponentContainer<ResourceComponent> //represents the amount of resources this object has
{
protected:
    double resource;
    int maxResource;
public:
    ResourceComponent(int amount, Entity& entity);
    int getResource()
    {
        return resource;
    }
    int getMaxResource()
    {
        return maxResource;
    }
    void setResource(double amount)
    {
        resource = std::max(std::min(resource + amount, (double)maxResource),0.0);
    }
    void collect(Unit& other);
    void render(const glm::vec3& rect, float z);
};

class CorpseComponent : public Component, public ComponentContainer<CorpseComponent> //the corpse component spawns a corpse object after the entity dies
{
    int amount = 0;
    RenderComponent* render = nullptr;
public:
    CorpseComponent(Unit& unit, int amount_) : Component(unit), ComponentContainer<CorpseComponent>(unit), amount(amount_), render(unit.getComponent<RenderComponent>())
    {

    }
    void onDeath();
};

class ResourceUnit : public Unit
{
public:
    ResourceUnit(int resources, const glm::vec4& rect);
};

class WanderMove : public MoveComponent, public ComponentContainer<WanderMove>
{
public:
    WanderMove(double speed, const glm::vec4& rect, Entity& unit) : MoveComponent(speed, rect, unit), ComponentContainer<WanderMove>(unit)
    {

    }
    void update();
};



class ApproachComponent : public Component, public ComponentContainer<ApproachComponent> //a component that can move towards a unit. It is NOT a movecomponent because it by itself cannot function. If the targetUnit is null, it uses the owning entity's movecomponent to decide what to do.
{
protected:
    glm::vec2 displacement = {0,0};
    std::weak_ptr<Unit> targetUnit;
    MoveComponent* move = nullptr;
    template<typename T>
    Unit* findNearestUnit(double radius); //finds the nearest unit that has a component T in radius radius. Returns null if none found
public:
    ApproachComponent(Unit& entity) : Component(entity), ComponentContainer<ApproachComponent>(entity), move(entity.getComponent<MoveComponent>())
    {

    }
    virtual void setTarget(const glm::vec2& target, const std::shared_ptr<Unit>* unit);
    virtual void setTarget(const std::shared_ptr<Unit>& unit);
    virtual void update();
    void setMove(MoveComponent& move_)
    {
        move = &move_;
    }
    Unit* getTargetUnit()
    {
        return targetUnit.lock().get();
    }
};

class Mushroom : public Unit
{
public:
    Mushroom(int x, int y);
};

class Bug : public Unit
{
public:
    Bug(int x, int y);
};

class Beetle : public Unit
{
    class BeetleMove : public ApproachComponent, public ComponentContainer<BeetleMove> // finds and attacks the nearest ant
    {
    public:
        BeetleMove(Unit& unit) : ApproachComponent(unit), ComponentContainer<BeetleMove>(unit)
        {

        }
        void update();
    };
public:
    Beetle(int x, int y);
};

class ResourceEatComponent : public ApproachComponent, public ComponentContainer<ResourceEatComponent> //finds and eats the nearest ResourceCountComponent, including anthills
{
public:
    ResourceEatComponent(Unit& unit) : ApproachComponent(unit), ComponentContainer<ResourceEatComponent>(unit)
    {

    }
    void update();
};

#endif // ENTITIES_H_INCLUDED
