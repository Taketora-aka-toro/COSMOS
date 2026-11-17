#pragma once
#include "MapParameter.h"
void InitMap();
void LoadMap();
void StartMap();
void DrawMap();
void FinMap();

void ChceckMapPlayerCollision();
bool CheckMapPlayerGoal();
void CheckMapBulletCollision();
// É}ÉbÉvêÿÇËë÷Ç¶
int CheckMapPlayerDoor();
void ChangeMap(MapID mapID);
MapID GetCurrentMap();
void SetPlayerSpawnPosition(float x, float y);
float GetPlayerSpawnX();
float GetPlayerSpawnY();