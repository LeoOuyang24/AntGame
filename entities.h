#ifndef ENTITIES_H_INCLUDED
#define ENTITIES_H_INCLUDED


#include "glInterface.h"
#include "SDLhelper.h"
#include "render.h"

#include "animation.h"
#include "components.h"
#include "navigation.h"


void renderMeter(const glm::vec3& xyWidth, const glm::vec4& color, double current, double maximum, float z);
void renderTimeMeter(const glm::vec4& rect, const glm::vec4& color, DeltaTime& alarm, double duration, float z);
class Unit;
Unit* convertPosToUnit(Positional* pos); //static casts pos to rectcomponent and casts owner to Unit

class TangibleComponent : public Component, public ComponentContainer<TangibleComponent> //returns a bool based on if the entity should update
{
    typedef bool (*TangibleFunction)(Entity* ent);
    TangibleFunction isTangible;
public:
    TangibleComponent(TangibleFunction isTangible_, Entity& entity);
    bool getTangible();
};

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
protected:
    SpriteParameter param; //only tint, effect, renderProgram, and z matter
    AnimationWrapper* sprite = nullptr;
    AnimationParameter animeParam;
public:
    AnimationComponent(AnimationWrapper& anime, Entity& entity, RenderCamera* camera);
    AnimationComponent(AnimationWrapper& anime, Entity& entity);
    virtual void render(const SpriteParameter& param);
    void setParam(const SpriteParameter& param, const AnimationParameter& animeParam = AnimationParameter());
    void update();
};

class UnitAnimationComponent : public AnimationComponent, public ComponentContainer<UnitAnimationComponent> //used to show different animations for different actions.
{
    AnimationWrapper* tempSprite = nullptr;
public:
    UnitAnimationComponent( AnimationWrapper& set, Unit& entity);
    void request(AnimationWrapper& sprite_, const SpriteParameter& param, const AnimationParameter& animeParam); //sets both sprite and params
    virtual bool doMirror(); //whether we should mirror the sprite
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

class ObjectComponent : public Component, public ComponentContainer<ObjectComponent>
{
     bool dead = false; //whether or not to remove the object
    bool movable = false; //whether or not the object moves when pushed by another unit. False for structures, true for units
    bool friendly = false; //whether or not the player can target the unit
    bool inactive = false; //whether or not to update this entity
public:
    ObjectComponent(bool dead_, bool movable_, bool friendly_, Entity& entity);
    bool getDead();
    bool getMovable();
    bool getFriendly();
    bool getInactive();
    void setMovable(bool movable);
    void setFriendly(bool val);
    void setDead(bool isDead);
    void setInactive(bool i);
};

class Object : public Entity//environment objects. Can be seen, have a hitbox, and can be clicked on
{
    friend class EntityAssembler;
protected:

    ClickableComponent* clickable = nullptr;
    RectComponent* rect = nullptr;
    RenderComponent* render = nullptr;
    ObjectComponent* object = nullptr;
public:
    Object(ClickableComponent& click, RectComponent& rect, RenderComponent& render, bool mov = true);
    Object(std::string name, const glm::vec4& rect, AnimationWrapper* rapper, bool mov = true);
    Object();
    void addRect(RectComponent* r);
    void addClickable(ClickableComponent* c);
    void addRender(RenderComponent* rend);
    void addObject(ObjectComponent* object);
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
    int tempArmor = 0; //used to track armor modifications
    int armor = 0;
    double health = 0;
    double maxHealth = 0;
    int displacement = 0; //height above the entity
    bool visible = true;
    std::weak_ptr<Object> lastAttacker;// the last thing that attacked this unit
    std::map<SpriteWrapper*,std::list<StatusEffect>> effects; //ordered map because we frequently iterate through this map. Each status effect should be uniquely identified by its sprite
    void addHealth(double amount); //increases health by amount. Health cannot exceed maxHealth nor go below 0
protected:
    DeltaTime invincible; //keeps track of how many frames of invincibility
    float invulTime = 10; //milliseconds of invincibility
    bool isDamaging(double amount); //returns true if amount is positive and we are not invincible
public:
    HealthComponent(Entity& entity, double health_,  int displacement_ = 20);
    void addArmor(int val);
    virtual void takeDamage(double amount, Object& attacker ); //the key difference between this and add health is that this keeps track of which attackComponent dealt the damage. Negative damage heals the target
    void addEffect(StatusEffect effect);
    double getHealth();
    double getMaxHealth();
    float isInvincible(); //returns proportion of time left invincible. <= 0 is no longer invincible. Returns 0 is invulTime is 0 or if invincible timer is not set
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
    Unit();
    void addHealth(HealthComponent* h);
    HealthComponent& getHealth();
    bool clicked();
    virtual void interact(Ant& ant);
    Manager* getManager();
    void setManager(Manager& manager);

};

class EntityForces : public ForcesComponent, public ComponentContainer<EntityForces>
{
    glm::vec4 curNodeRect = glm::vec4(0);
public:
    EntityForces(Entity& entity);
    void update();
};

class Structure : public Unit
{
public:
    Structure(ClickableComponent& click, RectComponent& rect, RenderComponent& render, HealthComponent& health);
};

class PathComponent : public MoveComponent, public ComponentContainer<PathComponent> //a MoveComponent that holds a whole path rather than just a single target
{
    glm::vec4 curNodeRect = {0,0,0,0}; //used in setPos to keep track of the nodeRect we were changed to last. This makes changePos more efficient if we have to call it numerous times (when units are pushing each other, for example)
    Path path;
public:
    PathComponent(double speed, const glm::vec4& rect, Entity& unit);
    bool atPoint(const glm::vec2& point);
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

typedef std::pair<SpriteParameter,AnimationParameter> ImgParameter;
class Attack //represents attacks
{
protected:
    struct AttackData //represents all parameters that can be modified by outside sources
    {
        float range = 0, damage = 0;
        int endLag = 0;
    };
    AttackData modData;
    DeltaTime coolDownTimer;
    int startAttack = -1; //represents the time at which we started attacking. Really only useful if there is a sequencer
    AnimationSequencer* sequencer = nullptr;
    AnimationWrapper* attackAnime = nullptr;
    virtual void doAttack(Object* attacker, const glm::vec2& pos); //the actual hitbox spawning attack.
    virtual ImgParameter getParam(Object* attacker, const glm::vec2& pos);
public:
    const AttackData baseData;
    Attack( float damage_, int endLag_, float range_,AnimationWrapper* attackAnime_ = nullptr, AnimationSequencer* sequencer_ = nullptr);
    virtual bool canAttack(Object* owner = nullptr,Object* ptr = nullptr); //returns true if we can attack the target. Doesn't take into account coolDownTimer. Returns false if either pointer is null
    bool offCooldown();//returns true if the attack is off cooldown
    int getCooldownRemaining(); //gets the remaining cooldown. 0 if off cooldown (offCooldown() returns true)
    virtual ImgParameter attack(Object* attacker,const glm::vec2& pos); //wrapper function for doAttack that also incorporates cooldowns. Returns animation info.
    AnimationWrapper* getAnimation();
    float getRange();
    float getDamage();
    int getEndLag();
    void setRange(float range);
    void setDamage(float damage);
    void setAttackSpeed(float increase);
    ~Attack();

};

class ProjectileComponent : public MoveComponent, public ComponentContainer<ProjectileComponent>
{

public:
    typedef void (*ProjCollideFunc)(Unit& other, ProjectileComponent& thisProjectile); //function for ProjectileComponents' collide function
    ProjectileComponent(double damage, bool friendly,const glm::vec2& target, double speed, const glm::vec4& rect, Object& entity,ProjCollideFunc collideFun_ = nullptr);
    ProjectileComponent(double damage, bool friendly, const glm::vec2& target, double xspeed, double yspeed, const glm::vec4& rect, Object& entity, ProjCollideFunc collideFun_ = nullptr);
    void setShooter(Object& obj);
    Object* getShooter();
    virtual void collide(Entity& other);
    virtual void update();
private:
    bool friendly = false;
    double damage = 0;
    ProjCollideFunc collideFunc = nullptr;
protected:
    Object* shooter = nullptr; //not the actual projectile unit, but whoever spawned the projectile unit. Primarily used to communicate who shot the projectile

    virtual void onCollide(Unit& other); //what to actually call when we collide, since the collide code doesn't really change.
};

class UnitAttackComponent : public ApproachComponent, public ComponentContainer<UnitAttackComponent> //Represents Enemy AI
{
    typedef std::pair<std::weak_ptr<Object>,glm::vec2> TargetInfo;
    std::weak_ptr<Object> shortTarget; //represents short-term target. Is attacked because it's in range
    TargetInfo longTarget; //represents a target that the player explicitly chose. Could be an empty position with no enemy to attack
    double damage;
    bool activated = false; //whether this component should affect MoveComponent. Exists solely to make sure our unit doesn't move to 0,0 upon spawn. Since all of our units are spawned at (0,0) and then moved, we can't just set the position in the constructor
    bool notFriendly = false; //the type of enemy to attack
    bool ignore = false; //whether to ignore enemies or not
    double searchRange = 0; //aggro range
    std::list<Attack*> attacks;
    void processAttack(Attack& attack);
public:
    UnitAttackComponent(double damage_, int endLag_, double range_,double searchRange_,bool f, Entity& entity);
    void addAttack(Attack& attack);
    void update();
    void collide(Entity& other);
    ~UnitAttackComponent();

};

class ProjectileAssembler;
class ProjectileAttack : public Attack //attack that shoots a projectile
{
protected:
    ProjectileAssembler* assembler = nullptr;
    void doAttack(Object* attacker, const glm::vec2& target);
public:
    ProjectileAttack(ProjectileAssembler& ass,int endLag, double range,
                     AnimationWrapper* attackAnime_ = nullptr, AnimationSequencer* sequencer_ = nullptr); //projectile Attack damage is based on projectile Assembler damage, so we don't need to provide it in the constructor
};

#endif // ENTITIES_H_INCLUDED
