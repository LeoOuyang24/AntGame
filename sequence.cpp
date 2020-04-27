#include "sequence.h"

Trigger::Trigger()
{

}

bool Trigger::getValue()
{
    return false;
}

LambdaTrigger::LambdaTrigger(boolFunc func) : func(func)
{

}

bool LambdaTrigger::getValue()
{
    return func();
}

ObjectTrigger::ObjectTrigger(boolFunc func, std::shared_ptr<Object> sPtr) :  obj(sPtr),func(func)
{

}

bool ObjectTrigger::getValue()
{
    return func(obj.lock().get());
}

ChainTrigger::ChainTrigger(std::shared_ptr<SequenceUnit> lPtr, int wait) : other(lPtr), delay(wait)
{

}

bool ChainTrigger::getValue()
{
    SequenceUnit* ptr = other.lock().get();
    bool chain= (!ptr || ptr->isDead()); //whether or not the last trigger has finished
    if (chain && !alarm.isSet())
    {
        alarm.set();
    }
    return chain && alarm.timePassed(delay);
}

void SequenceUnit::running()
{

}

void SequenceUnit::starting()
{

}

void SequenceUnit::ending()
{

}

SequenceUnit::SequenceUnit(Trigger& begin, Trigger& finish, int numTimes) : start(&begin), end(&finish), repeat(numTimes)
{

}

void SequenceUnit::update()
{
    auto startPtr = start.get();
    auto endPtr = end.get();
    if (startPtr && startPtr->getValue() && !isRunning)
    {
        starting();
        isRunning = true;
    }
    else if (endPtr && endPtr->getValue() && isRunning)
    {
        ending();
        repeat --;
        isRunning = false;
    }
    if (isRunning)
    {
        running();
    }
}

bool SequenceUnit::isDead()
{
    return repeat == 0;
}

void EZSequenceUnit::running()
{
    if (runFunc)
    {
        runFunc();
    }
}

void EZSequenceUnit::starting()
{
    if (startFunc)
    {
        startFunc();
    }
}

void EZSequenceUnit::ending()
{
    if (endFunc)
    {
        endFunc();
    }
}

EZSequenceUnit::EZSequenceUnit(voidFunc r, voidFunc s, voidFunc e,Trigger& begin, Trigger& finish, int numTimes) : SequenceUnit(begin, finish, numTimes),
runFunc(r), startFunc(s), endFunc(e)
{

}

void Label::running()
{
        float z = GameWindow::interfaceZ - .1;
        const Camera* camera = &(GameWindow::getCamera());
        auto mousePos = MouseManager::getMousePos();
        auto mousePosMod = camera->toWorld({mousePos.first, mousePos.second});
        float alpha = !pointInVec(box,mousePosMod.x,mousePosMod.y)*(color.a - .1) + .1;
        GameWindow::requestRect(box,glm::vec4(glm::vec3(color),alpha),true,0,z,false);
        Font::alef.requestWrite({message,camera->toScreen(box),0,{0,0,0,alpha},z});
        PolyRender::requestPolygon({glm::vec3(camera->toScreen({box.x + box.z/2 - box.z/2*.1, box.y + box.a}),z),
                                   glm::vec3(camera->toScreen({box.x + 1.1*box.z/2, box.y + box.a}),z),
                                   glm::vec3(camera->toScreen({point.x, point.y}),z)},{1,1,1,alpha});
    //GameWindow::requestNGon(3,{box.x + box.z/2, box.y + box.a + 10},box.z,{1,1,1,1},M_PI,true,GameWindow::interfaceZ,false);
}

Label::Label(const glm::vec4& box,std::string message, const glm::vec4& color, const glm::vec2& point, Trigger& begin, Trigger& finish, int numTimes) :
    SequenceUnit(begin,finish,numTimes), box(box), color(color), point(point), message(message)
{

}




