#ifndef ENEMYASSEMBLERS_H_INCLUDED
#define ENEMYASSEMBLERS_H_INCLUDED

#include "entities.h"

class EvilMoonAssembler : public UnitAssembler
{
public:
    EvilMoonAssembler();
    Object* assemble();
};

extern EvilMoonAssembler evilMoonAssembler;

#endif // ENEMYASSEMBLERS_H_INCLUDED
