#include "MapChip.h"
#include "DxLib.h"
#include "../GameSetting/GameSetting.h"
#include "MapParameter.h"
#include "Block.h"
#include "MapManager.h"

MapChipData g_MapChip[MAP_CHIP_Y_NUM][MAP_CHIP_X_NUM] = { 0 };
// マップごとのブロック破壊状態
bool g_DestroyedMapChip[MAP_ID_MAX]
[MAP_CHIP_Y_NUM][MAP_CHIP_X_NUM] = { false };

void LoadMapChipData(const char* fileName)
{
    FILE* fp;

    if (fopen_s(&fp, fileName, "rb") != 0)
    {
        printfDx("マップ読み込み失敗: %s\n", fileName);
        return;
    }

  

    for (int i = 0; i < MAP_CHIP_Y_NUM; i++)
    {
        for (int j = 0; j < MAP_CHIP_X_NUM; j++)
        {
            int map = fgetc(fp);

            // データが不足していた場合
            if (map == EOF)
            {
                fclose(fp);
                return;
            }

            g_MapChip[i][j].mapChip = map;
            // 破壊済みのタックルブロックなら空にする
            MapID mapID = GetCurrentMap();

            if (map == MAP_TACCKLE_BLOCK &&
                g_DestroyedMapChip[mapID][i][j])
            {
                g_MapChip[i][j].mapChip = MAP_CHIP_NONE;
            }
        }
    }

    fclose(fp);
}

void CreateMap()
{
    for (int i = 0; i < MAP_CHIP_Y_NUM; i++)
    {
        for (int j = 0; j < MAP_CHIP_X_NUM; j++)
        {
            int mapChip = g_MapChip[i][j].mapChip;

            // 何もない場所と敵の配置場所はブロックを作らない
            if (mapChip == 0 || mapChip == 3) continue;

            MapChipType type = (MapChipType)mapChip;

            VECTOR pos = VGet(
                j * MAP_CHIP_WIDTH,
                i * MAP_CHIP_HEIGHT,
                0.0f
            );

            g_MapChip[i][j].data = CreateBlock(type, pos);

            if (g_MapChip[i][j].data == nullptr)
            {
                // ブロックを生成できなかった
                return;
            }
        }
    }
}

MapChipData GetMapChipData(int x, int y)
{
	return g_MapChip[y][x];
}
void DestroyMapChip(int x, int y)
{
    // 現在のマップの破壊状態を記録
    MapID mapID = GetCurrentMap();
    g_DestroyedMapChip[mapID][y][x] = true;

    // マップチップを空にする
    g_MapChip[y][x].mapChip = MAP_CHIP_NONE;

    // ブロックを消す
    if (g_MapChip[y][x].data != nullptr)
    {
        g_MapChip[y][x].data->active = false;
        g_MapChip[y][x].data = nullptr;
    }
}
