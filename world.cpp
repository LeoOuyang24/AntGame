#include "world.h"

Object::Object(ClickableComponent& click, RectComponent& rect, RenderComponent& render) : Entity(), clickable(&click), rect(&rect), render(&render)
{
    addComponent(rect);
    addComponent(click);
    addComponent(render);


}

