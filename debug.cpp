#include "SDLhelper.h"

#include "debug.h"
#include "world.h"
#include "navigation.h"
#include "game.h"

void Debug::DebugNavMesh::DebugNavMesh::update()
{

    if (renderNodes)
    {

        mesh->nodeTree.map([](const Positional& pos){
                            const NavMesh::NavMeshNode* ptr = (static_cast<const NavMesh::NavMeshNode*>(&pos));
                            GameWindow::requestRect(ptr->getRect(),{0,0,0,1},false,0,1,false);
                            ptr->render();
                            });
    }
    if (Debug::getRenderPath())
    {
        auto mousePos = MouseManager::getMousePos();
        Camera* cam = &(GameWindow::getCamera());
       if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT)
       {
           left = cam->toWorld({mousePos.first, mousePos.second});
       }
       else if (MouseManager::getJustClicked() == SDL_BUTTON_RIGHT)
       {
           right = cam->toWorld({mousePos.first, mousePos.second});
       }
       if (left != right)
       {

           auto path = mesh->getPath(left,right, (KeyManager::findNumber(SDLK_F4) != -1 )*10*sqrt(2));
            showPath(path);
          // GameWindow::requestNGon(10,right,2,{0,1,0,1},0,true,0,false);
       }
    }
}

void Debug::DebugNavMesh::showPath(Path& path)
{
    if (path.size() >= 1)
    {
        Camera* cam = &(GameWindow::getCamera());

        auto end = path.end();
        glm::vec2* prev = &(*path.begin()), *next;

        GameWindow::requestNGon(10,*prev,2,{1,0,0,1},0,true,0,false);
        if( path.size() > 1)
        {
            for (auto it = path.begin() + 1; it != end; ++it)
           {
                next = &(*it);
               // GameWindow::requestNGon(10,*prev,2,{1,0,0,1},0,true,0,false);
                GameWindow::requestNGon(10,*next,2,{1,0,0,1},0,true,0,false);
                glm::vec2 p1 = cam->toScreen(*prev);
                glm::vec2 p2 = cam->toScreen(*next);
                PolyRender::requestLine({p1.x,p1.y,p2.x,p2.y},{1,1,1,1},GameWindow::interfaceZ);
                prev = next;
                //std::cout << p1.x << " " << p1.y << " " << p2.x << " " << p2.y << std::endl;
           }
           glm::vec2 startP = cam->toScreen(path.front());
           glm::vec2 endP = cam->toScreen(path.back());
            PolyRender::requestLine({startP.x, startP.y, endP.x, endP.y},{.5,0,.5,1},GameWindow::interfaceZ);
        }
    }
}

void Debug::DebugGameWindow::update()
{
    if (addTerrain && MouseManager::getJustReleased() == SDL_BUTTON_LEFT)
    {
        auto rect = GameWindow::getSelection();
        if (GameWindow::getLevel())
        {
            GameWindow::getLevel()->addTerrain(GameWindow::getSelection());
        }
    }
    if (KeyManager::getJustPressed() == SDLK_F5)
    {
        Debug::spawnCreatures = !Debug::spawnCreatures;
        std::string on = "ON";
        if (!Debug::spawnCreatures)
        {
            on = "OFF";
            GameWindow::getLevel()->clearEnemies();
        }

        fastPrint("Spawning creature status turned " + on + "\n");

    }
}

bool Debug::spawnCreatures = true;
bool Debug::renderPath = true;

Debug::Debug()
{

}

void Debug::init()
{
    if (GameWindow::getLevel())
    {
        meshDB.mesh = &(GameWindow::getLevel()->getMesh());
    }
}

void Debug::update()
{
    switch (KeyManager::getJustPressed())
    {
    case SDLK_F1:
        meshDB.renderNodes = !meshDB.renderNodes;
        break;
    case SDLK_F2:
        std::cout << "Rendering Path" << std::endl;
        renderPath = !renderPath;
        break;
    case SDLK_F3:
        std::cout << "Generating Terrain" << "\n";
        gameDB.addTerrain = !gameDB.addTerrain;
        break;
    }
    meshDB.update();
    gameDB.update();
}

bool Debug::getSpawnCreatures()
{
    return spawnCreatures;
}

bool Debug::getRenderPath()
{
    return renderPath;
}
