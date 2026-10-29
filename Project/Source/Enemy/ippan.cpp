#include "ippan.h"
#include "DxLib.h"
#include "../Camera/Camera.h"
#include"../Player/Player.h"
EnemyData g_Enemies[ENEMY_MAX];
int g_EnemyImage = -1;
#define IPPAN_STOP_FRAME_NUM 4
#define IPPAN_MOVE_FRAME_NUM 5
#define IPPAN_CHASE_DISTANCE 300.0f
#define IPPAN_MOVE_SPEED 1.5f
int g_IppanStopImages[IPPAN_STOP_FRAME_NUM];
int g_IppanMoveImages[IPPAN_MOVE_FRAME_NUM];
struct EnemyAnimationParam
{
    int interval;
    int frameNum;
    int width;
    int height;
};
void Initippan()
{
    for (int i = 0; i < ENEMY_MAX; i++)
    {
        g_Enemies[i].active = false;
        g_Enemies[i].handle = -1;
        g_Enemies[i].pos = VGet(0.0f, 0.0f, 0.0f);
        g_Enemies[i].hp = 0;
        g_Enemies[i].enemyAnim = IPPAN_ANIM_STOP;
        g_Enemies[i].animFrame = 0;
        g_Enemies[i].animTimer = 0;
        g_Enemies[i].prevPos = VGet(0.0f, 0.0f, 0.0f);
        g_Enemies[i].moveX = 0.0f;
        g_Enemies[i].moveY = 0.0f;
    }
}

EnemyData* Getippans()
{
    return g_Enemies;
}
EnemyData* Createippan(VECTOR pos)
{
    for (int i = 0; i < ENEMY_MAX; i++)
    {
        if (g_Enemies[i].active) continue;

        g_Enemies[i].active = true;
        g_Enemies[i].handle = g_EnemyImage;
        g_Enemies[i].pos = pos;
        g_Enemies[i].hp = 100;
        g_Enemies[i].enemyAnim = IPPAN_ANIM_STOP;
        g_Enemies[i].animFrame = 0;
        g_Enemies[i].animTimer = 0;
        g_Enemies[i].prevPos = pos;
        g_Enemies[i].moveX = 0.0f;
        g_Enemies[i].moveY = 0.0f;
        return &g_Enemies[i];
    }

    return nullptr;

}
void Loadippan()
{
    LoadDivGraph(
        "Data/Enemy/stop.png",
        IPPAN_STOP_FRAME_NUM,
        IPPAN_STOP_FRAME_NUM,
        1,
        65,
        77,
        g_IppanStopImages
    );

    LoadDivGraph(
        "Data/Enemy/move.png",
        IPPAN_MOVE_FRAME_NUM,
        IPPAN_MOVE_FRAME_NUM, 1,
        66, 77,
        g_IppanMoveImages
    
    );

  
}
void Stepippan()
{
    for (int i = 0; i < ENEMY_MAX; i++)
    {
        if (!g_Enemies[i].active) continue;
        // プレイヤーの位置を取得
       
        EnemyData* enemy = &g_Enemies[i];
        PlayerData player = GetPlayer();

        // プレイヤーとの横方向の距離
        float distanceX = player.posX - enemy->pos.x;

        // 近くにいるときだけ追いかける
        if (distanceX < IPPAN_CHASE_DISTANCE &&
            distanceX > -IPPAN_CHASE_DISTANCE)
        {
            if (distanceX > 0.0f)
            {
                enemy->pos.x += IPPAN_MOVE_SPEED;
            }
            else if (distanceX < 0.0f)
            {
                enemy->pos.x -= IPPAN_MOVE_SPEED;
            }
        }
        // 座標が変わったか確認
        bool isMoving =
            enemy->pos.x != enemy->prevPos.x ||
            enemy->pos.y != enemy->prevPos.y;

        // アニメーションを切り替える
        EnemyAnimationType nextAnim =
            isMoving ? IPPAN_ANIM_MOVE : IPPAN_ANIM_STOP;

        if (enemy->enemyAnim != nextAnim)
        {
            enemy->enemyAnim = nextAnim;
            enemy->animFrame = 0;
            enemy->animTimer = 0;
        }

        // コマ切り替え
        enemy->animTimer++;

        if (enemy->animTimer >= 8)
        {
            enemy->animTimer = 0;
            enemy->animFrame++;

            int frameNum =
                enemy->enemyAnim == IPPAN_ANIM_STOP
                ? IPPAN_STOP_FRAME_NUM
                : IPPAN_MOVE_FRAME_NUM;

            if (enemy->animFrame >= frameNum)
            {
                enemy->animFrame = 0;
            }
        }

        // 次回比較用に座標を保存
        enemy->prevPos = enemy->pos;
    }
}
void Drawippan()
{
    CameraData camera = GetCamera();
    PlayerData player = GetPlayer();
    for (int i = 0; i < ENEMY_MAX; i++)
    {
        if (!g_Enemies[i].active) continue;

        EnemyData* enemy = &g_Enemies[i];

        int imageHandle;
       
        if (enemy->enemyAnim == IPPAN_ANIM_STOP)
        {
            imageHandle =
                g_IppanStopImages[enemy->animFrame];
        }
        else
        {
            imageHandle =
                g_IppanMoveImages[enemy->animFrame];
        }
        
        int drawX = (int)(enemy->pos.x - camera.posX);
        int drawY = (int)(enemy->pos.y - camera.posY + 25);

        // プレイヤーが敵より左にいるか判定
        bool faceLeft = player.posX < enemy->pos.x;

        if (faceLeft)
        {
            DrawTurnGraph(drawX, drawY, imageHandle, TRUE);
        }
        else
        {
            DrawGraph(drawX, drawY, imageHandle, TRUE);
        }
    }
}

void Finippan()
{
}
