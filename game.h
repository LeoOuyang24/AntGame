#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "glInterface.h"
#include "glGame.h"
#include "vanilla.h"

#include "components.h"
//#include "entities.h"
#include "world.h"
#include "ants.h"
#include "antManager.h"
#include "debug.h"
#include "player.h"
#include "fog.h"
#include "worldMap.h"

extern SpriteWrapper frame;



class Ant;



class Manager //handles updating units
{
    constexpr static int maxTasks = 10;
    friend class AntManager;
    friend class GameWindow;
    struct TaskNode
    {
        AntManager task;
        std::shared_ptr<TaskNode> child[AntManager::maxChildren];
        TaskNode(Manager& manager); //for creating an empty AntManager
        TaskNode(AntManager&& t );
        bool hasChildren(); //returns true if any one of the children are not null
        ~TaskNode();
    };
    glm::vec2 target;
    std::vector<std::shared_ptr<TaskNode>> tasks;
  //  std::vector<UnitPtr> selectedUnits;
    ObjPtr selectedUnit;
    DeltaTime spawner; //marks the last time something spawned
    Unit* generateCreature(); //chooses a random creature to spawn
    //void spawnCreatures(); //spawn a creature at a random position
    void spawnCreatures(Anthill& hill, double minR, double maxR); //spawn creatures near an anthill at a certain radius
    std::weak_ptr<TaskNode> currentTask; //the current antmanager
    std::weak_ptr<TaskNode> parentTask; //the current parent task
    int processAntManagers(std::shared_ptr<TaskNode>& node, int index, int y, int x); //updates an AntManager and all of its children. y and x are the amount of displacement to render the task. index is the index of the antmanager, < 0 if its a child antmanager. Returns the y of the next AntManager
    void updateEntities();
    void split();
public:
    Manager();
    const ObjPtr getSelectedUnit() const;
    const AntManager* getCurrentTask() const;
    void init(const glm::vec4& region);
    void update(); //updates and spawn entities
    void updateAntManagers(); //updates antmanagers. Separate from update because we want this to be rendered away from the fog
    void reset();
};

class GameWindow;
class Camera : public RenderCamera, Entity
{
    glm::vec2 baseDimen = {0,0};
    glm::vec4 bounds;
    double zoomAmount = 1; //percentage of the base width and height of the camera
    double zoomGoal = -1;
    double zoomSpeed = .0001;
    double oldZoom;
    static double minZoom, maxZoom;
    MoveComponent* move;
public:
    Camera();
    void init(int w, int h);
    void update();
    void setBounds(const glm::vec4& newBounds);
    void center(const glm::vec2& point); //centers the camera around point
    void zoom(float amount, const glm::vec2& point); //zooms the camera in and out. Multiples both height and width by amount
    void zoom(float amount);
    void setZoomTarget(double goal); //sets a target for the camera to zoom down to
    void setZoomTarget(double goal,double speed); //sets a target for the camera to zoom down to
    void resetZoom();
    bool isZooming(); //if the camera is zooming towards a target
    void close();
    ~Camera();
};

class SequenceUnit;
class GameWindow : public Window //the gamewindow is mostly static because most of its functions/members are used everywhere in the program. These members can't be manipulated without first creating a GameWindow
{


    static float menuHeight;
    static Camera camera;
    static std::weak_ptr<Map> level;
    static Manager manager;
    static Window* gameOver;
    static Debug debug;
    static Player player;
    static FogMaker fogMaker;

    static bool renderAbsolute; //whether or not to renderAbsolute

    ObjPtr anthill; //pointer to the anthill. Keeps track of whether or not the player has lost
    WindowSwitchButton* switchToMap = nullptr; //button that swaps back to the world map
    std::vector<std::shared_ptr<SequenceUnit>> labels;
    struct QuitButton : public Button
    {
        GameWindow* window = nullptr;
        QuitButton(GameWindow& window_);
        void press();
    };
public:
    static float interfaceZ;
    static float fontZ; //z for rendering text on top of the interface
    bool quit = false;
    const static glm::vec4& getSelection();
    static Camera& getCamera();
    static const Manager& getManager();
    static Map* getLevel();
    static void setLevel(std::shared_ptr<Map>& map);
    static Player& getPlayer();
    static FogMaker& getFogMaker();
    void setWorldMap(WindowSwitchButton& butt);
    static void requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z, bool absolute = false); //easier way to render polygons without having to call getCamera();
    static void requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z, bool absolute = false); //if absolute is true, the coordinates are taken as screen coordinates
    GameWindow();
    void onSwitch(Window& from);
    void updateTop(float z);
    void renderTopBar();
    void renderSelectedUnits();
    static float getMenuHeight();
    static void close();
};

#endif // GAME_H_INCLUDED
