#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include "glInterface.h"

#include "entities.h"

class Factory;

class Player //tracks player stats (resources, money). Also handles player inputs
{
    int resource = 0;
    enum Mode{
        SELECTING, //default mode, player can select units
        BUILDING //player is building structures
    };
    bool updateSelect(); //updates select rect. Returns true if the mouse is down
    glm::vec4 selection = {0,0,0,0};
    glm::vec2 origin; //last point the mouse was at
    Mode mode;
    std::vector<UnitAssembler*> buildings; //list of structures we can create
    std::vector<UnitAssembler*> units; //list of units we can produce
    Window* buildingWindow = nullptr;
    UnitAssembler* currentBuilding = nullptr;
    class BuildingButton : public Button
    {
        Player* player;
        UnitAssembler* building;
    public:
        BuildingButton(const glm::vec4& rect, Player& player_, UnitAssembler& building_);
        void press();
    };
public:
    static const glm::vec4 selectColor;
    Player();
    void init(); //initiaites the start state. Sets the starting amount of resources, starting buildings, etc.
    int getResource();
    void addResource(int r); //used to increase or decrease resources. Resources can't be negative
    void update();
    void render(const glm::vec4& windowSize);
    const glm::vec4& getSelection();
    void setCurrentBuilding(UnitAssembler* building);
    void addBuilding(UnitAssembler& building);
    void addUnit(UnitAssembler& unit);
};

class InactiveComponent : public Component, public ComponentContainer<InactiveComponent> //represents how long an entity is inactive for. Usually because a building is under construction
{
    DeltaTime timeLeft;
    double waitTime = 0;
public:
    InactiveComponent(double duration, Entity& entity);
    void init(); //starts the timer
    bool done(); //returns true when the object is no longer inactive
    void render(); //what to render while this entity is inactive
};



#endif // PLAYER_H_INCLUDED
