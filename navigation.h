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
        void addNode(NavMeshNode& n); //adds a node as a neighbor. If the node is already a neighbor, update the intersection line
        bool isNextTo(NavMeshNode& n); //returns true if n is already in our neighbors list
        void removeNode(NavMeshNode& n);
        void setDimen(const glm::vec2& dimens);
        void setRect(const glm::vec4& rect);
        Neighbors& getNextTo();
        void render() const; //draws lines between this node and its neighbors
        ~NavMeshNode();
    };

    glm::vec4 bounds; //rect of the entire mesh
    QuadTree nodeTree; // a quadtree of all the nodes. Makes insertion and finding nodes faster
    QuadTree negativeTree ; //tree of RectPositionals that are not nodes. Essentially, tree of areas where objects can't move to.
  //  void addHelper(const glm::vec4& rect, NavMeshNode* current);
    NavMeshNode* getNode(const glm::vec2& point); //returns the node at the given position. Null if none
    NavMeshNode* getNearestNode(const glm::vec2& point); //returns the nearest node at the given position. The same as getNode if the point is in a node. Returns null if there are no nearby nodes

    void addNode(const glm::vec4& rect); //adds a node to the mesh assuming it doesn't collide with a preexisting node
    void addNode(NavMeshNode& node);

    void removeNode(NavMeshNode& node); //deletes node from both the vector and the nodeTree
    void splitNode(NavMeshNode& node, const glm::vec4& overlap); //creates new nodes and adds them to neighbors based on how overlap splits node. Does NOT delete node
    glm::vec2 displacePoint(const glm::vec2& point,const glm::vec4& line, const glm::vec4& nodeRect, double width); //Helper Function for getPath(). given a node's Rect and its boundary, this function predicts where walls are and moves the point accordingly.
public:
    NavMesh(const glm::vec4& bounds_, RawQuadTree& tree_);
    void init(ObjectStorage& storage); //initialize the mesh with given the vector of objects
    void init2(ObjectStorage& storage); //initializes the mesh by calling smartAddNode on every object
    void smartAddNode(const glm::vec4& rect); //adds a negative space
    void reset(); //clears nodeTree and negativeTree but keeps bounds
    glm::vec4 getWallRect(const glm::vec4& rect); //returns the wall that rect intersects with. glm::vec4(0) if there is no wall
    bool notInWall(const glm::vec4& rect); //returns true if rect is in a wall.
    glm::vec4 getNearestNodeRect(const glm::vec2& point); //returns the rect of the node that the point belongs to. glm::vec4(0) if there is no nearby node
    Path getPath(const glm::vec2& start, const glm::vec2& end, int width = 0); //returns a path from start to end using A*, returning the empty path if there is no path. Not guaranteed to be the shortest path. Width is the shortest distance we can be from any given negative area. This allows us to find paths for large objects without colliding with walls. This function is so convoluted there's a whole documentation in the documents folder!
    bool straightLine(const glm::vec4& line); //returns true if the line doesn't overlap with any negative space.
    glm::vec4 getRandomArea(const glm::vec2& origin, double minDist, double maxDist); //returns the area of a randomly chosen navmesh node that is between min and max distacne from origin. Guaranteed won't crash by generating a point out of bounds
    //note that this function just randomly generates a point and then chooses the nearest node. This means that larger nodes have larger chances of of being selected. This makes sense, but it should be noted that not all nodes have an equal chance of being selected.
    void removeWall(RectPositional& rect); //removes the wall at the given position and replaces with empty space
};

#endif // NAVIGATION_H_INCLUDED
