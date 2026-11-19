#pragma once
#include "MapParameter.h"

void LoadMapChipData(const char* fileName); 
void CreateMap();
MapChipData GetMapChipData(int x, int y);
// マップチップを破壊する
void DestroyMapChip(int x, int y);