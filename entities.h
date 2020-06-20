#ifndef ENTITIES_H_INCLUDED
#define ENTITIES_H_INCLUDED

#include <deque>

#include "glInterface.h"
#include "SDLhelper.h"

#include "components.h"

typedef std::deque<glm::vec2> Path;

void renderMeter(const glm::vec3& xyWidth, const glm::vec4& color, double current, double maximum, float z);

class ClickableComponent : public Component, public ComponentContainer<ClickableComponent> //clickable component handles user inputs, like when the user selects the unit or presses a button
{
    static const glm::vec2 spacing; //spacing between the buttons and the unit
    std::string name = "";
    bool clicked = false;
    std::vector<std::unique_ptr<Button>> buttons;
public:
    ClickableComponent(std::string name, Entity& entity);
    void update(); //essentially this class's version of update. It's useful
    void click(bool val);
    bool getClicked();
    void addButton(Button& button);
    virtual void display(const glm::vec4& rect);
    ~ClickableComponent();
};


class RectRenderComponent : public RenderComponent, public ComponentContainer<RectRenderComponent>
{
    glm::vec4 color;
public:
    RectRenderComponent(const glm::vec4& color, Entity& unit);
    void update();
    virtual void render(const SpriteParameter& param);
    ~RectRenderComponent();
};

class Object : public Entity//environment objects. Can be seen, have a hitbox, and can be clicked on
{
protected:
    bool dead = false;
    const bool movable = false; //whether or not the object moves when pushed by another unit. True for structures, false for units
    ClickableComponent* clickable = nullptr;
    RectComponent* rect = nullptr;
    RenderComponent* render = nullptr;
public:
    Object(ClickableComponent& click, RectComponent& rect, RenderComponent& render, bool mov = false);
    RectComponent& getRect() const;
    glm::vec2 getCenter();
    ClickableComponent& getClickable();
    RenderComponent& getRender();
    bool clicked();
    bool getDead();
    bool getMovable();
    void setDead(bool isDead);
    virtual ~Object();
};

class InteractionComponent : public Component, public ComponentContainer<InteractionComponent> //objects that can be interacted with
{
public:
    InteractionComponent(Object& unit);
    virtual void interact(Object& actor);
    ~InteractionComponent();
};

class Unit;
class AttackComponent;

class HealthComponent : public Component, public ComponentContainer<HealthComponent>
{
    double health = 0;
    double maxHealth = 0;
    int displacement = 0; //height above the entity
    bool visible = true;
    DeltaTime invincible; //keeps track of how many frames of invincibility
    std::weak_ptr<Object> lastAttacker;// the last thing that attacked this unit
    void addHealth(int amount); //increases health by amount. Health cannot exceed maxHealth nor go below 0
public:
    HealthComponent(Entity& entity, double health_,  int displacement_ = 20);
    void takeDamage(int amount, Object& attacker ); //the key difference between this and add health is that this keeps track of which attackComponent dealt the damage. Negative damage heals the target
    double getHealth();
    double getMaxHealth();
    void update(); //render health bar and reset invincibility frames
    void render(const glm::vec3& rect, float z); //renders the healthbar at the given location with the given width. The height will always be height so rect.a is the z value to render at.
    void setVisible(bool value);
    Object* getLastAttacker(); //returns the lastAttacker. May be null;
    ~HealthComponent();
};

class Ant;
class Manager;
class Unit : public Object //Units can be clicked on and seen and have health
{
protected:
    HealthComponent* health = nullptr;
    Manager* manager = nullptr;
public:
    Unit(ClickableComponent& click, RectComponent& rect, RenderComponent& render, HealthComponent& health, bool mov = true);

    HealthComponent& getHealth();
    bool clicked();
    virtual void interact(Ant& ant);
    Manager* getManager();
    void setManager(Manager& manager);

};

class RepelComponent : public Component, public ComponentContainer<RepelComponent> //component that repels objects on collision
{
public:
    RepelComponent(Object& unit);
    void collide(Entity& other);
};

class Structure : public Unit
{
public:
    Structure(ClickableComponent& click, RectComponent& rect, RenderComponent& render, HealthComponent& health);
};

class AttackComponent : public Component, public ComponentContainer<AttackComponent>
{
    float damage = 0;
    int endLag = 0; //how much time must pass before the attack can be reused.
    DeltaTime attackTimer;
public:
    AttackComponent(float damage_, int endLag_, Unit& unit);
    void attack(HealthComponent* health); //this is a pointer so you can legally pass in a null pointer. This function will make sure it's not null
    ~AttackComponent();
};

class ResourceComponent : public Component, public ComponentContainer<ResourceComponent> //represents the amount of resources this object has
{
protected:
    double resource;
    int maxResource;
public:
    ResourceComponent(int amount, Entity& entity);
    int getResource();
    int getMaxResource();
    void setResource(double amount);
    void collect(Object& other);
    void render(const glm::vec3& rect, float z);
    ~ResourceComponent();
};

class CorpseComponent : public Component, public ComponentContainer<CorpseComponent> //the corpse component spawns a corpse object after the entity dies
{
    int amount = 0;
    RenderComponent* render = nullptr;
public:
    CorpseComponent(Unit& unit, int amount_);
    void onDeath();
    virtual ~CorpseComponent();
};

class ResourceUnit : public Unit
{
public:
    ResourceUnit(int resources, const glm::vec4& rect);
    virtual ~ResourceUnit();
};

class PathComponent : public MoveComponent, public ComponentContainer<PathComponent> //a MoveComponent that holds a whole path rather than just a single target
{
    Path path;
public:
    PathComponent(double speed, const glm::vec4& rect, Entity& unit);
    virtual void setTarget(const glm::vec2& point);
    const glm::vec2& getTarget();
    void addPoint(const glm::vec2& point); //add a point to the path
    virtual void update();
};

class WanderMove : public MoveComponent, public ComponentContainer<WanderMove>
{
public:
    WanderMove(double speed, const glm::vec4& rect, Entity& unit);
    void update();
    ~WanderMove();
};

class ApproachComponent : public Component, public ComponentContainer<ApproachComponent> //a component that can move towards a unit. It is NOT a movecomponent because it by itself cannot function. If the targetUnit is null, it uses the owning entity's movecomponent to decide what to do.
{
protected:
    glm::vec2 displacement = {0,0};
    std::weak_ptr<Object> targetUnit;
    MoveComponent* move = nullptr;
    template<typename T>
    Object* findNearestUnit(double radius); //finds the nearest unit that has a component T in radius radius. Returns null if none found
public:
    ApproachComponent(Unit& entity);
    virtual void setTarget(const glm::vec2& target, const std::shared_ptr<Object>* unit);
    virtual void setTarget(const std::shared_ptr<Object>& unit);
    virtual void update();
    void setMove(MoveComponent& move_);
    Object* getTargetUnit();
    ~ApproachComponent();
};

class Anthill;
class SeigeComponent : public ApproachComponent, public ComponentContainer<SeigeComponent> //a component that causes the unit to attack anthills; ants if no anthilsl are nearby. USed when signalling
{
    std::weak_ptr<Anthill> targetHill; //this is separate from targetUnit because we want to move always move towards the same anthill, even after changing targets to attack something
public:
    SeigeComponent(Unit& entity, Anthill& hill);
    void update();
    ~SeigeComponent();
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
    ~Bug();
};

class Beetle : public Unit
{
    class BeetleMove : public ApproachComponent, public ComponentContainer<BeetleMove> // finds and attacks the nearest ant
    {
    public:
        BeetleMove(Unit& unit);
        void update();
        ~BeetleMove();
    };
public:
    Beetle(int x, int y);
    ~Beetle();
};

class ResourceEatComponent : public ApproachComponent, public ComponentContainer<ResourceEatComponent> //finds and eats the nearest ResourceCountComponent, including anthills
{
public:
    ResourceEatComponent(Unit& unit);
    void update();
    virtual ~ResourceEatComponent();
};

#endif // ENTITIES_H_INCLUDED
