#ifndef ANTS_H_INCLUDED
#define ANTS_H_INCLUDED

#include "components.h"
#include "entities.h"


class Anthill;
class Ant : public Unit
{
public:
    const static short dimen;
    class AntMoveComponent : public MoveComponent, public ComponentContainer<AntMoveComponent> //controls ant behavior based on the ant's home and what they are carrying
    {
        Anthill* home = nullptr;
        int carrying = 0;
        Unit* targetUnit = nullptr;
        glm::vec2 displacement = {0,0};
    public:
        AntMoveComponent(Anthill* hill, double speed, const glm::vec4& rect, Entity& entity);
        void setTarget(const glm::vec2& target, Unit* unit);
        void update();
        void setCarrying(int amount);
        int getCarrying();
        Anthill* getHome();
        Unit* getTargetUnit();
    };

    class AntRenderComponent : public RenderComponent, public ComponentContainer<AntRenderComponent>
    {
        float angle = 0;
        float goalAngle = 0;
        glm::vec4 color;
    public:
        AntRenderComponent(const glm::vec4& color, Entity& entity);
        void update();
    };

    class AntClickable : public ClickableComponent, public ComponentContainer<AntClickable>
    {
    public:
        AntClickable(std::string name, Unit& entity);
        void clicked();
    };
    enum Modes
    {

    };
    Ant(const glm::vec4& rect, Anthill& home);
    void setTarget(const glm::vec2& target, Unit* unit);
    void setTarget(const glm::vec2& target);
    void setTarget(Unit& unit); //calls the other target function but the target is the center of unit
    void setCarrying(int amount);
    void goHome();
    int getCarrying();
};

class AntHillRender : public RenderComponent, public ComponentContainer<AntHillRender>
{
    glm::vec4 color;
public:
    AntHillRender(Entity& entity);
    void update();
};

class ResourceCountComponent : public Component, public ComponentContainer<ResourceCountComponent> //used to count the number of resources in an anthill
{
    double resource;
    int maxResource;
public:
    ResourceCountComponent(int amount, Entity& entity);
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
    void collide(Entity& other);
    void update();
};

class Anthill : public Unit
{
    std::vector<std::weak_ptr<Ant>> ants;
    class CreateAnt : public Button
    {
        Anthill* hill = nullptr;
    public:
        CreateAnt(Anthill& hill);
        void press();
    };
public:
    Anthill(const glm::vec2& pos);
    void createAnt();
    std::vector<std::weak_ptr<Ant>>& getAnts()
    {
        return ants;
    }

};

#endif // ANTS_H_INCLUDED
