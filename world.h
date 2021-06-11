#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include "glInterface.h"
#include "SDLhelper.h"

#include "components.h"
#include "tiles.h"
#include "friendlyAssemblers.h"
#include "assemblerInit.h"
#include "entities.h"

class NavMesh;

class Object;
class Ant;


typedef  std::map<Object*, std::shared_ptr<Object>> ObjectStorage;
typedef  std::map<Ant*, std::shared_ptr<Ant>> AntStorage;

template <typename N>
using SPStorage = std::vector<std::shared_ptr<N>>; //Smart Point Storage

class Terrain : public Object
{
    class TerrainRender : public RectRenderComponent, public ComponentContainer<TerrainRender>
    {
    public:
        TerrainRender(const glm::vec4& color, Entity& ent);
        void update();
    };
public:
    Terrain(int x, int y, int w, int h);
};

typedef void (*CollideFunction) (Entity&); //called when colliding when an entity
class ItemComponent : public Component, public ComponentContainer<ItemComponent> //components that belong to objects that exist to be collided with once
{
protected:
    virtual void onUse(Entity& other); //the item's use effect
public:
    ItemComponent(Entity& owner);
    void collide(Entity& other);
};

class Shard : public ObjectAssembler
{
    class ShardComponent : public ItemComponent, public ComponentContainer<ShardComponent> //when touched, set this object to dead and increments the amount of shards found for the current chunk
    {
    public:
        ShardComponent(Entity& owner);
        void onUse(Entity& entity);
    };
public:
    Shard();
    Object* assemble();
};

class PickUpResource : public ObjectAssembler
{
    class PickUpResourceComponent : public ItemComponent, public ComponentContainer<PickUpResourceComponent>
    {
        int amount = 0;
    public:
        PickUpResourceComponent(int amount_, Entity& owner);
        void onUse(Entity& entity);
    };
public:
    PickUpResource();
    Object* assemble();
};

struct Room
{
    constexpr static int chunkDimen = 1500;
    constexpr static int maxObjectSize = 100; //the longest any one dimension of an entity can be
    const int tileDimen = 125; //size of the tiles for rendering the background
    UnitBucket* bucket =nullptr;
    Room(const glm::vec4& rect);
    void init(const glm::vec4& region);
    void nextLevel();

    std::shared_ptr<Object> addUnit(Object& entity,  bool friendly = false);//this method returns the shared_ptr in case any other class wants to share ownership. friendly determines if the unit is an enemy or not
    std::shared_ptr<Object> addUnit(Object& entity,  int x, int y, bool friendly = false); //x and y are the center of the entity
    void addUnit(const std::shared_ptr<Object>& entity, int x, int y, bool friendly = false); //add an entity that already exists to the level (usually the player)
    std::shared_ptr<Object>& getUnit(Object* unit);
    void addTerrain(const glm::vec4& rect);
    void moveObject(Object& obj, double x, double y); //Sets their center to x and y
    ObjectStorage& getEntities();
    NavMesh& getMesh(); //might be null if init hasn't been called
    UnitAssembler& generateCreature();
    void spawnCreature(); //spawn creatures at a random spot
    void remove(Object& unit);
    void clearEnemies();
    void render();
    void update();
    const glm::vec4& getRect();
    RawQuadTree* getTree();
    void reset();
    bool finishedLevel(); //returns whether or not the portal should spawn and end the level
    static Room* generateLevel(const glm::vec4& rect = {0,0,chunkDimen,chunkDimen}); //generates terrain and shards
    ~Room();


private:


    const static glm::vec4 playerArea; //area that walls can't spawn because the player's stuff will be there
    ObjectStorage entities;
    glm::vec4 rect;
    SPStorage<Terrain> terrain;
    std::shared_ptr<RawQuadTree> tree;
    std::list<std::pair<glm::vec2,SpriteWrapper*>> tiles;
    int foundShards = 0;
    friend class GameWindow;
  //  glm::vec4 rect = {0,0,0,0};
    std::shared_ptr<NavMesh> mesh;
};


typedef std::vector<std::shared_ptr<Room>> RoomStorage;
class Level
{
    RoomStorage rooms;
    Room* currentRoom;
    bool completed = false;
public:
    Level(int roomNum);
    Level(); //generate a random number of rooms
    Room* getCurrentRoom();
    void setCurrentRoom(Room* room);
    bool getCompleted();
    void setCompleted(bool comp);
    bool getAllCompleted(); //returns true if all rooms are completed
    ~Level();
};


class Gate : public Object
{
    class NextAreaComponent : public Component, public ComponentContainer<NextAreaComponent>
    {
        std::weak_ptr<Room> room; //current room
        std::weak_ptr<Room> next; //next room to go to
        Level* level = nullptr; //raw pointer because levels should always outlive any gates in the level
    public:
        NextAreaComponent(std::shared_ptr<Room>& room_, std::shared_ptr<Room>& next, Entity& entity);
        NextAreaComponent(std::shared_ptr<Room>& room_, Level& level_, Entity& entity);
        Room* getRoom();
        Level* getLevel();
        void collide(Entity& entity);
    };
    class GateRender : public AnimationComponent
    {
    public:
        GateRender(Entity& entity);
        void update();
    };
public:
    Gate(std::shared_ptr<Room>& room_, std::shared_ptr<Room>& next, int x, int y);
    Gate(std::shared_ptr<Room>& room_,Level& level_, int x, int y);
    ~Gate();
};
#endif // WORLD_H_INCLUDED
