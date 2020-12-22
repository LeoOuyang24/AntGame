#include <queue>

#include "world.h"
#include "ants.h"
#include "game.h"
#include "navigation.h"
#include "animation.h"

Terrain::TerrainRender::TerrainRender(const glm::vec4& color, Entity& ent) : RectRenderComponent(color, ent), ComponentContainer<Terrain::TerrainRender>(ent)
{

}

void Terrain::TerrainRender::update()
{
    GameWindow::requestRect(((Object*)entity)->getRect().getRect(),color,true,0,FogMaker::fogZ);
}

Terrain::Terrain(int x, int y, int w, int h) : Object(*(new ClickableComponent("Terrain", *this)), *(new RectComponent({x,y,w,h},*this)), *(new RectRenderComponent({.5,.5,.5,1},*this)))
{
    addComponent(*(new RepelComponent(*this)));
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
    if (GameWindow::getLevel())
    GameWindow::getLevel()->findShard();
}

Shard::Shard() : ObjectAssembler("Shard", {40,40},&shardAnime, true)
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

PickUpResource::PickUpResource() : ObjectAssembler("Resource", {40,40},&resourceAnime,false)
{

}

Object* PickUpResource::assemble()
{
    Object* obj = ObjectAssembler::assemble();
    obj->addComponent(*(new PickUpResourceComponent(rand()%100 + 50,*obj)));
    return obj;
}

Map::Map(const glm::vec4& rect) : rect(rect)
{
    init(rect);
}

void Map::init(const glm::vec4& region)
{
   // rect = region;
    nextLevel();
    tree.reset(new RawQuadTree(region));

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
    obj.getRect().setPos({x,y});
    tree->update(obj.getRect(), *tree.get());
    //std::cout << obj.getCenter().x << std::endl;
   // std::cout << getChunk(obj).ants.size() << oldChunk->ants.size() << std::endl;
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
    int width = rect.z/100;
    for (int i = 0; i < rect.z/width; i++)
    {
        for (int j = 0; j < rect.a/width; j ++)
        {
            GameWindow::requestRect({rect.x + i*width,rect.y + j*width,width,width},{i/(rect.z/width),j/(rect.a/width),1,1},true,0,-1);
           // drawRectangle(RenderProgram::basicProgram,{i/(rect.z/width),j/(rect.a/width),1},{rect.x + i*width,rect.y + j*width,width,width},0);
        }
    }
    int size = terrain.size();
   // std::cout << size << std::endl;
    for (int i = 0; i < size; ++i)
    {
        terrain[i]->update();
    }    GameWindow::getFogMaker().requestRectFog(playerArea);
  //  mesh->render();
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
    foundShards = 0;
}

void Map::setChangeLevel(bool l)
{
    changeLevel = l;
}
bool Map::getChangeLevel()
{
    return changeLevel;
}

void Map::findShard()
{
    foundShards++;
    if (foundShards == 5)
    {
        addUnit(*(new Gate(playerArea.x + playerArea.z/2, playerArea.y + playerArea.a/2*3/2)),false);
        foundShards = 0;
    }
}

int Map::getFoundShards()
{
    return foundShards;
}

Anthill* Map::getAnthill()
{
    return mainHill.lock().get();
}

Map* Map::generateLevel(const glm::vec4& levelRect) // Doesn't generate terrain on the bottom most row. It's simply too big a hassle to ensure that inaccessible areas don't spawn on the bottom row
{
    Map* chunk = new Map(levelRect);
    //currentChunk = new Chunk({0,0,chunkDimen,chunkDimen});
    glm::vec2 points = {chunkDimen/maxObjectSize,chunkDimen/maxObjectSize};
    //addTerrain({100,100,100,100});
    //addTerrain({200,200,100,100});
   // glm::vec2 playerPoints = {playerArea.z/maxObjectSize + 1, playerArea.a/maxObjectSize + 1}; //points we can't use because it's in the player area
    int luck = 10;
    std::queue<std::pair<glm::vec2,bool>>  dists; //vector of empty areas. x is the starting index and y is the # of spawn points are on the line. Bool is true if the line is empty
    std::vector<glm::vec2> emptySpots;
    dists.push({{0,points.x},true});

    for (int i = 0; i < points.y -1; ++i) //for every row except the bottom most...
    {
        int size = dists.size();
        glm::vec2 line = {0,0};
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
    Shard shard;
    PickUpResource resource;
    glm::vec2 dimen = shard.dimen;
    for (int i = 0; i < 5; ++i) //add shards
    {
        glm::vec2 chosen = emptySpots[rand()%emptySpots.size()];
        chunk->addUnit(*(shard.assemble()),chosen.x + fmod(rand(),(maxObjectSize/2 - dimen.x/2)),
                chosen.y + fmod(rand(),(maxObjectSize/2 - dimen.y/2)),false);
    }
    dimen = resource.dimen;
    for (int i = 0; i < rand()%100; ++i)
    {
            glm::vec2 chosen = emptySpots[rand()%emptySpots.size()];
            chunk->addUnit(*(resource.assemble()),chosen.x + fmod(rand(),(maxObjectSize/2 - dimen.x/2)),
            chosen.y + fmod(rand(),(maxObjectSize/2 - dimen.y/2)),false);
    }
    chunk->mainHill = std::static_pointer_cast<Anthill>(chunk->addUnit(*(new Anthill({chunkDimen/2,chunkDimen/2})),true));
    return chunk;
   // addUnit(*(new Gate({chunkDimen/2, chunkDimen/2 + 100})));
}

Map::~Map()
{

}

Map::Gate::NextAreaButton::NextAreaButton() : Button({0,0,64,16},nullptr,nullptr,{"Next Area!"},&Font::tnr,{0,1,0,1})
{

}

void Map::Gate::NextAreaButton::press()
{
    Map* level = (GameWindow::getLevel());
    if (level)
    {
        level->setChangeLevel(true);
        GameWindow::getPlayer().addGold(10);
    }
}

Map::Gate::NextAreaButton::~NextAreaButton()
{

}

Map::Gate::Gate(int x, int y) : Object(*(new ClickableComponent("Gate",*this)),*(new RectComponent({x,y,64,64},*this)), *(new AnimationComponent(portalAnime,*this)))
{
    getClickable().addButton(*(new NextAreaButton()));
}

Map::Gate::~Gate()
{

}

/*Map::Chunk::Chunk(const glm::vec4& rect_)
{
    this->rect = rect_;
    tree.reset(new RawQuadTree(rect_));
}


glm::vec4& Map::Chunk::getRect()
{
    return rect;
}

void Map::Chunk::remove(Object& unit)
{
    tree->remove(unit.getRect());
    entities.erase(&unit);
}

void Map::Chunk::clear()
{
    while (entities.size() > 0)
    {
        remove(*(entities.begin()->first));
    }
    terrain.clear();
}

Map::Chunk::~Chunk()
{
    clear();
}*/

const glm::vec4 Map::playerArea = {chunkDimen/2 - 1000,chunkDimen/2 - 1000,2000,2000};



