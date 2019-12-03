#include <iostream>

#include "vanilla.h"
#include "SDLHelper.h"
#include "render.h"

#include "components.h"

Component::Component(Entity& entity)
{
    this->entity = &entity;
}

Entity& Component::getEntity()
{
    return *entity;
}

RectComponent::RectComponent(const glm::vec4& rect, Entity& entity) : Component(entity), ComponentContainer(entity), RectPositional(rect)
{

}

void RectComponent::setRect(const glm::vec4& rect)
{
    this->rect = rect;
}

MoveComponent::MoveComponent(double speed, const glm::vec4& rect, Entity& entity) : speed(speed),RectComponent(rect, entity), ComponentContainer<MoveComponent>(entity)
{
    target = {rect.x + rect.z/2, rect.y + rect.a/2};
}

void MoveComponent::setTarget(const glm::vec2& point)
{
    target = point;
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
}

bool MoveComponent::atTarget()
{
    return pointDistance({rect.x + rect.z/2, rect.y + rect.a/2},target) <= .0001;
}

RenderComponent::RenderComponent(Entity& entity) : Component(entity), ComponentContainer<RenderComponent>(entity)
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
