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
    if (renderPath)
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

           auto path = mesh->getPath(left,right);
           auto end = path.end();
           glm::vec2* prev = &left, *next;
           for (auto it = path.begin(); it != end; ++it)
           {
                next = &(*it);
                GameWindow::requestNGon(10,*prev,2,{1,0,0,1},0,true,0,false);
                GameWindow::requestNGon(10,*next,2,{1,0,0,1},0,true,0,false);
                glm::vec2 p1 = cam->toScreen(*prev);
                glm::vec2 p2 = cam->toScreen(*next);
                PolyRender::requestLine({p1.x,p1.y,p2.x,p2.y},{1,1,1,1},1);
                prev = next;
               // std::cout << p1.x << " " << p1.y << " " << p2.x << " " << p2.y << std::endl;
           }
          // GameWindow::requestNGon(10,right,2,{0,1,0,1},0,true,0,false);
       }
    }
}

void Debug::DebugGameWindow::update()
{
    if (addTerrain && MouseManager::getJustReleased() == SDL_BUTTON_LEFT)
    {
        auto rect = GameWindow::getSelection();
        GameWindow::getLevel().addUnit(*(new Terrain(rect.x,rect.y,rect.z,rect.a)));
    }
}

Debug::Debug()
{

}

void Debug::init()
{
    meshDB.mesh = &(GameWindow::getLevel().getMesh());
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
        meshDB.renderPath = !meshDB.renderPath;
        break;
    case SDLK_F3:
        gameDB.addTerrain = !gameDB.addTerrain;
        break;
    }
    meshDB.update();
    gameDB.update();
}
