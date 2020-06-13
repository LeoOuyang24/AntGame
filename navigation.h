#ifndef NAVIGATION_H_INCLUDED
#define NAVIGATION_H_INCLUDED

#include <unordered_map>

#include "render.h"

#include "entities.h"
#include "world.h"

bool compareRect(const glm::vec4* o1, const glm::vec4* o2);

struct HashPoint
{
    std::size_t operator() (const glm::vec2& p1) const; //hash function for glm::vec2. As of now, glm::vec2s of negative points are mapped to the same location as glm::vec2s of positive points (-1,2) = (1,2)
};

class NavMesh //a navigation mesh of rectangle
{
    class NavMeshNode;
    typedef std::vector<glm::vec2> Path;
    typedef std::unordered_map<NavMeshNode*,glm::vec4> Neighbors; //pointer to the node and the line of intersection
    class NavMeshNode
    {
        //typedef std::vector<Neighbors> Adjacents;
         Neighbors nextTo;
        glm::vec4 area;
    public:

        NavMeshNode(const glm::vec4& rect);
        const glm::vec4& getArea();
        void addNode(NavMeshNode& n); //tests if n is adjacent to this node. Adds it if it is.
        void removeNode(NavMeshNode& n);
        void setDimen(const glm::vec2& dimens);
        void setRect(const glm::vec4& rect);
        Neighbors& getNextTo();
        void render(); //draws lines between this node and its neighbors
        ~NavMeshNode();
    };

    glm::vec2 left = {0,0}, right = {0,0};
    glm::vec4 bounds; //rect of the entire mesh
    std::vector<std::unique_ptr<NavMeshNode>> nodes;
    RawQuadTree* tree = nullptr;
  //  void addHelper(const glm::vec4& rect, NavMeshNode* current);
  NavMeshNode* getNode(const glm::vec2 point); //returns the node at the given position. Null if none
public:
    NavMesh(const glm::vec4& bounds_, RawQuadTree& tree_);
    void init(ObjectStorage& storage); //initialize the mesh with given the vector of objects
    //void add(const glm::vec4& rect); //adds a rect to the navmesh. UNFINISHED!!!!
    void render();
    Path getPath(const glm::vec2& start, const glm::vec2& end); //returns a path from start to end using dijkstra's. Not guaranteed to be the shortest path
};

#endif // NAVIGATION_H_INCLUDED
