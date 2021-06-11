#ifndef WORLDRoom_H_INCLUDED
#define WORLDRoom_H_INCLUDED

#include <unordered_set>

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
        LevelButton* prev;
        std::unordered_set<LevelButton*> next;
        WorldMapWindow* window = nullptr;
        Level* level = nullptr;
        int spriteNum= 0; //the planet sprite that this levelButton has
        LevelButton* left = 0, *right =0; //left and right are perhaps misleading names; it's more like highest (left) vs lowest (right) child.
        void setSprite(); //sets the planet's sprite portion so the correct planet sprite is rendered
    public:
        const int prevs = 0; //number of levels precede this one.
        LevelButton(int prevs_, LevelButton* prev_, WorldMapWindow& window, Level& level, const glm::vec4& rect);
        LevelButton* getLeft(); //return read-only
        LevelButton* getRight();
        Level* getLevel() const;
        void addNext(LevelButton* next_);
        void press();
        void update(float x, float y, float z,const glm::vec4& blit);
        const std::unordered_set<LevelButton*>& getNext() const;
    };

    int planetPlanetDistance; //distance between each planet
    std::unordered_map<Level*,std::shared_ptr<Level>> levels;
    std::unordered_map<Level*, LevelButton*> levelButtons;
    std::vector<std::vector<LevelButton*>> levelLayers;
    LevelButton* currentLevel = nullptr; //level button of the level that the player should go to.
    LevelButton* selectedLevel = nullptr; //level button of the level that the player has selected, but won't necessarily go to
    LevelButton* rootButton =nullptr; //pointer to the first level
    void setSelectedLevel(LevelButton& level);
    void setCurrentLevel(LevelButton& level);
    LevelButton* addLevel(Level& level, LevelButton* prev); //generates a levelbutton given a Room and adds it to the panels. Returns the levelButton generated
    void switchToGame(); //called when switching to the gamewindow
    MouseCamera camera;
    bool rerender = false; //location of levelButtons have to be set when the render function is called. Rerender keeps track of if levelButton positions have changed since the last frame.
    void buttonRerender(); //sets button positions to make it more visually appealing.
    void clearLevels(); //sets root to null and empties all the levelButton storages
public:
    WorldMapWindow();
    LevelButton* generate(int count = -1, LevelButton* start = nullptr, LevelButton* end = nullptr); //generates count levelButtons and levels. If count is -1, will generate a random # of levels. The first level generated will have start as its prev and the last level generated will have end as its end. Returns the first levelButton generated
    void update(float x,float y, float z, const glm::vec4& blit);
    const LevelButton* getCurrentLevel();
    const LevelButton* getSelectedLevel();
    int getPlanetPlanetDistance();
    const MouseCamera& getCamera();
    void switchToRoot(); //sets GameWindow currentLevel to root. Called at the beginning of a new run
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
