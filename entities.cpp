#include "SDLHelper.h"
#include "render.h"

#include "entities.h"
#include "game.h"
#include "ants.h"

void renderMeter(const glm::vec3& xyWidth, const glm::vec4& color, double current, double maximum, float z)
{
    const int height = 10;
    glm::vec4 renderRect = {xyWidth.x,xyWidth.y,xyWidth.z*current/maximum,height};
    PolyRender::requestRect(renderRect,color,true, 0, z);
    PolyRender::requestRect(renderRect,{0,0,0,1},false,0,z);
}

const glm::vec2 ClickableComponent::spacing = {10,5};

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
        GameWindow::requestRect({unitRect->x + unitRect->z/2 -dimen.x/2,unitRect->y + unitRect->a/2 -dimen.y/2,dimen.x,dimen.y},GameWindow::selectColor,true,0,0.01);
    }
    /*bool becomeClicked = (MouseManager::isPressed(SDL_BUTTON_LEFT) && vecIntersect(GameWindow::getSelection(),*unitRect));
        if (entity->getComponent<Ant::AntMoveComponent>())
        {
           // std::cout << stillClicked << " " << (MouseManager::getJustReleased() != SDL_BUTTON_LEFT && clicked) << std::endl;
        }*/
        //stillClicked = stillClicked ;//|| (MouseManager::getJustClicked() != SDL_BUTTON_LEFT && clicked);//  ||  becomeClicked;*/
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

void RectRenderComponent::update()
{
    GameWindow::requestRect(((Unit*)entity)->getRect().getRect(),color,true,0,0);
}

void RectRenderComponent::render(const SpriteParameter& param)
{
    PolyRender::requestRect(param.rect,color*param.tint,true,param.radians,param.z);
}


void HealthComponent::addHealth(int amount)
{
    if (amount < 0)
    {
        if (invincible.framesPassed(10) || !invincible.isSet())
        {
            invincible.set();
            health = std::max(0.0,std::min(health + amount, maxHealth));
        }
    }
    else
    {
        health = std::max(0.0,std::min(health + amount, maxHealth));
    }
}

void HealthComponent::update()
{
    RectComponent* rectComp = entity->getComponent<RectComponent>();
    if (rectComp && visible)
    {
        const glm::vec4* rect = &(rectComp->getRect());
        //GameWindow::requestRect({rect->x ,rect->y - displacement, rect->z, 0},{1,0,0,1},true,0,0);
        render(glm::vec3(GameWindow::getCamera().toScreen({rect->x,rect->y - displacement}), rect->z), 0);
    }
}

void HealthComponent::render(const glm::vec3& rect, float z)
{
    //PolyRender::requestRect({rect.x,rect.y,health/maxHealth*rect.z,height},{1,0,0,1},true,0,rect.a);
    renderMeter({rect.x,rect.y,rect.z},{1,0,0,1},health,maxHealth,z);
}

Unit::Unit(ClickableComponent& click, RectComponent& rect, RenderComponent& render, HealthComponent& health) : clickable(&click), rect(&rect), render(&render), health(&health)
{
    addComponent(rect);
    addComponent(click);
    addComponent(render);
    addComponent(health);
}

void Unit::setManager(Manager& manager)
{
    this->manager = &manager;
}

bool Unit::clicked()
{
    return clickable->getClicked();
}

glm::vec2 Unit::getCenter()
{
    return rect->getCenter();
}

void Unit::interact(Ant& ant)
{

}

Manager& Unit::getManager()
{
    return *manager;
}

void AttackComponent::attack(HealthComponent* health)
{
    if (health && (attackTimer.framesPassed(endLag) || !attackTimer.isSet()))
    {
        health->addHealth(-1*damage);
        attackTimer.set();
    }
}

ResourceComponent::ResourceComponent(int amount, Entity& entity) : Component(entity), ComponentContainer<ResourceComponent>(entity), resource(amount), maxResource(amount)
{

}

void ResourceComponent::render(const glm::vec3& rect, float z)
{
    renderMeter(rect,{0,1,0,1},resource,maxResource,z);
}

void ResourceComponent::collect(Unit& other)
{
    if (entity && !((Unit*)entity)->getDead())
    {
          Ant::AntMoveComponent* antMove = other.getComponent<Ant::AntMoveComponent>();
        if (antMove)
        {
            antMove->setCarrying(1);
        }
        resource -=1;
        if (resource <= 0)
        {
            ((Unit*)(entity))->setDead(true);
        }
    }

}


void CorpseComponent::onDeath()
{
    if (entity)
    {
        Manager* manager = &(((Unit*)entity)->getManager());
        ResourceUnit* resource = new ResourceUnit(amount,entity->getComponent<RectComponent>()->getRect());
        manager->addEntity(*(resource));
    }
}

ResourceUnit::ResourceUnit(int resources, const glm::vec4& rect) : Unit(*(new ClickableComponent("Resource", *this)), *(new RectComponent(rect, *this)), *(new RectRenderComponent({1,1,1,1},*this)), *(new HealthComponent(*this,1,false)))
{
    addComponent(*(new ResourceComponent(resources,*this)));
    health->setVisible(false);
}

void WanderMove::update()
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

template <typename T>
Unit* ApproachComponent::findNearestUnit(double radius)
{
    Unit* owner = ((Unit*)entity);
    Entity* closest = nullptr;
    double minDistance = -1;
    if (owner)
    {
        Manager* manager = &(owner->getManager());
        if (manager)
        {
            RawQuadTree* tree = manager->getQuadTree();
            if (tree)
            {
                glm::vec2 center = owner->getRect().getCenter();
                std::vector<Positional*> nearby = tree->getNearest( center , radius);
                int size = nearby.size();
                for (int i = 0; i < size; ++i)
                {
                    Entity* current = &(static_cast<RectComponent*>(nearby[i])->getEntity());
                    if (current->getComponent<T>())
                    {
                        double distance = current->getComponent<RectComponent>()->distance(center);
                        if ((distance < minDistance || minDistance == -1) && current != owner)
                        {
                            minDistance = distance;
                            closest = current;
                        }
                    }
                }
            }
        }
    }
    return static_cast<Unit*>(closest);
}

void ApproachComponent::setTarget(const glm::vec2& target, const std::shared_ptr<Unit>* unit)
{
    if (unit)
    {
        const glm::vec4* tarRect = &((*unit)->getRect().getRect());
        displacement = {target.x - (tarRect->x + tarRect->z/2) , target.y - (tarRect->y + tarRect->a/2)};
        targetUnit = *unit;
    }
    else
    {
        targetUnit.reset();
    }
    move->setTarget(target);
}

void ApproachComponent::setTarget(const std::shared_ptr<Unit>& unit)
{
    setTarget(unit->getCenter(),&unit);
}

void ApproachComponent::update()
{
    if (move)
    {
        Unit* ptr = targetUnit.lock().get();
        if (ptr)
        {
            glm::vec2 center = ptr->getRect().getCenter() + displacement;
            if (move->collides(ptr->getRect().getRect()))
            {
                move->setTarget(entity->getComponent<RectComponent>()->getCenter());
            }
            else
            {
                move->setTarget(center);
            }
        }
       // move->update();
    }
}

Mushroom::Mushroom(int x, int y) : Unit(*(new ClickableComponent("Mushroom", *this)), *(new RectComponent({x,y,10,10},*this)),*(new RectRenderComponent({0,.5,1,1},*this)),*(new HealthComponent(*this,1)))
{
    addComponent(*(new ResourceComponent(rand()%5,*this)));
    health->setVisible(false);
}

Bug::Bug(int x, int y) : Unit(*(new ClickableComponent("Bug", *this)), *(new WanderMove(.02,{x,y,64,64},*this)), *(new RectRenderComponent({1,.5,1,1},*this)),*(new HealthComponent(*this, 100)))
{
    //getComponent<MoveComponent>()->setTarget({0,0});
    addComponent(*(new CorpseComponent(*this, 100)));
    addComponent(*(new ResourceEatComponent(*this)));
}

Beetle::Beetle(int x, int y) : Unit(*(new ClickableComponent("Beetle", *this)), *(new WanderMove(.02,{x,y,64,64},*this)), *(new RectRenderComponent({1,.5,0,1},*this)),*(new HealthComponent(*this, 100)))
{
    addComponent(*(new AttackComponent(1,10,*this)));
    addComponent(*(new BeetleMove(*this)));
    addComponent(*(new CorpseComponent(*this,200)));
}

void Beetle::BeetleMove::update()
{
    Unit* targetPtr = getTargetUnit();
    Unit* owner = ((Unit*)entity);
    if (owner)
    {
        Manager* manager = &(owner->getManager());
        if (manager)
        {
            Unit* nearest = findNearestUnit<Ant::AntMoveComponent>(100);
            if (nearest)
            {
                setTarget(manager->getAnt(static_cast<Ant*>(nearest)));
            }
        }
        if (targetPtr)
        {
            if ( targetPtr->getRect().collides(owner->getRect().getRect()))
            {
                AttackComponent* attack = owner->getComponent<AttackComponent>();
                if (attack)
                {
                    attack->attack(targetPtr->getComponent<HealthComponent>());
                }
            }
        }

    }
    ApproachComponent::update();
}

void ResourceEatComponent::update()
{
    Unit* targetPtr = getTargetUnit();
    Unit* owner = ((Unit*)entity);
    if (owner)
    {
        if (!targetPtr)
        {
            Manager* manager = &(owner->getManager());
            if (manager)
            {
                Unit* nearest = findNearestUnit<ResourceComponent>(500);
                if (nearest)
                {
                    setTarget((manager->getUnit(static_cast<Unit*>(nearest))));
                }
            }
        }
        else
        {
            if ( targetPtr->getRect().collides(owner->getRect().getRect()))
            {
                targetPtr->getComponent<ResourceComponent>()->collect(*targetPtr);
            }
        }
    }
    ApproachComponent::update();
}

