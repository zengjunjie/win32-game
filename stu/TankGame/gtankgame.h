#ifndef GTANK_GAME_H
#define GTANK_GAME_H

#include <windows.h>
#include "gbitmap.h"
#include "gmap.h"
#include "gsprite.h"

#define	MAX_X			416
#define MAX_Y			416
#define SCREEN_W		640
#define SCREEN_H		480
#define OFFSETX			100
#define OFFSETY			48
#define BULLET_W		8
#define BULLET_H		8
#define PLAYER1_STARTX	130
#define PLAYER1_STARTY	386
#define PLAYER2_STARTX	258
#define PLAYER2_STARTY	386

#define KEY_UP				0x0001
#define	KEY_RIGHT			0x0002
#define KEY_DOWN			0x0004
#define KEY_LEFT			0x0008
#define KEY_DIRECTION		0x000f
#define KEY_FIRE			0x0010

#define PLAYER1_UP_KEY			'W'
#define PLAYER1_DOWN_KEY		'S'
#define PLAYER1_LEFT_KEY		'A'
#define PLAYER1_RIGHT_KEY		'D'
#define PLAYER1_FIRE_KEY		VK_SPACE

#define PLAYER2_UP_KEY			VK_UP
#define PLAYER2_DOWN_KEY		VK_DOWN
#define PLAYER2_LEFT_KEY		VK_LEFT
#define PLAYER2_RIGHT_KEY		VK_RIGHT
#define PLAYER2_FIRE_KEY		VK_RETURN

#define NUM_BITMAPS			15
#define NUM_EXPLODES		20
#define NUM_ENEMYS			6

// 音效数目
#define NUM_SOUND_EFFECTS	5

// 音效类型
enum EFFECT
{
	EFFECT_FIRE,
	EFFECT_EXPLODE,
	EFFECT_HIT,
	EFFECT_PICK,
	EFFECT_LIFE
};

// 播放音效
void PlaySoundEffect(EFFECT ect);

// 四舍五入
#define ROUND(x) x > 0 ? (int)(x+0.5f) : (int)(x-0.5f)

// 游戏状态枚举
enum GGameState
{
	GM_SPLASH,			// 进入游戏
	GM_ACTIVE,			// 游戏进行中
	GM_WIN,				// 游戏胜利
	GM_OVER,			// 游戏失败
};

// 坦克游戏类
class GTankGame
{
private:
	HINSTANCE		mHInst;						// 应用程序句柄
	HWND			mHWnd;						// 窗口句柄
	bool			mIsActive;					// 是否为激活状态
	GGameState		mGameState;					// 游戏状态
	bool			mIsSingle;					// 是否为单人游戏

	DWORD			mLastTime;					// 上一帧时间
	DWORD			mBornEmyTm;					// 敌方坦克产生时间

	HDC				mMemDC;						// 缓存绘图设备句柄
	HBITMAP			mMemBmp;					// 缓存图像句柄
	HDC				mDC;						// 窗口绘图设备句柄

	GBitmap			mBitmapList[NUM_BITMAPS];	// 图像资源列表
	int				mLevel;						// 关卡等级
	GGameMap		mGameMap;					// 游戏地图对象

	UINT			mKey;						// 用户按键(在非游戏状态时使用)
	UINT			mPlayerInput[2];			// 用户输入
	GPlayerTank		mPlayer[2];					// 玩家坦克
	GExplode		mExplode[NUM_EXPLODES];		// 爆炸效果对象
	GBonus			mBonus;						// 奖励
	GEnemyTank		mEnemy[NUM_ENEMYS];			// 敌方坦克
	int				mMaxEnemys;					// 同时允许存在的最大敌方坦克数目
	int				mNumEnemys;					// 当前关卡最大敌方坦克数目
	int				mEnemysLeft;				// 剩余敌人
	bool			mIsEnemyLocked;				// 敌方坦克是否被锁定
	DWORD			mLockTime;					// 锁定开始时间

protected:
	// 载入资源
	bool loadResource();

	// 重置玩家坦克参数
	void resetPlayer(int iplayer);

	// 初始化关卡
	bool initLevel();

	// 更新绘制一帧
	void updateFrame();

	// 游戏逻辑处理
	bool gameProcess();

	// 菜单逻辑处理
	void menuProcess();

	// 绘制游戏场景
	void drawGame(HDC dc);

	// 绘制主菜单
	void drawMainMenu(HDC dc);

	// 绘制游戏结束画面
	void drawGameOver(HDC dc);

	// 在精灵sprite所在位置上产生爆炸效果
	void explode(const GSprite &sprite, bool isExplode=false);

	// 玩家坦克吃到奖励
	void eatBonus(GPlayerTank &player);

	// 产生敌方坦克
	bool bornEnemy();

	// 产生奖励物品
	void bornBonus();

	// 玩家坦克被击中
	void playerBeenShooted(int iplayer);

public:
	GTankGame();
	virtual ~GTankGame();

	// 初始化游戏
	bool init(HINSTANCE hInst, HWND hWnd);

	// 运行游戏
	int run();

	// 处理键盘按下
	void onKeyDown(UINT key);

	// 处理键盘松开
	void onKeyUp(UINT key);
};

#endif