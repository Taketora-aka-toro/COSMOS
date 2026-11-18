#include "DxLib.h"
#include "../../Scene/SceneManager.h"
#include "../../Map/MapManager.h"
#include "../../Player/Player.h"
#include "../../Camera/Camera.h"
#include "../../Collision/Collision.h"
#include "../../Bullet/StraightBullet.h"
#include "../../Bullet/BulletManager.h"
#include "../../Enemy/EnemyManager.h"
#include"../../BackGround/BackGround.h"
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
	InitBackGround();
}

void LoadPlayScene()
{
	LoadBackGround("Data/BackGround/bg.jpg", BACK_GROUND_LAYER_1);
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
	UpdateBackGround();
	UpdatePlayer();
	UpdateBullet();
	
	// 当たり判定
	CheckCollision();

	// 扉との接触判定
	int door = CheckMapPlayerDoor();

	// エリアAの2番扉 → エリアB
	if (GetCurrentMap() == MAP_ID_A &&
		door == MAP_CHIP_GOAL)
	{
		SetPlayerSpawnPosition(100.0f, 700.0f);

		ChangeMap(MAP_ID_B);
		ChangeScene(SCENE_PLAY);
	}
	// エリアBの4番扉 → エリアA
	else if (GetCurrentMap() == MAP_ID_B &&
		door == MAP_CHIP_GOAL_B)
	{
		SetPlayerSpawnPosition(
			4800.0f - PLAYER_WIDTH - 5.0f,
			700.0f
		);

		ChangeMap(MAP_ID_A);
		ChangeScene(SCENE_PLAY);
	}
}

void DrawPlayScene()
{
	DrawBackGround();
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
	FinBackGround();
	FinEnemy();
	// プレイヤー終了
	FinPlayer();
}
