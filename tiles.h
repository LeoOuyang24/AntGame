#ifndef TILES_H_INCLUDED
#define TILES_H_INCLUDED

#include "render.h"

typedef std::vector<SpriteWrapper*> Tileset;

Tileset loadTileSet(std::string folder);
SpriteWrapper* getRandomTile(Tileset& set);

extern void initTileSets();
extern Tileset grassSet;

#endif // TILES_H_INCLUDED
