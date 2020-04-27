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

template <typename T>
struct B
{
    int y = 0;
    B()
    {
        std::cout << "B CREATED\n";
    }
    ~B()
    {
        std::cout <<"ASDF" << std::endl;
    }
};

struct B1 : public B<B1>
{
    int x = 1;
} ;

struct B2 : public B1, public B<B2>
{
    B2()
    {
        std::cout << B1::y << x << std::endl;
    }
};


struct C : public A, public B<C>
{

};


int main(int args, char* argsc[])
{
    const int screenWidth = 960;
    const int screenHeight = 960;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    srand(time(NULL));
    SDL_Window* window = SDL_CreateWindow("Project",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,screenWidth, screenHeight, SDL_WINDOW_OPENGL);
    SDL_StopTextInput();
    SDL_GL_CreateContext(window);

    RenderProgram::init(screenWidth,screenHeight);
    PolyRender::init(screenWidth,screenHeight);
    Font::init(screenWidth, screenHeight);
    SDL_Event e;
    bool quit = false;

    bool eventsEmpty = true;
        //std::cout << tree.count() << std::endl;
    Interface interface;
    GameWindow game;
    interface.switchCurrent(&game);

   SpriteWrapper spr;
    spr.init("image.png",true);

  //  Font comic = Font("betterComicSans.ttf");
  //  FontManager::addFont(comic);

    SpriteManager::addSprite(spr);
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
            e.type = 0;
            KeyManager::getKeys(e);
            MouseManager::update(e);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        interface.update();
        if (game.quit)
        {
            quit = true;
        }

       //drawRectangle(RenderProgram::lineProgram,{1,1,1},{0,0,64,64},0);
      // comic.requestWrite({"1234567890asdf",{320,320,128,64},0,{0,0,0,1},1});
        SpriteManager::render();
        PolyRender::render();
        FontManager::update();
        SDL_GL_SwapWindow(window);
        DeltaTime::update();
        eventsEmpty = true;
      //  std::cout << DeltaTime::deltaTime << std::endl;
    }
    game.close();
    return 0;
}
