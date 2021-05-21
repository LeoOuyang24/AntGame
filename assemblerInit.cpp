#include "assemblerInit.h"
#include "friendlyAssemblers.h"
#include "enemyAssemblers.h"

UnitAssembler* AssemblerParent::getPtr()
{
    return nullptr;
}

template <typename T>
UnitAssembler* AssemblerWrapper<T>::getPtr()
{
    return static_cast<UnitAssembler*>(ptr.get());
}

template <typename T>
AssemblerWrapper<T>& getAssemblerWrapper()
{
    if (!StaticMount<T>::assembler.ptr.get())
    {
        StaticMount<T>::assembler.ptr.reset(new T());
    }
    return StaticMount<T>::assembler;
}

template <typename T>
AssemblerPtr<T>& getAssemblerPtr()
{

    return getAssemblerWrapper<T>().ptr;
}

template <typename T>
void initAssemblers()
{
    getAssemblerPtr<T>().reset(new T());
}

template <typename T, typename A, typename... Args>
void initAssemblers()
{
    initAssemblers<T>();
    initAssemblers<A,Args...>();
}

void initAssemblers()
{
    initAssemblers<Dinosaur,AttackAnt,TurtFrog>();
}


template <typename T>
void addUnitToBucket(UnitBucket& bucket)
{
    bucket.emplace_back(&getAssemblerWrapper<T>());
}

template <typename T1, typename T2, typename... Args>
void addUnitToBucket(UnitBucket& bucket)
{
    addUnitToBucket<T1>(bucket);
    addUnitToBucket<T2,Args...>(bucket);
}


UnitAssembler* getRandomAssembler(UnitBucket& bucket)
{
    if (bucket.size() ==0)
    {
        throw std::logic_error("getRandomAssembler: bucket size is 0!");
    }
    int it = rand()%(bucket.size());

    return bucket[it]->getPtr();
}

UnitBucket grassEnemies;

void initBuckets()
{
    addUnitToBucket<TurtFrog,Dinosaur,AttackAnt>(grassEnemies);
}
