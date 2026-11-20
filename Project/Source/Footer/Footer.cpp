#include "Footer.h"
#include "DxLib.h"
#include "../GameSetting/GameSetting.h"
#include "../Player/Player.h"
// フッター画像
int g_FooterHandle = 0;

void InitFooter()
{
    g_FooterHandle = 0;
}

void LoadFooter()
{
    g_FooterHandle = LoadGraph("Data/Player/hutta.png");
}

void DrawFooter()
{
    if (g_FooterHandle == 0) return;

    // フッター画像
    DrawGraph(0, 900, g_FooterHandle, TRUE);

    // プレイヤー情報を取得
    PlayerData player = GetPlayer();

    // ゲージの最大幅
    int gaugeWidth = 450;
    int gaugeHeight = 18;

    // HPゲージ
    int hp = player.hp;
    if (hp < 0) hp = 0;
    if (hp > 100) hp = 100;

    DrawBox(60, 1030, 60 + gaugeWidth, 1030 + gaugeHeight,
        GetColor(40, 40, 40), TRUE);

    DrawBox(60, 1030, 60 + gaugeWidth * hp / 100,
        1030 + gaugeHeight, GetColor(40, 220, 80), TRUE);

    // ENERGYゲージ
    int energy = player.energy;
    if (energy < 0) energy = 0;
    if (energy > 100) energy = 100;

    DrawBox(710, 1030, 710 + gaugeWidth, 1030 + gaugeHeight,
        GetColor(40, 40, 40), TRUE);

    DrawBox(710, 1030, 710 + gaugeWidth * energy / 100,
        1030 + gaugeHeight, GetColor(30, 170, 255), TRUE);
}

void FinFooter()
{
    if (g_FooterHandle != 0)
    {
        DeleteGraph(g_FooterHandle);
        g_FooterHandle = 0;
    }
}