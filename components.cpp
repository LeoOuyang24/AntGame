#include <iostream>

#include "vanilla.h"
#include "SDLHelper.h"
#include "render.h"

#include "components.h"

Component::Component(Entity& entity) : ComponentContainer<Component>(entity)
{
    this->entity = &entity;
}

Entity& Component::getEntity()
{
    return *entity;
}

RectComponent::RectComponent(const glm::vec4& rect, Entity& entity) : Component(entity), ComponentContainer<RectComponent>(entity), RectPositional(rect)
{

}

void RectComponent::setRect(const glm::vec4& rect)
{
    this->rect = rect;
}

MoveComponent::MoveComponent(double speed, const glm::vec4& rect, Entity& entity) : RectComponent(rect, entity), ComponentContainer<MoveComponent>(entity), speed(speed)
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
}

bool MoveComponent::atTarget()
{
    return pointDistance(getCenter(),target) <= .0001;
}

void MoveComponent::teleport(const glm::vec2& point)
{
    rect.x = point.x - rect.z/2; //rect.x += point.x - (rect.x + rect.z/2) -> rect.x = point.x - rect.z/2
    rect.y = point.y - rect.a/2;
    setTarget(point);

}

RenderComponent::RenderComponent(Entity& entity) : Component(entity), ComponentContainer<RenderComponent>(entity)
{

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
