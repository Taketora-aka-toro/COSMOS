#pragma once
#include "BulletParameter.h"

// íeëSëÃÇÃèàóù
void InitBullet();
void LoadBullet();
void StepBullet();
void UpdateBullet();
void DrawBullet();
void FinBullet();

// íeî≠éÀ
void FireBullet(BulletCategory category, int typeID, FireBulletData fireData, BulletCollisionTag tag);
void RequestFireBullet(int life, float posX, float posY, float moveX, float moveY, float radius);
