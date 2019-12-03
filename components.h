#ifndef COMPONENTS_H_INCLUDED
#define COMPONENTS_H_INCLUDED

#include <memory.h>
#include <map>

#include "glGame.h"

class Entity;
class Component;

template<class C>
struct ComponentContainer
{
    static std::map<Entity*,Component*> components;
    static std::map<ComponentContainer*, Entity*> entities;
    ComponentContainer(Entity& entity)
    {
        components[&entity] = static_cast<C*>(this);
        entities[this] = &entity;
    }
    ~ComponentContainer()
    {
        components[entities[this]] = nullptr;
        entities.erase(entities.find(this));
    }
};

template<class C>
std::map<Entity*,Component*> ComponentContainer<C>::components;

template<class C>
std::map<ComponentContainer<C>*, Entity*> ComponentContainer<C>::entities;


class Component
{
protected:
    Entity* entity;

public:
    Component(Entity& entity);
    virtual void update()
    {

    }
    virtual void collide(Entity& other)
    {

    }
    Entity& getEntity();
};

class RectComponent : public Component, public ComponentContainer<RectComponent>, public RectPositional
{
public:
    RectComponent(const glm::vec4& rect, Entity& entity);
    void setRect(const glm::vec4& rect);
};

class MoveComponent : public RectComponent, public ComponentContainer<MoveComponent>
{
    double angle = 0; //Direction we are moving in. This would be calculated every update call if this wasn't a member variable
protected:
    double speed = 0;
    glm::vec2 target; //point to move towards
public:
    MoveComponent(double speed, const glm::vec4& rect, Entity& entity);
    void setTarget(const glm::vec2& point);
    virtual void update();
    bool atTarget(); //returns whether or not we have arrived at the target
};

class RenderComponent : public Component, public ComponentContainer<RenderComponent>
{
public:
    RenderComponent(Entity& entity);
};

class Entity
{
public:

    std::vector<std::shared_ptr<Component>> components;
public:
    Entity()
    {

    }
    void update();
    void collide(Entity& entity);
    template<typename T>
    T* getComponent()
    {
        return (static_cast<T*>((ComponentContainer<T>::components[this])));
    }
    void addComponent(Component& comp);

};


#endif // COMPONENTS_H_INCLUDED
