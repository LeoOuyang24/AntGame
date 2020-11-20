#ifndef ANTS_H_INCLUDED
#define ANTS_H_INCLUDED

#include <deque>

#include "components.h"
#include "entities.h"
#include "friendlyAssemblers.h"

class Anthill;
class AntManager;

class UnitAttackComponent : public AttackComponent, public ComponentContainer<UnitAttackComponent>
{
    typedef std::pair<std::weak_ptr<Object>,glm::vec2> TargetInfo;
    std::weak_ptr<Object> shortTarget; //represents short-term target. Is attacked because it's in range
    TargetInfo longTarget; //represents a target that the player explicitly chose. Could be an empty position with no enemy to attack
    enum class IgnoreState
    {
        IDLE, //when the unit is idle
        ATTACK,  //unit attacks other units when nearby
        IGNORESTATE //unit ignores everything and moves towards its target. IGNORE raises a syntax error, so we use IGNORESTATE
    };
    IgnoreState ignore = IgnoreState::IDLE; //whether or not to ignore enemies that are nearby
    bool activated = false; //whether this component should affect MoveComponent. Exists solely to make sure our unit doesn't move to 0,0 upon spawn. Since all of our units are spawned at (0,0) and then moved, we can't just set the position in the constructor
    bool notFriendly = false; //the type of enemy to attack
    double searchRange = 0; //aggro range
public:
    UnitAttackComponent(float damage_, int endLag_, double range_,double searchRange_,bool f, Entity& entity);
    void update();
    void setLongTarget(const glm::vec2& point, std::shared_ptr<Object>* unit); //sets longTarget. Essentially only used when the player sets the target. Ignores the point if a target unit is provided
    void setShortTarget(std::shared_ptr<Object>* unit); //sets shortTarget. Only used when there is a nearby enemy
};

class CommandableComponent : public Component, public ComponentContainer<CommandableComponent> //represents that can be commanded by AntManagers
{
    AntManager* currentTask = nullptr;
public:
    CommandableComponent(Entity& entity);
    void setCurrentTask(AntManager* task);
    AntManager* getCurrentTask();
};

class Ant : public Unit
{
    AntManager* currentTask = nullptr;
public:
    const static short dimen;
    class AntMoveComponent : public PathComponent, public ComponentContainer<AntMoveComponent> //controls ant behavior based on the ant's home and what they are carrying
    {
        Anthill* home = nullptr;
        int carrying = 0;
        glm::vec2 displacement = {0,0};
    public:
        AntMoveComponent(Anthill* hill, double speed, const glm::vec4& rect, Entity& entity);
        void collide(Entity& entity);
        void setCarrying(int amount);
        int getCarrying();
        Anthill* getHome();
        void update();
        ~AntMoveComponent();
    };

    class AntRenderComponent : public RenderComponent, public ComponentContainer<AntRenderComponent>
    {
        float angle = 0;
        float goalAngle = 0;
        glm::vec4 color;
    public:
        AntRenderComponent(const glm::vec4& color, Entity& entity);
        void update();
        void render(const SpriteParameter& param);
        ~AntRenderComponent();
    };

    class AntClickable : public ClickableComponent, public ComponentContainer<AntClickable>
    {
    public:
        AntClickable(std::string name, Entity& entity);
        void clicked();
        ~AntClickable();
    };
    Ant(const glm::vec4& rect, Anthill& home);
    void setTarget(const glm::vec2& target, std::shared_ptr<Object>* unit);
    void setTarget(const glm::vec2& target);
    void setTarget(std::shared_ptr<Object>& unit); //calls the other target function but the target is the center of unit
    void setCarrying(int amount);
    void goHome();
    int getCarrying();
    AntManager* getCurrentTask();
    void setCurrentTask(AntManager& newTask);
};

class AntHillRender : public RenderComponent, public ComponentContainer<AntHillRender>
{
    glm::vec4 color;
    double radius = 0;
public:
    AntHillRender(Entity& entity);
    void update();
    void render(const SpriteParameter& param);
    virtual ~AntHillRender();
};

class ProduceUnitComponent : public ClickableComponent, public ComponentContainer<ProduceUnitComponent> //has the ability to create units
{

    class ProduceUnitButton : public Button
    {
        UnitAssembler* assembler;
        ProduceUnitComponent* owner = nullptr;
    public:
        ProduceUnitButton(UnitAssembler& obj, ProduceUnitComponent* own);
        void press();
    };
    DeltaTime produceTimer; //tracks how much time has passed for the unit currently in production
    UnitAssembler* beingProduced = nullptr; //unit currently in production
    std::deque<UnitAssembler*> toProduce; //queue of objects to produce Does not own the UnitAssemblers
public:
    ProduceUnitComponent(std::string name, Unit& entity);
    void produceUnit(UnitAssembler& assembler);
    void update();
    void display(const glm::vec4& rect);
};

class Anthill : public Structure
{
    class CreateAnt : public Button
    {
        Anthill* hill = nullptr;
    public:
        CreateAnt(Anthill& hill);
        void press();
    };
    class StartSignal : public Button
    {
        Anthill* hill = nullptr;
    public:
        StartSignal(Anthill& hill);
        void press();
    };
public:
    Anthill(const glm::vec2& pos);
    void createAnt();
    virtual ~Anthill();

};

#endif // ANTS_H_INCLUDED
