#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "glInterface.h"
#include "glGame.h"

#include "components.h"
#include "entities.h"


extern SpriteWrapper frame;

class Map
{
    friend class GameWindow;
    glm::vec4 rect = {0,0,0,0};
    std::vector<glm::vec4> chunks;
public:
    Map()
    {

    }
    void init(const glm::vec4& region);
    void render();
    const glm::vec4& getRect() const
    {
        return rect;
    }

};

class Ant;

class AntManager //handles ant movement and targeting
{
    const static int spacing; //spacing between ants when they move
    Manager* manager = nullptr;
    std::vector<std::shared_ptr<Ant>> selected;
    std::weak_ptr<Unit> targetUnit; //used to keep track of an ant group's main target
    void change(std::shared_ptr<Unit> newUnit, glm::vec2& newPoint); //sets the member variables and notifies the ants
public:
    AntManager(Manager& newManager) : manager(&newManager)
    {

    }
    ~AntManager()
    {
        clear();
    }
    void addAnt(std::shared_ptr<Ant>& ant)
    {
        selected.push_back(ant);
    }
    void update();
    void remove(Unit& unit);
    void clear()
    {
        selected.clear();
    }
    const Unit* getTargetUnit()
    {
        return targetUnit.lock().get();
    }
};

class Manager
{
    friend class AntManager;
    glm::vec2 target;
    std::map<Unit*, std::shared_ptr<Unit>> entities;
    std::vector<std::shared_ptr<Ant>> ants;
    std::unique_ptr<RawQuadTree> tree;
    void spawnCreatures(); //spawn a creature at a random position
    AntManager antManager;
public:
    Manager() : antManager(*this)
    {

    }
    void init(const glm::vec4& region);
    void update();
    void addEntity(Unit& entity);
    void addAnt(Ant& ant);
    void remove(Unit& unit);
};

class GameWindow;
class Camera
{
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
    glm::vec4 toScreen(const glm::vec4& rect) const;//converts a rect from the world coordinate to the screen coordinate
    glm::vec2 toScreen(const glm::vec2& point) const;
    glm::vec4 toWorld(const glm::vec4& rect) const;  //converts a rect from the screen coordinate to the world coordinate
    glm::vec2 toWorld(const glm::vec2& point) const;
};

class GameWindow : public Window //the gamewindow is mostly static because most of its functions/members are used everywhere in the program. These members can't be manipulated without first creating a GameWindow
{
    static glm::vec2 origin; //last point the mouse was at
    static glm::vec4 selection;
    static Camera camera;
    static Map level;
    static Manager manager;
    bool updateSelect(); //updates the selection window and returns whether or not the player is selecting
public:
    const static glm::vec4 selectColor;
    const static glm::vec4& getSelection()
    {
        return selection;
    }
    static const Camera& getCamera()
    {
        return camera;
    }
    static const Map& getLevel()
    {
        return level;
    }
    static void requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z); //easier way to render polygons without having to call getCamera();
    static void requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z);
    GameWindow();
    void update(int x, int y, bool clicked);
};

#endif // GAME_H_INCLUDED
