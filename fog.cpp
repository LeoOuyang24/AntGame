#include "game.h"
#include "fog.h"

const int FogMaker::fogZ = 1;

void FogMaker::renderFog()
{
    int size = fogRequests.size();;
    for (int i = 0; i < size; ++i)
    {
        if (fogRequests[i].isRect == true)
        {
            GameWindow::requestRect(fogRequests[i].rect,{0,0,0,0},true,0,fogZ + 1,false);
        }
        else
        {
            GameWindow::requestNGon(fogRequests[i].sides, fogRequests[i].center,cos(180/fogRequests[i].sides*(fogRequests[i].sides-2))*fogRequests[i].radius,
                                    {0,0,0,0},0,true,fogZ + 1,0);
        }
    }
    PolyRender::render(); //render everything so far. This is basically everything we don't want to be affected by the fog. This includes vision points
    SpriteManager::render();
    fogRequests.clear();
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
    glStencilFunc(GL_ALWAYS,1,0xFF);
    glStencilMask(0xFF);
    GameWindow::requestRect(GameWindow::getCamera().getRect(),{0,0,0,.5},true,0,fogZ,false); //render fog
    PolyRender::renderPolygons();
  //  GameWindow::requestRect(GameWindow::getCamera().getRect(),{0,0,0,0},true,0,fogZ + 1, false);
}

void FogMaker::requestRectFog(const glm::vec4& rect)
{
    FogRequest r;
    r.isRect = true;
    r.rect = rect;
    fogRequests.push_back(r);
}
void FogMaker::requestPolyFog(const glm::vec2& center, double radius, int sides)
{
    FogRequest r;
    r.isRect = false;
    r.center = center;
    r.radius = radius;
    r.sides = sides;
    fogRequests.push_back(r);
}
