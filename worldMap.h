#ifndef WORLDMAP_H_INCLUDED
#define WORLDMAP_H_INCLUDED

#include "glInterface.h"

#include "world.h"
#include "player.h"

class ShopButton : public Button
{
    UnitAssembler* assembler = nullptr;
    Player* player = nullptr;
    bool isStructure = false; //whether or not this button will buy a structure
    bool soldOut = false; //whether the player has bought this or not
public:
    ShopButton(bool isStructure, Player& player, UnitAssembler& obj, const glm::vec4& rect);
    void press();
    void update(float x, float y, float z, const glm::vec4& scale);
};

class WorldMapWindow : public Window
{
    class LevelButton : public Button
    {
        WorldMapWindow* window = nullptr;
        Map* level = nullptr;
    public:
        LevelButton(WorldMapWindow& window, Map& level, const glm::vec4& rect);
        void press();
        void render(bool hover, float x, float y, float z, float xScale, float yScale);
    };

    friend LevelButton;
    std::unordered_map<Map*,std::shared_ptr<Map>> levels;
    Map* currentLevel = nullptr;
    Window* shopWindow;
    void setCurrentLevel(Map& level);
    void addLevel(Map& level);
public:
    WorldMapWindow();
    void generate();
    Map* getCurrentLevel();
    void switchTo(Window& swapTo);
    class WorldSwitchToGame : public CondSwitchButton
    {
        WorldMapWindow* worldMap =nullptr;
    public:
        WorldSwitchToGame(const glm::vec4& box, Interface& interface, Window& to, WorldMapWindow& worldMap);
        bool doSwitch();
    };
};

#endif // WORLDMAP_H_INCLUDED
