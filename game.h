#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "glInterface.h"
#include "glGame.h"
#include "vanilla.h"

#include "components.h"
#include "world.h"
#include "ants.h"
#include "antManager.h"
#include "player.h"
#include "fog.h"

extern SpriteWrapper frame;



class Ant;



class Manager //handles updating units
{
    constexpr static int maxTasks = 10;
    friend class AntManager;
    friend class GameWindow;
    glm::vec2 target;
  //  std::vector<UnitPtr> selectedUnits;
    ObjPtr selectedUnit;
    DeltaTime spawner; //marks the last time something spawned


    void updateEntities();
public:
    Manager();
    const ObjPtr getSelectedUnit() const;
    void init(const glm::vec4& region);
    void update(); //updates and spawn entities
};

class GameWindow;

class Camera : public RenderCamera, Entity
{
    glm::vec4 bounds = glm::vec4(0);
    glm::vec2 baseDimen = {0,0};
    double zoomAmount = 1; //percentage of the base width and height of the camera
    double zoomGoal = -1;
    double zoomSpeed = .0001;
    double oldZoom;
    static double minZoom, maxZoom;
    std::weak_ptr<Unit> player;
    MoveComponent* move;
public:
    Camera();
    void init(int w, int h);
    void setPlayer(const std::shared_ptr<Unit>& player);
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
    static std::weak_ptr<Room> upcomingRoom;
    static std::weak_ptr<Level> level;
    static Manager manager;
    static Window* gameOver;
    static GameWindow* actualWindow; //the singleton window

    static Debug debug;
    static Player player;
    static FogMaker fogMaker;

    static bool renderAbsolute; //whether or not to renderAbsolute

    WindowSwitchButton* switchToMap = nullptr; //button that swaps back to the world Room
    std::vector<std::shared_ptr<SequenceUnit>> labels;

public:
    static float interfaceZ;
    static float fontZ; //z for rendering text on top of the interface
    static Camera& getCamera();
    static const Manager& getManager();
    static Level* getLevel();
    static Room* getRoom(); //throws if either level or room is null. use getLevel()->getRoom(), if you don't want the error.
    static void setLevel(std::shared_ptr<Level>& level_);
    static Player& getPlayer();
    static FogMaker& getFogMaker();
    void setWorldMap(WindowSwitchButton& butt);
    static void setCurrentRoom(const std::shared_ptr<Room>& next); //sets level->currentRoom after next gameplay loop
    static void staticAddPanel(Panel& panel, bool absolute);
    static void requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z, bool absolute = false); //easier way to render polygons without having to call getCamera();
    static void requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z, bool absolute = false); //if absolute is true, the coordinates are taken as screen coordinates
    static void requestLine(const glm::vec4& line, const glm::vec4& color, float z, bool absolute);
    GameWindow();
    void reset();
    void onSwitch(Window& from);
    void updateTop(float z);
    void renderTopBar();
    void renderSelectedUnits();
    static float getMenuHeight();
    static void close();
};

class QuitButton : public Button
{
    GameWindow* window = nullptr;
public:
    QuitButton(const glm::vec4& rect, GameWindow& window_);
    void press();
};

class TitleScreen : public Window
{
public:
    TitleScreen();
    void onSwitch(Window& previous);
};

class WorldMapWindow;
class Game : public Interface //represents the game state
{
    std::unique_ptr<GameWindow> gameWindow;
    std::unique_ptr<WorldMapWindow> worldMap;
    std::unique_ptr<TitleScreen> title;
    bool quit = false;
public:
    static Game game;
    void init(); //one-time use function to set up all static variables
    void reset(); //multiple use function to reset game state
    void setQuit(bool val);
    bool getQuit();
    GameWindow* getGameWindow();
    WorldMapWindow* getWorldMap();
    TitleScreen* getTitleScreen();
    void close();
};

#endif // GAME_H_INCLUDED
