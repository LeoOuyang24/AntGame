#ifndef SEQUENCE_H_INCLUDED
#define SEQUENCE_H_INCLUDED

#include "entities.h"
#include "game.h"
//handles action sequences

class Trigger
{
public:
    Trigger();
    virtual bool getValue();
};

class LambdaTrigger : public Trigger //triggers via lambda
{
    typedef bool (*boolFunc) ();
    boolFunc func = nullptr;
public:
    LambdaTrigger(boolFunc func);
    bool getValue();
};

class ObjectTrigger : public Trigger //triggers when a state change occurs within an entity
{
    typedef bool (*boolFunc) (Object* ptr);
    std::weak_ptr<Object> obj;
    boolFunc func = nullptr;
public:
    ObjectTrigger(boolFunc func, std::shared_ptr<Object> sPtr);
    bool getValue();

};

class Label
{
    bool visible = false;
    glm::vec4 box = {0,0,0,0};
    glm::vec4 color = {0,0,0,0};
    glm::vec2 point = {0,0};
    std::string message = "";
    std::unique_ptr<Trigger> start; //this triggers makes the label visible
    std::unique_ptr<Trigger> end;  //this trigger makes the label invisible
    int repeat = 1; //This label can be turned on, then off, "repeat" times. Negative numbers indicate that the label has an unlimited life span.
public:
    Label(const glm::vec4& box,std::string message, const glm::vec4& color, const glm::vec2& point, Trigger& begin, Trigger& finish, int numTimes = 1);
    void update();
    bool isDead(); //whether this label can be removed. Usually because the label has been shown the right number of times
};

#endif // SEQUENCE_H_INCLUDED
