#include "ippan.h"
#include "DxLib.h"
#include <cmath>
#include "../Camera/Camera.h"
#include"../Player/Player.h"
#include "../Bullet/BulletManager.h"
#include "../Bullet/StraightBullet.h"
#include"../Sound/SoundManager.h"
EnemyData g_Enemies[ENEMY_MAX];
int g_EnemyImage = -1;
#define IPPAN_STOP_FRAME_NUM 1
#define IPPAN_MOVE_FRAME_NUM 3
#define IPPAN_CHASE_DISTANCE 300.0f
#define IPPAN_MOVE_SPEED 1.5f
extern bool g_IsTackling;
int g_IppanStopImages[IPPAN_STOP_FRAME_NUM];
int g_IppanMoveImages[IPPAN_MOVE_FRAME_NUM];
#define IPPAN_STOP_DISTANCE (400.0f) // 攻撃を始める距離
#define IPPAN_SHOT_INTERVAL (60)     // 発射間隔（約1秒）
int g_IppanDieImage = -1;
#define IPPAN_DIE_TIME (60) // 死亡演出の長さ
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
        g_Enemies[i].shotTimer = 0;
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
        g_Enemies[i].isDead = false;
        g_Enemies[i].deathTimer = 0;
        g_Enemies[i].isDeathVisible = true;
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
        g_Enemies[i].hitByTackle = false;
        if (g_Enemies[i].active) continue;
        g_Enemies[i].shotTimer = 0;
        g_Enemies[i].active = true;
        g_Enemies[i].handle = g_EnemyImage;
        g_Enemies[i].pos = pos;
        g_Enemies[i].hp = 10;
        g_Enemies[i].isDead = false;
        g_Enemies[i].deathTimer = 0;
        g_Enemies[i].isDeathVisible = true;
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
    g_IppanDieImage = LoadGraph("Data/Enemy/die2.png");
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
        EnemyData* enemy = &g_Enemies[i];

        // 死亡中
        if (enemy->isDead)
        {
            enemy->deathTimer--;

            // 3フレームごとに点滅
            enemy->isDeathVisible =
                (enemy->deathTimer / 3) % 2 == 0;

            // 演出終了
            if (enemy->deathTimer <= 0)
            {
                enemy->active = false;
                enemy->isDead = false;
            }

            continue;
        }
        if (!g_Enemies[i].active) continue;

        // プレイヤーの位置を取得
       
      
        PlayerData player = GetPlayer();
        // プレイヤーのタックルが敵に当たったか判定
        if (g_IsTackling && !enemy->hitByTackle)
        {
            float distanceX = fabsf(player.posX - enemy->pos.x);
            float distanceY = fabsf(player.posY - enemy->pos.y);

            if (distanceX < 70.0f && distanceY < 70.0f)
            {
                PlaySE(SE_PLAYRE_SHOT_HIT);
                enemy->hp -= 10;
                enemy->hitByTackle = true;

                // HPが0以下なら敵を倒す
                if (enemy->hp <= 0)
                {
                    enemy->hp = 0;
                    enemy->isDead = true;
                    enemy->deathTimer = IPPAN_DIE_TIME;
                    enemy->isDeathVisible = true;
                }
            }
        }

        // タックル終了後、次のタックルで再び当たれるようにする
        if (!g_IsTackling)
        {
            enemy->hitByTackle = false;
        }
        // ダメージ中の点滅処理
        if (enemy->damageTimer > 0)
        {
            enemy->damageTimer--;

            // 3フレームごとに表示・非表示を切り替える
            enemy->isDamageVisible = (enemy->damageTimer / 3) % 2 == 0;
        }
        else
        {
            enemy->isDamageVisible = true;
        }
        // プレイヤーとの横方向の距離
        float distanceX = player.posX - enemy->pos.x;
        float absDistanceX = (float)fabs(distanceX);

        // 一定距離より遠ければ追いかける
        if (absDistanceX > IPPAN_STOP_DISTANCE &&
            absDistanceX < IPPAN_CHASE_DISTANCE)
        {
            if (distanceX > 0.0f)
            {
                enemy->pos.x += IPPAN_MOVE_SPEED;
            }
            else if (distanceX < 0.0f)
            {
                enemy->pos.x -= IPPAN_MOVE_SPEED;
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
            enemy->shotTimer = 0;
        }
        else if (
            absDistanceX <= IPPAN_STOP_DISTANCE &&
            fabsf(player.posY - enemy->pos.y) <= 80.0f
            )
        {
            // プレイヤーが近ければ待機アニメーションにする
            if (enemy->enemyAnim != IPPAN_ANIM_STOP)
            {
                enemy->enemyAnim = IPPAN_ANIM_STOP;
                enemy->animFrame = 0;
                enemy->animTimer = 0;
            }

            // プレイヤーが近ければ停止して攻撃
            enemy->shotTimer++;

            if (enemy->shotTimer >= IPPAN_SHOT_INTERVAL)
            {
                PlaySE(SE_PLAYRE_SHOT);
                enemy->shotTimer = 0;

                FireBulletData fireData = {};

                fireData.posX = enemy->pos.x;
                fireData.posY = enemy->pos.y + 50.0f;

                float dirX = player.posX - fireData.posX;
                float dirY = player.posY - fireData.posY+30;
                float length = sqrtf(dirX * dirX + dirY * dirY);

                if (length > 0.0f)
                {
                    float speed = 5.0f;

                    // プレイヤーが右なら右、左なら左へ撃つ
                    fireData.moveX =
                        (player.posX > enemy->pos.x) ? speed : -speed;

                    fireData.moveY = 0.0f;
                    fireData.life = 120;

                    FireStraightBullet(
                        STRAIGHT_BULLET_TYPE_KAPU,
                        fireData,
                        BULLET_COLLISION_TAG_ENEMY
                    );
                }
            }
        }
        else
        {
            // 追跡範囲外なら待機アニメーションにする
            if (enemy->enemyAnim != IPPAN_ANIM_STOP)
            {
                enemy->enemyAnim = IPPAN_ANIM_STOP;
                enemy->animFrame = 0;
                enemy->animTimer = 0;
            }

            enemy->shotTimer = 0;
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
        int drawX = (int)(enemy->pos.x - camera.posX);
        int drawY = (int)(enemy->pos.y - camera.posY + 25);

        // プレイヤーが敵より左にいるか
        bool faceLeft = player.posX < enemy->pos.x;

        // 死亡中
        if (enemy->isDead)
        {
            if (enemy->isDeathVisible)
            {
                if (faceLeft)
                {
                    DrawGraph(drawX, drawY, g_IppanDieImage, TRUE);
                }
                else
                {
                    DrawTurnGraph(drawX, drawY, g_IppanDieImage, TRUE);
                }
            }

            continue;
        }

        // 通常のダメージ点滅
        if (!enemy->isDamageVisible) continue;

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
        
      
        // プレイヤーが敵より左にいるか判定
       // プレイヤーが敵より左にいるか判定
     

        if (faceLeft)
        {
            DrawGraph(drawX, drawY, imageHandle, TRUE);
        }
        else
        {
            DrawTurnGraph(drawX, drawY, imageHandle, TRUE);
        }
           
    }
}

void Finippan()
{
    DeleteGraph(g_IppanDieImage);

    for (int i = 0; i < IPPAN_STOP_FRAME_NUM; i++)
    {
        DeleteGraph(g_IppanStopImages[i]);
    }

    for (int i = 0; i < IPPAN_MOVE_FRAME_NUM; i++)
    {
        DeleteGraph(g_IppanMoveImages[i]);
    }
}