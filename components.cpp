#include <iostream>

#include "vanilla.h"
#include "SDLHelper.h"
#include "render.h"

#include "components.h"



Component::Component(Entity& entity) : ComponentContainer<Component>(nullptr)
{
    this->entity = &entity;
}

Entity& Component::getEntity()
{
    return *entity;
}

void Component::update()
{

}
void Component::collide(Entity& other)
{

}
void Component::onDeath()
{

}
Component::~Component()
{
   // std::cout << "Deleted!" << std::endl;
}

RectComponent::RectComponent(const glm::vec4& rect, Entity& entity) : Component(entity), ComponentContainer<RectComponent>(&entity), RectPositional(rect)
{

}

void RectComponent::setRect(const glm::vec4& rect)
{
    this->rect = rect;
}

void RectComponent::setPos(const glm::vec2& pos)
{
    this->rect.x = pos.x;
    this->rect.y = pos.y;
}

glm::vec2 RectComponent::getCenter()
{
    return {rect.x + rect.z/2, rect.y + rect.a/2};
}

RectComponent::~RectComponent()
{

}

MoveComponent::MoveComponent(double speed, const glm::vec4& rect, Entity& entity) : RectComponent(rect, entity), ComponentContainer<MoveComponent>(&entity), speed(speed)
{
    target = {rect.x + rect.z/2, rect.y + rect.a/2};
}

void MoveComponent::update()
{
    glm::vec2 center = {rect.x + rect.z/2, rect.y + rect.a/2};
    angle = atan2((target.y - (center.y)),(target.x - (center.x)));
    if (!atTarget())
    {
        rect.x += absMin(cos(angle)*speed*DeltaTime::deltaTime,target.x - center.x);
        rect.y += absMin(sin(angle)*speed*DeltaTime::deltaTime, target.y - center.y);
    }
    velocity = pointDistance({rect.x + rect.z/2, rect.y + rect.a/2}, center);
}

bool MoveComponent::atTarget()
{
    return pointDistance(getCenter(),target) <= 0.0001;
}

void MoveComponent::teleport(const glm::vec2& point)
{
    rect.x = point.x - rect.z/2; //rect.x += point.x - (rect.x + rect.z/2) -> rect.x = point.x - rect.z/2
    rect.y = point.y - rect.a/2;
    setTarget(point);
}

const glm::vec2& MoveComponent::getTarget()
{
    return target;
}

double MoveComponent::getVelocity()
{
    return velocity;
}

void MoveComponent::setTarget(const glm::vec2& point)
{
    target = point;
}

void MoveComponent::setSpeed(double newspeed)
{
    speed = newspeed;
}

MoveComponent::~MoveComponent()
{

}

RenderComponent::RenderComponent(Entity& entity) : Component(entity), ComponentContainer<RenderComponent>(&entity)
{

}

void RenderComponent::render(const SpriteParameter& param) //every rendercomponent can take in a SpriteParameter and render it accordingly
{

}

RenderComponent::~RenderComponent()
{

}

Entity::Entity()
{

}


void Entity::addComponent(Component& comp)
{
    components.emplace_back(&comp);
}

void Entity::update()
{
    for (int i = components.size() - 1; i >= 0; --i)
    {
        components[i]->update();
    }
}

void Entity::collide(Entity& entity)
{
    for (int i= components.size() - 1; i >= 0; --i)
    {
        components[i]->collide(entity);
    }
}

void Entity::onDeath()
{
    for (int i = components.size() - 1; i >= 0; --i)
    {
        components[i]->onDeath();
    }
}

Entity::~Entity()
{
    components.clear();
}
