#ifndef ANTMANAGER_H_INCLUDED
#define ANTMANAGER_H_INCLUDED

#include <memory>
#include "entities.h"

typedef std::weak_ptr<Object> ObjPtr;

class Manager;
struct AntManager //handles ant movement and targeting
{
    enum Task
    {
        IDLE, //not doing anything.
        ATTACK, //moving to destroy a unit
        MOVE, //moving to a point on the map rather than a unit
        COLLECT, //in the process of collecting, even if all the ants are moving towards the target
        INTERACT
    };
    static constexpr int maxChildren = 4;
    AntManager(Manager& newManager, const glm::vec4& selectColor);
    ~AntManager();
    const glm::vec2& getCenter();
    Task getCurrentTask();
    const std::vector<std::weak_ptr<Entity>>& getAnts() const;
    void clear();
    const Object* getTargetUnit() const;
    glm::vec2 getTargetPoint() const;
    void setShortTarget(std::shared_ptr<Object>& obj); //sets the shortTarget of all ants to obj
    void getInput(); //handles input, such as clicks. Sets targetPoint and targetUnit
    void updateAnts(); //updates ants. The key distinction between this and getInput is that this runs regardless of whether this is the current AntManager
    void remove(Unit& unit);
    void addAnt(const std::shared_ptr<Entity>& ant);
    void render(const glm::vec4& rect, std::string c); //renders the AntManager on the left side of the screen. i is the index of the antManager in Manager
    std::vector<AntManager> split(int index);
    void setTask(Task t);
    double getMaxDistance();
private:
    double maxDistance = 0; //maxdistance a unit is from the center that has made it the center. Used to calculate how big the
    constexpr static float targetDist = 1; //maximum distance a unit can be away from its targetPoint and still be at its target
    Task currentTask = IDLE;
    const static int spacing; //spacing between ants when they move
    Manager* manager = nullptr; //we use raw pointers for this because the manager will always be deleted first
    DeltaTime clickTimer; //a timer to see when was the last time the player clicked. Controls the animation that plays when the player clicks somewhere
    bool attackMove = false;
   // std::vector<std::shared_ptr<Ant>> selected;
    ObjPtr targetUnit; //used to keep track of an ant group's main target
    glm::vec2 targetPoint = {0,0}; //point to move ants to
    glm::vec2 antsCenter = {0,0}; //the center of all the ants
    glm::vec4 selectColor = {0,0,0,0};
    std::vector<std::weak_ptr<Entity>> selected;
    void change(std::shared_ptr<Unit> newUnit, glm::vec2& newPoint); //sets the member variables and notifies the ants
    void addChildAnt(const std::shared_ptr<Entity>& ant); //adds an ant to a child so it doesn't remove from parent
    glm::vec4 getChildColor(int index); //given the index-th child, gets the selectColor

};

#endif // ANTMANAGER_H_INCLUDED
