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

const glm::vec4 Map::playerArea = {chunkDimen/2 - 500,chunkDimen/2 - 500,1000,1000};

Map::Map(const glm::vec4& rect) : rect(rect)
{
    init(rect);
}

void Map::init(const glm::vec4& region)
{
   // rect = region;
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

void Map::nextLevel()
{
    entities.clear();
    terrain.clear();
    tree.reset(new RawQuadTree(rect));
    mesh.reset(new NavMesh(rect,*(tree.get())));
//    generateLevel();
}

std::shared_ptr<Object> Map::addUnit(Object& entity, bool friendly)
{
    std::shared_ptr<Object> ptr = std::shared_ptr<Object>(&entity);
    //std::shared_ptr<Object> obj = std::shared_ptr<Object>((new Bug(200,200)));
    //obj.reset();
    entity.setFriendly(friendly);
    entities[&entity] = ptr;
    if (!entity.getComponent<ProjectileComponent>())
    {
        tree->add(entity.getRect());
    }
    if (!entity.getMovable())
    {
        mesh->smartAddNode(entity.getRect().getRect());
    }
    //ptr.reset();
   // remove(entity);

    return ptr;
}

std::shared_ptr<Object> Map::addUnit(Object& entity,int x, int y, bool friendly)
{
    auto asdf = addUnit(entity,friendly);
    moveObject(entity,x,y);
    return asdf;
}


void Map::addTerrain(const glm::vec4& rect)
{
    Terrain* terr = (new Terrain(rect.x,rect.y,rect.z,rect.a));
    terrain.emplace_back(terr);
    mesh->smartAddNode(rect);
}

std::shared_ptr<Object>& Map::getUnit(Object* unit)
{
    if (unit)
    {
        if (entities.find(unit) != entities.end())
        {
            return entities[unit];
        }
        throw std::runtime_error("Map.GetUnit: Can't find unit!");
    }
    else
    {
        throw std::runtime_error ("Map.GetUnit:Unit is null!");
    }
}

void Map::moveObject(Object& obj, double x, double y)
{

    obj.getRect().setPos({x - obj.getRect().getRect().z/2,y - obj.getRect().getRect().a/2});
    tree->update(obj.getRect(), *tree.get());
    //std::cout << obj.getCenter().x << std::endl;
   // std::cout << getChunk(obj).ants.size() << oldChunk->ants.size() << std::endl;
}

UnitAssembler& Map::generateCreature()
{
    int random = rand()%5;
    return evilMoonAssembler;
}

void Map::spawnCreature()
{
    UnitAssembler* toSpawn = &generateCreature();
  //  const glm::vec4* camera = &(GameWindow::getCamera().getRect());
    glm::vec2 entityRect = (toSpawn->dimen);
    auto area = getMesh().getRandomArea({rect.x + rect.z/2, rect.y + rect.a/2}, 0, std::min(rect.z/2,rect.a/2));
    if (area.z - entityRect.x > 0 && area.a - entityRect.y > 0) //if we have enough space
    {
        int x = rand()%((int)(area.z -  entityRect.x)) + area.x; //we want to make sure our object spawns outside of the camera's view and doesn't spawn partially out of the map
        int y = rand() % ((int)(area.a - entityRect.y)) + area.y;
        addUnit(*static_cast<Unit*>(toSpawn->assemble()),x + entityRect.x/2,y + entityRect.y/2,false);//adjust for center
    }
}

void Map::remove(Object& unit)
{
    if (!unit.getMovable())
    {
        mesh->removeWall(unit.getRect());

    }
    tree->remove(unit.getRect());
    entities.erase(&unit);
}

void Map::clearEnemies()
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

ObjectStorage& Map::getEntities()
{
    return entities;
}

NavMesh& Map::getMesh()
{
    auto ptr = mesh.get();
    if (ptr)
    {
        return *mesh.get();
    }
    throw std::logic_error("Map::getMesh(): Tried to return null mesh!");
}

void Map::render()
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

void Map::update()
{
    render();

}

const glm::vec4& Map::getRect() //returns rect of the current Chunk
{
   return rect;
}
RawQuadTree* Map::getTree()
{
    return tree.get();
}

void Map::reset()
{

}

void Map::setChangeLevel(bool l)
{
    changeLevel = l;
}
bool Map::getChangeLevel()
{
    return changeLevel;
}

bool Map::finishedLevel()
{
     bool noEnemies = true;
    for (auto it = entities.begin(); it != entities.end(); ++it)
    {
        if (!it->first->getFriendly())
        {
            noEnemies = false;
            break;
        }
    }
    return  noEnemies;
}

Map* Map::generateLevel(const glm::vec4& levelRect) // Doesn't generate terrain on the bottom most row. It's simply too big a hassle to ensure that inaccessible areas don't spawn on the bottom row
{
    Map* chunk = new Map(levelRect);
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

    int rando = 1;
    for (int i = 0; i < rando; ++i)
    {
        //chunk->spawnCreature();
    }
    //chunk->addUnit(*(attackAnt.assemble()),chunkDimen/2 - 100,chunkDimen/2,false);
    //chunk->addUnit(*(turtFrog.assemble()),chunkDimen/2 + 100,chunkDimen/2,false);
    chunk->addUnit(*(dinosaur.assemble()),chunkDimen/2 + 100,chunkDimen/2 + 10,false);
    chunk->addUnit(*(new Gate(*chunk,chunkDimen/2,chunkDimen/2)),true);
    return chunk;
}

Map::~Map()
{

}

Map::Gate::NextAreaComponent::NextAreaComponent(Map& map_, Entity& entity) : Component(entity), ComponentContainer<NextAreaComponent>(&entity), level(&map_)
{

}

Map* Map::Gate::NextAreaComponent::getLevel()
{
    return level;
}

void Map::Gate::NextAreaComponent::collide(Entity& entity)
{
    if (&entity == GameWindow::getPlayer().getPlayer() && level && KeyManager::findNumber(SDLK_e) != -1)
    {
        level->setChangeLevel(true);
        GameWindow::getPlayer().addGold(10);
    }
}

Map::Gate::Gate(Map& level, int x, int y) : Object(*(new ClickableComponent("Gate",*this)),*(new RectComponent({x,y,64,64},*this)), *(new AnimationComponent(portalAnime,*this)))
{
    addComponent(*(new NextAreaComponent(level, *this)));
    addComponent(*(new TangibleComponent([](Entity* entity){
                                         return entity->getComponent<Map::Gate::NextAreaComponent>()->getLevel()->finishedLevel();
                                         },*this)));
   // getClickable().addButton(*(new NextAreaButton()));
}

Map::Gate::~Gate()
{

}




