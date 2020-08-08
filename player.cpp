
#include "vanilla.h"
#include "FreeTypeHelper.h"

#include "player.h"
#include "game.h"
#include "navigation.h"
#include "animation.h"

const glm::vec4 Player::selectColor = {1,1,1,.5};

bool Player::updateSelect()
{
    if (MouseManager::isPressed(SDL_BUTTON_LEFT))
    {
        std::pair<int,int> mouse_Pos = MouseManager::getMousePos();
        glm::vec2 mousePos = GameWindow::getCamera().toWorld({mouse_Pos.first, mouse_Pos.second});
      //  std::cout << mousePos.x << " " << mousePos.y << std::endl;
        if (MouseManager::getJustClicked() != SDL_BUTTON_LEFT)
        {

            double width = mousePos.x - origin.x;
            double height = mousePos.y - origin.y;
            selection = absoluteValueRect({origin.x,origin.y, width, height});
        }
        else
        {
            origin.x = mousePos.x;
            origin.y = mousePos.y;
        }
        return true;
    }
    return false;
}

int Player::getResource()
{
    return resource;
}

void Player::addResource(int r)
{
    resource = std::max(resource + r,0);
}

void Player::update()
{
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
   // Font::tnr.requestWrite({"Resources: " + convert(resource),GameWindow::getCamera().toAbsolute({screenDimen.x - 200, 50, 100,100}),0,{0,0,0,1},GameWindow::interfaceZ});

    switch(KeyManager::getJustPressed())
    {
    case SDLK_b:
        if (mode == BUILDING)
        {
            mode = SELECTING;
        }
        else
        {
            mode = BUILDING;
        }
        break;
    }

    switch (mode)
    {
        case SELECTING:
            if (updateSelect())
            {
                GameWindow::requestRect(selection,selectColor,true,0,-.1);
            }
            break;
        case BUILDING:
            auto mousePos = GameWindow::getCamera().toWorld(pairtoVec(MouseManager::getMousePos()));
            glm::vec4 color = selectColor;
            auto assembler = FactoryAssembler();
            glm::vec2 dimen = assembler.getDimen();
            glm::vec4 structRect = {mousePos.x - dimen.x/2,mousePos.y - dimen.y/2,dimen.x, dimen.y};
            bool collides = !GameWindow::getLevel().getMesh().notInWall(structRect);
            if (!collides)
            {
                Map* level = &GameWindow::getLevel();
                RawQuadTree* tree = level->getTree();
                auto vec = tree->getNearest(structRect);
                int size = vec.size();
                for (int i = 0; i < size; ++i)
                {
                    if (vec[i]->collides(structRect))
                    {
                        collides = true;
                        break;
                    }
                }
            }
            if (collides)
            {
                color = {1,0,0,1};
            }
            GameWindow::requestRect(structRect,color,true,0,0,0);
            if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT && !collides && getResource() >= assembler.getProdCost())
            {
                Object* ptr = (assembler.assemble());
                RectComponent* rect = &ptr->getRect();
                rect->setPos({mousePos.x - dimen.x/2, mousePos.y - dimen.y/2});
                InactiveComponent* inactive = new InactiveComponent(assembler.getProdTime(),*ptr);
                ptr->addComponent(*inactive);
                GameWindow::getLevel().addUnit(*(ptr), ptr->getFriendly());
                inactive->init();
                addResource(-1*assembler.getProdCost());
            }
            break;
    }
    if (mode != SELECTING)
    {
        selection = {0,0,0,0};
    }

}

const glm::vec4& Player::getSelection()
{
    return selection;
}

InactiveComponent::InactiveComponent(double duration, Entity& entity) : waitTime(duration), Component(entity), ComponentContainer<InactiveComponent>(entity)
{

}

void InactiveComponent::init()
{
    timeLeft.set();
}

bool InactiveComponent::done()
{
    return timeLeft.timePassed(waitTime);
}

void InactiveComponent::render()
{
    RenderComponent* render = entity->getComponent<RenderComponent>();
    RectComponent* rect = entity->getComponent<RectComponent>();
    if (render && rect)
    {
        glm::vec4 renderRect = GameWindow::getCamera().toScreen(rect->getRect());
        render->render({renderRect,0,NONE,{.5,.5,.5,1}});
        renderTimeMeter({renderRect.x,renderRect.y + 1.1*renderRect.a, renderRect.z,20 },
                        {.5,.5,.5,1},timeLeft,waitTime,GameWindow::interfaceZ);
      //  Font::tnr.requestWrite({convert((waitTime - (SDL_GetTicks() - timeLeft.getTime()))/1000.0),renderRect,0,{1,1,1,1},GameWindow::fontZ});
    }
}

CreateEnergyComponent::CreateEnergyComponent(Player& player_, int frames, Entity& entity) : player(&player_), waitTime(frames), Component(entity), ComponentContainer<CreateEnergyComponent>(entity)
{

}

void CreateEnergyComponent::update()
{
    if (player && alarm.framesPassed(waitTime))
    {
        player->addResource(1);
        alarm.set();
    }
    else if (!alarm.isSet())
    {
        alarm.set();
    }
}

Factory::Factory(int x, int y) : Structure(*(new ClickableComponent("Factory", *this)),*(new RectComponent({x,y,30,30}, *this)),
                                           *(new RectRenderComponent({.5,.5,.5,1},*this)), *(new HealthComponent(*this,100)))
{
    addComponent(*(new CreateEnergyComponent(GameWindow::getPlayer(),1000, *this)));
}

FactoryAssembler::FactoryAssembler() : UnitAssembler("Factory",{30,30}, &defaultAnime, false, 100,1000)
{
    prodCost = 10;
}

Object* FactoryAssembler::assemble()
{
    Unit* stru = static_cast<Unit*>(UnitAssembler::assemble());
    stru->setFriendly(true);
    stru->addComponent(*(new CreateEnergyComponent(GameWindow::getPlayer(),1000,*stru)));

    return stru;
}
