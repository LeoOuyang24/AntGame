#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include "glInterface.h"
#include "SDLhelper.h"

#include "components.h"
#include "entities.h"

class NavMesh;

class Object;
class Ant;


typedef  std::map<Object*, std::shared_ptr<Entity>> ObjectStorage;
typedef  std::map<Ant*, std::shared_ptr<Ant>> AntStorage;

class Terrain : public Object
{
public:
    Terrain(int x, int y, int w, int h);
};

class Map
{

public:
    class Chunk;
    Map();
    void init(const glm::vec4& region);
    std::shared_ptr<Object> addUnit(Object& entity, bool friendly = false);//this method returns the shared_ptr in case any other class wants to share ownership. friendly determines if the unit is an enemy or not
    std::shared_ptr<Object>& getUnit(Object* unit);
    void addTerrain(const glm::vec4& rect);
    void moveObject(Object& obj, double x, double y); //can move either ants or objects
    Chunk& getChunk(int x, int y); //assumes 0,0 = [5][5]
    Chunk& getChunk(Object& unit);
    void setCurrentChunk(Chunk& chunk);
    ObjectStorage& getEntities(Chunk& chunk);
    NavMesh& getMesh(); //might be null if init hasn't been called
    void remove(Object& unit);
    Chunk& getCurrentChunk();
    void render();
    const glm::vec4& getRect(Chunk& chunk);
    RawQuadTree* getTreeOf(Object& unit);
    RawQuadTree* getTree(Chunk&chunk);
    void reset();
    ~Map();

    class Gate : public Object
    {
        class NextAreaComponent : public InteractionComponent, public ComponentContainer<NextAreaComponent>
        {
            std::weak_ptr<Gate> dest;
        public:
            NextAreaComponent(Object& unit);
            void setDest(const std::shared_ptr<Gate>& other);
            void interact(Object& actor);
            Gate* getDest();
            ~NextAreaComponent();
        };
        class NextAreaButton : public Button
        {
            Gate* gate = nullptr;
        public:
            NextAreaButton(Gate& unit);
            void press();
            ~NextAreaButton();
        };
    NextAreaComponent* nextArea = nullptr;
    public:
        Gate(int x, int y);
        void setDest(const std::shared_ptr<Gate>& other);
        ~Gate();
    };
    struct Chunk
    {
        //friend class Map;
        glm::vec4 rect = {0,0,0,0};
        ObjectStorage entities;
        std::vector<std::shared_ptr<Terrain>> terrain;
      //  std::vector<std::shared_ptr<Label>> labels;
        std::shared_ptr<RawQuadTree> tree;
        void clear();
        void update();
        Chunk(const glm::vec4& rect_);
        ~Chunk();
    };
private:
    constexpr static int levels = 11;
    constexpr static int chunkDimen = 5000;
    constexpr static int maxObjectSize = 50; //the longest any one dimension of an entity can be
    const static glm::vec4 playerArea; //area that walls can't spawn because the player's stuff will be there
    friend class GameWindow;
  //  glm::vec4 rect = {0,0,0,0};
    std::shared_ptr<NavMesh> mesh;
    std::shared_ptr<Chunk> chunks[levels][levels];
    Chunk* currentChunk = nullptr;
    void addGatePair(int x1, int y1, int x2, int y2);
    void generateLevel();
};



#endif // WORLD_H_INCLUDED
