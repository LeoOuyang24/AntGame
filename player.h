#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

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
public:
    static const glm::vec4 selectColor;

    void addResource(int r); //used to increase or decrease resources. Resources can't be negative
    void update();
    const glm::vec4& getSelection();
};

class CreateEnergyComponent : public Component, public ComponentContainer<CreateEnergyComponent>
{
    DeltaTime alarm; //the timer for when to generate energy.
    int waitTime = 0; //the number of frames before an energy is generated
    Player* player;
public:
    CreateEnergyComponent(Player& player_, int frames, Entity& entity);
    void update();
};

class Factory : public Structure
{
public:
    Factory(int x, int y);
};

class FactoryAssembler : public UnitAssembler
{
public:
    FactoryAssembler();
    Entity* assemble();
};

#endif // PLAYER_H_INCLUDED
