#include <dirent.h>
#include "tiles.h"

Tileset loadTileSet(std::string folder)
{
    DIR *dir;
    struct dirent *ent;

    /* Open directory stream */
    dir = opendir (folder.c_str());
    if (!dir)
    {
        std::cerr << "Couldn't open folder "<< folder << "\n";
        throw std::logic_error("Couldn't open folder " + folder);
    }
    else{
    Tileset tiles;
    while((ent = readdir(dir)) != nullptr)
    {
        switch (ent->d_type){
        case DT_REG:
            SpriteWrapper* spr = new SpriteWrapper();
            spr->init(folder + "/" + ent->d_name);
            tiles.push_back(spr);
            break;
        }
    }
    return tiles;
    }
}

void initTileSets()
{
    grassSet = loadTileSet("sprites/tiles/grass");
}

SpriteWrapper* getRandomTile(Tileset& set)
{
    if (set.size() == 0)
    {
        throw std::logic_error ("Can't load tile from empty tileset");
    }
    return set[rand()%set.size()];
}

Tileset grassSet;
