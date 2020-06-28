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
    if (vecIntersect(*otherArea,rect))
    {
        glm::vec4 intersect = vecIntersectRegion(*otherArea,rect);
        nextTo.insert({&n,{intersect.x,intersect.y, intersect.x + intersect.z, intersect.y + intersect.a}});
        if (n.getNextTo().count(this) == 0) //if the other node hasn't added us, add it!
        {
            n.addNode(*this);
        }
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

NavMesh::NavMeshNode* NavMesh::getNode(const glm::vec2& point)
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

NavMesh::NavMeshNode* NavMesh::getNearestNode(const glm::vec2& point)
{
    Positional p(point);
    auto vec = nodeTree.getNearest(p);
    int size = vec.size();
    if (size == 0)
    {
        return nullptr;
    }
    double minDistance = vec[0]->distance(point);
    Positional* ptr = vec[0];
    for (int i = 1; i < size; ++i)
    {
        double distance = vec[i]->distance(point);
        if (distance == 0)
        {
            return static_cast<NavMeshNode*>(vec[i]);
        }
        else if (distance < minDistance)
        {
            minDistance = distance;
            ptr = vec[i];
        }
    }
    return static_cast<NavMeshNode*>(ptr);
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

void NavMesh::removeNode(NavMeshNode& node)
{
    nodeTree.remove(node);
}

void NavMesh::splitNode(NavMeshNode& node, const glm::vec4& overlap)
{
    const glm::vec4* nodeRect = &(node.getRect());
    glm::vec4 region = vecIntersectRegion(*nodeRect,overlap);
    glm::vec4 borders[4] = { //the four new nodes of the current node is being split into
        {nodeRect->x, region.y, region.x - nodeRect->x, region.a}, //left
        {nodeRect->x, nodeRect->y,nodeRect->z, region.y - nodeRect->y }, //top
        {region.x + region.z, region.y,nodeRect->x + nodeRect->z - region.x - region.z, region.a}, //right
        {nodeRect->x, region.y + region.a, nodeRect->z, nodeRect->y + nodeRect->a - region.y - region.a} //bottom
        };
    NavMeshNode* left = nullptr, *right = nullptr, *top = nullptr, *bottom = nullptr;
    for (int i = 0; i < 4; ++i)
    {
        if (borders[i].z != 0 && borders[i].a != 0)
        {
            NavMeshNode* newNode = new NavMeshNode(borders[i]);
            addNode(*newNode); //add each node if they have non-0 dimensions and add them to neighbors
            auto neigh = &(node.getNextTo());
            auto end = neigh->end();
            for (auto j = neigh->begin(); j != end; ++ j)
            {
                if (j->first != newNode)
                {
                    j->first->addNode(*newNode);
                }
            }
            switch (i)
            {
            case 0:
                left = newNode;
                break;
            case 1:
                top = newNode;
                break;
            case 2:
                right = newNode;
                break;
            case 3:
                bottom = newNode;
                break;
            }

        }
    }
    if (left)
    {
        if (top)
        {
            left->addNode(*top);
        }
        if (bottom)
        {
            left->addNode(*bottom);
        }
    }
    if (right)
    {
        if (top)
        {
            right->addNode(*top);
        }
        if (bottom)
        {
            right->addNode(*bottom);
        }
    }
}

NavMesh::NavMesh(const glm::vec4& bounds_, RawQuadTree& tree_) : bounds(bounds_), negativeTree(bounds), nodeTree(bounds)
{

}

void NavMesh::init(ObjectStorage& storage)
{
    typedef std::pair<glm::vec3,std::vector<NavMeshNode*>> scanLine;
    std::vector<const glm::vec4*> vec;
    for (auto it = storage.begin(); it != storage.end(); ++it)
    {
        negativeTree.add(*(new RectPositional(it->first->getRect().getRect())));
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
    //removeNode(*(getNode({0,0})));

}

void NavMesh::smartAddNode(const glm::vec4& rect)
{
    if (vecContains(rect,bounds))
    {
        std::vector<Positional*> vec = nodeTree.getNearest(rect);
        for (int i = vec.size() - 1; i >= 0 ; i--) //find the first node that collides with rect. Once we've found it, we use the helper to finish the job. This is slightly more efficient since we only have to process nodes that are guaranteed to collide with the rect
        {
            if (vec[i]->collides(rect))
            {
                NavMeshNode* ptr = static_cast<NavMeshNode*>(vec[i]);
                splitNode(*ptr,rect);
                if (vecContains(ptr->getRect(),rect))
                {
                    i = 0; //we use i = 0 rather than break because we want to removeNode. We can't remove node earlier as then vecContains may be undefined
                }
                removeNode(*ptr);
            }
        }
        negativeTree.add(*(new RectPositional(rect)));
        //std::cout << nodeTree.size() << std::endl;
    }
}

Path NavMesh::getPath(const glm::vec2& startPoint, const glm::vec2& endPoint)
{
    NavMeshNode* startNode = getNearestNode(startPoint);  //ndoe we start off with. Repurposed later to be the node we are currently working on
    NavMeshNode* endNode = getNearestNode(endPoint);
    if (startNode && endNode)
    {
        glm::vec2 end = closestPointOnVec(endNode->getRect(),endPoint);
        glm::vec2 start = closestPointOnVec(startNode->getRect(),startPoint); //sometimes, our start/end isn't on a node. In that case, move to the closest point possible
        //GameWindow::requestRect(endNode->getRect(),{.5,.5,1,1},true,0,0,false);
        if (startNode == endNode)
        {
            return {start,end}; //if both the start and end is in the same node then just move lol
        }
        std::unordered_map<glm::vec2,std::tuple<double,glm::vec2,std::vector<NavMeshNode*>>,HashPoint> paths; //shortest distance from start to paths as well as the closest point that leads to it. Used for backtracking. A node can lie on many nodes so we also keep track the nodes we've visited
        MinHeap<std::pair<glm::vec2,NavMeshNode*>> heap; //finds the next node to process. We have to also store what node the point is associated with since the points all lie on the border of two nodes.
        heap.add({start,startNode},0);
        //double bestDist = -1; //the distance of the best path found so far. -1 until one path is found
       // glm::vec2 bestPoint = {0,0}; //the last point of the best path found so far. origin until one path is found
        glm::vec2 curPoint; //current point to analyze
        NavMeshNode* curNode = startNode; //current node to analyze
        int num = 0;
        while (heap.size() != 0 && heap.peak().first != end) //we end either when the heap is empty (no path) or when the top of the heap is the end (there is a path)
        {
            curPoint = heap.peak().first;
            //curPoint.x = floor(curPoint.x);
            curNode = heap.peak().second;
          //  GameWindow::requestRect(curNode->getRect(),{0,0,1,1},true,0,.1,false);
         // GameWindow::requestNGon(10,curPoint,10,{0,1,0,1},0,true,.9,false);
            heap.pop();
            if (curNode == endNode ) //if we are in the endNode, we can go directly to the end. This may not be the shortest path, so we keep searching
            {
                double score = pointDistance(curPoint,end) + std::get<0>(paths[curPoint]);
                if (paths.count(end) == 0)
                {
                    heap.add({end,endNode},score);
                    std::get<0>(paths[end]) = score;
                    std::get<1>(paths[end]) = curPoint;
                }
                else if (score < std::get<0>(paths[end]))
                {
                    heap.update({end,endNode},std::get<0>(paths[end]),score);
                    std::get<0>(paths[end]) = score;
                    std::get<1>(paths[end]) = curPoint;
                }

                continue;
            }
            Font::tnr.requestWrite({convert(num),GameWindow::getCamera().toScreen({curPoint.x,curPoint.y,10,10}),0,{1,1,1,1},2});
            Neighbors* nextTo = &(curNode->getNextTo());
            auto endIt = nextTo->end(); //get the end iterator
            for (auto it = nextTo->begin(); it != endIt; ++it)
            {

                if ((curPoint.x >= it->second.x && curPoint.x <= it->second.z) && (curPoint.y >= it->second.y && curPoint.y <= it->second.a)) //since all lines are horizontal or vertical, this tests to see if the our current point is on the intersection of our neighbor.
                {                    //We don't want to process this as it makes no progress
                    continue;
                }

                glm::vec2 midpoint;  //this is not actually the midpoint, but rather the point on the intersection line we think will be closest to the goal
                midpoint = lineLineIntersectExtend(curPoint,end,{it->second.x,it->second.y},{it->second.z,it->second.a});//this ensures that if there is a direct path to the end, we work towards it.
                glm::vec2 a = {it->second.x, it->second.y}, b = {it->second.z, it->second.a}; //the endpoints of the intersection line segment.

                if (!lineInLine(midpoint,end,a,b)) //if the midpoint isn't on the intersection, choose one of the endpoints
                {
                    double aDist = pointDistance(end,a) + pointDistance(curPoint,a);
                    double bDist = pointDistance(end,b) + pointDistance(curPoint,b);
                    if (aDist <= bDist)
                    {
                        midpoint = a;
                    }
                    else
                    {
                        midpoint = b;
                    }
                }
                else //of the midpoint and the endpoints of the intersection, find which is the best point to move to
                {
                    double midDist = pointDistance(end,midpoint) + pointDistance(curPoint,midpoint);
                    double aDist = pointDistance(end,a) + pointDistance(curPoint,a);
                    double bDist = pointDistance(end,b) + pointDistance(curPoint,b);
                    if (aDist <= bDist && aDist <= midDist)
                    {
                        midpoint = a;
                    }
                    else if (bDist <= aDist && bDist <= midDist)
                    {
                        midpoint = b;
                    }
                    //else, use midpoint
                }
                double newDistance = pointDistance(curPoint,midpoint) + std::get<0>(paths[curPoint]);
                double score = newDistance + pointDistance(midpoint,end); //the final score that also uses the heuristic
                bool newPoint = paths.count(midpoint) == 0;
                bool lowDist = newPoint || std::get<0>(paths[midpoint]) > newDistance;
                auto vec = &(std::get<2>(paths[midpoint]));
                bool newNode = std::find(vec->begin(), vec->end(),it->first) == vec->end();
                if ( newPoint || lowDist || newNode )
                    //if we found the new shortest distance from start to this point, update.
                { //if we have never reached this point before or if we have never been to this node before, add them to the heap.
                    if (newPoint || newNode) //add a never before visited point/node to the heap
                    {
                        heap.add({midpoint,it->first},score);
                        if (newPoint)
                        {
                            std::get<0>(paths[midpoint]) = newDistance;
                            std::get<1>(paths[midpoint])= curPoint;
                            auto ptr = &(std::get<2>(paths[midpoint]));
                            ptr->reserve(4);
                        }
                    }
                    if (lowDist && !newPoint) //otherwise, we either found a new, shorter path, and/or found that the same point is connected to another node. Update if the distance is shorter, add if it's a new node.
                    {
                        heap.update({midpoint,it->first},std::get<0>(paths[midpoint]),score);
                        std::get<0>(paths[midpoint]) = newDistance;
                        std::get<1>(paths[midpoint]) = curPoint;
                    }
                    if (newNode)
                    std::get<2>(paths[midpoint]).push_back(it->first);
                }
            }
            num++;
        }

        Path finalPath;
        curPoint = end;
        if (heap.size() != 0) //the end node is always guaranteed to be still in the heap if a path was found.
        {
           // std::cout << "Repeat\n";
            while (curPoint != start )
            {
                //std::cout << curPoint.x << " " << curPoint.y << std::endl;
                finalPath.push_front(curPoint);
                curPoint = std::get<1>(paths[curPoint]);
            }

            finalPath.push_front(start);
        }
        else
        {
            std::cout << "No path" << std::endl;
        }
        return finalPath;
    }
    else
    {
        throw std::logic_error("Can't get path with no starting or ending node!");
    }
}

bool NavMesh::straightLine(const glm::vec4& line)
{
    auto vec = negativeTree.getNearest({line.x,line.y,line.z - line.x, line.a - line.y});
    int size = vec.size();
    glm::vec2 a = {line.x, line.y}, b = {line.z, line.a};
    for (int i = 0; i < size; ++i)
    {
        if (lineInVec(a,b,(static_cast<RectPositional*>(vec[i]))->getRect(),0))
        {
            return false;
        }
    }
    return true;
}

