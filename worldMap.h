#ifndef WORLDRoom_H_INCLUDED
#define WORLDRoom_H_INCLUDED

#include "glInterface.h"

#include "world.h"
#include "player.h"
#include "game.h"

class ShopButton : public Button
{
    UnitAssembler* assembler = nullptr;
    Player* player = nullptr;
    bool soldOut = false; //whether the player has bought this or not
public:
    ShopButton(Player& player, UnitAssembler*  obj, const glm::vec4& rect);
    void press();
    void update(float x, float y, float z, const glm::vec4& scale);
    void changeAssembler(UnitAssembler* assembler); //allows us to update the shopbuttons in a shop instead of adding new ones
};

class ShopWindow : public Window
{
    static constexpr int shopItems = 16;
    ShopButton* buttons[shopItems];
public:
    ShopWindow();
    void onSwitch(Window& previous);
    //don't need a destructor to clear buttons because we add the buttons as panels
};

class MouseCamera : public RenderCamera//camera that moves with mouse
{
    glm::vec4 bounds;
public:
    void init(int w, int h, const glm::vec4& bounds_);
    void update();
};

class WorldMapWindow : public Window
{
    class LevelButton : public Button
    {
        LevelButton* prev = nullptr, *next = nullptr;
        WorldMapWindow* window = nullptr;
        Level* level = nullptr;
    public:
        LevelButton(LevelButton* prev_, LevelButton* next_,WorldMapWindow& window, Level& level, const glm::vec4& rect);
        void setNext(LevelButton* next_);
        void press();
        void update(float x, float y, float z,const glm::vec4& blit);
    };

    int planetPlanetDistance; //distance between each planet
    std::unordered_map<Level*,std::shared_ptr<Level>> levels;
    Level* currentLevel = nullptr;
    void setCurrentLevel(Level& level);
    LevelButton* addLevel(Level& level, LevelButton* prev, LevelButton* next); //generates a levelbutton given a Room and adds it to the panels. Returns the levelButton generated
    void switchToGame(); //called when switching to the gamewindow
    MouseCamera camera;
public:
    WorldMapWindow();
    LevelButton* generate(int count = -1, LevelButton* start = nullptr, LevelButton* end = nullptr); //generates count levelButtons and levels. If count is -1, will generate a random # of levels. The first level generated will have start as its prev and the last level generated will have end as its end. Returns the first levelButton generated
    void update(float x,float y, float z, const glm::vec4& blit);
    Level* getCurrentLevel();
    int getPlanetPlanetDistance();
    const MouseCamera& getCamera();
    class WorldSwitchToGame : public CondSwitchButton
    {
        WorldMapWindow* worldRoom =nullptr;
    public:
        WorldSwitchToGame(const glm::vec4& box, Interface& interface, Window& to, WorldMapWindow& worldRoom);
        bool doSwitch();
        void press();
    };
    friend LevelButton;
    friend WorldSwitchToGame;
};

#endif // WORLDRoom_H_INCLUDED
