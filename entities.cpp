#include "SDLHelper.h"
#include "render.h"

#include "entities.h"
#include "game.h"
#include "ants.h"

const glm::vec2 ClickableComponent::spacing = {10,5};

ClickableComponent::ClickableComponent(std::string name, Unit& entity) : Component(entity), ComponentContainer<ClickableComponent>(entity), name(name)
{

}

void ClickableComponent::click(bool val)
{
    clicked = val;
}

bool ClickableComponent::getClicked()
{
    return clicked;
}

void ClickableComponent::update()
{
    const glm::vec4* unitRect = &(entity->getComponent<RectComponent>()->getRect());
    int size = buttons.size();
    bool stillClicked = false;
    glm::vec2 mousePos = GameWindow::getCamera().toWorld({MouseManager::getMousePos().first, MouseManager::getMousePos().second});
    if (clicked)
    {
        int offset = 0;
        for (int i = 0; i < size; ++i) //render and update all buttons
        {
            const glm::vec4* rect = &(buttons[i]->getRect());
            glm::vec4 buttonRect = {unitRect->x + unitRect->z + spacing.x, unitRect->y + offset,rect->z,rect->a};
            buttons[i]->changeRect(buttonRect);
            glm::vec4 disp = GameWindow::getCamera().getRect();
            buttons[i]->render(-disp.x,-disp.y);
            //GameWindow::requestRect(buttonRect,{0,1,0,1},true,0,0);
            if (pointInVec(buttonRect,mousePos.x,mousePos.y,0))
            {
                stillClicked = true;
                if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT)
                {
                        buttons[i]->press();
                }
                break;
            }
            offset += rect->a + spacing.y;
        }
        glm::vec2 dimen = {unitRect->z, unitRect->a};
        GameWindow::requestRect({unitRect->x + unitRect->z/2 -dimen.x/2,unitRect->y + unitRect->a/2 -dimen.y/2,dimen.x,dimen.y},GameWindow::selectColor,true,0,1);
    }
        stillClicked = stillClicked || (!MouseManager::isPressed(SDL_BUTTON_LEFT) && clicked)  || (MouseManager::isPressed(SDL_BUTTON_LEFT) && vecIntersect(GameWindow::getSelection(),*unitRect));
        click(stillClicked);
}
void ClickableComponent::display(const glm::vec4& rect)
{
    Font::alef.write(Font::wordProgram,{name,rect});
}

void ClickableComponent::addButton(Button& button)
{
    buttons.emplace_back(&button);
}

RectRenderComponent::RectRenderComponent(const glm::vec4& color, Unit& unit) : RenderComponent(unit), ComponentContainer<RectRenderComponent>(unit), color(color)
{

}

void RectRenderComponent::update()
{
    GameWindow::requestRect(((Unit*)entity)->getRect().getRect(),color,true,0,0);
}

Unit::Unit(ClickableComponent& click, RectComponent& rect, RenderComponent& render, HealthComponent& health) : clickable(&click), rect(&rect), render(&render), health(&health)
{
    addComponent(click);
    addComponent(rect);
    addComponent(render);
    addComponent(health);
}

RectComponent& Unit::getRect()
{
    return *rect;
}

ClickableComponent& Unit::getClickable()
{
    return *clickable;
}

RenderComponent& Unit::getRender()
{
    return *render;
}

bool Unit::clicked()
{
    return clickable->getClicked();
}

glm::vec2 Unit::getCenter()
{
    const glm::vec4* ptr = &(rect->getRect());
    return {ptr->x + ptr->z/2, ptr->y + ptr->a/2};
}

void Unit::interact(Ant& ant)
{

}

Manager& Unit::getManager()
{
    return *manager;
}

void Unit::setManager(Manager& manager)
{
    this->manager = &manager;
}

ResourceComponent::ResourceComponent(Entity& entity, int amount) : Component(entity),ComponentContainer<ResourceComponent>(entity), amount(amount)
{

}

void ResourceComponent::collect(Ant& other)
{
    Ant::AntMoveComponent* antMove = other.getComponent<Ant::AntMoveComponent>();
    if (antMove)
    {
        antMove->setCarrying(1);
        amount -=1;
    }
}

void CorpseComponent::collect(Entity& other)
{
    /*if (entity->getComponent<HealthComponent>()->getHealth() <= 0)
    {
        ResourceComponent::collide(other);
    }*/
}

void CorpseComponent::update()
{
    if (entity->getComponent<HealthComponent>()->getHealth() <= 0)
    {
        if (render)
        {
            render->update();
        }
        if (amount <= 0)
        {
            ((Unit*)entity)->setDead(true);
        }
    }

}

Resource::ResourceRender::ResourceRender(Entity& entity) : RenderComponent(entity), ComponentContainer<ResourceRender>(entity)
{

}

void Resource::ResourceRender::update()
{
    GameWindow::requestRect(entity->getComponent<RectComponent>()->getRect(),{0,.5,1,1},true,0,0);
}

Resource::Resource(int x, int y, int amount) : Unit(*(new ClickableComponent("Resource",*this)), *(new RectComponent({x,y,100,100},*this)), *(new ResourceRender(*this)), *(new HealthComponent(*this, 10)))
{
    this->amount = amount;
}

void Resource::interact(Ant& ant)
{
    ant.setCarrying(10);
    amount -= 10;
    const glm::vec4* pos = &(rect->getRect());
    rect->setRect({pos->x,pos->y,pos->z*.9,pos->a*.9});
}

HealthComponent::HealthComponent(Entity& entity, double h, int height, int displacement) : Component(entity), ComponentContainer<HealthComponent>(entity),health(h), maxHealth(h), height(height), displacement(displacement)
{

}

void HealthComponent::addHealth(int amount)
{
    if (amount < 0)
    {
        if (!invincible.isSet())
        {
            invincible.set();
            health = std::max(0.0,std::min(health + amount, maxHealth));
        }
    }
}

int HealthComponent::getHealth()
{
    return health;
}

void HealthComponent::update()
{
    RectComponent* rectComp = entity->getComponent<RectComponent>();
    if (rectComp && visible)
    {
        const glm::vec4* rect = &(rectComp->getRect());
        GameWindow::requestRect({rect->x ,rect->y - displacement, health/maxHealth*rect->z, height},{1,0,0,1},true,0,0);
    }

    if (invincible.isSet() && invincible.framesPassed(10))
    {
        invincible.reset();
    }
}

Bug::BugMove::BugMove(double speed, const glm::vec4& rect, Unit& unit) : MoveComponent(speed, rect, unit), ComponentContainer<BugMove>(unit)
{

}

void Bug::BugMove::update()
{
    if (atTarget())
    {
        double angle = rand()%360*M_PI/180.0;
        int maxDimen = std::max(rect.z,rect.a);
        double radius = rand()%(100 - 10) + maxDimen + 10;
        glm::vec2 point = {rect.x + rect.z/2 + cos(angle)*radius, rect.y + rect.a/2 + sin(angle)*radius};
        const glm::vec4* levelRect = &(GameWindow::getLevel().getRect());
        point.x = std::max(levelRect->x, std::min(point.x, levelRect->x + levelRect->z - rect.z));
        point.y = std::max(levelRect->y, std::min(point.y, levelRect->y + levelRect->a - rect.a));
        setTarget(point);
    }
    else
    {
        MoveComponent::update();
    }
}

Bug::Bug(int x, int y) : Unit(*(new ClickableComponent("Bug", *this)), *(new BugMove(.02,{x,y,64,64},*this)), *(new RectRenderComponent({1,.5,0,1},*this)),*(new HealthComponent(*this, 100)))
{
    //getComponent<MoveComponent>()->setTarget({0,0});
    addComponent(*(new CorpseComponent(*this, 101)));
}
