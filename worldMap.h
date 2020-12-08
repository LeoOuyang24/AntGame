#ifndef WORLDMAP_H_INCLUDED
#define WORLDMAP_H_INCLUDED

#include "glInterface.h"

#include "world.h"
#include "player.h"

class ShopButton : public Button
{
    UnitAssembler* assembler = nullptr;
    Player* player = nullptr;
    bool soldOut = false; //whether the player has bought this or not
public:
    ShopButton(Player& player, UnitAssembler& obj, const glm::vec4& rect);
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

class WorldMapWindow : public Window
{
    class LevelButton : public Button
    {
        WorldMapWindow* window = nullptr;
        Map* level = nullptr;
    public:
        LevelButton(WorldMapWindow& window, Map& level, const glm::vec4& rect);
        void press();
        void update(float x, float y, float z,const glm::vec4& blit);
    };


    std::unordered_map<Map*,std::shared_ptr<Map>> levels;
    Map* currentLevel = nullptr;
    void setCurrentLevel(Map& level);
    void addLevel(Map& level);
    void switchToGame(); //called when switching to the gamewindow
public:
    WorldMapWindow();
    void generate();
    Map* getCurrentLevel();
    class WorldSwitchToGame : public CondSwitchButton
    {
        WorldMapWindow* worldMap =nullptr;
    public:
        WorldSwitchToGame(const glm::vec4& box, Interface& interface, Window& to, WorldMapWindow& worldMap);
        bool doSwitch();
        void press();
    };
    friend LevelButton;
    friend WorldSwitchToGame;
};

#endif // WORLDMAP_H_INCLUDED
