#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "glInterface.h"
#include "glGame.h"
#include "vanilla.h"

#include "components.h"
#include "entities.h"
#include "world.h"
#include "ants.h"
#include "antManager.h"
#include "debug.h"

extern SpriteWrapper frame;




class Ant;




class Manager
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
    Anthill* signalling = nullptr; //the anthill currently conquering
    Unit* generateCreature(); //chooses a random creature to spawn
    //void spawnCreatures(); //spawn a creature at a random position
    void spawnCreatures(Anthill& hill, double minR, double maxR); //spawn creatures near an anthill at a certain radius
    std::weak_ptr<TaskNode> currentTask; //the current antmanager
    std::weak_ptr<TaskNode> parentTask; //the current parent task
    int processAntManagers(std::shared_ptr<TaskNode>& node, int index, int y, int x); //updates an AntManager and all of its children. y and x are the amount of displacement to render the task. index is the index of the antmanager, < 0 if its a child antmanager. Returns the y of the next AntManager
    void updateAntManagers();
    void updateEntities();
    void split();
public:
    Manager();
    const ObjPtr getSelectedUnit() const;
    const AntManager* getCurrentTask() const;
    void init(const glm::vec4& region);
    void update();
    void setSignalling(Anthill& hill);
};

class GameWindow;
class Camera : public Entity
{
    glm::vec2 baseDimen = {0,0};
    const glm::vec4* bounds = nullptr;
    glm::vec4 rect = {0,0,0,0};
    double zoomAmount = 1; //percentage of the base width and height of the camera
    double zoomGoal = -1;
    double zoomSpeed = .0001;
    static double minZoom, maxZoom;
    MoveComponent* move;
public:
    Camera();
    void init(int w, int h);
    void update();
    const glm::vec4& getRect() const;
    void setBounds(const glm::vec4* newBounds);
    glm::vec4 toScreen(const glm::vec4& rect) const;//converts a rect from the world coordinate to the screen coordinate
    glm::vec2 toScreen(const glm::vec2& point) const;
    glm::vec4 toWorld(const glm::vec4& rect) const;  //converts a rect from the screen coordinate to the world coordinate
    glm::vec2 toWorld(const glm::vec2& point) const;
    glm::vec4 toAbsolute(const glm::vec4& rect) const;
    glm::vec2 toAbsolute(const glm::vec2& point) const; //given a screen coordinate, renders it to that point on the screen regardless of zoom
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
    static glm::vec2 origin; //last point the mouse was at
    static glm::vec4 selection;
    static Camera camera;
    static Map level;
    static Manager manager;
    static Window* gameOver;
    static Debug debug;
    static bool renderAbsolute; //whether or not to renderAbsolute

    ObjPtr anthill; //pointer to the anthill. Keeps track of whether or not the player has lost
    bool updateSelect(); //updates the selection window and returns whether or not the player is selecting
    std::vector<std::shared_ptr<SequenceUnit>> labels;
    struct QuitButton : public Button
    {
        GameWindow* window = nullptr;
        QuitButton(GameWindow& window_);
        void press();
    };
public:
    static float interfaceZ;
    bool quit = false;
    const static glm::vec4 selectColor;
    const static glm::vec4& getSelection();
    static Camera& getCamera();
    static const Manager& getManager();
    static Map& getLevel();
    static void requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z, bool absolute = false); //easier way to render polygons without having to call getCamera();
    static void requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z, bool absolute = false); //if absolute is true, the coordinates are taken as screen coordinates
    GameWindow();
    void update(int x, int y, bool clicked);
    void renderSelectedUnits();
    static float getMenuHeight();
    static void close();
};

#endif // GAME_H_INCLUDED
