#include <iostream>
#include <time.h>
#include <typeinfo>
#include <SDL.h>
#include <SDL_syswm.h>

#include "render.h"
#include "SDLHelper.h"
#include "FreeTypeHelper.h"
#include "glInterface.h"

#include "components.h"
#include "entities.h"
#include "ants.h"
#include "game.h"
#include "worldMap.h"
#include "tiles.h"
#include "animation.h"

struct A
{
    A()
    {
        std::cout << "CReated: " << this << "\n";
    }
    virtual void func()
    {
        std::cout << "I am A\n";
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


struct C : public A
{
    void func()
    {
        std::cout << "I am C\n";
    }
};

int main(int args, char* argsc[])
{
    const int screenWidth = 960;
    const int screenHeight = 960;
    const int windowMode =  SDL_WINDOW_OPENGL ;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }
   // std::cout << lineInVec({82.9492,10},{83,-90},{-10,-33,200,-10},0) << std::endl;

    srand(time(NULL));
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,1);
    SDL_Window* window = SDL_CreateWindow("Project",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,screenWidth, screenHeight,windowMode);
    SDL_StopTextInput();
    SDL_GL_CreateContext(window);

    RenderProgram::init(screenWidth,screenHeight);
    PolyRender::init(screenWidth,screenHeight);
    Font::init(screenWidth, screenHeight);

    initTileSets();
    initSprites();

    SDL_Event e;
    bool quit = false;
    bool eventsEmpty = true;
        //std::cout << tree.count() << std::endl;
    Interface interface;
    GameWindow game;

            std::cout << "Time to setup everything before worldMap: " << SDL_GetTicks() << "\n";

    WorldMapWindow worldMap;
    interface.switchCurrent(&worldMap);
    worldMap.generate();

    ShopWindow shopWindow;

    shopWindow.addPanel(*(new WindowSwitchButton({.05*screenWidth,.9*screenHeight,64,64},nullptr,interface,worldMap,{"Map"},&Font::tnr,{1,1,1,1})));
    shopWindow.addPanel(*(new Message({.8*screenWidth,.1*screenHeight,128,64},nullptr,{"Player Gold: "},&Font::tnr,{1,1,1,0},
                                      [](){return "Player Gold: " + convert(GameWindow::getPlayer().getGold());})));
    worldMap.addPanel(*(new WorldMapWindow::WorldSwitchToGame({.8*screenWidth,.8*screenHeight,.1*screenWidth,.1*screenHeight},
                                                interface,game,worldMap)),true);
    worldMap.addPanel(*(new WindowSwitchButton({100,100,100,100},nullptr,interface,shopWindow,{"Shop"},&Font::tnr,{1,1,1,1})),true);

    game.setWorldMap(*(new WindowSwitchButton({0,0,0,0},nullptr,interface,worldMap,{},nullptr,{0,0,0,0})));
   SpriteWrapper spr;
    spr.init("image.png");
    AnimationWrapper anime;
    anime.init(new BaseAnimation("oldMan.png",-1, 6,1));
  //  glClearColor(0,0,1,1);
  //  Font comic = Font("betterComicSans.ttf");
  //  FontManager::addFont(comic);
   // glDepthMask(false);
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        interface.update();
        if (game.quit)
        {
            quit = true;
        }

       //drawRectangle(RenderProgram::lineProgram,{1,1,1},{0,0,  64,64},0);
       //glm::vec4 rect = {320,320,128,64};
      // spr.request({rect,0,NONE,{0,1,0,1}});
      //SpriteWrapper* ptr = &anime;
      //turretSprite.request({{0,0,64,64}});
       //PolyRender::requestRect(rect,{0,0,0,1},true,0,.1);
     //  Font::tnr.requestWrite({"hella",GameWindow::getCamera().toWorld(rect),0,{1,0,0,1},1});
   //  PolyRender::requestNGon(10,GameWindow::getCamera().toAbsolute(pairtoVec(MouseManager::getMousePos())),10,{1,0,0,1},0,true,3);
        PolyRender::render();
        SpriteManager::render();
        FontManager::update();
        SDL_GL_SwapWindow(window);
        DeltaTime::update();
        eventsEmpty = true;
        fastPrint("Ticks: " + convert(DeltaTime::deltaTime) + "\n");
        //SDL_Delay(10);
      //  std::cout << DeltaTime::deltaTime << std::endl;
    }
    game.close();
    return 0;
}
