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

// ��Ч��Ŀ
#define NUM_SOUND_EFFECTS	5

// ��Ч����
enum EFFECT
{
	EFFECT_FIRE,
	EFFECT_EXPLODE,
	EFFECT_HIT,
	EFFECT_PICK,
	EFFECT_LIFE
};

// ������Ч
void PlaySoundEffect(EFFECT ect);

// ��������
#define ROUND(x) x > 0 ? (int)(x+0.5f) : (int)(x-0.5f)

// ��Ϸ״̬ö��
enum GGameState
{
	GM_SPLASH,			// ������Ϸ
	GM_ACTIVE,			// ��Ϸ������
	GM_WIN,				// ��Ϸʤ��
	GM_OVER,			// ��Ϸʧ��
};

// ̹����Ϸ��
class GTankGame
{
private:
	HINSTANCE		mHInst;						// Ӧ�ó�����
	HWND			mHWnd;						// ���ھ��
	bool			mIsActive;					// �Ƿ�Ϊ����״̬
	GGameState		mGameState;					// ��Ϸ״̬
	bool			mIsSingle;					// �Ƿ�Ϊ������Ϸ

	DWORD			mLastTime;					// ��һ֡ʱ��
	DWORD			mBornEmyTm;					// �з�̹�˲���ʱ��

	HDC				mMemDC;						// �����ͼ�豸���
	HBITMAP			mMemBmp;					// ����ͼ����
	HDC				mDC;						// ���ڻ�ͼ�豸���

	GBitmap			mBitmapList[NUM_BITMAPS];	// ͼ����Դ�б�
	int				mLevel;						// �ؿ��ȼ�
	GGameMap		mGameMap;					// ��Ϸ��ͼ����

	UINT			mKey;						// �û�����(�ڷ���Ϸ״̬ʱʹ��)
	UINT			mPlayerInput[2];			// �û�����
	GPlayerTank		mPlayer[2];					// ���̹��
	GExplode		mExplode[NUM_EXPLODES];		// ��ըЧ������
	GBonus			mBonus;						// ����
	GEnemyTank		mEnemy[NUM_ENEMYS];			// �з�̹��
	int				mMaxEnemys;					// ͬʱ������ڵ����з�̹����Ŀ
	int				mNumEnemys;					// ��ǰ�ؿ����з�̹����Ŀ
	int				mEnemysLeft;				// ʣ�����
	bool			mIsEnemyLocked;				// �з�̹���Ƿ�����
	DWORD			mLockTime;					// ������ʼʱ��

protected:
	// ������Դ
	bool loadResource();

	// �������̹�˲���
	void resetPlayer(int iplayer);

	// ��ʼ���ؿ�
	bool initLevel();

	// ���»���һ֡
	void updateFrame();

	// ��Ϸ�߼�����
	bool gameProcess();

	// �˵��߼�����
	void menuProcess();

	// ������Ϸ����
	void drawGame(HDC dc);

	// �������˵�
	void drawMainMenu(HDC dc);

	// ������Ϸ��������
	void drawGameOver(HDC dc);

	// �ھ���sprite����λ���ϲ�����ըЧ��
	void explode(const GSprite &sprite, bool isExplode=false);

	// ���̹�˳Ե�����
	void eatBonus(GPlayerTank &player);

	// �����з�̹��
	bool bornEnemy();

	// ����������Ʒ
	void bornBonus();

	// ���̹�˱�����
	void playerBeenShooted(int iplayer);

public:
	GTankGame();
	virtual ~GTankGame();

	// ��ʼ����Ϸ
	bool init(HINSTANCE hInst, HWND hWnd);

	// ������Ϸ
	int run();

	// ������̰���
	void onKeyDown(UINT key);

	// ��������ɿ�
	void onKeyUp(UINT key);
};

#endif