#include "MapManager.h"
#include "MapParameter.h"
#include "MapChip.h"
#include "Block.h"
#include "../Player/Player.h"
#include"../Bullet/BulletManager.h"
#include"../Bullet/StraightBullet.h"
#include"../Enemy/EnemyManager.h"
#include"../Enemy/ippan.h"
#include"../Sound/SoundManager.h"
// キャラクターの周囲何マスまでチェックするか
#define CHECK_ROUND_NUM (2)
// プレイヤーの次回出現位置
float g_PlayerSpawnX = 100.0f;
float g_PlayerSpawnY = 600.0f;
// 現在読み込んでいるマップ
MapID g_CurrentMap = MAP_ID_B;
void InitMap()
{
	InitBlock();
}
void ChangeMap(MapID mapID)
{
	g_CurrentMap = mapID;
}
MapID GetCurrentMap()
{
	return g_CurrentMap;
}
void SetPlayerSpawnPosition(float x, float y)
{
	g_PlayerSpawnX = x;
	g_PlayerSpawnY = y;
}

float GetPlayerSpawnX()
{
	return g_PlayerSpawnX;
}

float GetPlayerSpawnY()
{
	return g_PlayerSpawnY;
}
int CheckMapPlayerDoor()
{
	PlayerData player = GetPlayer();
	float playerX = player.isTurn ?
		player.posX + PLAYER_WIDTH - player.boxCollision.posX - player.boxCollision.width :
		player.posX + player.boxCollision.posX;
	float playerY = player.posY + player.boxCollision.posY;
	float playerW = player.boxCollision.width;
	float playerH = player.boxCollision.height;

	for (int y = 0; y < MAP_CHIP_Y_NUM; y++)
	{
		for (int x = 0; x < MAP_CHIP_X_NUM; x++)
		{
			MapChipData mapChipData = GetMapChipData(x, y);
			/*printfdx("接触した扉id：%d\n", mapchipdata.mapchip);*/
			if (mapChipData.mapChip != MAP_CHIP_GOAL &&
				mapChipData.mapChip != MAP_CHIP_GOAL_B)
			{
				continue;
			}

			float doorX = x * MAP_CHIP_WIDTH;
			float doorY = y * MAP_CHIP_HEIGHT;

			if (CheckSquareSquare(
				playerX, playerY, playerW, playerH,
				doorX, doorY, MAP_CHIP_WIDTH, MAP_CHIP_HEIGHT))
			{
				return mapChipData.mapChip;
			}
		}
	}

	return 0;
}
void LoadMap()
{
	if (g_CurrentMap == MAP_ID_A)
	{
		LoadMapChipData("Data/Map/Map.bin");
	}
	else if (g_CurrentMap == MAP_ID_B)
	{
		LoadMapChipData("Data/Map/Map2.bin");
	}

	LoadBlock();
}

void StartMap()
{
	CreateMap();
}

void DrawMap()
{
	// ブロック描画
	DrawBlock();
}

void FinMap()
{
}

void ChceckMapPlayerCollision()
{
	PlayerData player = GetPlayer();
	// プレイヤーの足元の座標をマップチップのインデックスに変換
	int playerX = (int)(player.posX / MAP_CHIP_WIDTH);
	int playerY = (int)((player.posY + PLAYER_HEIGHT) / MAP_CHIP_HEIGHT);
	// プレイヤーの左上にあるマップチップインデックス
	int left = playerX - CHECK_ROUND_NUM;
	int top = playerY - CHECK_ROUND_NUM;
	// プレイヤーの右下にあるマップチップインデックス
	int right = playerX + CHECK_ROUND_NUM;
	int bottom = playerY + CHECK_ROUND_NUM;

	// まずはX軸でチェックする

	// 左上から3マスずつチェックしていく
	for (int y = top; y <= bottom; y++)
	{
		// マップチップからはみ出したら処理しなくていい
		if (y < 0 || y >= MAP_CHIP_Y_NUM) continue;

		for (int x = left; x <= right; x++)
		{
			// マップチップからはみ出したら処理しなくていい
			if (x < 0 || x >= MAP_CHIP_X_NUM) continue;

			// マップチップ番号を取得
			MapChipData mapChipData = GetMapChipData(x, y);
			switch (mapChipData.mapChip)
			{
			case NORMAL_BLOCK:
				PlayerHitNormalBlockX(mapChipData);
				break;
			}
		}
	}

	// X軸チェック後にY軸チェックする

	// 左上から3マスずつチェックしていく
	for (int y = top; y <= bottom; y++)
	{
		// マップチップからはみ出したら処理しなくていい
		if (y < 0 || y >= MAP_CHIP_Y_NUM) continue;

		for (int x = left; x <= right; x++)
		{
			// マップチップからはみ出したら処理しなくていい
			if (x < 0 || x >= MAP_CHIP_X_NUM) continue;

			// マップチップ番号を取得
			MapChipData mapChipData = GetMapChipData(x, y);
			switch (mapChipData.mapChip)
			{
			case NORMAL_BLOCK:
				PlayerHitNormalBlockY(mapChipData);
				break;
			}
		}
		
	}
	
}

bool CheckMapPlayerGoal()
{
	PlayerData player = GetPlayer();

	// プレイヤーの当たり判定を計算
	float playerX = player.isTurn ?
		player.posX + PLAYER_WIDTH - player.boxCollision.posX - player.boxCollision.width :
		player.posX + player.boxCollision.posX;

	float playerY = player.posY + player.boxCollision.posY;
	float playerW = player.boxCollision.width;
	float playerH = player.boxCollision.height;

	// 出口のマスを探す
	for (int y = 0; y < MAP_CHIP_Y_NUM; y++)
	{
		for (int x = 0; x < MAP_CHIP_X_NUM; x++)
		{
			MapChipData mapChipData = GetMapChipData(x, y);

			if (mapChipData.mapChip != MAP_CHIP_GOAL) continue;

			// 出口の座標
			float goalX = x * MAP_CHIP_WIDTH;
			float goalY = y * MAP_CHIP_HEIGHT;

			// プレイヤーと出口が重なっているか
			if (CheckSquareSquare(
				playerX, playerY, playerW, playerH,
				goalX, goalY, MAP_CHIP_WIDTH, MAP_CHIP_HEIGHT))
			{
				return true;
			}
		}
	}

	return false;
}

void CheckMapBulletCollision()
{
	StraightBulletData* bullets = GetStraightBullet();
	EnemyData* enemies = Getippans();
	PlayerData player = GetPlayer();

	for (int i = 0; i < STRAIGHT_BULLET_MAX; i++)
	{
		StraightBulletData* bullet = &bullets[i];

		if (!bullet->active) continue;

		// マップとの当たり判定
		int mapX = (int)(bullet->posX / MAP_CHIP_WIDTH);
		int mapY = (int)(bullet->posY / MAP_CHIP_HEIGHT);

		if (mapX < 0 || mapX >= MAP_CHIP_X_NUM ||
			mapY < 0 || mapY >= MAP_CHIP_Y_NUM)
		{
			bullet->active = false;
			continue;
		}

		MapChipData mapChipData = GetMapChipData(mapX, mapY);

		if (mapChipData.mapChip == NORMAL_BLOCK)
		{
			bullet->active = false;
			continue;
		}

		// 弾の矩形
		float bulletLeft = bullet->posX;
		float bulletTop = bullet->posY;
		float bulletRight = bullet->posX + bullet->width;
		float bulletBottom = bullet->posY + bullet->height;

		// --------------------------------
		// プレイヤーが撃った弾 → 敵との判定
		// --------------------------------
		if (bullet->tag == BULLET_COLLISION_TAG_PLAYER)
		{
			for (int j = 0; j < ENEMY_MAX; j++)
			{
				EnemyData* enemy = &enemies[j];

				if (!enemy->active) continue;

				float enemyLeft = enemy->pos.x;
				float enemyTop = enemy->pos.y + 25.0f;
				float enemyRight = enemyLeft + 66.0f;
				float enemyBottom = enemyTop + 77.0f;

				if (bulletRight > enemyLeft &&
					bulletLeft < enemyRight &&
					bulletBottom > enemyTop &&
					bulletTop < enemyBottom)
				{
					enemy->hp -= 5;
					PlaySE(SE_PLAYRE_SHOT_HIT);
					// 敵の点滅を開始
					enemy->damageTimer = 30;
					enemy->isDamageVisible = false;

					// 弾を消す
					StraightBulletHitEnemy(i);

					if (enemy->hp <= 0)
					{
						enemy->active = false;
					}

					break;
				}
			}
		}
		// --------------------------------
		// 敵が撃った弾 → プレイヤーとの判定
		// --------------------------------
		else if (bullet->tag == BULLET_COLLISION_TAG_ENEMY)
		{
			// プレイヤーの矩形
			float playerLeft = player.posX + player.boxCollision.posX;
			float playerTop = player.posY + player.boxCollision.posY;
			float playerRight =
				playerLeft + player.boxCollision.width;
			float playerBottom =
				playerTop + player.boxCollision.height;

			// 左右反転している場合は当たり判定を補正
			if (player.isTurn)
			{
				playerLeft = player.posX + PLAYER_WIDTH
					- player.boxCollision.posX
					- player.boxCollision.width;

				playerRight = playerLeft + player.boxCollision.width;
			}

			if (bulletRight > playerLeft &&
				bulletLeft < playerRight &&
				bulletBottom > playerTop &&
				bulletTop < playerBottom)
			{
				// プレイヤーに10ダメージ
				DamagePlayer(3);

				// 命中した敵弾を消す
				StraightBulletHitPlayer(i);
			}
		}
	}
}
