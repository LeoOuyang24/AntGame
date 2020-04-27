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

class SequenceUnit;
class ChainTrigger : public Trigger //triggers when another label is dead. Good for chaining together events
{
    std::weak_ptr<SequenceUnit> other;
    int delay; //number of frames to wait after the last trigger
    DeltaTime alarm;
public:
    ChainTrigger(std::shared_ptr<SequenceUnit> lPtr, int wait = 0);
    bool getValue();

};

class SequenceUnit //a part of a sequence. Consists of a start and an end. Does something when this sequence starts and when it ends.
{
    bool isRunning = false; //whether or not the sequence is currently running
    std::unique_ptr<Trigger> start; //triggers sequence
    std::unique_ptr<Trigger> end;  //finishes sequence;
    int repeat = 1; //This sequence can be turned on, then off, "repeat" times. Negative numbers indicate that the sequence has an unlimited life span.
protected:
    virtual void running(); //what to do while the sequence has started and hasn't ended
    virtual void starting(); //run when the sequenceUnit is first started
    virtual void ending(); //run when this unit ends
public:
    SequenceUnit(Trigger& begin, Trigger& finish, int numTimes = 1);
    void update();
    bool isDead(); //whether this label can be removed. Usually because the label has been shown the right number of times
};

class EZSequenceUnit : public SequenceUnit //good to use when the running, starting, and ending functions are easy enough to write a lambda for
{
    typedef void (*voidFunc) ();

    voidFunc runFunc;
    voidFunc startFunc;
    voidFunc endFunc;
protected:
    void running();
    void starting();
    void ending();
public:
    EZSequenceUnit(voidFunc r, voidFunc s, voidFunc e,Trigger& begin, Trigger& finish, int numTimes = 1);
};

class Label : public SequenceUnit
{
    bool visible = false;
    glm::vec4 box = {0,0,0,0};
    glm::vec4 color = {0,0,0,0};
    glm::vec2 point = {0,0};
    std::string message = "";
protected:
    void running();
public:
    Label(const glm::vec4& box,std::string message, const glm::vec4& color, const glm::vec2& point, Trigger& begin, Trigger& finish, int numTimes = 1);
};

#endif // SEQUENCE_H_INCLUDED
