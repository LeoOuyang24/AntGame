#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include <set>

#include "glInterface.h"

#include "friendlyAssemblers.h"

class Factory;

class InventoryComponent : public Component, public ComponentContainer<InventoryComponent>
{
    Entity* pistol;
public:
    InventoryComponent(Unit& player);
    Entity* getWeapon();
    void update();
};

class WeaponAnimationComponent : public UnitAnimationComponent, public ComponentContainer<WeaponAnimationComponent> //renders weapons and arms. Should be added to weapon OWNERS, not the weapons themselves
{
    AnimationWrapper* arm = nullptr; //renders the arm over the weapon
    glm::vec2 armOffset;
public:
    WeaponAnimationComponent( AnimationWrapper* arm_, const glm::vec2& armOffset_, AnimationWrapper& wrap, Unit& owner);
    bool doMirror();
    void update();
};

class PlayerAssembler : public UnitAssembler
{
    class PlayerControls : public MoveComponent
    {
    public:
        PlayerControls(float speed, const glm::vec4& rect, Unit& player);
        void update();
    };
    class PlayerHealth : public HealthComponent
    {
        ObjectAssembler* assembler = nullptr; //in case the player dies, this keeps track of what entity killed us.
                                            //We don't care about the specific entity but rather the properties of that entity, so this works even if the killer
                                            //is dead
    public:
        PlayerHealth(Entity& entity, float health);
        void takeDamage(double amount, Object& attacker);
        ObjectAssembler* getAssembler();
    };
    class PlayerRender : public WeaponAnimationComponent
    {
    public:
        PlayerRender( AnimationWrapper* arm_, const glm::vec2& armOffset_, AnimationWrapper& wrap, Unit& owner);
        void update();
    };
public:
    PlayerAssembler();
    Object* assemble();
    ~PlayerAssembler();
};

class Player //tracks player stats (resources, money). Also handles player inputs
{
    PlayerAssembler playerAssembler;

    int resource;
    int gold = 100;
    std::shared_ptr<Unit> player; //making this a shared pointer is convenient as each room will need their own copy. This ensures that all shared pointers of the player are the same
public:
    static const glm::vec4 selectColor;
    Player();
    void init(); //initiaites the start state. Sets the starting amount of resources, starting buildings, etc.
    void reset();
    Unit* getPlayer();
    std::shared_ptr<Unit>& getPlayerPtr();
    int getResource();
    void addResource(int r); //used to increase or decrease resources. Resources can't be negative
    int getGold();
    void addGold(int g);
    void renderUI();
    ~Player();
};

class InactiveComponent : public Component, public ComponentContainer<InactiveComponent> //represents how long an entity is inactive for. Usually because a building is under construction
{
    DeltaTime timeLeft;
    double waitTime = 0;
public:
    InactiveComponent(double duration, Entity& entity);
    void init(); //starts the timer
    bool done(); //returns true when the object is no longer inactive
    void render(); //what to render while this entity is inactive
};



#endif // PLAYER_H_INCLUDED
