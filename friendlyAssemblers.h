#ifndef FRIENDLYASSEMBLERS_H_INCLUDED
#define FRIENDLYASSEMBLERS_H_INCLUDED

#include "entities.h"

void initAssemblers();

class AntAssembler : public UnitAssembler
{
public:
    AntAssembler();
    Object* assemble();
};
extern AntAssembler antAssembler;

#endif // FRIENDLYASSEMBLERS_H_INCLUDED
