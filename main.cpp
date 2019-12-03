#include <iostream>
#include <time.h>
#include <typeinfo>
#include <SDL.h>

#include "render.h"
#include "SDLHelper.h"
#include "FreeTypeHelper.h"
#include "glInterface.h"

#include "components.h"
#include "entities.h"
#include "ants.h"
#include "game.h"

struct A
{
    A()
    {
        std::cout << "CReated: " << this << "\n";
    }
    ~A()
    {
        std::cout << "Deleted: " << this << "\n";
    }
};

int main(int args, char* argsc[])
{
    const int screenWidth = 640;
    const int screenHeight = 640;
    srand(time(NULL));
    SDL_Window* window = SDL_CreateWindow("Project",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,screenWidth, screenHeight, SDL_WINDOW_OPENGL);
    SDL_StopTextInput();
    SDL_GL_CreateContext(window);
    RenderProgram::init(screenWidth,screenHeight);
    PolyRender::init(screenWidth,screenHeight);
    Font::init(screenWidth, screenHeight);
    SDL_Event e;
    bool quit = false;
    glClearColor(1,1,1,1);
    glEnable(GL_PRIMITIVE_RESTART);
    bool eventsEmpty = true;
        //std::cout << tree.count() << std::endl;
    Interface interface;
    GameWindow game;
    interface.switchCurrent(&game);


    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            eventsEmpty = false;
            KeyManager::getKeys(e);
            MouseManager::update(e);
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
        }
        if (eventsEmpty)
        {
            KeyManager::getKeys(e);
            MouseManager::update(e);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        interface.update();
       // squares = SDL_GetTicks()/500;
       //PolyRender::requestNGon(4,{320,320},20,{0,0,0,1},90,false,0);
        //PolyRender::requestRect({0,0,64,64},{1,0,0,.5},true,-.1);

        //int move = SDL_GetTicks()/10.0;
      // wrap.request({{0,0,640,640}});

       // sideWall.request({{move,move, 64,64},0,NONE,{1,1,1},&RenderProgram::basicProgram});
       //Font::alef.write(Font::wordProgram,{"peilo",{320,320,64,64},1,{1,1,1}});
       //drawRectangle(RenderProgram::lineProgram,{1,1,1},{0,0,64,64},0);
        SpriteManager::render();
        PolyRender::render();
       // Font::alef.write(Font::wordProgram,"asdf",320,320,0,1,{0,0,0});
        SDL_GL_SwapWindow(window);
        DeltaTime::update();
        eventsEmpty = true;
        //std::cout << DeltaTime::deltaTime << std::endl;
    }
    return 0;
}
