#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include "render.h"

#include "navigation.h"

struct Debug //class to hold all debug controls
{
    struct DebugNavMesh //for debugging NavMesh
    {
        bool renderNodes = false; //whether or not to render all the NavMesh nodes
        glm::vec2 left = {1,1}, right = {1,1};
        NavMesh* mesh = nullptr;
        void update();
        static void showPath(Path& p);
    };
    struct DebugGameWindow
    {
        bool addTerrain = false; //whether or not selecting a portion of the screen creates terrain
        void update();
    };
    Debug();
    void init();
    void update();
    static bool getSpawnCreatures();
    static bool getRenderPath();

private:
    static bool spawnCreatures;
    static bool renderPath; //whether or not to render left and right and their path
    DebugNavMesh meshDB;
    DebugGameWindow gameDB;

};

#endif // DEBUG_H_INCLUDED
