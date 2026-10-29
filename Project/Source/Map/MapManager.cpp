#include "MapManager.h"
#include "MapParameter.h"
#include "MapChip.h"
#include "Block.h"
#include "../Player/Player.h"
#include"../Bullet/BulletManager.h"
#include"../Bullet/StraightBullet.h"
// キャラクターの周囲何マスまでチェックするか
#define CHECK_ROUND_NUM (2)

void InitMap()
{
}

void LoadMap()
{
	LoadMapChipData();
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

	for (int i = 0; i < STRAIGHT_BULLET_MAX; i++)
	{
		StraightBulletData* bullet = &bullets[i];

		if (!bullet->active) continue;

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
		}
	}
}
