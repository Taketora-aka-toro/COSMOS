#include "DxLib.h"
#include "../../Scene/SceneManager.h"
#include "../../Map/MapManager.h"
#include "../../Player/Player.h"
#include "../../Camera/Camera.h"
#include "../../Collision/Collision.h"
#include "../../Bullet/StraightBullet.h"
#include "../../Bullet/BulletManager.h"
#include "../../Enemy/EnemyManager.h"
// 背景のスピード
#define BACK_GROUND_SPEED (-1.5f)

void InitPlayScene()
{
	// プレイヤー初期化
	InitPlayer();

	// マップ初期化
	InitMap();

	// カメラ初期化
	InitCamera();
	InitEnemy();
}

void LoadPlayScene()
{
	// プレイヤーロード
	LoadPlayer();
	LoadBullet();
	// マップをロード
	LoadMap();
	LoadEnemy();
}

void StartPlayScene()
{
	// プレイヤー開始
	StartPlayer();

	// マップを開始
	StartMap();
}

void StepPlayScene()
{
	StepPlayer();
	StepEnemy();
	StepBullet();
	StepCamera();
	StepEnemySpawnSystem();
}

void UpdatePlayScene()
{
	// マップ更新
	// プレイヤー更新
	UpdatePlayer();
	UpdateBullet();
	// 当たり判定
	CheckCollision();
}

void DrawPlayScene()
{
	// マップ描画
	DrawMap();
	DrawEnemy();


	// 弾描画
	DrawBullet();
	// プレイヤー描画
	DrawPlayer();
	// カメラ描画
	DrawCamera();
}

void FinPlayScene()
{
	// マップ終了
	FinMap();
	FinEnemy();
	// プレイヤー終了
	FinPlayer();
}
