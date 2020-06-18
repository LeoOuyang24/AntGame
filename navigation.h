#ifndef NAVIGATION_H_INCLUDED
#define NAVIGATION_H_INCLUDED

#include <unordered_map>

#include "render.h"

#include "entities.h"
#include "world.h"
#include "debug.h"

bool compareRect(const glm::vec4* o1, const glm::vec4* o2);

struct HashPoint
{
    std::size_t operator() (const glm::vec2& p1) const; //hash function for glm::vec2. As of now, glm::vec2s of negative points are mapped to the same location as glm::vec2s of positive points (-1,2) = (1,2)
};

class NavMesh //a navigation mesh of rectangle
{
    friend class Debug::DebugNavMesh;
    class NavMeshNode;
    typedef std::unordered_map<NavMeshNode*,glm::vec4> Neighbors; //pointer to the node and the line of intersection
    class NavMeshNode : public RectPositional
    {
        //typedef std::vector<Neighbors> Adjacents;
         Neighbors nextTo;
    public:

        NavMeshNode(const glm::vec4& rect);
        void addNode(NavMeshNode& n); //tests if n is adjacent to this node. Adds it if it is.
        void removeNode(NavMeshNode& n);
        void setDimen(const glm::vec2& dimens);
        void setRect(const glm::vec4& rect);
        Neighbors& getNextTo();
        void render() const; //draws lines between this node and its neighbors
        ~NavMeshNode();
    };

    glm::vec4 bounds; //rect of the entire mesh
    QuadTree nodeTree; // a quadtree of all the nodes. Makes insertion and finding nodes faster
    RawQuadTree* tree = nullptr;
  //  void addHelper(const glm::vec4& rect, NavMeshNode* current);
    NavMeshNode* getNode(const glm::vec2 point); //returns the node at the given position. Null if none

    void addNode(const glm::vec4& rect); //adds a node to the mesh assuming it doesn't collide with a preexisting node
    void addNode(NavMeshNode& node);

    void removeNode(NavMeshNode& node); //deletes node from both the vector and the nodeTree

    void splitNode(NavMeshNode& node, const glm::vec4& overlap); //creates new nodes and adds them to neighbors based on how overlap splits node. Does NOT delete node
public:
    NavMesh(const glm::vec4& bounds_, RawQuadTree& tree_);
    void init(ObjectStorage& storage); //initialize the mesh with given the vector of objects
    void smartAddNode(const glm::vec4& rect); //adds a node assuming it may collide with a preexisting node

    //void add(const glm::vec4& rect); //adds a rect to the navmesh. UNFINISHED!!!!
    //void render();
    Path getPath(const glm::vec2& start, const glm::vec2& end); //returns a path from start to end using A*. Not guaranteed to be the shortest path
};

#endif // NAVIGATION_H_INCLUDED
