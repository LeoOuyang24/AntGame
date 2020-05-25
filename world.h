#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include "glInterface.h"
#include "SDLhelper.h"
#include "components.h"
#include "entities.h"


class Object;
class Ant;

typedef  std::map<Object*, std::shared_ptr<Object>> ObjectStorage;
typedef  std::map<Ant*, std::shared_ptr<Ant>> AntStorage;

class Map
{
    constexpr static int levels = 11;
    constexpr static int chunkDimen = 2000;
    friend class GameWindow;
  //  glm::vec4 rect = {0,0,0,0};
    struct Chunk
    {
        //friend class Map;
        glm::vec4 rect = {0,0,0,0};
        std::map<Object*, std::shared_ptr<Object>> entities;
      //  std::vector<std::shared_ptr<Label>> labels;
        std::shared_ptr<RawQuadTree> tree;
        void clear();
        Chunk(const glm::vec4& rect_);
        ~Chunk();
    };
    std::shared_ptr<Chunk> chunks[levels][levels];
    Chunk* currentChunk = nullptr;
    void addGatePair(int x1, int y1, int x2, int y2);
public:
    Map();
    void init(const glm::vec4& region);
    std::shared_ptr<Object> addUnit(Object& entity);//this method returns the shared_ptr in case any other class wants to share ownership.
    std::shared_ptr<Object>& getUnit(Object* unit);
    void moveObject(Object& obj, double x, double y); //can move either ants or objects
    Chunk& getChunk(int x, int y); //assumes 0,0 = [5][5]
    Chunk& getChunk(Object& unit);
    void setCurrentChunk(Chunk& chunk);
    ObjectStorage& getEntities(Chunk& chunk);
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
};



#endif // WORLD_H_INCLUDED
