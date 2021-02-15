#ifndef ENTITIES_H_INCLUDED
#define ENTITIES_H_INCLUDED


#include "glInterface.h"
#include "SDLhelper.h"
#include "render.h"

#include "components.h"
#include "navigation.h"


void renderMeter(const glm::vec3& xyWidth, const glm::vec4& color, double current, double maximum, float z);
void renderTimeMeter(const glm::vec4& rect, const glm::vec4& color, DeltaTime& alarm, double duration, float z);
class Unit;
Unit* convertPosToUnit(Positional* pos); //static casts pos to rectcomponent and casts owner to Unit


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

class AnimationComponent : public RenderComponent, public ComponentContainer<AnimationComponent>
{
    glm::vec4 tint = glm::vec4(1); //sometimes we want to modify our animation from outside sources (usually status effects)
    AnimationWrapper* sprite = nullptr;
    AnimationParameter animeParam;
public:
    AnimationComponent(AnimationWrapper& anime, Entity& entity, RenderCamera* camera);
    AnimationComponent(AnimationWrapper& anime, Entity& entity);
    void render(const SpriteParameter& param);
    void setTint(const glm::vec4& param);
    void update();
};

class RectRenderComponent : public RenderComponent, public ComponentContainer<RectRenderComponent>
{
protected:
    glm::vec4 color;
public:
    RectRenderComponent(const glm::vec4& color, Entity& unit, RenderCamera* camera);
    RectRenderComponent(const glm::vec4& color, Entity& unit);
    void update();
    virtual void render(const SpriteParameter& param);
    ~RectRenderComponent();
};

class Object : public Entity//environment objects. Can be seen, have a hitbox, and can be clicked on
{
    friend class EntityAssembler;
protected:

    ClickableComponent* clickable = nullptr;
    RectComponent* rect = nullptr;
    RenderComponent* render = nullptr;
    bool dead = false; //whether or not to remove the object
    const bool movable = false; //whether or not the object moves when pushed by another unit. False for structures, true for units
    bool friendly = false; //whether or not the player can target the unit
    bool isProjectile = false; //whether or not the object is a projectile. Biggest difference is that projectiles aren't added to quadtrees
public:
    Object(ClickableComponent& click, RectComponent& rect, RenderComponent& render, bool mov = true);
    Object(std::string name, const glm::vec4& rect, AnimationWrapper* rapper, bool mov = true);
    Object(bool mov);
    void addRect(RectComponent* r);
    void addClickable(ClickableComponent* c);
    void addRender(RenderComponent* rend);
    RectComponent& getRect() const;
    glm::vec2 getCenter();
    ClickableComponent& getClickable();
    RenderComponent& getRender();
    bool clicked();
    bool getDead();
    bool getMovable();
    bool getFriendly();
    void setFriendly(bool val);
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
class StatusEffect;
class HealthComponent : public Component, public ComponentContainer<HealthComponent>
{
    double health = 0;
    double maxHealth = 0;
    int displacement = 0; //height above the entity
    bool visible = true;
    DeltaTime invincible; //keeps track of how many frames of invincibility
    std::weak_ptr<Object> lastAttacker;// the last thing that attacked this unit
    std::map<SpriteWrapper*,std::list<StatusEffect>> effects; //ordered map because we frequently iterate through this map. Each status effect should be uniquely identified by its sprite
    void addHealth(double amount); //increases health by amount. Health cannot exceed maxHealth nor go below 0
public:
    HealthComponent(Entity& entity, double health_,  int displacement_ = 20);
    void takeDamage(double amount, Object& attacker ); //the key difference between this and add health is that this keeps track of which attackComponent dealt the damage. Negative damage heals the target
    void addEffect(StatusEffect effect);
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
    Unit(std::string name, const glm::vec4& rect, AnimationWrapper* anime, bool mov, double maxHealth);
    Unit(bool mov);
    void addHealth(HealthComponent* h);
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
    glm::vec4 curNodeRect = {0,0,0,0}; //used in setPos to keep track of the nodeRect we were changed to last. This makes changePos more efficient if we have to call it numerous times (when units are pushing each other, for example)
    Path path;
public:
    PathComponent(double speed, const glm::vec4& rect, Entity& unit);
    bool atPoint(const glm::vec2& point);
    void setPos(const glm::vec2& pos); //same as MoveComponent::setPos if the target is moving, changePos if the target is standing still or part of an IDLE task. Prevents units from moving into walls. Sets top right corner
    void changePos(const glm::vec2& pos); // sets the center and target so the unit won't move
    virtual void setTarget(const glm::vec2& point);
    const glm::vec2& getTarget(); //gets the final target. //atTarget() returns whether this object is at the next point, not the final point
    bool atFinalTarget(); //returns if our path size is only 1 point or less, which is equivalent to whether or not we've reached our final destination.
    glm::vec2 getNextTarget(); //gets the next point to move to
    void addPoint(PathPoint& point); //add a point to the path
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
    Object* findNearestUnit(double radius, bool friendly, RawQuadTree& tree)  //finds the nearest unit that has a component T in radius radius and has friendly level = friendly. Returns null if none found. The fact that this is a templated function means it must be in this header file which is why GameWindow::getLevel.getTree() needs to be manually passed in as #include "game.h" creates a circular dependency
    {
        Object* owner = ((Object*)entity);
        Entity* closest = nullptr;
        double minDistance = -1;
        if (owner)
        {
            glm::vec2 center = owner->getRect().getCenter();
            std::vector<Positional*> nearby = tree.getNearest( center , radius);
            int size = nearby.size();
            for (int i = 0; i < size; ++i)
            {
                Object* current = (static_cast<Object*>(&static_cast<RectComponent*>(nearby[i])->getEntity()));
                if (current->getComponent<T>())
                {
                    double distance = current->getComponent<RectComponent>()->distance(center);
                    if ((distance < minDistance || minDistance == -1) && current != owner && current->getFriendly() == friendly)
                    {
                        minDistance = distance;
                        closest = current;
                    }
                }
            }
        }
        return static_cast<Object*>(closest);
    }
public:
    ApproachComponent(Entity& entity);
    virtual void setTarget(const glm::vec2& target,std::shared_ptr<Object>* unit); //unit is a pointer so you can move to a point rather than a unit
    virtual void setTarget(std::shared_ptr<Object>& unit);
    virtual void update();
    void setMove(MoveComponent& move_);
    Object* getTargetUnit();
    ~ApproachComponent();
};

class AttackComponent : public ApproachComponent, public ComponentContainer<AttackComponent>
{
protected:
    double range = 0;
    double damage = 0;
    int endLag = 0; //how much time must pass before the attack can be reused.
    DeltaTime attackTimer;
    virtual bool canAttack(Object* ptr); //returns true if we can attack the target. Doesn't take into account attackTimer because even if we are waiting for the timer to tick down, we should still stop moving
public:
    AttackComponent(double damage_, int endLag_, double range_, Entity& unit);
    virtual void attack(HealthComponent* health); //this is a pointer so you can legally pass in a null pointer. This function will make sure it's not null
    virtual void update();
    using ApproachComponent::setTarget;
    virtual void setTarget(const glm::vec2& target,  std::shared_ptr<Object>* unit); //unit is a pointer so you can move to a point rather than a unit
    ~AttackComponent();
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

class ResourceEatComponent : public ApproachComponent, public ComponentContainer<ResourceEatComponent> //finds and eats the nearest ResourceCountComponent, including anthills
{
public:
    ResourceEatComponent(Unit& unit);
    void update();
    virtual ~ResourceEatComponent();
};

class ProjectileComponent : public MoveComponent, public ComponentContainer<ProjectileComponent>
{

public:
    typedef void (*ProjCollideFunc)(Unit& other, ProjectileComponent& thisProjectile); //function for ProjectileComponents' collide function
    ProjectileComponent(double damage, bool friendly,const glm::vec2& target, double speed, const glm::vec4& rect, Unit& entity,ProjCollideFunc collideFun_ = nullptr);
    ProjectileComponent(double damage, bool friendly, const glm::vec2& target, double xspeed, double yspeed, const glm::vec4& rect, Unit& entity, ProjCollideFunc collideFun_ = nullptr);
    void setShooter(Object& obj);
    void collide(Entity& other);
    virtual void update();
private:
    bool friendly = false;
    double damage = 0;
    Object* shooter = nullptr; //not the actual projectile unit, but whoever spawned the projectile unit. Primarily used to communicate who shot the projectile
    ProjCollideFunc collideFunc = nullptr;
protected:
    virtual void onCollide(Unit& other); //what to actually call when we collide, since the collide code doesn't really change.
};

class UnitAttackComponent : public AttackComponent, public ComponentContainer<UnitAttackComponent>
{
    typedef std::pair<std::weak_ptr<Object>,glm::vec2> TargetInfo;
    std::weak_ptr<Object> shortTarget; //represents short-term target. Is attacked because it's in range
    TargetInfo longTarget; //represents a target that the player explicitly chose. Could be an empty position with no enemy to attack

    bool activated = false; //whether this component should affect MoveComponent. Exists solely to make sure our unit doesn't move to 0,0 upon spawn. Since all of our units are spawned at (0,0) and then moved, we can't just set the position in the constructor
    bool notFriendly = false; //the type of enemy to attack
    bool ignore = false; //whether to ignore enemies or not
    double searchRange = 0; //aggro range
public:
    UnitAttackComponent(double damage_, int endLag_, double range_,double searchRange_,bool f, Entity& entity);
    void update();
    void setTarget(const glm::vec2& point,  std::shared_ptr<Object>* unit); //sets long target before calling AttackComponent::setTarget
    void setLongTarget(const glm::vec2& point, std::shared_ptr<Object>* unit, bool ignore); //sets longTarget. Essentially only used when the player sets the target. Ignores the point if a target unit is provided. Doesn't immediately set the target, you must wait for update to set the target
    void setShortTarget(std::shared_ptr<Object>& unit); //sets shortTarget. Only used when there is a nearby enemy. Immediately sets unit as the new target
};

class ProjectileAssembler;
class ProjectileAttackComponent : public UnitAttackComponent, public ComponentContainer<ProjectileAttackComponent> //attack component that shoots a projectile
{
    ProjectileAssembler* assembler = nullptr;
public:
    ProjectileAttackComponent(ProjectileAssembler& ass, int endLag, double range, double searchRange_,bool f, Unit& entity);
    virtual void attack(HealthComponent* health);
};

#endif // ENTITIES_H_INCLUDED
