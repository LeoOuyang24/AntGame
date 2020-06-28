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
                PolyRender::requestLine({p1.x,p1.y,p2.x,p2.y},{1,1,1,1},1);
                prev = next;
                //std::cout << p1.x << " " << p1.y << " " << p2.x << " " << p2.y << std::endl;
           }
           glm::vec2 startP = cam->toScreen(path.front());
           glm::vec2 endP = cam->toScreen(path.back());
            PolyRender::requestLine({startP.x, startP.y, endP.x, endP.y},{.5,0,.5,1},1);
        }
    }
}

void Debug::DebugGameWindow::update()
{
    if (addTerrain && MouseManager::getJustReleased() == SDL_BUTTON_LEFT)
    {
        auto rect = GameWindow::getSelection();
        GameWindow::getLevel().addTerrain(GameWindow::getSelection());
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
        std::cout << "Generating Terrain" << "\n";
        gameDB.addTerrain = !gameDB.addTerrain;
        break;
    }
    meshDB.update();
    gameDB.update();
}
