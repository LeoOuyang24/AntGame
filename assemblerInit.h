#ifndef ASSEMBLERINIT_H_INCLUDED
#define ASSEMBLERINIT_H_INCLUDED

#include <memory>
#include <vector>

//beware reader, the following code will make you want to rip your eyes out.

//This file and the corresponding .cpp file use template black magic to add static UnitAssemblers to UnitBuckets as well as initialize any
//variables these Assemblers may have. All of the code here is effectively "meta-code" that allows us to create a bunch of static UnitAssemblers
//without having explicitly declare them in global scope. Basically, we are using code to automate writing code :japanese_goblin:

template <typename T>
using AssemblerPtr = std::shared_ptr<T>; //a typedef that represents a ptr to a Assembler.

class UnitAssembler;

struct AssemblerParent //dummy parent class so we can have a generic pointer to Assemblers
{
    virtual UnitAssembler* getPtr();
};

template <typename T>
struct AssemblerWrapper : public AssemblerParent //this class exists so AssemblerPtr can have a parent class
{
    AssemblerPtr<T> ptr;
    UnitAssembler* getPtr();
};

template <typename T>
struct StaticMount //This class exists for the sole purpose of mounting a static Assemblers into the global definition for each assembler
{
    static AssemblerWrapper<T> assembler;
    static UnitAssembler* getPtr();
};

typedef std::vector<AssemblerParent*> UnitBucket; //we use a raw pointer because Assemblers are all just wrappers of static variables, which don't go out of scope until the program ends

template <typename T>
AssemblerWrapper<T> StaticMount<T> ::assembler;

template <typename T>
AssemblerWrapper<T>& getAssemblerWrapper();

template <typename T>
AssemblerPtr<T>& getAssemblerPtr();

template <typename T1>
void initAssemblers();

template <typename T1, typename T2, typename... Args>
void initAssemblers();

template <typename T>
void addUnitToBucket(UnitBucket& bucket);

template <typename T1, typename T2, typename... Args>
void addUnitToBucket(UnitBucket& bucket);

UnitAssembler* getRandomAssembler(UnitBucket& bucket);

extern UnitBucket allUnits; //vector of all units and structures
extern UnitBucket allStructures;
extern UnitBucket allShopItems;

extern UnitBucket grassEnemies;

void initBuckets();


#endif // ASSEMBLERINIT_H_INCLUDED
