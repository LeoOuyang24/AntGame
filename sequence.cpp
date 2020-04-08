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


Label::Label(const glm::vec4& box,std::string message, const glm::vec4& color, const glm::vec2& point, Trigger& begin, Trigger& finish, int numTimes) :
    box(box), message(message), color(color), point(point), start(&begin), end(&finish), repeat(numTimes)
{

}

void Label::update()
{
    if (start.get()->getValue() && !visible)
    {
        visible = true;
    }
    else if (end.get()->getValue() && visible)
    {
        visible = false;
        repeat --;
    }
    if (visible)
    {
        float z = GameWindow::interfaceZ - .1;
        const Camera* camera = &(GameWindow::getCamera());
        GameWindow::requestRect(box,color,true,0,z,false);
        Font::alef.write(Font::wordProgram,{message,camera->toScreen(box),0,{0,0,0},z});
        PolyRender::requestPolygon({glm::vec3(camera->toScreen({box.x + box.z/2 - box.z/2*.1, box.y + box.a}),z),
                                   glm::vec3(camera->toScreen({box.x + 1.1*box.z/2, box.y + box.a}),z),
                                   glm::vec3(camera->toScreen({point.x, point.y}),z)},{1,1,1,1});
    }

    //GameWindow::requestNGon(3,{box.x + box.z/2, box.y + box.a + 10},box.z,{1,1,1,1},M_PI,true,GameWindow::interfaceZ,false);
}

bool Label::isDead()
{
    return repeat == 0;
}
