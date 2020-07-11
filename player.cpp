
#include "vanilla.h"
#include "FreeTypeHelper.h"

#include "player.h"
#include "game.h"
#include "navigation.h"

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


void Player::addResource(int r)
{
    resource = std::max(resource + r,0);
}

void Player::update()
{
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    Font::tnr.requestWrite({"Resources: " + convert(resource),GameWindow::getCamera().toAbsolute({screenDimen.x - 200, 50, 100,100}),0,{0,0,0,1},1});

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
                RawQuadTree* tree = level->getTree(level->getCurrentChunk());
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
            if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT && !collides)
            {
                Unit* ptr = static_cast<Unit*>(assembler.assemble());
                RectComponent* rect = &ptr->getRect();
                rect->setPos({mousePos.x - dimen.x/2, mousePos.y - dimen.y/2});
                GameWindow::getLevel().addUnit(*(ptr));
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

FactoryAssembler::FactoryAssembler() : UnitAssembler({30,30},"Factory", 100)
{

}

Entity* FactoryAssembler::assemble()
{
    Structure* stru = new Structure(*(new ClickableComponent(name, *stru)),*(new RectComponent({0,0,dimen.x,dimen.y}, *stru)),
                                    *(new RectRenderComponent({.5,.5,.5,1},*stru)), *(new HealthComponent(*stru,maxHealth)));
    stru->addComponent(*(new CreateEnergyComponent(GameWindow::getPlayer(),1000,*stru)));

    return stru;
}
