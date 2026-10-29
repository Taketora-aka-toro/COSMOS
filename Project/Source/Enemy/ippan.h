
#pragma once
#include "DxLib.h"
#include "../Animation/Animation.h"
#include "../Collision/Collision.h"

#define ENEMY_MAX 128
enum EnemyAnimationType
{

	IPPAN_ANIM_STOP,
	IPPAN_ANIM_MOVE,

	IPPAN_ANIM_MAX,
	IPPAN_ANIM_NONE = -1
};
struct EnemyData
{
	EnemyAnimationType enemyAnim;
	int animFrame;
	int animTimer;
	VECTOR prevPos;
    bool active;
    int handle;
    VECTOR pos;
	float moveX;  // X方向の移動量
	float moveY;  // Y方向の移動量
    int hp;
	AnimationData animation[IPPAN_ANIM_MAX];	// アニメーション
	BoxCollision boxCollision;					// 矩形当たり判定
};

void Initippan();
void Loadippan();
void Drawippan();
void Finippan();

EnemyData* Createippan(VECTOR pos);
EnemyData* Getippans();
void Stepippan();