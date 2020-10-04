#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include "glInterface.h"
#include "SDLhelper.h"

#include "components.h"
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

struct Map
{
    class Chunk;
    Map();
    void init(const glm::vec4& region);
    void nextLevel();

    std::shared_ptr<Object> addUnit(Object& entity,  bool friendly = false);//this method returns the shared_ptr in case any other class wants to share ownership. friendly determines if the unit is an enemy or not
    std::shared_ptr<Object> addUnit(Object& entity,  int x, int y, bool friendly = false);

    std::shared_ptr<Object>& getUnit(Object* unit);
    void addTerrain(const glm::vec4& rect);
    void moveObject(Object& obj, double x, double y); //can move either ants or objects
    void setCurrentChunk(Chunk& chunk);
    ObjectStorage& getEntities(Chunk& chunk);
    NavMesh& getMesh(); //might be null if init hasn't been called
    void remove(Object& unit);
    Chunk& getCurrentChunk();
    void render();
    const glm::vec4& getRect(Chunk& chunk);
    RawQuadTree* getTree();
    void reset();
    void setChangeLevel(bool l);
    bool getChangeLevel();
    void findShard(); //finds a shard (increments foundShards by 1)
    int getFoundShards();
    Anthill* getAnthill();
    ~Map();

    class Gate : public Object
    {
        class NextAreaButton : public Button
        {
        public:
            NextAreaButton();
            void press();
            ~NextAreaButton();
        };
    public:
        Gate(int x, int y);
        ~Gate();
    };
    class Chunk
    {
        //friend class Map;
    public:
        glm::vec4 rect = {0,0,0,0};
        ObjectStorage entities;
        SPStorage<Terrain> terrain;
      //  std::vector<std::shared_ptr<Label>> labels;
        std::shared_ptr<RawQuadTree> tree;
        void update();

        glm::vec4& getRect();
        void remove(Object& unit);
        void clear(); //removes all entities and terrain
        Chunk(const glm::vec4& rect_);
        ~Chunk();

    };
private:

    constexpr static int chunkDimen = 5000;
    constexpr static int maxObjectSize = 50; //the longest any one dimension of an entity can be
    const static glm::vec4 playerArea; //area that walls can't spawn because the player's stuff will be there
    bool changeLevel = false; //whether or not to changeLevel
    std::weak_ptr<Anthill> mainHill;
    int foundShards = 0;

    friend class GameWindow;
  //  glm::vec4 rect = {0,0,0,0};
    std::shared_ptr<NavMesh> mesh;
    SPStorage<Chunk> levels;
    Chunk* currentChunk = nullptr;
    void generateLevel(); //generates terrain and shards. Should only be called when currentChunk and mesh are not null.
};



#endif // WORLD_H_INCLUDED
