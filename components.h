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
    virtual ~ComponentContainer()
    {
    //components.erase(components.find(entities[this]));
      // entities.erase(entities.find(this));
    }

};

template<class C>
std::map<Entity*,Component*> ComponentContainer<C>::components;

template<class C>
std::map<ComponentContainer<C>*, Entity*> ComponentContainer<C>::entities;


class Component : public ComponentContainer<Component>
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
    virtual void onDeath()
    {

    }
    Entity& getEntity();
};

class RectComponent : public Component, public ComponentContainer<RectComponent>, public RectPositional
{
public:
    RectComponent(const glm::vec4& rect, Entity& entity);
    void setRect(const glm::vec4& rect);
    glm::vec2 getCenter()
    {
        return {rect.x + rect.z/2, rect.y + rect.a/2};
    }
};

class MoveComponent : public RectComponent, public ComponentContainer<MoveComponent>
{
    double angle = 0; //Direction we are moving in. This would be calculated every update call if this wasn't a member variable
protected:
    double speed = 0;
    glm::vec2 target; //point to move towards
public:
    MoveComponent(double speed, const glm::vec4& rect, Entity& entity);
    void teleport(const glm::vec2& point); //centers the entity at the point and sets it as the new target
    virtual void update();
    bool atTarget(); //returns whether or not we have arrived at the target
    void setTarget(const glm::vec2& point)
    {
        target = point;
    }
    const glm::vec2& getTarget()
    {
        return target;
    }

};

class RenderComponent : public Component, public ComponentContainer<RenderComponent>
{
public:
    RenderComponent(Entity& entity);
    virtual void render(const SpriteParameter& param) //every rendercomponent can take in a SpriteParameter and render it accordingly
    {

    }

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
        return (static_cast<T*>(ComponentContainer<T>::components[this]));
    }
    void addComponent(Component& comp)
    {
        components.emplace_back(&comp);
    }
    void onDeath();

};


#endif // COMPONENTS_H_INCLUDED
