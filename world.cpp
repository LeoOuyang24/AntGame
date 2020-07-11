#include "world.h"
#include "entities.h"
#include "ants.h"
#include "game.h"
#include "navigation.h"

Terrain::Terrain(int x, int y, int w, int h) : Object(*(new ClickableComponent("Terrain", *this)), *(new RectComponent({x,y,w,h},*this)), *(new RectRenderComponent({.5,.5,.5,1},*this)))
{
    addComponent(*(new RepelComponent(*this)));
}

Map::Map()
{

}

void Map::init(const glm::vec4& region)
{
   // rect = region;
   setCurrentChunk(*(new Chunk({0,0,chunkDimen,chunkDimen})));
    mesh.reset(new NavMesh(currentChunk->rect,*(currentChunk->tree.get())));

    generateLevel();
    //addUnit(*(new Anthill({0,0})));
  //  addUnit(*(new Terrain(-10,-33,200,10)));
 //   addUnit(*(new Terrain(-33,10,10,200)));
  //  mesh->init2(getCurrentChunk().entities);
   // Terrain* t1 = new Terrain(-33,10,10,200);
   // Terrain* t2 = new Terrain(-73,10,10,200);
    //addUnit(*(t));
  //  mesh->smartAddNode(t1->getRect().getRect());
   // mesh->smartAddNode(t2->getRect().getRect());
}

std::shared_ptr<Object> Map::addUnit(Object& entity, bool friendly)
{
    std::shared_ptr<Object> ptr = std::shared_ptr<Object>(&entity);
    //std::shared_ptr<Object> obj = std::shared_ptr<Object>((new Bug(200,200)));
    //obj.reset();
    entity.setFriendly(friendly);
    currentChunk->entities[&entity] = ptr;
    currentChunk->tree->add(entity.getRect());
    if (!entity.getMovable())
    {
        mesh->smartAddNode(entity.getRect().getRect());
    }
    //ptr.reset();
   // remove(entity);

    return ptr;
}

void Map::addTerrain(const glm::vec4& rect)
{
    Terrain* terr = (new Terrain(rect.x,rect.y,rect.z,rect.a));
    currentChunk->terrain.emplace_back(terr);
    mesh->smartAddNode(rect);
}

std::shared_ptr<Object>& Map::getUnit(Object* unit)
{
    if (unit)
    {
        if (currentChunk->entities.find(unit) != currentChunk->entities.end())
        {
            return currentChunk->entities[unit];
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
    Chunk* oldChunk = &(getChunk(obj));
    obj.getRect().setPos({x,y});
    oldChunk->tree->remove(obj.getRect());
    Chunk* newChunk = &(getChunk(obj));
    newChunk->tree->add(obj.getRect());
    //std::cout << getChunk(obj).ants.size() << oldChunk->ants.size() << std::endl;

    newChunk->entities[&obj] = oldChunk->entities[&obj];
    oldChunk->entities.erase(&obj);
    //std::cout << obj.getCenter().x << std::endl;
   // std::cout << getChunk(obj).ants.size() << oldChunk->ants.size() << std::endl;
}

void Map::setCurrentChunk(Chunk& chunk)
{
    currentChunk = &chunk;
    GameWindow::getCamera().setBounds(&(chunk.rect));
}

void Map::remove(Object& unit)
{
    Chunk* chunk = &(getChunk(unit));
    chunk->tree->remove(unit.getRect());
    chunk->entities.erase(&unit);
}

Map::Chunk& Map::getChunk(Object& unit)
{
    glm::vec2 center = unit.getCenter();
   // std::cout << center.x << " " << center.y << std::endl;
    center.x = ((center.x) + chunkDimen*levels/2.0)/chunkDimen - levels/2;
    if (center.x < 0)
    {
        center.x = floor(center.x);
    }
    center.y = ((center.y) + chunkDimen*levels/2.0)/chunkDimen - levels/2;;
    if (center.y < 0)
    {
        center.y = floor(center.y);
    }
    return getChunk(center.x,center.y);
}

Map::Chunk& Map::getChunk(int x, int y)
{
    if (abs(x) > levels/2 || abs(y) > levels/2)
    {
        throw std::runtime_error("index out of bounds in Map.getChunks()!");
    }
    else
    {
        //std::cout << x + levels/2 << " " << y + levels/2 << std::endl;
        return (*(chunks[x+levels/2][y+levels/2]).get());
    }

}
Map::Chunk& Map::getCurrentChunk()
{
    if (!currentChunk)
    {
        throw std::runtime_error("No current chunk!");
    }
    return *currentChunk;
}
ObjectStorage& Map::getEntities(Chunk& chunk)
{
    return chunk.entities;
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
    glm::vec4 rect = getCurrentChunk().rect;
    int width = rect.z/100;
    for (int i = 0; i < rect.z/width; i++)
    {
        for (int j = 0; j < rect.a/width; j ++)
        {
            GameWindow::requestRect({rect.x + i*width,rect.y + j*width,width,width},{i/(rect.z/width),j/(rect.a/width),1,1},true,0,-1);
           // drawRectangle(RenderProgram::basicProgram,{i/(rect.z/width),j/(rect.a/width),1},{rect.x + i*width,rect.y + j*width,width,width},0);
        }
    }
  //  mesh->render();
}

const glm::vec4& Map::getRect(Chunk& chunk) //returns rect of the current Chunk
{
   return chunk.rect;
}
RawQuadTree* Map::getTree(Chunk& chunk)
{
    return chunk.tree.get();
}
RawQuadTree* Map::getTreeOf(Object& unit)
{
    return getChunk(unit).tree.get();
}

void Map::reset()
{
    for(int i = 0; i < levels; i++)
    {
        for (int j = 0; j < levels; j ++)
        {
            Chunk* current = chunks[i][j].get();
            if (current)
            {
                int size = current->entities.size();
                for (int it = 0 ; it < size; ++it)
                    {
                        remove(*(current->entities.begin()->second.get()));
                    }
                chunks[i][j].reset();
            }
        }
    }
}

Map::~Map()
{

}

Map::Gate::NextAreaComponent::NextAreaComponent(Object& unit) : InteractionComponent(unit), ComponentContainer<Gate::NextAreaComponent>(&unit)
{

}

void Map::Gate::NextAreaComponent::setDest(const std::shared_ptr<Gate>& other)
{
    dest = other;
}

void Map::Gate::NextAreaComponent::interact(Object& other)
{
    Gate* destination = dest.lock().get();
    if (destination && entity)
    {
        Map* level = &(GameWindow::getLevel());
        //level->setCurrentChunk(level->getChunk(*destination));
       // GameWindow::getCamera().setBounds(&(GameWindow::getLevel().getRect(GameWindow::getLevel().getChunk(*destination))));
        //other.getRect().setPos({destination->getRect().getCenter().x, destination->getRect().getCenter().y});
        level->moveObject(other,destination->getRect().getCenter().x, destination->getRect().getCenter().y);
    }
}

Map::Gate* Map::Gate::NextAreaComponent::getDest()
{
    return dest.lock().get();
}

Map::Gate::NextAreaComponent::~NextAreaComponent()
{

}

Map::Gate::NextAreaButton::NextAreaButton(Gate& unit) : Button({0,0,64,16},nullptr,nullptr,{"Next Area!"},&Font::tnr,{0,1,0,1}), gate(&unit)
{

}

void Map::Gate::NextAreaButton::press()
{
  Map* level = &(GameWindow::getLevel());
  Gate* dest = gate->getComponent<NextAreaComponent>()->getDest();
  if (dest)
  {
    level->setCurrentChunk(level->getChunk(*(dest)));
  }
}

Map::Gate::NextAreaButton::~NextAreaButton()
{

}

Map::Gate::Gate(int x, int y) : Object(*(new ClickableComponent("Gate",*this)),*(new RectComponent({x,y,64,64},*this)), *(new RectRenderComponent({1,.1,.1,1},*this)))
{
    nextArea = new Gate::NextAreaComponent(*this);
    addComponent(*nextArea);
    getClickable().addButton(*(new NextAreaButton(*this)));
}

void Map::Gate::setDest(const std::shared_ptr<Gate>& other)
{
    if (nextArea)
    {
        nextArea->setDest(other);
    }
    else
    {
        throw std::logic_error("No nextAreaComponent");
    }
}

Map::Gate::~Gate()
{

}

Map::Chunk::Chunk(const glm::vec4& rect_)
{
    this->rect = rect_;
    tree.reset(new RawQuadTree(rect_));
}

void Map::Chunk::clear()
{
    entities.clear();
}

void Map::Chunk::update()
{
    int size = terrain.size();
   // std::cout << size << std::endl;
    for (int i = 0; i < size; ++i)
    {
        terrain[i]->update();
    }
}

Map::Chunk::~Chunk()
{
    clear();
}

const glm::vec4 Map::playerArea = {chunkDimen/2 - 1000,chunkDimen/2 - 1000,2000,2000};

void Map::addGatePair(int x1, int y1, int x2, int y2)
{
    Map::Gate* gate1 = (new Gate(x1,y1));
    Map::Gate* gate2 = (new Gate(x2,y2));
    gate1->setDest(std::dynamic_pointer_cast<Gate>(addUnit(*gate2)));
    gate2->setDest(std::dynamic_pointer_cast<Gate>(addUnit(*gate1)));
    //addUnit(*gate1);
    //addUnit(*gate2);
}

void Map::generateLevel()
{
    //currentChunk = new Chunk({0,0,chunkDimen,chunkDimen});
    int walls = 100;
    glm::vec2 points = {chunkDimen/maxObjectSize,chunkDimen/maxObjectSize};
    int size =points.x*points.y;
    int maxDimen = 10*maxObjectSize;
    //addTerrain({100,100,100,100});
    //addTerrain({200,200,100,100});
   // glm::vec2 playerPoints = {playerArea.z/maxObjectSize + 1, playerArea.a/maxObjectSize + 1}; //points we can't use because it's in the player area
    int luck = 1;
    for (int i = 0; i < size; ++i)
    {
        if (rand()%luck == 0)
        {
            int x = fmod(i,points.x)*maxObjectSize;
            int y = floor(i/points.x)*maxObjectSize;
            if (pointInVec(playerArea,x,y,0))
            {
                continue;
            }
            addTerrain({x,y,maxObjectSize,maxObjectSize});
            luck/=2;
            if (luck == 0)
            {
                luck = 10;
            }
        }
    }

}
