#include "SDLHelper.h"
#include "vanilla.h"

#include "debug.h"
#include "game.h"
#include "sequence.h"
#include "navigation.h"
#include "enemyAssemblers.h"

SpriteWrapper frame;

Manager::Manager()
{

}
const ObjPtr Manager::getSelectedUnit() const
{
    return selectedUnit;
}

void Manager::init(const glm::vec4& region)
{
   // tree.reset(new RawQuadTree(region));
}

void Manager::updateEntities()
{
    Map* level = (GameWindow::getLevel());
    glm::vec4 levelRect = level->getRect();
    ObjectStorage* entities = &(level->getEntities());
    RawQuadTree* tree = level->getTree();
    AntManager* newTask = nullptr;
    auto it2 = entities->begin();
    auto end = entities->end(); //we do this in case we added an object while looping. This ensures we still iterate as if the object wasn't added.
   // const glm::vec4* selectRect = &(GameWindow::getSelection());
    bool clicked = MouseManager::getJustClicked() == SDL_BUTTON_LEFT;
    int released = MouseManager::getJustReleased();
   // chunk->update();
   // std::cout << mousePos.x << " " << mousePos.y << std::endl;
    Object* newSelected = nullptr;
    int index = 0;
    for (auto i = entities->begin(); i !=  end; i = it2)
    {
        ++it2;
        Object* current = i->first;
        TangibleComponent* tang = current->getComponent<TangibleComponent>();
        if (tang && !tang->getTangible()) //don't process intangible objects
        {
            continue;
        }
        RectPositional* rectPos = &(current->getRect());
      //  std::cout << rectPos->getRect().x << std::endl;
        RawQuadTree* oldTree = tree->find(*rectPos);
        HealthComponent* health = current->getComponent<HealthComponent>();
        if (oldTree && vecIntersect(rectPos->getRect(),levelRect) && !current->getDead() && (!health || health->getHealth() > 0))
        {
            InactiveComponent* inactive = current->getComponent<InactiveComponent>();
            if (!inactive || inactive->done())
            {
                current->update();
                if (vecIntersect(rectPos->getRect(),levelRect))
                {
                    tree->update(*rectPos,*oldTree);
                    positionalVec vec = tree->getNearest(*(rectPos));
                    for (int j = vec.size() - 1; j >= 0; j --)
                    {
                        Entity* ptr = &(((RectComponent*)vec[j])->getEntity());
                        if ( ptr != i->first && (!static_cast<Object*>(ptr)->getDead()) && vec[j]->collides(rectPos->getRect()))
                        {
                            i->second->collide(*ptr);
                            ptr->collide(*(i->second.get()));
                        }
                    }
                    if (current->getFriendly())
                    {
                        //std::cout << current << std::endl;
                        GameWindow::getFogMaker().requestPolyFog(rectPos->getCenter(),100,10);
                    }
                }
                else
                {
                    current->onDeath();
                    level->remove(*(current));
                }
            }
            else
            {
                inactive->render();
            }
        }
        else
        {
            current->onDeath();
            level->remove(*(current));
        }
        index ++;
    }
    if (clicked)
    {
        if (!newSelected)
        {
            selectedUnit.reset();
        }
        else
        {
            selectedUnit = entities->at(newSelected);
        }
    }
}

void Manager::update()
{

    updateEntities();


}


class AntClickable;

double Camera::minZoom = .1, Camera::maxZoom = 2;
Camera::Camera()
{

}

void Camera::init(Unit& play, int w, int h)
{
    RenderCamera::init(w,h);
    baseDimen = {w,h};
    player = &play;
    move = new MoveComponent(1,rect,*this);
    addComponent(*(move));
}

void Camera::update()
{
    if ( (zoomAmount != zoomGoal && zoomGoal != -1))
    {
        if (!move->atTarget())
        {
            move->update();
            rect = move->getRect();
        }
        if (zoomAmount != zoomGoal)
        {
            zoom(absMin(zoomGoal - zoomAmount,convertTo1(zoomGoal - zoomAmount)*zoomSpeed*DeltaTime::deltaTime));
        }
    }
    else
    {
        auto mousePos = MouseManager::getMousePos();
        auto screenDimen = RenderProgram::getScreenDimen();
        /*int speed = 1;
        if (mousePos.first >= screenDimen.x - 1 || mousePos.first <= 0)
        {
            rect.x += absMin((mousePos.first - speed), (mousePos.first - screenDimen.x +1 + speed ))*DeltaTime::deltaTime;
            //std::cout << absMin((mousePos.first - speed), (mousePos.first - screenDimen.x +1 + speed ))*DeltaTime::deltaTime << "\n";
        }
        if (mousePos.second >= screenDimen.y - 1 || mousePos.second <= 0)
        {
            rect.y +=  absMin((mousePos.second - speed), (mousePos.second - screenDimen.y + 1+ speed ))*DeltaTime::deltaTime;
        }*/
        rect.x = std::max(bounds.x,std::min(bounds.x + bounds.z - rect.z,player->getCenter().x - rect.z/2));
        rect.y = std::max(bounds.y, std::min(bounds.y + bounds.a  - rect.a , player->getCenter().y - rect.a/2));
        zoomGoal = -1;

        auto mouseWheel = MouseManager::getMouseWheel();
        if (mouseWheel.second > 0)
        {

            zoom(-.1);//,toWorld({mousePos.first, mousePos.second}));
        }
        if (mouseWheel.second < 0)
        {
            zoom(.1);//,toWorld({mousePos.first, mousePos.second}));
        }
        if (KeyManager::getJustPressed() == SDLK_SPACE)
        {
            resetZoom();
        }
        //    GameWindow::requestRect({screenDimen.x/2 - 5, screenDimen.y/2 - 5,10,10},{0,0,0,1},true,0,0,true);

    }
      //  GameWindow::requestNGon(4,{screenDimen.x/2, screenDimen.y/2},10,{0,0,0,1},0,true,0,true ); //renders a small square at the camera's center

}

void Camera::setBounds(const glm::vec4& newBounds)
{
    bounds = newBounds;
}


void Camera::center(const glm::vec2& point)
{
    rect.x = point.x - rect.z/2;
    rect.y = point.y - rect.a/2;
}

void Camera::zoom(float amount)
{
    zoomAmount = std::max(minZoom,std::min(maxZoom,zoomAmount+ amount));
    glm::vec2 zoomDimen = {zoomAmount*baseDimen.x,zoomAmount*baseDimen.y};

    //if (vecContains(glm::vec4((rectCenter - glm::vec2(rect.z/2, rect.a/2)),rect.z,rect.a), bounds))
    rect.x = std::min(std::max(bounds.x,rect.x - (zoomDimen.x - rect.z)/2),bounds.x + bounds.z - zoomDimen.x);
    rect.y = std::min(std::max(bounds.y,rect.y - (zoomDimen.y - rect.a)/2),bounds.y + bounds.a - zoomDimen.y);
    rect.z = zoomDimen.x;
    rect.a = zoomDimen.y;
    RenderProgram::setXRange(0,rect.z);
    RenderProgram::setYRange(0,rect.a);
}

void Camera::zoom(float amount, const glm::vec2& point)
{
    center(point);
    zoom(amount);
}

void Camera::setZoomTarget(double goal)
{
    zoomGoal = (goal);
}

void Camera::setZoomTarget(double goal, double speed )
{
    setZoomTarget(goal);
    zoomSpeed = speed;
}

void Camera::resetZoom()
{
    zoom(1- zoomAmount);
}

bool Camera::isZooming()
{
    return zoomGoal != -1;
}

void Camera::close()
{
    removeComponent<MoveComponent>();
}

Camera::~Camera()
{

}

float GameWindow::menuHeight = 1; //is set in the GameWindow constructor
Camera GameWindow::camera;
Manager GameWindow::manager;
std::weak_ptr<Map> GameWindow::level;
Debug GameWindow::debug;
Window* GameWindow::gameOver = nullptr;
GameWindow* GameWindow::actualWindow = nullptr;
Player GameWindow::player;
FogMaker GameWindow::fogMaker;
float GameWindow::interfaceZ = 3;
float GameWindow::fontZ = GameWindow::interfaceZ + 1;
bool GameWindow::renderAbsolute = false;

GameWindow::GameWindow() : Window({0,0},nullptr,{0,0,0,0})
{
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    menuHeight = .2*screenDimen.y;
    rect.z = screenDimen.x;
    rect.a = screenDimen.y;
    player.init();
    camera.init(*player.getPlayer(),screenDimen.x,screenDimen.y);
    actualWindow = this;
    Window::camera = &camera;
  /*  auto ptr = evilMoonAssembler.assemble();
    ptr->getComponent<UnitAttackComponent>()->setLongTarget({0,0},&level.getUnit(level.getAnthill()));
    level.addUnit(*ptr,0,0,false);*/

   // level.addUnit(*(new Dummy(levelRect.z/2 - 100,levelRect.a/2)));

   // camera.setZoomTarget(.5);
   // manager.clear();
}

void GameWindow::onSwitch(Window& from)
{
    camera.resetZoom();
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    Map* levelPtr = level.lock().get();
    if (levelPtr)
    {
        glm::vec4 levelRect = levelPtr->getRect();
        camera.setBounds({levelRect.x,levelRect.y,levelRect.z,levelRect.a + menuHeight});
        camera.center({levelRect.z/2,levelRect.a/2});
        levelPtr->addUnit(*player.getPlayer(),levelRect.z/2,levelRect.a/2,true);
        gameOver = new Window({screenDimen.x/10, screenDimen.y/10},nullptr, {1,0,0,1});
        gameOver->addPanel(*(new QuitButton(*this)));
        manager.init(levelPtr->getRect());
        //levelPtr->addUnit(*(playerAssembler.assemble()),levelRect.z/2 + 100,levelRect.a/2 + 100,true);
    }
    debug.init();
   // player.init();
}

void GameWindow::updateTop(float z)
{
   /* if (!hill)
    {
        gameOver->update(x,y,clicked);
        level.render();
    }
    else */if (KeyManager::getJustPressed() == SDLK_n)
    {
        //level.nextLevel();
        level.lock().get()->setChangeLevel(true);
    }
    else
    {
        renderAbsolute = false;
        camera.update();

        int size = labels.size();
        for (int i = 0; i < size; ++i)
        {
            if (!labels[i]->isDead())
            {
                labels[i]->update();
            }
        }

        debug.update();
        level.lock().get()->update();
        manager.update();
        renderAbsolute = true;
    // std::cout << camera.getRect().x << " " << camera.getRect().x + camera.getRect().z << std::endl;
       // camera.reserveZoom();
        renderTopBar();
        //renderSelectedUnits();

        if (level.lock().get()->getChangeLevel())
        {
            switchToMap->press();
        }

        Window::updateTop(z);
                //camera.goBack();
        //requestRect({0,0,320,320},{1,0,1,1},true,0,1,1);

    }
    //printRect(getCamera().getRect());
    //std::cout << ComponentContainer<RenderComponent>::components.size() << std::endl;
}

void GameWindow::renderTopBar()
{
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    glm::vec4 menuRect = camera.toAbsolute({0,0,screenDimen.x,menuHeight});
    //PolyRender::requestRect(menuRect,{1,0,0,1},true,0,interfaceZ);
    Font::tnr.requestWrite({"Resources: " + convert(player.getResource()),camera.toAbsolute({screenDimen.x - .2*screenDimen.x, .01*screenDimen.y
                                                                                    , -1,.6}),0,{1,1,1,1},GameWindow::fontZ});
    Font::tnr.requestWrite({"Gold: " + convert(player.getGold()),camera.toAbsolute({screenDimen.x - .2*screenDimen.x, .05*screenDimen.y
                                                                                    , -1,.6}),0,{1,1,1,1},GameWindow::fontZ});

}

void GameWindow::renderSelectedUnits()
{
    const glm::vec4* cameraRect = &(camera.getRect());
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    glm::vec4 wholeRect = {0,rect.a - menuHeight,rect.z, menuHeight};
   // printRect(wholeRect);
    //player.render({wholeRect.x + wholeRect.z - 1.1*wholeRect.a, wholeRect.y - wholeRect.a, wholeRect.a,wholeRect.a});
    glm::vec4 selectedAntsRect = {0,wholeRect.y, .7*wholeRect.z, wholeRect.a};
    glm::vec4 selectedUnitRect = {selectedAntsRect.x + selectedAntsRect.z, wholeRect.y, wholeRect.z - selectedAntsRect.z, wholeRect.a};
    glm::vec2 margin = {.05*wholeRect.z,.2*menuHeight}; //the horizontal and vertical margins to render the selected ants and selected unit
    int antsPerRow = 10;
    float antRectWidth = .03*rect.z; //width and height of each of the outlineRects. This is a bit of a magic number and was chosen just because it looks good
    glm::vec2 spacing = {(selectedAntsRect.z - margin.x*2 - antsPerRow*antRectWidth)/antsPerRow,.3*menuHeight}; //horizontal and vertical spacing between ants and unit and health bar

    PolyRender::requestLine({selectedUnitRect.x*cameraRect->z/screenDimen.x,
                            selectedUnitRect.y*cameraRect->a/screenDimen.y,
                            selectedUnitRect.x*cameraRect->z/screenDimen.x,
                             (selectedUnitRect.y + selectedUnitRect.a)*cameraRect->a/screenDimen.y},{0,0,0,1},interfaceZ);

    /*Object* selectedUnit = manager.getSelectedUnit().lock().get();
    if (selectedUnit)
    {
        glm::vec4 selectedRect = {selectedUnitRect.x + margin.x, selectedUnitRect.y + margin.y ,selectedUnitRect.a - margin.y*2, selectedUnitRect.a - margin.y*2};
        selectedUnit->getRender().render({camera.toAbsolute(selectedRect),0,NONE,{1,1,1,1},&RenderProgram::basicProgram,fontZ});
        //printRect(camera.toAbsolute(selectedRect));

       // GameWindow::requestRect(selectedRect,{0,0,0,1},false,0,interfaceZ);
        HealthComponent* health = selectedUnit->getComponent<HealthComponent>();
        if (health)
        {
            health->render({selectedRect.x + selectedRect.z + spacing.x, selectedRect.y , selectedRect.z}, interfaceZ);
        }
        selectedUnit->getClickable().click(true);
        selectedUnit->getClickable().display(selectedAntsRect);
    }*/
    GameWindow::requestRect(wholeRect,{0,1,0,1},true,0,interfaceZ,true);
}

float GameWindow::getMenuHeight()
{
    return menuHeight;
}

Camera& GameWindow::getCamera()
{
    return camera;
}
const Manager& GameWindow::getManager()
{
    return manager;
}
Map* GameWindow::getLevel()
{
    return level.lock().get();
}

void GameWindow::setLevel(std::shared_ptr<Map>& map)
{
    level = map;
}

Player& GameWindow::getPlayer()
{
    return player;
}

FogMaker& GameWindow::getFogMaker()
{
    return fogMaker;
}

void GameWindow::setWorldMap(WindowSwitchButton& butt)
{
    switchToMap = &butt;
}

void GameWindow::staticAddPanel(Panel& panel, bool absolute)
{
    if (actualWindow)
    {
        actualWindow->addPanel(panel,absolute);
    }
}

void GameWindow::requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z, bool absolute)
{
    bool oldAbs = renderAbsolute;
    renderAbsolute = renderAbsolute || absolute;
    glm::vec2 c = camera.toScreen(center);
    const glm::vec4* cameraRect = &(camera.getRect());
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    if (renderAbsolute)
    {
        c = {(center.x )/screenDimen.x*cameraRect->z, (center.y)/screenDimen.y*cameraRect->a};
        side = side/screenDimen.x*cameraRect->z;
    }
    PolyRender::requestNGon(n,c ,side,color,angle,filled,z);
    if (absolute)
    {
        renderAbsolute = oldAbs;
    }
}

void GameWindow::requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z, bool absolute)
{
    bool oldAbs = renderAbsolute;
    renderAbsolute = renderAbsolute || absolute;
    glm::vec4 renderRect = rect;
    if (renderAbsolute)
    {

        renderRect = camera.toAbsolute(renderRect);
    }
    else
    {
        renderRect = camera.toScreen(rect);
    }
    PolyRender::requestRect(renderRect,color,filled,angle,z);
    if (absolute)
    {
        renderAbsolute = oldAbs;
    }
}

void GameWindow::requestLine(const glm::vec4& line, const glm::vec4& color, float z, bool absolute)
{
    glm::vec4 lineCopy = line;
    if (absolute)
    {
        lineCopy = glm::vec4(
                            getCamera().toAbsolute(glm::vec2(line.x,line.y)),
                            getCamera().toAbsolute(glm::vec2(line.z,line.a))
                             );
    }
    else
    {
        lineCopy =      glm::vec4(
                        getCamera().toScreen(glm::vec2(line.x,line.y)),
                        getCamera().toScreen(glm::vec2(line.z,line.a))
                         );
    }
    PolyRender::requestLine(
                             lineCopy,
                            color,
                            z);
}

void GameWindow::close()
{
    camera.close();
    level.reset();
}

GameWindow::QuitButton::QuitButton(GameWindow& window_) : Button({10,50,32,32},nullptr,nullptr, {"Quit"},&Font::tnr,{0,1,0,1}), window(&window_)
{

}
void GameWindow::QuitButton::press()
{
    window->quit = true;
}
