#include "DxLib.h"
#include "Player.h"
#include "../Input/Input.h"
#include "../Collision/Collision.h"
#include "../Camera/Camera.h"
#include "../Bullet/BulletManager.h"
#include "../Bullet/StraightBullet.h"
#include"../Sound/SoundManager.h"
#include "../Map/MapManager.h"
// アニメーション用パラメータ
struct PlayerAnimationParam
{
	int interval;
	int frameNum;
	int width;
	int height;
};
const PlayerAnimationParam PLAYER_ANIM_PARAM[PLAYER_ANIM_MAX] =
{
	{ 8, 4, 90, 90 }, // IDLE
	{ 5, 4, 78, 66 }, // RUN
	{ 8, 1, 78, 90 }, // JUMP
	{ 8, 4, 80, 90 }, // FALL
	{ 2, 4, 85, 90 }, // TACKLE
	{ 8, 1, 78, 66 }, // LANDING
	{ 8, 1, 78, 66 }, // ATTACK
	{ 8, 2, 74, 72 }, // AIMJUMP
	
};

// プレイヤー設定関連
#define PLAYER_DEFAULT_POS_X (100.0f)
#define PLAYER_DEFAULT_POS_Y (600.0f)
#define PLAYER_MOVE_SPEED (5.0f)
int g_GlideSETimer = 0;
// ジャンプ力
#define PLAYER_JUMP_POWER (12.0f)
// 重力
#define PLAYER_GRAVITY (0.4f)
// 連射の間隔
#define PLAYER_SHOT_INTERVAL (15)

// 連射用タイマー
int g_PlayerShotTimer = 0;
// 射撃角度（0：横、1：斜め上）
int g_ShotAngle = 0;
// マップ衝突判定用のプレイヤーサイズ補正
#define PLAYER_MAP_COLLISION_OFFSET (0.1f)
// 矩形判定の位置補正
#define PLAYER_BOX_COLLISION_OFFSET_X (16)
#define PLAYER_BOX_COLLISION_OFFSET_Y (10)
// ダッシュ状態
bool g_IsDashing = false;
// 矩形判定のサイズ
#define PLAYER_BOX_COLLISION_WIDTH (44)
#define PLAYER_BOX_COLLISION_HEIGHT (60)
// 斜め上向き画像
int g_PlayerAimUpHandle = -1;
int g_PlayerAimUpLeftHandle = -1;
PlayerData g_PlayerData = { 0 };
PlayerData g_PrevPlayerData = { 0 };
// タックル状態
bool g_IsTackling = false;

// タックルの残り時間
int g_TackleTimer = 0;

// タックル設定
#define PLAYER_TACKLE_SPEED (8.0f)
#define PLAYER_TACKLE_TIME (18)
// このCPPでのみ使用する関数の宣言
void StartPlayerAnimation(PlayerAnimationType anim);	// アニメーション再生
void UpdatePlayerAnimation();							// アニメーション更新
void CalcBoxCollision(PlayerData player, float& x, float& y, float& w, float& h);

void InitPlayer()
{
	g_PlayerData.posX = GetPlayerSpawnX();
	g_PlayerData.posY = GetPlayerSpawnY();
	g_PlayerData.moveX = 0.0f;
	g_PlayerData.moveY = 0.0f;
	g_PlayerData.active = false;
	g_PlayerData.playAnim = PLAYER_ANIM_NONE;
	g_PlayerData.hp = 100;
	for (int i = 0; i < PLAYER_ANIM_MAX; i++)
	{
		InitAnimation(&g_PlayerData.animation[i]);
	}

	// 構造体はmemsetで0初期化できる
	memset(&g_PlayerData.boxCollision, 0, sizeof(g_PlayerData.boxCollision));
}

void LoadPlayer()
{
	g_PlayerData.animation[PLAYER_ANIM_IDLE].handle = LoadGraph("Data/Player/stop6.png");
	g_PlayerData.animation[PLAYER_ANIM_RUN].handle = LoadGraph("Data/Player/run2.png");
	g_PlayerData.animation[PLAYER_ANIM_JUMP].handle = LoadGraph("Data/Player/PlayerJump.png");
	g_PlayerData.animation[PLAYER_ANIM_FALL].handle = LoadGraph("Data/Player/Playerfall.png");
	g_PlayerData.animation[PLAYER_ANIM_ATTACK].handle = LoadGraph("Data/Player/attack.png");
	g_PlayerData.animation[PLAYER_ANIM_LANDING].handle = LoadGraph("Data/Player/PlayerLanding.png");
	g_PlayerData.animation[PLAYER_ANIM_AIMJUMP].handle = LoadGraph("Data/Player/aimjump.png");
	g_PlayerAimUpHandle = LoadGraph("Data/Player/aimup.png");
	g_PlayerAimUpLeftHandle = LoadGraph("Data/Player/aimupleft.png");
	g_PlayerData.animation[PLAYER_ANIM_TACKLE].handle =
		LoadGraph("Data/Player/tackle.png");
}

void StartPlayer()
{
	// 生存フラグを立てる
	g_PlayerData.active = true;

	// 矩形判定設定
	g_PlayerData.boxCollision.posX = PLAYER_BOX_COLLISION_OFFSET_X;
	g_PlayerData.boxCollision.posY = PLAYER_BOX_COLLISION_OFFSET_Y;
	g_PlayerData.boxCollision.width = PLAYER_BOX_COLLISION_WIDTH;
	g_PlayerData.boxCollision.height = PLAYER_BOX_COLLISION_HEIGHT;

	// 待機アニメーションから開始する
	StartPlayerAnimation(PLAYER_ANIM_IDLE);
}

void StepPlayer()
{
	

	// 死んでいたら何もしない
	if (!g_PlayerData.active)
	{
		return;
	}
	
	// 連射用タイマーを進める
	if (g_PlayerShotTimer < PLAYER_SHOT_INTERVAL)
	{
		g_PlayerShotTimer++;
	}
	// 前回のデータとして記憶	
	g_PrevPlayerData = g_PlayerData;

	// X移動量は毎回リセットする
	g_PlayerData.moveX = 0.0f;

	// Xキーを押した瞬間にタックル開始
	if (!g_IsTackling &&
		IsTriggerKey(KEY_X) &&
		!g_PlayerData.isAir)
	{
		PlaySE(SE_PLAYER_TACKLE);
		g_IsTackling = true;
		g_TackleTimer = PLAYER_TACKLE_TIME;

		StartPlayerAnimation(PLAYER_ANIM_TACKLE);
	}

	// タックル中
	if (g_IsTackling)
	{
		// 向いている方向へ突進
		if (g_PlayerData.isTurn)
		{
			g_PlayerData.moveX = -PLAYER_TACKLE_SPEED;
		}
		else
		{
			g_PlayerData.moveX = PLAYER_TACKLE_SPEED;
		}

		// 残り時間を減らす
		g_TackleTimer--;

		// タックル終了
		if (g_TackleTimer <= 0)
		{
			g_IsTackling = false;
			g_PlayerData.moveX = 0.0f;
		}

		return;
	}
	// Y移動量に重力を反映
	// 空中でAキーを押している間は滑空
	// 空中でAキーを押している間は滑空
	// 空中でAキーを押している間は滑空
	if (g_PlayerData.isAir && IsInputKey(KEY_A))
	{
		// 効果音タイマー
		if (g_GlideSETimer > 0)
		{
			g_GlideSETimer--;
		}

		// 3秒経過したら効果音を鳴らす
		if (g_GlideSETimer <= 0)
		{
			PlaySE(SE_PLAYER_GURAIDO);
			g_GlideSETimer = 180;
		}

		g_PlayerData.moveY += 0.1f;

		// 落下速度に上限をつける
		if (g_PlayerData.moveY > 2.0f)
		{
			g_PlayerData.moveY = 2.0f;
		}
	}
	else
	{
		// 滑空をやめたらタイマーをリセット
		g_GlideSETimer = 0;

		// 通常の重力
		g_PlayerData.moveY += PLAYER_GRAVITY;
	}
	// L3（KEY_SHIFT）を押した瞬間にダッシュ状態を切り替える
	if (IsTriggerKey(KEY_SHIFT))
	{
		g_IsDashing = !g_IsDashing;
	}
	// 左入力
	if (!IsInputKey(KEY_UP) && IsInputKey(KEY_LEFT))
	{
		
		if (g_IsDashing)
		{
			PlaySE(SE_PLAYER_WALK);
			g_PlayerData.moveX = -8;
			g_PlayerData.isTurn = true;

	    }
		else
		{
			PlaySE(SE_PLAYER_WALK);
			g_PlayerData.moveX = -PLAYER_MOVE_SPEED;
			g_PlayerData.isTurn = true;
		}
	}
	// 右入力
	else if (!IsInputKey(KEY_UP) && IsInputKey(KEY_RIGHT))
	{
		
		if (g_IsDashing)
		{
			PlaySE(SE_PLAYER_WALK);
			g_PlayerData.moveX = 8;
			g_PlayerData.isTurn = false;
		}
		else
		{
			PlaySE(SE_PLAYER_WALK);
			g_PlayerData.moveX = PLAYER_MOVE_SPEED;
			g_PlayerData.isTurn = false;
		}
	}

	// 上キーを押している間は斜め上
	if (IsInputKey(KEY_UP))
	{
		g_ShotAngle = 1;
	}
	else
	{
		g_ShotAngle = 0;
	}
// Zキーを押していて、連射間隔に達したら発射
	if (IsInputKey(KEY_Z) &&
		g_PlayerShotTimer >= PLAYER_SHOT_INTERVAL)
	{
		if (IsInputKey(KEY_Z) && IsInputKey(KEY_A))
		{

		}
		else
		{
			PlaySE(SE_PLAYRE_SHOT);
			// 発射データを作成
			FireBulletData fireData = { 0 };

			// 発射位置
			fireData.posX = g_PlayerData.posX + PLAYER_WIDTH / 2.0f;
			fireData.posY = g_PlayerData.posY + 20.0f;

			if (g_ShotAngle == 0)
			{
				// 横撃ち
				fireData.moveX = g_PlayerData.isTurn ? -12.0f : 12.0f;
				fireData.moveY = 0.0f;
			}
			else
			{
				// 斜め上撃ち
				fireData.moveX = g_PlayerData.isTurn ? -10.0f : 10.0f;
				fireData.moveY = -6.0f;
			}
			// 弾の寿命
			fireData.life = 50;

			// プレイヤーの通常弾を発射
			FireStraightBullet(
				STRAIGHT_BULLET_TYPE_PLAYER_NORMAL,
				fireData,
				BULLET_COLLISION_TAG_PLAYER

			);

			// タイマーをリセット
			g_PlayerShotTimer = 0;
		}
	}
	if (IsInputKey(KEY_UP) && g_PlayerData.isAir)
	{
		StartPlayerAnimation(PLAYER_ANIM_AIMJUMP);
	}// スペースキーを押して、地面にいる場合だけジャンプ
	// SHIFT + SPACE：強いジャンプ
	if (IsTriggerKey(KEY_SPACE) && IsInputKey(KEY_RSHIFT)
		&& !g_PlayerData.isAir)
	{
		PlaySE(SE_PLAYER_JUMP);
		g_PlayerData.moveY = -PLAYER_JUMP_POWER * 1.5f;
		g_PlayerData.isAir = true;
	}
	// SPACE：通常ジャンプ
	else if (IsTriggerKey(KEY_SPACE) && !g_PlayerData.isAir)
	{
		PlaySE(SE_PLAYER_JUMP);
		g_PlayerData.moveY = -PLAYER_JUMP_POWER;
		g_PlayerData.isAir = true;
	}
	

}

void UpdatePlayer()
{
	// 死んでいたら処理しない
	if (!g_PlayerData.active) return;

	// 上昇しているもしくは落ちている場合は空中フラグを立てる
	if (g_PlayerData.moveY < 0.0f || g_PlayerData.moveY > PLAYER_GRAVITY)
	{
		g_PlayerData.isAir = true;
	}

	// 移動処理
	g_PlayerData.posX += g_PlayerData.moveX;
	g_PlayerData.posY += g_PlayerData.moveY;

	UpdatePlayerAnimation();
}

void DrawPlayer()
{
	if (!g_PlayerData.active) return;

	
	
	CameraData camera = GetCamera();

	PlayerAnimationType animType = g_PlayerData.playAnim;
	AnimationData* animData = &g_PlayerData.animation[animType];

	float drawX = g_PlayerData.posX;

	if (g_PlayerData.isTurn)
	{
		drawX -= animData->frameWidth - PLAYER_WIDTH;
	}

	// ① タックル中
	if (g_IsTackling)
	{
		AnimationData* tackleAnim =
			&g_PlayerData.animation[PLAYER_ANIM_TACKLE];
		
		float tackleDrawX = g_PlayerData.posX;

		if (g_PlayerData.isTurn)
		{
			tackleDrawX -= tackleAnim->frameWidth - PLAYER_WIDTH;
		}

		DrawAnimation(
			tackleAnim,
			tackleDrawX - camera.posX,
			g_PlayerData.posY - camera.posY + 5,
			g_PlayerData.isTurn
		);
	}
	// ② 空中にいるときはジャンプ・落下アニメーション
	else if (g_PlayerData.isAir)
	{
		DrawAnimation(
			animData,
			drawX - camera.posX,
			g_PlayerData.posY - camera.posY + 5,
			g_PlayerData.isTurn
		);
	}
	// ③ スペースキーを押しているときの照準ジャンプ
	else if (IsInputKey(KEY_SPACE) && g_PlayerData.isAir)
	{
		AnimationData* aimJumpAnim =
			&g_PlayerData.animation[PLAYER_ANIM_AIMJUMP];

		float aimJumpDrawX = g_PlayerData.posX;

		if (g_PlayerData.isTurn)
		{
			aimJumpDrawX -=
				aimJumpAnim->frameWidth - PLAYER_WIDTH;
		}

		DrawAnimation(
			aimJumpAnim,
			aimJumpDrawX - camera.posX,
			g_PlayerData.posY - camera.posY + 5,
			g_PlayerData.isTurn
		);
	}
	// ④ 上方向を向いているとき
	else if (IsInputKey(KEY_UP))
	{
		int aimHandle;

		if (g_PlayerData.isTurn)
		{
			aimHandle = g_PlayerAimUpLeftHandle;
		}
		else
		{
			aimHandle = g_PlayerAimUpHandle;
		}

		int imageWidth, imageHeight;
		GetGraphSize(aimHandle, &imageWidth, &imageHeight);

		float aimDrawX = g_PlayerData.posX;

		if (g_PlayerData.isTurn)
		{
			aimDrawX -= imageWidth - PLAYER_WIDTH;
		}

		DrawGraph(
			(int)(aimDrawX - camera.posX),
			(int)(g_PlayerData.posY - camera.posY + 5),
			aimHandle,
			TRUE
		);
	}
	// ⑤ 通常アニメーション
	else
	{
		DrawAnimation(
			animData,
			drawX - camera.posX,
			g_PlayerData.posY - camera.posY + 5,
			g_PlayerData.isTurn
		);
	}

	// 当たり判定のデバッグ表示が必要なら、ここに書く
}
void FinPlayer()
{
	for (int i = 0; i < PLAYER_ANIM_MAX; i++)
	{
		DeleteGraph(g_PlayerData.animation[i].handle);
		
	}
	DeleteGraph(g_PlayerAimUpHandle);
	DeleteGraph(g_PlayerAimUpLeftHandle);
}

PlayerData GetPlayer()
{
	return g_PlayerData;
}

void PlayerHitNormalBlockX(MapChipData mapChipData)
{
	PlayerData player = g_PlayerData;
	BlockData* block = mapChipData.data;
	const float POS_OFFSET = PLAYER_MAP_COLLISION_OFFSET;
	const float SIZE_OFFSET = PLAYER_MAP_COLLISION_OFFSET * 2;

	// ターンフラグは前回のものにしないと反転した分ずれる
	player.isTurn = g_PrevPlayerData.isTurn;

	// Y移動を戻し、横に当たっているかチェック
	player.posX = g_PlayerData.posX;
	player.posY = g_PrevPlayerData.posY;

	// 当たり判定のボックス計算
	float x, y, w, h;
	CalcBoxCollision(player, x, y, w, h);

	if (CheckSquareSquare(x + POS_OFFSET, y + POS_OFFSET, w - SIZE_OFFSET, h - SIZE_OFFSET,
		block->pos.x, block->pos.y, MAP_CHIP_WIDTH, MAP_CHIP_HEIGHT))
	{
		// 左からあたったか
		if (player.moveX > 0.0f)
		{
			// 左に押し出す
			g_PlayerData.posX -= (x + w) - block->pos.x;
		}
		// 右からあたったか
		else if(player.moveX < 0.0f)
		{ 
			// 右に押し出す
			g_PlayerData.posX += (block->pos.x + MAP_CHIP_WIDTH) - x;
		}
	}
}

void PlayerHitNormalBlockY(MapChipData mapChipData)
{
	PlayerData player = g_PlayerData;
	BlockData* block = mapChipData.data;
	const float POS_OFFSET = PLAYER_MAP_COLLISION_OFFSET;
	const float SIZE_OFFSET = PLAYER_MAP_COLLISION_OFFSET * 2;

	// ターンフラグは前回のものにしないと反転した分ずれる
	player.isTurn = g_PrevPlayerData.isTurn;

	// 当たり判定のボックス計算
	float x, y, w, h;
	CalcBoxCollision(player, x, y, w, h);

	// まだ当たっているなら縦に当たっている
	if (CheckSquareSquare(x + POS_OFFSET, y + POS_OFFSET, w - SIZE_OFFSET, h - SIZE_OFFSET,
		block->pos.x, block->pos.y, MAP_CHIP_WIDTH, MAP_CHIP_HEIGHT))
	{
		// Y移動量を0にする
		g_PlayerData.moveY = 0.0f;

		// 上からあたったか
		if (player.moveY > 0.0f)
		{
			// 上に押し出す
			g_PlayerData.posY -= (y + h) - block->pos.y;
			g_PlayerData.isAir = false;
		}
		// 下からあたったか
		else if (player.moveY < 0.0f)
		{
			// 下に押し出す
			g_PlayerData.posY += (block->pos.y + MAP_CHIP_WIDTH) - y;
		}
	}
}

void StartPlayerAnimation(PlayerAnimationType anim)
{
	// 再生中のアニメーションであれば何もしない
	if (anim == g_PlayerData.playAnim) return;

	// 再生中アニメーション設定
	g_PlayerData.playAnim = anim;

	// 再生に必要なデータを取得
	AnimationData* animData = &g_PlayerData.animation[anim];
	PlayerAnimationParam animParam = PLAYER_ANIM_PARAM[anim];

	// アニメーション再生
	StartAnimation(animData, g_PlayerData.posX, g_PlayerData.posY, animParam.interval, animParam.frameNum, animParam.width, animParam.height, true);
}

void UpdatePlayerAnimation()
{
	// 地上にいるか
	if (!g_PlayerData.isAir)
	{// タックル中
		if (g_IsTackling)
		{
			StartPlayerAnimation(PLAYER_ANIM_TACKLE);

			AnimationData* animData =
				&g_PlayerData.animation[PLAYER_ANIM_TACKLE];

			UpdateAnimation(animData);
			return;
		}
		// 横に移動していれば走っている
		if (g_PlayerData.moveX < 0.0f || g_PlayerData.moveX > 0.0f)
		{
			// 走るアニメーション
			StartPlayerAnimation(PLAYER_ANIM_RUN);
		}
		else
		{
			// 移動していなければ待機
			StartPlayerAnimation(PLAYER_ANIM_IDLE);
		}
		
	}
	// 空中にいる
	else
	{
		
		// 上昇していればジャンプアニメーション
		if (g_PlayerData.moveY < 0.0f)
		{
			StartPlayerAnimation(PLAYER_ANIM_JUMP);
		}
		// 下降していれば落下アニメーション
		else if (g_PlayerData.moveY > 0.0f)
		{
			StartPlayerAnimation(PLAYER_ANIM_FALL);
		}
	}

	// アニメーション更新
	PlayerAnimationType animType = g_PlayerData.playAnim;
	AnimationData* animData = &g_PlayerData.animation[animType];
	UpdateAnimation(animData);
}

void CalcBoxCollision(PlayerData player, float& x, float& y, float& w, float& h)
{
	x = player.isTurn ?
		player.posX + PLAYER_WIDTH - player.boxCollision.posX - player.boxCollision.width :
		player.posX + player.boxCollision.posX;
	y = player.posY + player.boxCollision.posY;
	w = player.boxCollision.width;
	h = player.boxCollision.height;
}
void DamagePlayer(int damage)
{
	if (!g_PlayerData.active) return;

	g_PlayerData.hp -= damage;
	PlaySE(SE_PLAYRE_SHOT_HIT);
	if (g_PlayerData.hp <= 0)
	{
		g_PlayerData.hp = 0;
		g_PlayerData.active = false;
	}
}