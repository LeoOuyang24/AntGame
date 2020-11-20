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
public:
    ShopButton(bool isStructure, Player& player, UnitAssembler& obj, const glm::vec4& rect);
    void press();
    void render(bool hover, float x, float y, float z, float xScale, float yScale);
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
};

#endif // WORLDMAP_H_INCLUDED