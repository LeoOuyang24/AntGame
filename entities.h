#ifndef ENTITIES_H_INCLUDED
#define ENTITIES_H_INCLUDED

#include "glInterface.h"
#include "SDLhelper.h"

#include "components.h"


class Unit;
class ClickableComponent : public Component, public ComponentContainer<ClickableComponent> //clickable component handles user inputs, like when the user selects the unit or presses a button
{
    static const glm::vec2 spacing; //spacing between the buttons and the unit
    std::string name = "";
    bool clicked = false;
    std::vector<std::unique_ptr<Button>> buttons;
public:
    ClickableComponent(std::string name, Entity& entity) : Component(entity), ComponentContainer<ClickableComponent>(entity), name(name)
    {

    }
    virtual void update();
    void click(bool val);
    bool getClicked();
    void addButton(Button& button);
    virtual void display(const glm::vec4& rect);
};

class RectRenderComponent : public RenderComponent, public ComponentContainer<RectRenderComponent>
{
    glm::vec4 color;
public:
    RectRenderComponent(const glm::vec4& color, Entity& unit) : RenderComponent(unit), ComponentContainer<RectRenderComponent>(unit), color(color)
    {

    }
    void update();
};


class HealthComponent : public Component, public ComponentContainer<HealthComponent>
{
    double health = 0;
    double maxHealth = 0;
    int height = 0; //height of the health bar
    int displacement = 0; //height above the entity
    bool visible = true;
    DeltaTime invincible; //keeps track of how many frames of invincibility
public:
    HealthComponent(Entity& entity, double health_, int height_ = 10, int displacement_ = 20) : Component(entity), ComponentContainer<HealthComponent>(entity), health(health_), maxHealth(health_), height(height_), displacement(displacement_)//height defaults to 10 and displacement defaults to 20
    {

    }
    void addHealth(int amount); //increases health by amount. Health cannot exceed maxHealth nor go below 0
    int getHealth();
    void update(); //render health bar and reset invincibility frames
    void inline setVisible(bool value)
    {
        visible = value;
    }
};

class Ant;
class Manager;
class Unit : public Entity //Units can be clicked on and seen
{
protected:
    bool dead = false;
    ClickableComponent* clickable = nullptr;
    RectComponent* rect = nullptr;
    RenderComponent* render = nullptr;
    HealthComponent* health = nullptr;
    Manager* manager = nullptr;
public:
    Unit(ClickableComponent& click, RectComponent& rect, RenderComponent& render, HealthComponent& health);
    RectComponent& getRect();
    ClickableComponent& getClickable();
    RenderComponent& getRender();
    bool clicked();
    virtual void interact(Ant& ant);
    glm::vec2 getCenter();
    Manager& getManager();
    void setManager(Manager& manager);
    bool inline getDead()
    {
        return dead;
    }
    void inline setDead(bool isDead)
    {
        dead = isDead;
    }

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

class ResourceComponent : public Component, public ComponentContainer<ResourceComponent> //when Ants collide with this unit, they collect resources
{
protected:
    int amount;
public:
    ResourceComponent(Entity& entity, int amount) : Component(entity),ComponentContainer<ResourceComponent>(entity), amount(amount)
    {

    }
    virtual void collect(Ant& other);
};

class CorpseComponent : public ResourceComponent, public ComponentContainer<CorpseComponent> //the corpse component handles everything about the entity after death
{
    RenderComponent* render = nullptr;
public:
    CorpseComponent(Unit& unit, int amount) : ResourceComponent(unit, amount), ComponentContainer<CorpseComponent>(unit), render(unit.getComponent<RenderComponent>())
    {

    }
    void collect(Entity& other);
    void update();
};

class Resource : public Unit
{
    int amount;
    class ResourceRender : public RenderComponent, public ComponentContainer<ResourceRender>
    {
    public:
        ResourceRender(Entity& entity) : RenderComponent(entity), ComponentContainer<ResourceRender>(entity)
        {

        }
        void update();
    };
public:
    Resource(int x, int y, int amount);
    void interact(Ant& ant);
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
public:
    ApproachComponent(Unit& entity) : Component(entity), ComponentContainer<ApproachComponent>(entity), move(entity.getComponent<MoveComponent>())
    {

    }
    virtual void setTarget(const glm::vec2& target, const std::shared_ptr<Unit>* unit);
    virtual void setTarget(const std::shared_ptr<Unit>& unit);
    void setMove(MoveComponent& move_)
    {
        move = &move_;
    }
    virtual void update();
    Unit* getTargetUnit()
    {
        return targetUnit.lock().get();
    }
};

class Bug : public Unit
{
public:
    Bug(int x, int y);
};

class Beetle : public Unit
{
    class BeetleMove : public ApproachComponent, public ComponentContainer<BeetleMove>
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

#endif // ENTITIES_H_INCLUDED
