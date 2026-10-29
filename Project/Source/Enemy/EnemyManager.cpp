#include "EnemyManager.h"
#include "../Map/MapChip.h"
#include "ippan.h"
int g_EnemySpawnTimer = 0;
bool g_EnemySpawned = false;
void InitEnemy()
{
	// エネミー出現タイマーを初期化
	g_EnemySpawnTimer = 0;
    g_EnemySpawned = false;
}

void LoadEnemy()
{
    Loadippan();
}

void StepEnemy()
{
    Stepippan();
}
void UpdateEnemy()
{
	// 各エネミーを更新
}

void DrawEnemy()
{
    DrawFormatString(
        10, 50,
        GetColor(255, 0, 0),
        "DrawEnemy called"
    );

    Drawippan();
}

void FinEnemy()
{
	// 各エネミーを終了
    Finippan;
}

void StepEnemySpawnSystem()
{
    if (g_EnemySpawned) return;
    g_EnemySpawned = true;

    int spawnCount = 0;

    for (int y = 0; y < MAP_CHIP_Y_NUM; y++)
    {
        for (int x = 0; x < MAP_CHIP_X_NUM; x++)
        {
            MapChipData mapChipData = GetMapChipData(x, y);

            if (mapChipData.mapChip != 3) continue;

            spawnCount++;

            VECTOR pos = VGet(
                x * MAP_CHIP_WIDTH,
                y * MAP_CHIP_HEIGHT,
                0.0f
            );

            Createippan(pos);
        }
    }

    printfDx("敵の配置数：%d\n", spawnCount);
}