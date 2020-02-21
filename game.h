#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "glInterface.h"
#include "glGame.h"

#include "components.h"
#include "entities.h"
#include "ants.h"


extern SpriteWrapper frame;

typedef  std::map<Unit*, std::shared_ptr<Unit>> unitStorage;
typedef  std::map<Ant*, std::shared_ptr<Ant>> antStorage;

class Map
{
    constexpr static int levels = 11;
    constexpr static int chunkDimen = 2000;
    friend class GameWindow;
  //  glm::vec4 rect = {0,0,0,0};
    struct Chunk
    {
        //friend class Map;
        glm::vec4 rect = {0,0,0,0};
        std::map<Unit*, std::shared_ptr<Unit>> entities;
        std::map<Ant*, std::shared_ptr<Ant>> ants;
        std::shared_ptr<RawQuadTree> tree;
        Chunk(const glm::vec4& rect_);
        void clear()
        {
            entities.clear();
            ants.clear();
        }
    };
    std::shared_ptr<Chunk> chunks[levels][levels];
    Chunk* currentChunk = nullptr;
public:

    Map()
    {

    }
    void init(const glm::vec4& region);
    std::shared_ptr<Unit> addUnit(Unit& entity);//this method returns the shared_ptr in case any other class wants to share ownership.
    std::shared_ptr<Ant> addAnt(Ant& ant);
    std::shared_ptr<Ant>& getAnt(Ant* ant);
    std::shared_ptr<Unit>& getUnit(Unit* unit);
    Chunk& getChunk(int x, int y); //assumes 0,0 = [5][5]
    Chunk& getChunk(Unit& unit);
    void setCurrentChunk(Chunk& chunk);
    std::map<Ant*, std::shared_ptr<Ant>>& getAnts(Chunk& chunk)
    {
        return chunk.ants;
    }
    std::map<Unit*, std::shared_ptr<Unit>>& getEntities(Chunk& chunk)
    {
        return chunk.entities;
    }
    void remove(Unit& unit);
    void render();
   const glm::vec4& getRect(Chunk& chunk) //returns rect of the current Chunk
   {
       return chunk.rect;
   }
    Chunk& getCurrentChunk();
    RawQuadTree* getTree(Chunk& chunk)
    {
        return chunk.tree.get();
    }
    RawQuadTree* getTreeOf(Unit& unit)
    {
        return getChunk(unit).tree.get();
    }

};

class Ant;

typedef std::weak_ptr<Unit> UnitPtr;

class AntManager //handles ant movement and targeting
{
    enum Task
    {
        IDLE, //not doing anything.
        ATTACK, //moving to destroy a unit
        MOVE, //moving to a point on the map rather than a unit
        COLLECT //in the process of collecting, even if all the ants are moving towards the target
    };
    Task currentTask = IDLE;
    const static int spacing; //spacing between ants when they move
    Manager* manager = nullptr;
   // std::vector<std::shared_ptr<Ant>> selected;
    UnitPtr targetUnit; //used to keep track of an ant group's main target
    glm::vec2 clumpDimen = {0,0}; //how many ants horizontally and vertically
    glm::vec2 space = {0,0}; //space between ants horizontally and vertically
    glm::vec2 targetPoint = {0,0}; //point to move ants to
    glm::vec2 antsCenter = {0,0}; //the center of all the ants
    std::vector<std::weak_ptr<Ant>> selected;
    void change(std::shared_ptr<Unit> newUnit, glm::vec2& newPoint); //sets the member variables and notifies the ants

public:
    AntManager(Manager& newManager) : manager(&newManager)
    {

    }
    ~AntManager()
    {
        clear();
    }
    const glm::vec2& getCenter()
    {
        return antsCenter;
    }
    std::vector<std::weak_ptr<Ant>>& getAnts()
    {
        return selected;
    }
    void clear()
    {
        selected.clear();
        targetUnit.reset();
    }
    const Unit* getTargetUnit()
    {
        return targetUnit.lock().get();
    }
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
    UnitPtr selectedUnit;
    void spawnCreatures(); //spawn a creature at a random position
    AntManager* currentTask = nullptr; //the current antmanager
    void updateAntManagers();
    void updateAnts();
    void updateEntities();
public:
    Manager()
    {

    }
    UnitPtr getSelectedUnit()
    {
        return selectedUnit;
    }
    AntManager* getCurrentTask()
    {
        return currentTask;
    }
    void init(const glm::vec4& region);
    void update();
};

class GameWindow;
class Camera
{
    const glm::vec4* bounds = nullptr;
    glm::vec4 rect = {0,0,0,0};
public:
    Camera()
    {

    }
    void init(int w, int h);
    void update();
    const glm::vec4& getRect() const
    {
        return rect;
    }
    void setBounds(const glm::vec4* newBounds)
    {
        bounds = newBounds;
    }
    glm::vec4 toScreen(const glm::vec4& rect) const;//converts a rect from the world coordinate to the screen coordinate
    glm::vec2 toScreen(const glm::vec2& point) const;
    glm::vec4 toWorld(const glm::vec4& rect) const;  //converts a rect from the screen coordinate to the world coordinate
    glm::vec2 toWorld(const glm::vec2& point) const;
    void center(const glm::vec2& point); //centers the camera around point
};

class GameWindow : public Window //the gamewindow is mostly static because most of its functions/members are used everywhere in the program. These members can't be manipulated without first creating a GameWindow
{
    static glm::vec2 origin; //last point the mouse was at
    static glm::vec4 selection;
    static Camera camera;
    static Map level;
    static Manager manager;
    static Window* gameOver;
    UnitPtr anthill; //pointer to the anthill. Keeps track of whether or not the player has lost
    bool updateSelect(); //updates the selection window and returns whether or not the player is selecting
    struct QuitButton : public Button
    {
        GameWindow* window = nullptr;
        QuitButton(GameWindow& window_) : Button({10,50,32,32},nullptr,nullptr, {"Quit"},&Font::alef,{0,1,0,1}), window(&window_)
        {

        }
        void press()
        {
            window->quit = true;
        }
    };
public:
    bool quit = false;
    const static glm::vec4 selectColor;
    const static glm::vec4& getSelection()
    {
        return selection;
    }
    static Camera& getCamera()
    {
        return camera;
    }
    static Map& getLevel()
    {
        return level;
    }
    static void requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z); //easier way to render polygons without having to call getCamera();
    static void requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z);
    GameWindow();
    void update(int x, int y, bool clicked);
    void renderSelectedUnits();
};

#endif // GAME_H_INCLUDED
