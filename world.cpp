#include "world.h"
#include "entities.h"
#include "ants.h"
#include "game.h"

Map::Chunk::Chunk(const glm::vec4& rect_)
{
    this->rect = rect_;
    tree.reset(new RawQuadTree(rect_));
}

void Map::Chunk::clear()
{
    entities.clear();
}

Map::Chunk::~Chunk()
{
    clear();
}

void Map::addGatePair(int x1, int y1, int x2, int y2)
{
    Map::Gate* gate1 = (new Gate(x1,y1));
    Map::Gate* gate2 = (new Gate(x2,y2));
    gate1->setDest(std::dynamic_pointer_cast<Gate>(addUnit(*gate2)));
    gate2->setDest(std::dynamic_pointer_cast<Gate>(addUnit(*gate1)));
    //addUnit(*gate1);
    //addUnit(*gate2);
}

Map::Map()
{

}

void Map::init(const glm::vec4& region)
{
   // rect = region;
    for (int i = 0; i < levels; ++i)
    {
        for (int j = 0; j < levels; ++j)
        {
            glm::vec2 topLeft= {i*chunkDimen-levels/2.0*chunkDimen,(j%(levels))*chunkDimen - levels/2.0*chunkDimen};
            chunks[i][j].reset(new Chunk({{topLeft.x,topLeft.y,chunkDimen,chunkDimen}}));
        }
    }
    for (int i = 0; i < levels; ++i)
    {
        for (int j = 0; j < levels; ++j)
        {
            const glm::vec4* currentRect = &(chunks[i][j]->rect);
          //  remove(*g);*/
          if ( i != levels - 1)
          {
            addGatePair(currentRect->x + chunkDimen - 64, currentRect->y + chunkDimen/2 - 32, chunks[i+1][j]->rect.x, currentRect->y + chunkDimen/2 - 32); //rightmost gate
          }
          if (j != levels - 1)
          {
            addGatePair(currentRect->x + chunkDimen/2 - 32, currentRect->y + chunkDimen - 64, chunks[i][j+1]->rect.x + chunkDimen/2,chunks[i][j+1]->rect.y); //downmost gate
          }
        }
    }
    setCurrentChunk((getChunk(0,0)));
}

std::shared_ptr<Object> Map::addUnit(Object& entity)
{
    Chunk* chunk = &(getChunk(entity));
    if (chunk == nullptr)
    {
        throw std::logic_error("Tried to add object to non-existing chunk!");
    }
    std::shared_ptr<Object> ptr = std::shared_ptr<Object>(&entity);
    //std::shared_ptr<Object> obj = std::shared_ptr<Object>((new Bug(200,200)));
    //obj.reset();
    chunk->entities[&entity] = ptr;
    chunk->tree->add(entity.getRect());
    //ptr.reset();
   // remove(entity);

    return ptr;
}

std::shared_ptr<Object>& Map::getUnit(Object* unit)
{
    if (unit)
    {
        Chunk* chunk = &(getChunk(*unit));
        if (chunk->entities.find(unit) != chunk->entities.end())
        {
            return chunk->entities[unit];
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
