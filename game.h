#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "glInterface.h"
#include "glGame.h"

#include "components.h"
#include "entities.h"
#include "world.h"
#include "ants.h"


extern SpriteWrapper frame;




class Ant;

typedef std::weak_ptr<Object> ObjPtr;

class AntManager //handles ant movement and targeting
{
    enum Task
    {
        IDLE, //not doing anything.
        ATTACK, //moving to destroy a unit
        MOVE, //moving to a point on the map rather than a unit
        COLLECT, //in the process of collecting, even if all the ants are moving towards the target
        INTERACT
    };
    Task currentTask = IDLE;
    const static int spacing; //spacing between ants when they move
    Manager* manager = nullptr;
   // std::vector<std::shared_ptr<Ant>> selected;
    ObjPtr targetUnit; //used to keep track of an ant group's main target
    glm::vec2 clumpDimen = {0,0}; //how many ants horizontally and vertically
    glm::vec2 space = {0,0}; //space between ants horizontally and vertically
    glm::vec2 targetPoint = {0,0}; //point to move ants to
    glm::vec2 antsCenter = {0,0}; //the center of all the ants
    std::vector<std::weak_ptr<Ant>> selected;
    void change(std::shared_ptr<Unit> newUnit, glm::vec2& newPoint); //sets the member variables and notifies the ants

public:
    AntManager(Manager& newManager);
    ~AntManager();
    const glm::vec2& getCenter();
    std::vector<std::weak_ptr<Ant>>& getAnts();
    void clear();
    const Object* getTargetUnit();
    void getInput(); //handles input, such as clicks
    void updateAnts(); //updates ants. The key distinction between this and getInput is that this runs regardless of whether this is the current AntManager
    void remove(Unit& unit);
    void addAnt(std::shared_ptr<Ant>& ant);
    void render(const glm::vec4& rect, int i); //renders the AntManager on the left side of the screen. i is the index of the antManager in Manager
};

class Manager
{
    constexpr static int maxTasks = 10;
    friend class AntManager;
    friend class GameWindow;

    glm::vec2 target;
    std::vector<std::shared_ptr<AntManager>> tasks;
  //  std::vector<UnitPtr> selectedUnits;
    ObjPtr selectedUnit;
    void spawnCreatures(); //spawn a creature at a random position
    AntManager* currentTask = nullptr; //the current antmanager
    void updateAntManagers();
    void updateAnts();
    void updateEntities();
public:
    Manager();
    ObjPtr getSelectedUnit();
    AntManager* getCurrentTask();
    void init(const glm::vec4& region);
    void update();
};

class GameWindow;
class Camera
{
    glm::vec2 baseDimen = {0,0};
    const glm::vec4* bounds = nullptr;
    glm::vec4 rect = {0,0,0,0};
    float zoomAmount = 1; //percentage of the base width and height of the camera
    static float minZoom, maxZoom;
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
    void resetZoom();
};


class GameWindow : public Window //the gamewindow is mostly static because most of its functions/members are used everywhere in the program. These members can't be manipulated without first creating a GameWindow
{
    static float menuHeight;
    static glm::vec2 origin; //last point the mouse was at
    static glm::vec4 selection;
    static Camera camera;
    static Map level;
    static Manager manager;
    static Window* gameOver;
    ObjPtr anthill; //pointer to the anthill. Keeps track of whether or not the player has lost
    bool updateSelect(); //updates the selection window and returns whether or not the player is selecting
    struct QuitButton : public Button
    {
        GameWindow* window = nullptr;
        QuitButton(GameWindow& window_);
        void press();
    };
public:
    bool quit = false;
    const static glm::vec4 selectColor;
    const static glm::vec4& getSelection();
    static Camera& getCamera();
    static Map& getLevel();
    static void requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z, bool absolute = false); //easier way to render polygons without having to call getCamera();
    static void requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z, bool absolute = false); //if absolute is true, the coordinates are taken as screen coordinates
    GameWindow();
    void update(int x, int y, bool clicked);
    void renderSelectedUnits();
    static float getMenuHeight();
};

#endif // GAME_H_INCLUDED
