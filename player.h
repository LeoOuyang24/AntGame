#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include <set>

#include "glInterface.h"

#include "friendlyAssemblers.h"

class Factory;

class Player //tracks player stats (resources, money). Also handles player inputs
{
    int resource;
    int gold = 100;
    Unit* player = nullptr;
public:
    static const glm::vec4 selectColor;
    Player();
    void init(); //initiaites the start state. Sets the starting amount of resources, starting buildings, etc.
    Unit* getPlayer();
    int getResource();
    void addResource(int r); //used to increase or decrease resources. Resources can't be negative
    int getGold();
    void addGold(int g);
};

class PlayerAssembler : public UnitAssembler
{
    class PlayerControls : public MoveComponent
    {
    public:
        PlayerControls(float speed, const glm::vec4& rect, Unit& player);
        void update();
    };
public:
    PlayerAssembler();
    Object* assemble();
};

extern PlayerAssembler playerAssembler;

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
