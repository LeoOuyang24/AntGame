#include <algorithm>

#include "navigation.h"
#include "game.h"

bool compareRect(const glm::vec4* o1, const glm::vec4* o2)
{
    return o1->x < o2->x;
}

std::size_t HashPoint::operator()(const glm::vec2& p1) const
{
    return pow(10,floor(log(abs(p1.x))))*abs(p1.x) + abs(p1.y);
}

NavMesh::NavMeshNode::NavMeshNode(const glm::vec4& rect) : RectPositional(rect)
{

}

void NavMesh::NavMeshNode::addNode(NavMeshNode& n)
{
    const glm::vec4* otherArea = &(n.getRect());
    glm::vec4 intersect = vecIntersectRegion(*otherArea,rect);
    nextTo.insert({&n,{intersect.x,intersect.y, intersect.x + intersect.z, intersect.y + intersect.a}});
    if (n.getNextTo().count(this) == 0) //if the other node hasn't added us, add it!
    {
        n.addNode(*this);
    }
}

void NavMesh::NavMeshNode::removeNode(NavMeshNode& n)
{
    auto found = nextTo.find(&n);
    if ( (found) != nextTo.end())
    {
        nextTo.erase(found);
    }
}

void NavMesh::NavMeshNode::setDimen(const glm::vec2& dimens)
{
    rect.z = dimens.x;
    rect.a = dimens.y;
}
void NavMesh::NavMeshNode::setRect(const glm::vec4& rect)
{
   this->rect = rect;
}

NavMesh::Neighbors& NavMesh::NavMeshNode::getNextTo()
{
    return nextTo;
}

void NavMesh::NavMeshNode::render() const
{
    Camera* cam = &(GameWindow::getCamera());
    for (auto it = nextTo.begin(); it != nextTo.end(); ++it)
    {
        GameWindow::requestNGon(4,{(it->second.x + it->second.z)/2,(it->second.y + it->second.a)/2},10,{0,1,0,1},0,true,1,false);
        glm::vec2 center = cam->toScreen({rect.x + rect.z/2, rect.y + rect.a/2});
        glm::vec2 otherCenter = cam->toScreen({it->first->getRect().x + it->first->getRect().z/2, it->first->getRect().y + it->first->getRect().a/2});
        PolyRender::requestLine({center.x,center.y,otherCenter.x,otherCenter.y},{center.x,0,otherCenter.x,1},1);
    }
}

NavMesh::NavMeshNode::~NavMeshNode()
{
    for (auto j = nextTo.begin(); j != nextTo.end(); ++j)
    {
        if (j->first)
        {
            j->first->removeNode(*this);
        }
    }
}

NavMesh::NavMeshNode* NavMesh::getNode(const glm::vec2 point)
{
    Positional p(point);
    auto vec = nodeTree.getNearest(p);
    int size = vec.size();
    for (int i = 0; i < size; ++i)
    {
        if (vec[i]->distance(point) == 0)
        {
            return static_cast<NavMeshNode*>(vec[i]);
        }
    }
    return nullptr;
}


void NavMesh::NavMesh::addNode(const glm::vec4& rect)
{
    NavMeshNode* node = new NavMeshNode(rect);
    addNode(*node);
}

void NavMesh::addNode(NavMeshNode& node)
{
    nodeTree.add(node);
}

void NavMesh::smartAddNodeHelper(const glm::vec4& rect, NavMeshNode& node)
{
    splitNode(node,rect);
    if (!vecContains(rect,node.getRect())) //if the rect is completely encompassed by node, there is no reason to check neighbors as it is impossible for them to intersect with the rect
    {
        auto neigh = &(node.getNextTo());
        auto end = neigh->end();
        for (auto i = neigh->begin(); i != end; ++i)
        {
            if (i->first->collides(rect))
            {
                smartAddNodeHelper(rect,*(i->first));
            }
        }
    }
    removeNode(node);

}

void NavMesh::smartAddNode(const glm::vec4& rect)
{
    std::vector<Positional*> vec = nodeTree.getNearest(rect);
    for (int i = vec.size() - 1; i >= 0 ; i--) //find the first node that collides with rect. Once we've found it, we use the helper to finish the job. This is slightly more efficient since we only have to process nodes that are guaranteed to collide with the rect
    {
        if (vec[i]->collides(rect))
        {
            NavMeshNode* ptr = static_cast<NavMeshNode*>(vec[i]);
            smartAddNodeHelper(rect,*ptr);
            break;
        }
    }
}

void NavMesh::removeNode(NavMeshNode& node)
{
    nodeTree.remove(node);
}

void NavMesh::splitNode(NavMeshNode& node, const glm::vec4& overlap)
{
    const glm::vec4* nodeRect = &(node.getRect());
    glm::vec4 region = vecIntersectRegion(*nodeRect,overlap);
    glm::vec4 borders[4] = { //the four new nodes of the current node is being split into
        {nodeRect->x, nodeRect->y, region.x - nodeRect->x, nodeRect->a}, //left
        {region.x, nodeRect->y,region.z, region.y - nodeRect->y }, //top
        {region.x + region.z, nodeRect->y,nodeRect->x + nodeRect->z - region.x - region.z, nodeRect->a}, //right
        {region.x, region.y + region.a, region.z, nodeRect->y + nodeRect->a - region.y + region.a} //bottom
        };
    for (int i = 0; i < 4; ++i)
    {
        if (borders[i].z != 0 && borders[i].a != 0)
        {
            NavMeshNode* node = new NavMeshNode(borders[i]);
            addNode(*node); //add each node if they have non-0 dimensions and add them to neighbors
            auto neigh = &(node->getNextTo());
            auto end = neigh->end();
            for (auto j = neigh->begin(); j != end; ++ j)
            {
                j->first->addNode(*node);
            }
        }
    }
}

NavMesh::NavMesh(const glm::vec4& bounds_, RawQuadTree& tree_) : bounds(bounds_), tree(&tree_), nodeTree(bounds)
{

}

void NavMesh::init(ObjectStorage& storage)
{
    typedef std::pair<glm::vec3,std::vector<NavMeshNode*>> scanLine;
    std::vector<const glm::vec4*> vec;
    for (auto it = storage.begin(); it != storage.end(); ++it)
    {
        vec.push_back(&(it->first->getRect().getRect()));
    }
    glm::vec4* edge = new glm::vec4({bounds.x + bounds.z, bounds.y, 1, bounds.a});
    vec.push_back(edge);
    std::sort(vec.begin(), vec.end(),compareRect);
    int size = vec.size();
    std::vector<scanLine> frontLine;
    frontLine.push_back({{bounds.x, bounds.y, bounds.a},{}});
   // std::cout << frontLine[0].second << std::endl;
    for (int i = 0; i < size; ++i)
    {
        const glm::vec4* rect = vec[i];
        scanLine top = {{0,0,0},{}}, bottom = {{0,0,0},{}}; //the lines that will result from the top and bottom
        unsigned int it;
        //std::cout << frontLine[0].first.y << " " << frontLine[frontLine.size()-1].first.y << std::endl;
        for ( it = 0; it < frontLine.size(); )
        {
            glm::vec3* line = &(frontLine[it].first);
            if (vecIntersect({line->x,line->y,rect->x - line->x,line->z},*rect )) //if the next object intersects with a front line
                {
                    NavMeshNode* node = nullptr;
                    if (rect->x - line->x > 0) //no reason to create a node if it has a width of 0
                    {
                        node = new NavMeshNode({line->x,line->y, rect->x - line->x, line->z}); //create the new node that is from the line up to the object
                        addNode(*node);
                        int size1 = frontLine[it].second.size();
                        for (int j = 0; j < size1; ++j) //add all of the neighbors we've encountered so far
                        {
                            node->addNode(*(frontLine[it].second[j]));
                        }
                        if (it < frontLine.size() - 1 && frontLine[it + 1].first.x <= rect->x) //same but now for the line above. We don't have to check if this line is actually the previous line because its impossible for the bottom line to already be erased
                        {
                            frontLine[it + 1].second.push_back(node);
                        }
                        if (it > 0 && frontLine[it - 1].first.x <= rect->x && frontLine[it - 1].first.y + frontLine[it - 1].first.z == line->y) //if the line directly below this one is left of the rect, then its resulting node must be a neighbor
                        {
                            frontLine[it - 1].second.push_back(node);
                        }
                    }


                    glm::vec3 pushRect = {rect->x , line->y,rect->y - line->y}; //top line
                    if (pushRect.z > 0)
                    {
                        top.first = pushRect;
                        if (node)
                        {
                            top.second.push_back(node);
                        }
                    }
                    pushRect = {rect->x, rect->y + rect->a, line->y + line->z - (rect->y + rect->a) }; //bottom line
                    frontLine.erase(frontLine.begin() + it); //we erase here because if we erase any earlier, we wouldn't be able to access line's variables. If we erase later, we may not erase due to the break statement.
                    if (pushRect.z > 0)//if this line creates a bottom line, then we are certainly done; the next line would be even lower and will certainly not intersect with our object
                    {
                        bottom.first = pushRect;
                        if(node)
                        {
                            bottom.second.push_back(node);
                        }
                        break;
                    }
                }
            else
                {
                    ++it;
                }
        }
        if (top.first.z > 0) //add the new top node
        {
            frontLine.insert(frontLine.begin() + it, top);
            it++;
        }
        frontLine.insert(frontLine.begin() + it,{{rect->x + rect->z ,rect->y, rect->a },{}}); //middle node
        it++;
        if (bottom.first.z > 0) //bottom node
        {
            frontLine.insert(frontLine.begin() + it, bottom);
        }
       /* for (int a = 0; a < frontLine.size(); a++)
        {
            std::cout << frontLine[a].first.y  << " " << frontLine[a].first.z << std::endl;
        }
        std::cout << "\n";*/
    }

}


Path NavMesh::getPath(const glm::vec2& start, const glm::vec2& end)
{
    NavMeshNode* startNode = getNode(start);  //ndoe we start off with. Repurposed later to be the node we are currently working on
    NavMeshNode* endNode = getNode(end);
    if (startNode && endNode)
    {
        if (startNode == endNode)
        {
            return {start,end}; //if both the start and end is in the same node then just move lol
        }
        std::unordered_map<glm::vec2,std::pair<double,glm::vec2>,HashPoint> paths; //shortest distance from start to paths as well as the closest node. Used for backtracking
        MinHeap<std::pair<glm::vec2,NavMeshNode*>> heap; //finds the next node to process. We have to also store what node the point is associated with since the points all lie on the border of two nodes.
        heap.add({start,startNode},0);
        double bestDist = -1; //the distance of the best path found so far. -1 until one path is found
        glm::vec2 bestPoint = {0,0}; //the last point of the best path found so far. origin until one path is found
        glm::vec2 curPoint; //current point to analyze
        NavMeshNode* curNode = startNode; //current node to analyze
        while (heap.size() != 0)
        {
            curPoint = heap.peak().first;
            //curPoint.x = floor(curPoint.x);
            curNode = heap.peak().second;
            heap.pop();
            int direct = paths[curPoint].first + pointDistance(curPoint,end);
            bool isDirect = false;
            if ( direct < bestDist || bestDist == -1) //if the direct path between the current point and the end is smaller than the best known path, check to make sure there are no obstacles
            {
                glm::vec4 tempRect = {std::min(curPoint.x,end.x), std::min(curPoint.y,end.y), std::max(abs(end.x - curPoint.x),1.0f), std::max(1.0f,abs(end.y - curPoint.y))};
                auto vec = tree->getNearest(tempRect);
                int size = vec.size();
                if (size == 0)
                {
                    isDirect = true;
                }
                else
                {
                    for (int i = 0; i < size; ++i)
                    {
                    //    printRect(static_cast<RectPositional*>(vec[i])->getRect());
                        if (lineInVec(curPoint,end,(static_cast<RectPositional*>(vec[i]))->getRect())) //unfortunately, there is not a direct path
                        {
                            break;
                        }
                        if (i == size-1) //there is a direct path!
                        {
                            isDirect = true;
                        }
                    }
                }
                if (isDirect) //if there is a direct path
                {
                    bestPoint = curPoint;
                    bestDist = direct;
                    continue;
                }
            } //if there is no direct path, check neighbors for a path
            Neighbors* nextTo = &(curNode->getNextTo());
            auto endIt = nextTo->end(); //get the end iterator
            for (auto it = nextTo->begin(); it != endIt; ++it)
            {
                if ((curPoint.x >= it->second.x && curPoint.x <= it->second.z) && (curPoint.y >= it->second.y && curPoint.y <= it->second.a)) //since all lines are horizontal or vertical, this tests to see if the our current point is on the intersection of our neighbor.
                    //We don't want to process this as it makes no progress
                {
                    continue;
                }
                glm::vec2 midpoint; //this is not actually the midpoint, but rather the point on the intersection line we think will be closest to the goal
                if (it->second.y == it->second.a) //if the intersection is horizontal
                {
                    float left = std::min(it->second.x,it->second.z);
                    float right = it->second.x + it->second.z - left;
                   midpoint = {std::max(std::min(right,(end.x + curPoint.x)/2),left), it->second.y} ; //find the best point. Sometimes, the best point is off the line, so we take either edge point
                }
                else //if the intersection is vertical
                {
                    float high = std::max(it->second.y, it->second.a);
                    float low = it->second.y + it->second.a - high;
                   midpoint = {it->second.x, std::max(std::min(high,(end.y + curPoint.y)/2),low)};
                }
                double newDistance = pointDistance(curPoint,midpoint) + paths[curPoint].first;
                double score = newDistance + pointDistance(midpoint,end); //the final score that also uses the heuristic


                if (((bestDist != -1 && score < bestDist)  || bestDist == -1) &&
                    (paths.count(midpoint) == 0 || paths[midpoint].first > newDistance)) //if we found the new shortest distance from start to this point, update. If this distance is longer than the shortest known path to the destination, don't even bother recording it
                {
                    if (it->first != endNode)
                    {
                        if (paths.count(midpoint) == 0)
                        {
                            heap.add({midpoint,it->first},score);
                        }
                        else
                        {
                            heap.update({midpoint,it->first},paths[midpoint].first,score);
                        }
                    }
                    else //don't add a point with a direct path to the end since the shortest path from it to the end is just their direct path; there's no reason to process it further
                    {
                        bestDist = score; //score is now the actual distance from the point to the end since they are in the same node
                        bestPoint = midpoint;
                    }
                    paths[midpoint].first = newDistance;
                    paths[midpoint].second = curPoint;
                }
            }
        }

        Path finalPath;
        if (bestDist != -1)
        {
            curPoint = bestPoint;
            while (curPoint != start )
            {

                finalPath.push_front(curPoint);
                curPoint = paths[curPoint].second;
            }
            finalPath.push_back(end); //we always have to add the start and end so we just do it at the very end.
            finalPath.push_front(start);
        }
            return finalPath;
    }
    else
    {
        throw std::logic_error("Can't get path with no starting or ending node!");
    }
}
