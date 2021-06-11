#include <queue>

#include "world.h"
#include "ants.h"
#include "game.h"
#include "navigation.h"
#include "animation.h"
#include "tiles.h"
#include "enemyAssemblers.h"

Terrain::TerrainRender::TerrainRender(const glm::vec4& color, Entity& ent) : RectRenderComponent(color, ent), ComponentContainer<Terrain::TerrainRender>(ent)
{

}

void Terrain::TerrainRender::update()
{
    GameWindow::requestRect(((Object*)entity)->getRect().getRect(),color,true,0,FogMaker::fogZ);
}

Terrain::Terrain(int x, int y, int w, int h) : Object(*(new ClickableComponent("Terrain", *this)), *(new RectComponent({x,y,w,h},*this)), *(new RectRenderComponent({.5,.5,.5,1},*this)))
{

}

void ItemComponent::onUse(Entity& other)
{

}

ItemComponent::ItemComponent(Entity& owner) : Component(owner), ComponentContainer<ItemComponent>(owner)
{

}

void ItemComponent::collide(Entity& other)
{
    if (static_cast<Object*>(&other)->getFriendly())
    {
        onUse(other);
        static_cast<Object*>(entity)->setDead(true);
    }
}

Shard::ShardComponent::ShardComponent(Entity& owner) : ItemComponent(owner), ComponentContainer<ShardComponent>(owner)
{

}

void Shard::ShardComponent::onUse(Entity& other)
{

}

Shard::Shard() : ObjectAssembler("Shard", {40,40},{shardAnime}, true)
{

}

Object* Shard::assemble()
{
    Object* stru = (ObjectAssembler::assemble());
    stru->addComponent(*(new ShardComponent(*stru)));
    return stru;
}

PickUpResource::PickUpResourceComponent::PickUpResourceComponent(int amount_, Entity& owner) : ItemComponent(owner),
ComponentContainer<PickUpResource::PickUpResourceComponent>(owner), amount(amount_)
{

}

void PickUpResource::PickUpResourceComponent::onUse(Entity& entity)
{
    GameWindow::getPlayer().addResource(amount);
}

PickUpResource::PickUpResource() : ObjectAssembler("Resource", {40,40},{resourceAnime},true)
{

}

Object* PickUpResource::assemble()
{
    Object* obj = ObjectAssembler::assemble();
    obj->addComponent(*(new PickUpResourceComponent(rand()%100 + 50,*obj)));

    return obj;
}

const glm::vec4 Room::playerArea = {chunkDimen/2 - chunkDimen/12,chunkDimen/2 - chunkDimen/12,chunkDimen/6,chunkDimen/6};

Room::Room(const glm::vec4& rect) : rect(rect)
{
    init(rect);
}

void Room::init(const glm::vec4& region)
{
   // rect = region;
    bucket = &grassEnemies;
    nextLevel();
    tree.reset(new RawQuadTree(region));
    for (int i = 0; i < chunkDimen/tileDimen; ++i)
    {
        for (int j = 0; j < chunkDimen/tileDimen; ++j)
        {
            tiles.push_back({{i*tileDimen,j*tileDimen},getRandomTile(grassSet)});
        }
    }
}

void Room::nextLevel()
{
    entities.clear();
    terrain.clear();
    tree.reset(new RawQuadTree(rect));
    mesh.reset(new NavMesh(rect,*(tree.get())));
//    generateLevel();
}

std::shared_ptr<Object> Room::addUnit(Object& entity, bool friendly)
{

    glm::vec2 center = entity.getCenter();

    return addUnit(entity,center.x,center.y,friendly);
}

std::shared_ptr<Object> Room::addUnit(Object& entity,int x, int y, bool friendly)
{
    std::shared_ptr<Object> ptr = std::shared_ptr<Object>(&entity);

    addUnit(ptr,x,y,friendly);
    return ptr;
}

void Room::addUnit(const std::shared_ptr<Object>& ptr, int x, int y, bool friendly)
{
    Object* entity = ptr.get();
    if (entity && entities.count(entity) == 0)
    {
        //std::shared_ptr<Object> obj = std::shared_ptr<Object>((new Bug(200,200)));
        //obj.reset();
        entity->setFriendly(friendly);

        entities[entity] = ptr;
        if (!entity->getComponent<ProjectileComponent>())
        {
            tree->add(entity->getRect());
        }
        if (!entity->getMovable())
        {
            mesh->smartAddNode(entity->getRect().getRect());
        }
        glm::vec2 center = entity->getCenter();
        if (center.x != x || center.y != y)
        {
            moveObject(*entity,x,y);
        }

        //ptr.reset();
       // remove(entity);

    }
}

void Room::addTerrain(const glm::vec4& rect)
{
    Terrain* terr = (new Terrain(rect.x,rect.y,rect.z,rect.a));
    terrain.emplace_back(terr);
    mesh->smartAddNode(rect);
}

std::shared_ptr<Object>& Room::getUnit(Object* unit)
{
    if (unit)
    {
        if (entities.find(unit) != entities.end())
        {
            return entities[unit];
        }
        throw std::runtime_error("Room.GetUnit: Can't find unit!");
    }
    else
    {
        throw std::runtime_error ("Room.GetUnit:Unit is null!");
    }
}

void Room::moveObject(Object& obj, double x, double y)
{
    glm::vec2 center=obj.getCenter();
    if (center.x != x || center.y != y)
    {
        obj.getRect().setPos({x - obj.getRect().getRect().z/2,y - obj.getRect().getRect().a/2});
        tree->update(obj.getRect(), *tree.get());
    }
    //std::cout << obj.getCenter().x << std::endl;
   // std::cout << getChunk(obj).ants.size() << oldChunk->ants.size() << std::endl;
}

UnitAssembler& Room::generateCreature()
{
    if (bucket)
    {
        return *getRandomAssembler(*bucket);
    }
    throw std::logic_error("Room::generateCreature: bucket is null!");
}

void Room::spawnCreature()
{
    UnitAssembler* toSpawn = &generateCreature();
  //  const glm::vec4* camera = &(GameWindow::getCamera().getRect());
    glm::vec2 entityRect = (toSpawn->dimen);
    auto area = getMesh().getRandomArea({rect.x + rect.z/2, rect.y + rect.a/2}, playerArea.z, std::min(rect.z/2,rect.a/2));
    if (area.z - entityRect.x > 0 && area.a - entityRect.y > 0) //if we have enough space
    {
        int x = rand()%((int)(area.z -  entityRect.x)) + area.x; //we want to make sure our object spawns outside of the camera's view and doesn't spawn partially out of the Room
        int y = rand() % ((int)(area.a - entityRect.y)) + area.y;
        addUnit(*static_cast<Unit*>(toSpawn->assemble()),x + entityRect.x/2,y + entityRect.y/2,false);//adjust for center
    }
}

void Room::remove(Object& unit)
{
    if (!unit.getMovable())
    {
        mesh->removeWall(unit.getRect());

    }
    tree->remove(unit.getRect());
    entities.erase(&unit);
}

void Room::clearEnemies()
{
    auto end = entities.end();
    for (auto i = entities.begin(); i != end; ++i)
    {
        if (i->first->getFriendly() == false)
        {
            i->first->setDead(true);
        }
    }
}

ObjectStorage& Room::getEntities()
{
    return entities;
}

NavMesh& Room::getMesh()
{
    auto ptr = mesh.get();
    if (ptr)
    {
        return *mesh.get();
    }
    throw std::logic_error("Room::getMesh(): Tried to return null mesh!");
}

void Room::render()
{
    auto end = tiles.end();
    for (auto start = tiles.begin(); start != end; ++start)
    {
        SpriteParameter param;
        param.rect = GameWindow::getCamera().toScreen({start->first.x,start->first.y,tileDimen,tileDimen});
        param.z = -1;
        start->second->request(param);
    }
    int size = terrain.size();
   // std::cout << size << std::endl;
    for (int i = 0; i < size; ++i)
    {
        terrain[i]->update();
    }    GameWindow::getFogMaker().requestRectFog(playerArea);

  //  mesh->render();
}

void Room::update()
{
    render();

}

const glm::vec4& Room::getRect() //returns rect of the current Chunk
{
   return rect;
}
RawQuadTree* Room::getTree()
{
    return tree.get();
}

void Room::reset()
{

}

bool Room::finishedLevel()
{
     bool noEnemies = true;
    for (auto it = entities.begin(); it != entities.end(); ++it)
    {
            //a room is complete if it has no enemies left. An enemy in this case is any entity that is:
        if (!it->first->getFriendly() && //not friendly,
            (!it->first->getComponent<TangibleComponent>() ||it->first->getComponent<TangibleComponent>()->getTangible()) && //doesn't have a tangible component or is tangible,
            !it->first->getComponent<ProjectileComponent>()) //and is not a projectile
        {
            noEnemies = false;
            break;
        }
    }

    return  noEnemies;
}

Room* Room::generateLevel(const glm::vec4& levelRect) // Doesn't generate terrain on the bottom most row. It's simply too big a hassle to ensure that inaccessible areas don't spawn on the bottom row
{
    Room* chunk = new Room(levelRect);
    //currentChunk = new Chunk({0,0,chunkDimen,chunkDimen});
    glm::vec2 points = {chunkDimen/maxObjectSize,chunkDimen/maxObjectSize};
    //addTerrain({100,100,100,100});
    //addTerrain({200,200,100,100});
   // glm::vec2 playerPoints = {playerArea.z/maxObjectSize + 1, playerArea.a/maxObjectSize + 1}; //points we can't use because it's in the player area
    int luck = 100;
    std::queue<std::pair<glm::vec2,bool>>  dists; //vector of empty areas. x is the starting index and y is the # of spawn points are on the line. Bool is true if the line is empty
    std::vector<glm::vec2> emptySpots;
    dists.push({{0,points.x},true});

    for (int i = 0; i < points.y -1; ++i) //for every row except the bottom most...
    {
        int size = dists.size();
        glm::vec2 line = {0,0}; //x is the starting coordinate. y is the width
        bool empty = true;
        glm::vec2 popped;
        bool poppedEmpty = false; //whether the line is empty or a line of blocks
        for (int j = 0; j < size; j ++) //for each line
        {
            int spawned = 0; //the number of spawned blocks
            int y = i*maxObjectSize;
            //std::cout << i << " " << blankSpace <<std::endl;
            popped =  dists.front().first;
            poppedEmpty = dists.front().second;
            //std::cout << i << " " << j << " " << popped.x << " " << popped.y << " " << std::endl;
            dists.pop();
            for (int g = 0; g < popped.y; ++g) //for each possible spawn point on a line
            {
                int x = (g + popped.x)*maxObjectSize;
                bool inPlayerArea = line.x >= playerArea.x && line.x + line.y < playerArea.x + playerArea.z && y >= playerArea.y && y < playerArea.y + playerArea.a;
                if (rand()%luck == 0 && (!poppedEmpty || spawned < popped.y - 1) && !pointInVec(playerArea,x,y,0)) //we have to get lucky, not be in the player area and either not hit the spawn limit or be before the line
                {

                    chunk->addTerrain({x,y,maxObjectSize,maxObjectSize});
                    luck/=2;
                    if (luck == 0)
                    {
                        luck = 10;
                    }
                    if (line.y > 0 && empty) //if we have started generating walls, end the wall of empty space
                    {
                        if (!(inPlayerArea)) //this isn't quite the same thing as checking if the point is in the rect because we want the resulting rects to not collide with player rect
                        {
                          //  std::cout << line.x<< " "<< line.y <<" " << empty<< std::endl;
                            dists.push({line,empty});
                        }
                        line = {line.x + (line.y ),1 };
                        empty = false;
                    }
                    else
                    {
                        line.y ++ ;
                    }
                    if (!empty)
                    {
                        spawned ++;
                    }
                }
                else
                {
                    if ( pointVecDistance(playerArea,x,y) > .1*chunkDimen)
                    {
                        emptySpots.push_back({x,y});
                    }
                    if (line.y > 0 && !empty) //if we have finished spawning walls, start generating empty space
                    {
                        if (!(inPlayerArea)) //this isn't quite the same thing as checking if the point is in the rect because we want the resulting rects to not collide with player rect
                        {
                          //  std::cout << line.x<< " "<< line.y<< " " << empty << std::endl;
                            dists.push({line,empty});
                            //chunk->addTerrain({x,y,maxObjectSize*(line.y),maxObjectSize});
                        }
                        line = {line.x + line.y, 1};
                        empty = true;
                    }
                    else
                    {
                        line.y ++;

                    }
                }
            }
        }
        if (line.y > 0)
        {
          //  std::cout << line.x << " " << line.y << std::endl;
            dists.push({line,empty});
        }
    }

    int rando = 3;
    for (int i = 0; i < rando; ++i)
    {
        chunk->spawnCreature();
    }
    //chunk->addUnit(*(attackAnt.assemble()),chunkDimen/2 - 100,chunkDimen/2,false);
    //chunk->addUnit(*(turtFrog.assemble()),chunkDimen/2 + 100,chunkDimen/2,false);
    //chunk->addUnit(*(dinosaur.assemble()),chunkDimen/2 + 100,chunkDimen/2 + 10,false);
    //chunk->addUnit(*(new Gate(*chunk,chunkDimen/2,chunkDimen/2)),true);
    return chunk;
}

Room::~Room()
{
  //  std::cout << "Deleted Room " << this << "\n";
}


Level::Level(int roomNum)
{
    for (int i = 0; i < roomNum; ++i)
    {
        rooms.emplace_back(Room::generateLevel());
    }
    for (int i =0; i< roomNum - 1; ++i)
    {
        rooms[i]->addUnit(*(new Gate(rooms[i],rooms[i+1],Room::chunkDimen/2,Room::chunkDimen/2)),true);
    }
    rooms[roomNum-1]->addUnit(*(new Gate(rooms[roomNum-1],*this,Room::chunkDimen/2,Room::chunkDimen/2)),true);
    setCurrentRoom(rooms[0].get());
}

Level::Level() : Level(3)
{

}

Room* Level::getCurrentRoom()
{
    return currentRoom;
}

void Level::setCurrentRoom(Room* room)
{
    currentRoom = room;
}

bool Level::getCompleted()
{
    return completed;
}

void Level::setCompleted(bool comp)
{
    completed = comp;
}

bool Level::getAllCompleted()
{
    int size = rooms.size();
    for (int i = 0; i < size; ++i)
    {
       if (!rooms[i]->finishedLevel())
       {
           return false;
       }
    }
    return true;
}

Level::~Level()
{
    //std::cout <<"Deleted Level: " << ;
}

Gate::NextAreaComponent::NextAreaComponent(std::shared_ptr<Room>& level_, std::shared_ptr<Room>& next_, Entity& entity) : Component(entity), ComponentContainer<NextAreaComponent>(&entity), room(level_), next(next_)
{

}

Gate::NextAreaComponent::NextAreaComponent(std::shared_ptr<Room>& room_, Level& level_, Entity& entity) : Component(entity), ComponentContainer<NextAreaComponent>(entity), level(&level_), room(room_)
{

}

Room* Gate::NextAreaComponent::getRoom()
{
    return room.lock().get();
}

Level* Gate::NextAreaComponent::getLevel()
{
    return level;
}

void Gate::NextAreaComponent::collide(Entity& entity)
{
    if (&entity == GameWindow::getPlayer().getPlayer() && KeyManager::getJustPressed() == SDLK_e)
    {
        if (next.lock().get())
        {
            GameWindow::setCurrentRoom(next.lock());
            next.lock().get()->addUnit(GameWindow::getPlayer().getPlayerPtr(),Room::chunkDimen/2,Room::chunkDimen/2,true);
        }
        else if (level)
        {
            level->setCompleted(true);
        }
        GameWindow::getPlayer().addGold(10);
    }
}

Gate::GateRender::GateRender(Entity& entity) : AnimationComponent(portalAnime,entity)
{

}

void Gate::GateRender::update()
{
    Gate::NextAreaComponent* nextArea = nullptr;
    if (sprite != &goldenPortalAnime && entity && (nextArea = entity->getComponent<Gate::NextAreaComponent>()))
    {
        if (nextArea->getLevel() && nextArea->getLevel()->getAllCompleted())
        {
            sprite = &goldenPortalAnime;
        }
    }
    AnimationComponent::update();
}

Gate::Gate(std::shared_ptr<Room>& room_, std::shared_ptr<Room>& next, int x, int y) : Object(*(new ClickableComponent("Gate",*this)),
                                                                                              *(new RectComponent({x,y,64,64},*this)),
                                                                                              *(new AnimationComponent(portalAnime,*this)))
{
    setFriendly(true);
    addComponent(*(new NextAreaComponent(room_,next, *this)));
    addComponent(*(new TangibleComponent([](Entity* entity){
                                         return entity->getComponent<Gate::NextAreaComponent>()->getRoom()->finishedLevel();
                                         },*this)));
   // getClickable().addButton(*(new NextAreaButton()));
}

Gate::Gate(std::shared_ptr<Room>& room_,Level& level,int x, int y) : Object(*(new ClickableComponent("Gate",*this)),
                                              *(new RectComponent({x,y,64,64},*this)),
                                              *(new GateRender(*this)))
{
    setFriendly(true);
     addComponent(*(new NextAreaComponent(room_,level, *this)));
    addComponent(*(new TangibleComponent([](Entity* entity){
                                         return entity->getComponent<Gate::NextAreaComponent>()->getRoom()->finishedLevel();
                                         },*this)));
}

Gate::~Gate()
{

}
