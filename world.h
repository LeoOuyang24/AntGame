#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include "glInterface.h"
#include "SDLhelper.h"
#include "components.h"

class ClickableComponent : public Component, public ComponentContainer<ClickableComponent> //clickable component handles user inputs, like when the user selects the unit or presses a button
{
    static const glm::vec2 spacing; //spacing between the buttons and the unit
    std::string name = "";
    bool clicked = false;
    std::vector<std::unique_ptr<Button>> buttons;
public:
    ClickableComponent(std::string name, Entity& entity) : Component(entity), ComponentContainer<ClickableComponent>(entity), name(name)
    {

    }
    virtual void update();
    void click(bool val);
    bool getClicked();
    void addButton(Button& button);
    virtual void display(const glm::vec4& rect);
};

class RectRenderComponent : public RenderComponent, public ComponentContainer<RectRenderComponent>
{
    glm::vec4 color;
public:
    RectRenderComponent(const glm::vec4& color, Entity& unit) : RenderComponent(unit), ComponentContainer<RectRenderComponent>(unit), color(color)
    {

    }
    void update();
    virtual void render(const SpriteParameter& param);
};

class Object : public Entity//environment objects. Can be seen, have a hitbox, and can be clicked on
{
protected:
    bool dead = false;
    ClickableComponent* clickable = nullptr;
    RectComponent* rect = nullptr;
    RenderComponent* render = nullptr;
public:
    Object(ClickableComponent& click, RectComponent& rect, RenderComponent& render);
    RectComponent& getRect()
    {
        return *rect;
    }
    glm::vec2 getCenter()
    {
        return rect->getCenter();
    }
    ClickableComponent& getClickable()
    {
        return *clickable;
    }
    RenderComponent& getRender()
    {
        return *render;
    }
    bool clicked()
    {
        return clickable->getClicked();
    }
    bool inline getDead()
    {
        return dead;
    }
    void inline setDead(bool isDead)
    {
        dead = isDead;
    }
};

class Gate : public Object
{
public:
    Gate(int x, int y) : Object(*(new ClickableComponent("Gate",*this)),*(new RectComponent({x,y,64,64},*this)), *(new RenderComponent(*this)))
    {

    }
};

#endif // WORLD_H_INCLUDED
