#include <algorithm>
#include <unordered_set>

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
    glm::vec4 line = {intersect.x,intersect.y, intersect.x + intersect.z, intersect.y + intersect.a};
    nextTo[&n] = line;
    if (n.getNextTo().count(this) == 0 || n.getNextTo()[this] != line) //if the other node hasn't added us or doesn't have the same intersection line, add it!
    {
        n.addNode(*this);
    }

}

bool NavMesh::NavMeshNode::isNextTo(NavMeshNode& n)
{
    return nextTo.count(&n) != 0;
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
    /*Camera* cam = &(GameWindow::getCamera());
    for (auto it = nextTo.begin(); it != nextTo.end(); ++it)
    {
        glm::vec2 center = cam->toScreen({rect.x + rect.z/2, rect.y + rect.a/2});
        glm::vec2 otherCenter = cam->toScreen({it->first->getRect().x + it->first->getRect().z/2, it->first->getRect().y + it->first->getRect().a/2});
       // PolyRender::requestLine({center.x,center.y,otherCenter.x,otherCenter.y},{center.x,0,otherCenter.x,1},1);
        PolyRender::requestRect(GameWindow::getCamera().toScreen({it->second.x,it->second.y - 10*(center.y > otherCenter.y),it->second.z,10}),{1,1,1,1},true,0,1);
    }*/
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
                glm::vec4 intersect = vecIntersectRegion(j->first->getRect(), borders[i]);
                if (j->first != newNode && (vecIntersect(j->first->getRect(), borders[i]) && (intersect.z != 0 || intersect.a != 0))) //the second statement ensures that rectangles that share a common side will still be neighbors but neighbors touching at the corner won't
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

glm::vec2 NavMesh::displacePoint(const glm::vec2& point, const glm::vec4& line, const glm::vec4& nodeRect, double width)
{
    glm::vec2 nextPoint = point;
    if (nextPoint.x == line.x + width) //on right side of obstacle
    {
        if ((nextPoint.y == nodeRect.y && nextPoint.x == nodeRect.x + width) || (nextPoint.y == nodeRect.y + nodeRect.a && nextPoint.x > nodeRect.x + width) ) //top right of obstacle
        {
            nextPoint.y -= width;
        }
        else //bot right
        {
            nextPoint.y += width;
        }
    }
    else if (nextPoint.x == line.z - width) //on left side of obstacle
    {
         if ((nextPoint.y == nodeRect.y && nextPoint.x == nodeRect.x + nodeRect.z - width) || (nextPoint.y == nodeRect.y + nodeRect.a && nodeRect.x + nodeRect.z -width > nextPoint.x)) //top left
         {
             nextPoint.y -= width;
         }
         else //bot left
         {
             nextPoint.y += width;
         }
    }
    return nextPoint;
}

NavMesh::NavMesh(const glm::vec4& bounds_, RawQuadTree& tree_) : bounds(bounds_), negativeTree(bounds), nodeTree(bounds)
{
    addNode(bounds);
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

void NavMesh::init2(ObjectStorage& storage)
{
    addNode(bounds);
    auto end = storage.end();
    for (auto i = storage.begin(); i != end; ++i)
    {
        smartAddNode(i->first->getRect().getRect());
    }
}


void NavMesh::smartAddNode(const glm::vec4& rect)
{
    if (nodeTree.size() == 0)
    {
        addNode(bounds);
    }
        std::vector<Positional*> vec = nodeTree.getNearest(rect);
        for (int i = vec.size() - 1; i >= 0 ; i--) //find the first node that collides with rect. Once we've found it, we use the helper to finish the job. This is slightly more efficient since we only have to process nodes that are guaranteed to collide with the rect
        {
            NavMeshNode* ptr = static_cast<NavMeshNode*>(vec[i]);
            if (vecInside(ptr->getRect(),rect))
                {
                    splitNode(*ptr,rect);
                    if (vecContains(ptr->getRect(),rect))
                    {
                        i = 0; //we use i = 0 rather than break because we want to removeNode. We can't remove node earlier as then vecContains may be undefined
                    }

                    removeNode(*ptr);
                    //std::cout << "Done removing" << std::endl;
                }
        }
        negativeTree.add(*(new RectPositional(rect)));
       // std::cout << nodeTree.size() << std::endl;
}

void NavMesh::reset()
{
    negativeTree.clear();
    nodeTree.clear();
}

bool NavMesh::notInWall(const glm::vec4& rect)
{
    auto near = negativeTree.getNearest(rect);
    int size = near.size();
    for (int i = 0; i < size; ++i)
    {
        if (near[i]->collides(rect))
        {
            return false;
        }
    }
    return true;
}

Path NavMesh::getPath(const glm::vec2& startPoint, const glm::vec2& endPoint, int width)
{
    NavMeshNode* startNode = getNearestNode(startPoint);  //ndoe we start off with. Repurposed later to be the node we are currently working on
    NavMeshNode* endNode = getNearestNode(endPoint);
    if (startNode && endNode)
    {
        glm::vec2 start = closestPointOnVec(startNode->getRect(),startPoint); //sometimes, our start/end isn't on a node. In that case, move to the closest point possible
        glm::vec2 end = closestPointOnVec(endNode->getRect(),endPoint);
        //GameWindow::requestRect(endNode->getRect(),{.5,.5,1,1},true,0,0,false);
        if (startNode == endNode)
        {
            return {start,end}; //if both the start and end is in the same node then just move lol
        }
        std::unordered_set<NavMeshNode*> visited; //set of visited nodes
        std::unordered_map<glm::vec2,NavMeshNode*,HashPoint> pointAndNodes; //map of points to their nodes. If a point is on the border of two nodes, it's the node that the previous point is not a part of
        std::unordered_map<glm::vec2,std::pair<double,glm::vec2>,HashPoint> paths; //shortest distance from start to paths as well as the closest point that leads to it. Used for backtracking. A node can lie on many nodes so we also keep track the nodes we've visited
        typedef std::pair<glm::vec2,NavMeshNode*> HeapPair ; //an important type for pathfinding
        MinHeap<HeapPair> heap; //finds the next node to process. We have to also store what node the point is associated with since the points all lie on the border of two nodes.
        heap.add({start,startNode},0);
        pointAndNodes[end] = endNode;
        pointAndNodes[start] = startNode;
        //double bestDist = -1; //the distance of the best path found so far. -1 until one path is found
       // glm::vec2 bestPoint = {0,0}; //the last point of the best path found so far. origin until one path is found
        glm::vec2 curPoint; //current point to analyze
        NavMeshNode* curNode = startNode; //current node to analyze
       // GameWindow::requestRect(startNode->getRect(), {1,0,0,1},true,0,1,0);
        bool startEdge = false; //there's a fun edge case where if the start point is on the edge of two nodes, the algorithm will skirt around the neighboring node. This helps fix that (see documentation).
                       auto time = SDL_GetTicks();
        while (heap.size() != 0 && heap.peak().first != end) //we end either when the heap is empty (no path) or when the top of the heap is the end (there is a path)
        {
            curPoint = heap.peak().first;
            //curPoint.x = floor(curPoint.x);
            curNode = heap.peak().second;
            const glm::vec4* curRect = &(curNode->getRect());
          //  GameWindow::requestRect(curNode->getRect(),{0,0,1,1},true,0,.1,false);
            //GameWindow::requestNGon(10,curPoint,10,{0,1,0,1},0,true,.9,false);
            heap.pop();
            if (curNode == endNode ) //if we are in the endNode, we can go directly to the end. This may not be the shortest path, so we keep searching
            {
                double score = pointDistance(curPoint,end) + paths[curPoint].first;
                if (paths.count(end) == 0)
                {
                    heap.add({end,endNode},score);
                    paths[end].first = score;
                    paths[end].second = curPoint;
                }
                else if (score < std::get<0>(paths[end]))
                {
                    paths[end].first = score;
                    paths[end].second = curPoint;
                }
                continue;
            }
            visited.insert(curNode); //if our node is not the endnode add it to the visited nodes set
           // Font::tnr.requestWrite({convert(num),GameWindow::getCamera().toScreen({curPoint.x,curPoint.y,10,10}),0,{1,1,1,1},2});
            Neighbors* nextTo = &(curNode->getNextTo());
            auto endIt = nextTo->end(); //get the end iterator
            for (auto it = nextTo->begin(); it != endIt; ++it)
            {
                if (visited.count(it->first) > 0)
                {
                    continue;
                }
                if (((curPoint.x >= it->second.x && curPoint.x <= it->second.z) && (curPoint.y == it->second.y)) //since all lines are horizontal or vertical, this tests to see if the our current point is on the intersection of our neighbor.
                     || it->second.z - it->second.x < width) //also don't process if the line is too narrow
                {
                    if (curPoint == start && !startEdge) //The case where curPoint == start deserves special attention only once. startEdge ensures we only do it once
                    {
                        double distance = pointDistance(curPoint,end) + paths[curPoint].first ;
                        heap.add({curPoint,it->first},distance);  //The closest point between our current point and a line that we are already on is obviously just curPoint.
                        startEdge = true;
                    }
                    continue;
                }

                glm::vec2 a = {it->second.x + width, it->second.y},
                b = {it->second.z - width, it->second.a}; //the endpoints of the intersection line segment.
                glm::vec2 midpoint;  //this is not actually the midpoint, but rather the point on the intersection line we think will be closest to the goal
                midpoint = lineLineIntersectExtend(curPoint,end,a,b);//this ensures that if there is a direct path to the end, we work towards it.
               // PolyRender::requestLine(glm::vec4(GameWindow::getCamera().toScreen(midpoint),GameWindow::getCamera().toScreen(curPoint)),{1,.5,.5,1},10);
                //GameWindow::requestRect((it)->first->getRect(),{0,1,0,1},true,0,1,0);

                if (!lineInLine(midpoint,end,a,b)) //if the midpoint isn't on the intersection, choose one of the endpoints
                {
                    midpoint = pointDistance(end,a) + pointDistance(curPoint,a) < pointDistance(end,b) + pointDistance(curPoint,b) ? a : b;
                }
                else
                {
                    midpoint = pointDistance(end,midpoint) + pointDistance(curPoint,midpoint) < pointDistance(end,a) + pointDistance(curPoint,a) ? midpoint : a;
                    midpoint = pointDistance(end,midpoint) + pointDistance(curPoint,midpoint) < pointDistance(end,b) + pointDistance(curPoint,b) ? midpoint : b;
                }
                const glm::vec4* nodeRect = &(it->first->getRect());
                midpoint = displacePoint(midpoint,it->second,*nodeRect,width);
                //std::cout << it->first << std::endl;
                //printRect(*nodeRect);
                if (Debug::getRenderPath())
                {
                    GameWindow::requestNGon(10,midpoint,1,{.5,1,0,1},0,true,.9,false);
                }
                double newDistance = pointDistance(curPoint,midpoint) + std::get<0>(paths[curPoint]);
                bool newPoint = paths.count(midpoint) == 0;
                //if we found the new shortest distance from start to this point, update.
                if (newPoint ||paths[midpoint].first > newDistance) //add a never before visited point/node to the heap or update if we found a new short distance
                {
                    paths[midpoint].first = newDistance;
                    paths[midpoint].second = curPoint;
                   /* if (pointAndNodes.find(midpoint) != pointAndNodes.end())
                    {
                        fastPrint("Replaced: ");
                        std::cout << midpoint.x << " " << midpoint.y << std::endl;
                    }*/
                    pointAndNodes[midpoint] = it->first;
                    if (newPoint)
                    {
                        //the final score that also uses the heuristic
                        heap.add({midpoint,it->first},newDistance + pointDistance(midpoint,end));
                     //   heap.print();
                    }
                }
            }
        }
        Path finalPath;
        curPoint = end;
        if (heap.size() != 0) //the end node is always guaranteed to be still in the heap if a path was found.
        {
           // std::cout << "Repeat\n";
           glm::vec2 shortCut = curPoint; //shortcut is the last point that we know curPoint can move to without hitting a wall that has yet to be added to finalPath
           finalPath.push_back(curPoint); //curPoint is the last point that was added to our final Path
            while (curPoint != start && shortCut != start)
            {
                //std::cout << paths.count(curPoint) << " " << curPoint.x << " " << curPoint.y << std::endl;
                glm::vec2 nextPoint = paths[shortCut].second; //nextPoint is the nextPoint to process.
                //std::cout << shortCut.x << " " << shortCut.y << " " << &(pointAndNodes[shortCut]->getNextTo()) << "\n";
                if (pointAndNodes[shortCut]->getNextTo().count(pointAndNodes[nextPoint])!= 0) //this is only false in the beginning since the end point and the second-last point are both in the endNode
                {

                    glm::vec4 line = pointAndNodes[shortCut]->getNextTo()[pointAndNodes[nextPoint]];// - glm::vec4(width,0,width,0);
                    glm::vec4 nodeRect = pointAndNodes[nextPoint]->getRect();
                    if ((!lineInLine(curPoint,nextPoint,{line.x,line.y},{line.z,line.a})
                         &&
                         (std::max(curPoint.y,nextPoint.y) >= line.y && std::min(curPoint.y,nextPoint.y) <= line.y))
                        /*||
                        (!lineInLine({nodeRect.x,nodeRect.y},{nodeRect.x + nodeRect.z,nodeRect.y},nextPoint,curPoint)
                          &&
                        !lineInLine({nodeRect.x, nodeRect.y + nodeRect.a},{nodeRect.x + nodeRect.z, nodeRect.y + nodeRect.a},nextPoint,curPoint))*/
                        ||
                        ((nextPoint.x < line.x || nextPoint.x > line.z) ||
                         (curPoint.x < line.x || curPoint.x > line.z)
                                     )) //if we can't move to nextPoint, then shortCut is the furthest we can move.
                        {

                            if (Debug::getRenderPath())
                            {
                                GameWindow::requestNGon(10,shortCut,1,{1,1,0,1},0,true,1,false);
                                                PolyRender::requestLine(glm::vec4({GameWindow::getCamera().toScreen({line.x,line.y}),
                              GameWindow::getCamera().toScreen({line.z,line.a})}),{1,1,0,1},2);
                            }
                            finalPath.push_front(shortCut);
                            curPoint = shortCut;
                        }
                        else
                        {
                            if (Debug::getRenderPath())
                            {
                                GameWindow::requestNGon(10,shortCut,1,{1,0,1,1},0,true,1,false);
                            }
                        }

                }
                    shortCut = nextPoint;
                  //  GameWindow::requestNGon(10,shortCut,10,{.5,1,0,1},0,true,.9,false);

            }

            finalPath.push_front(start);
        }
        else
        {
            return {};
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
        glm::vec4 wallRect = (static_cast<RectPositional*>(vec[i]))->getRect();
        bool intersect = lineInVec(a,b,wallRect,0);
        //std::cout << "Points: ";
        //printRect(line);
        //printRect(wallRect);
        if (intersect && !pointInVec(wallRect,a.x,a.y,0) && !pointInVec(wallRect,b.x,b.y,0))//sometimes, buildings may target or be targeted so we want to make sure we don't mind if the line starts or ends in a wall.
        {
            return false;
        }
    }
    //PolyRender::requestLine(glm::vec4(GameWindow::getCamera().toScreen(a),GameWindow::getCamera().toScreen(b)),{1,0,0,1},10);
    return true;
}

glm::vec4 NavMesh::getRandomArea(const glm::vec2& origin, double minDist, double maxDist)
{
    if (nodeTree.size() == 0)
    {
        throw std::logic_error ("Can't get random area of blank navigation mesh!");
    }
    NavMeshNode* node = nullptr;
    while (!node)
    {
        double radius = fmod(rand(),(maxDist - minDist)) + minDist;
        double theta = rand()%360*M_PI/180;
        glm::vec2 point = {origin.x + radius*cos(theta), origin.y + radius*sin(theta)};
            //std::cout << "Point: " << point.x << " " << point.y << std::endl;

        point.x = std::max(std::min(bounds.x + bounds.z, point.x), bounds.x);
        point.y = std::max(std::min(bounds.y + bounds.a, point.y), bounds.y); //clamp the points so that the point can't be out of bounds
       node  = getNearestNode(point);
    }
    return node->getRect();
}

void NavMesh::removeWall(RectPositional& positional)
{
    glm::vec4 rect = positional.getRect();

   negativeTree.remove(positional,[](const Positional& p1, const Positional& p2)
                                {
                                   glm::vec4 r1 =  static_cast<const RectPositional*>(&p1)->getRect();
                                   glm::vec4 r2 = static_cast<const RectPositional*>(&p2)->getRect();
                                    return  r1 == r2;
                                });
    auto vec = nodeTree.getNearest({rect.x - 1, rect.y - 1, rect.z + 2, rect.a + 2}); //find nearest nodes. We use a slightly larger rect in case the rect is teh exact size of a quadtree node
    std::sort(vec.begin(),vec.end(), [](Positional* p1, Positional* p2){
              return p1->getPos().y < p2->getPos().y;
              }); //sort positionals by lowest y coord to highest
    std::vector<RectPositional*> top, bottom; //represents nodes at the top and bottom of the wall
    int size = vec.size();
    bool skippedFirst = false; //skip the first node that is in line with the wall
    NavMeshNode* baseNode = nullptr, //baseNode is the unfinished node,
                *topNode = nullptr, //topNode is the topMostNode,
                *botNode = nullptr; // botNode is the botMostNode
    bool sideWall = rect.x == bounds.x || rect.x + rect.z == bounds.x + bounds.z; //true if the wall borders the left or right edge of the mesh
    for (int i = 0; i < size; ++i)
    {
        NavMeshNode* node = static_cast<NavMeshNode*>(vec[i]);
        glm::vec4 blankRect = node->getRect();
        if (vecIntersect(blankRect,rect))
        {
            bool inWall = blankRect.x >= rect.x && blankRect.x + blankRect.z <= rect.x + rect.z; //if the rect is entirely enclosed in the wall
            if (blankRect.y + blankRect.a == rect.y && inWall)
            {
                top.push_back(node);
            }
            else if (blankRect.y == rect.y + rect.a && inWall)
            {
                bottom.push_back(node);
            }
            else if (blankRect.y >= rect.y && blankRect.y < rect.y + rect.a)
            {
                if (sideWall)
                {
                    //life is really easy if sideWall is true because we don't have to worry about a node on the other side of the wall.
                    //Simply adjust the dimensions of our current node
                    node->setRect({
                                  std::min(rect.x,blankRect.x),
                                  blankRect.y,
                                  blankRect.z + rect.z,
                                  blankRect.a
                                  });
                }
                else //if not sideWall, then the next rectPositional should be on the right side of the wall due to how we organized our list
                {
                    if (!skippedFirst || !baseNode)
                    {
                        skippedFirst = true;
                        baseNode = node;
                    }
                    else
                    {
                        glm::vec4 baseRect = baseNode->getRect();
                       NavMeshNode* finalNode = baseRect.a < blankRect.a ? baseNode : node; //finalNode is the one that we are done with
                       glm::vec4 finalRect = {
                                          std::min(baseRect.x,blankRect.x),
                                          baseRect.y, //it shouldn't matter which y we use because it should be the same
                                          baseRect.z + blankRect.z + rect.z,
                                          finalNode->getRect().a
                                            };
                       finalNode->setRect(finalRect);
                        if (finalRect.y == rect.y)
                        {
                            topNode = finalNode;
                        }
                        else if (finalRect.y == rect.y + rect.a)
                        {
                            botNode = finalNode;
                        }
                        auto nextTo = baseNode->getNextTo();
                        auto nextSize = nextTo.end();
                        for (auto j = nextTo.begin(); j != nextSize; ++j) //add all neighbors to the finalNode
                        {
                            if (finalNode->collides((j->first->getRect()))) //if the resized node collides with the neighbor and doesn't already have the node as a neighbor
                            {
                                finalNode->addNode(*(j->first));
                            }
                        }
                        if (blankRect.a == baseRect.a) //we start over
                        {
                            nodeTree.remove(*baseNode);
                            baseNode = nullptr;
                        }
                        else //otherwise, set baseNode to the remaining, smaller node
                        {
                            baseNode = finalNode == baseNode ? node : baseNode; //set baseNode to the unfinished node
                            baseNode->setRect({
                              baseNode->getRect().x,
                              finalRect.y + finalRect.a,
                              baseNode->getRect().z,
                              baseRect.y + baseNode->getRect().a - finalNode->getRect().a
                              });
                        }
                    }
                }
            }
        }
        else
        {
            continue;
        }
    }
    if (baseNode) //this should only occur if sideWall is also true
    {
        glm::vec4 baseRect = baseNode->getRect();
        baseNode->setRect({
                          std::min(rect.x,baseRect.x),
                          baseRect.y,
                          baseRect.z + rect.z,
                          baseRect.a
                          });
        if (baseRect.y == rect.y)
        {
            topNode = baseNode;
        }
        botNode = baseNode;
    }
    int topSize = top.size();
    int botSize = bottom.size();
    for (int i = 0; i < topSize; ++i)
    {
        if (i < topSize)
        {
            topNode->addNode(*static_cast<NavMeshNode*>(top[i]));
        }
    }
    for (int i = 0; i < botSize; ++i)
    {
        if (i < botSize)
        {
            botNode->addNode(*static_cast<NavMeshNode*>(bottom[i]));
        }
    }
}

