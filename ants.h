#ifndef ANTS_H_INCLUDED
#define ANTS_H_INCLUDED

#include <deque>
#include <unordered_set>

#include "components.h"
#include "entities.h"
#include "friendlyAssemblers.h"

class Anthill;
class AntManager;



class CommandableComponent : public Component, public ComponentContainer<CommandableComponent> //handles communication with AntManagers. Tracks if a unit is done with a task
{
    bool towardsCenter = true; //whether the last moveTask has moved us towards our current AntManger center or not. True means that once we've reached a destination, units in the same AntManager should stop moving once reaching us, false menas they can still push us.
    bool completedTask = true;
    AntManager* currentTask = nullptr;
public:
    CommandableComponent(Entity& entity);
    void setTarget(const glm::vec2& moveTo, std::shared_ptr<Object>* obj);
    void update();
    void collide(Entity& entity);
    void startTask(); //sets completedTask to false
    void setCurrentTask(AntManager* task);
    AntManager* getCurrentTask();
    bool getCompletedTask();
    bool getTowardsCenter();

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
        std::unordered_set<ForcesComponent*> appliedForces; //list of forceComponents that we don't get to push because they've already pushed us
    public:
        AntMoveComponent(Anthill* hill, double speed, const glm::vec4& rect, Entity& entity);
        void addForceApplier(ForcesComponent& force);
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

class ProduceUnitComponent;
class ProduceUnitButton : public Button
{
    UnitAssembler* assembler;
    ProduceUnitComponent* owner = nullptr;
public:
    ProduceUnitButton(UnitAssembler& obj, ProduceUnitComponent* own);
    void press();
};

class ProduceUnitComponent : public ClickableComponent, public ComponentContainer<ProduceUnitComponent> //has the ability to create units
{


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
    void setButtons(); //based on what units the player owns, adds buttons to create such units
    virtual ~Anthill();

};

#endif // ANTS_H_INCLUDED
