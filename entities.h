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
    ClickableComponent(std::string name, Unit& entity);
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
    RectRenderComponent(const glm::vec4& color, Unit& unit);
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
    HealthComponent(Entity& entity, double h, int height = 10, int displacement = 20); //height defaults to 10 and displacement defaults to 20
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

class ResourceComponent : public Component, public ComponentContainer<ResourceComponent> //when Ants collide with this unit, they collect resources
{
protected:
    int amount;
public:
    ResourceComponent(Entity& component, int amount);
    virtual void collide(Entity& other);
};

class CorpseComponent : public ResourceComponent, public ComponentContainer<CorpseComponent> //the corpse component handles everything about the entity after death
{
    RenderComponent* render = nullptr;
public:
    CorpseComponent(Unit& unit, int amount) : ResourceComponent(unit, amount), ComponentContainer<CorpseComponent>(unit), render(unit.getComponent<RenderComponent>())
    {

    }
    void collide(Entity& other);
    void update();
};

class Resource : public Unit
{
    int amount;
    class ResourceRender : public RenderComponent, public ComponentContainer<ResourceRender>
    {
    public:
        ResourceRender(Entity& entity);
        void update();
    };
public:
    Resource(int x, int y, int amount);
    void interact(Ant& ant);
};



class Bug : public Unit
{
    class BugMove : public MoveComponent, public ComponentContainer<BugMove>
    {
    public:
        BugMove(double speed, const glm::vec4& rect, Unit& unit);
        void update();
    };
public:
    Bug(int x, int y);
};

#endif // ENTITIES_H_INCLUDED
