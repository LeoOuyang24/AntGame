#ifndef CREATURE_H_INCLUDED
#define CREATURE_H_INCLUDED

class Predator : public Unit
{
    class PredatorMove : public MoveComponent, ComponentContainer<MoveComponent>
    {

    };
public:
    Predator(int x, int y);
};

#endif // CREATURE_H_INCLUDED
