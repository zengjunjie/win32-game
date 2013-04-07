#ifndef GSPRITE_H
#define GSPRITE_H

#include <windows.h>
#include "gbitmap.h"

// ����ö��
enum GDirection
{
	DIR_UP,
	DIR_RIGHT,
	DIR_DOWN,
	DIR_LEFT
};

// ����ö��
enum GBonusType
{
	BONUS_LIFE,			// ����
	BONUS_CLOCK,		// ����
	BONUS_SHOVEL,		// ����
	BONUS_BOMB,			// ը��
	BONUS_STAR,			// ����
	BONUS_HELMET		// ����
};

// ������
class GSprite  
{
public:
	bool	mIsActive;				// �Ƿ�Ϊ����״̬
	float	mX;						// X����
	float	mY;						// Y����
	int		mWidth;					// ���
	int		mHeight;				// �߶�

public:
	GSprite();

	// ��ʼ��
	virtual void init(int w, int h, bool active)
	{
		mWidth = w;
		mHeight = h; 
		mIsActive = active;
	}

	// ��ײ���
	virtual bool collisionTest(const GSprite& sprite);	

	// ����
	virtual void draw(HDC dc) {};
};

// ������
class GBonus : public GSprite
{
public:
	GBitmap		*mpBmp;			// λͼ��Դ
	GBonusType	mType;			// ��������
	DWORD		mLastTime;		// ��¼����ʱ��
	DWORD		mFlickerTime;	// ��¼��˸ʱ��

public:
	GBonus();

	// ����λͼ��Դ
	void attachBmpRes(GBitmap *pBmp)
	{ mpBmp = pBmp; }

	// ����
	virtual void draw(HDC dc);
};

// ��ըЧ����
class GExplode: public GSprite
{
public:
	GBitmap		*mpBmps[2];		// λͼ��Դ
	bool		mIsExplode;		// �Ƿ������ը
	DWORD		mLastTime;		// ��ըʱ��
public:
	GExplode();

	// ����λͼ��Դ(Сͼ, ��ͼ)
	void attachBmpRes(GBitmap *pBmp1, GBitmap *pBmp2)
	{
		mpBmps[0] = pBmp1;
		mpBmps[1] = pBmp2;
	}

	// ����
	virtual void draw(HDC dc);
};

// ̹������ʱ����˸��
class GBorn : public GSprite
{
public:
	GBitmap		*mpBmp;			// λͼ��Դ
	int			mFrame;			// ����֡��
	DWORD		mLastTime;
	bool		mIsAdvance;

public:
	GBorn();

	// ���ɳ�ʼ״̬
	void born();

	// ����λͼ��Դ
	void attachBmpRes(GBitmap *pBmp)
	{ mpBmp = pBmp; }

	// ����
	virtual void draw(HDC dc);
};

// ���ƶ�������
class GMoveableSprite: public GSprite
{
public:
	GDirection		mDirection;		// �ƶ�����
	float			mSpeed;			// �ƶ��ٶ�

public:
	GMoveableSprite();

	// �ƶ�(����ƶ�����Ч��Χ֮�ⷵ��false)
	virtual bool move(DWORD time);
};

// �ӵ���
class GBullet : public GMoveableSprite
{
public:
	GBitmap		*mpBmp;			// λͼ��Դ

public:
	GBullet();
	
	// ����λͼ��Դ
	void attachBmpRes(GBitmap *pBmp)
	{ mpBmp = pBmp; }

	// ����
	virtual void draw(HDC dc);
};

// ̹�˻���
class GTank : public GMoveableSprite
{
public:
	GBitmap		*mpTankBmp;				// ̹��ͼ��
	GBitmap		*mpShieldBmp;			// ������ͼ��
	GBullet		mBullet[2];				// �ӵ�����
	DWORD		mLastTime;				// ��һ�ο���ʱ��
	int			mFrame;					// ֡
	int			mGrade;					// �ȼ�
	bool		mIsShield;				// �Ƿ��ڱ���״̬
	int			mShieldFrame;			// ����ͼ��֡��
	DWORD		mMaxShieldTime;			// ��󱣻�ʱ��
	DWORD		mShieldTime;			// ��������ʱ��
	DWORD		mFlickerTime;			// ��˸ʱ��
	bool		mIsBorning;				// �Ƿ��ڲ���״̬
	GBorn		mBorn;					// ������˸����

protected:
	// ���Ʊ�����
	void drawShield(HDC dc);

public:
	GTank();

	// ��ʼ��̹�˶���
	virtual void init(int w, int h, bool isActive)
	{
		GMoveableSprite::init(w, h, isActive);
		mBorn.init(32, 32, true);
	}	

	// ����λͼ��Դ
	void attachBmpRes(GBitmap *pTankBmp, GBitmap *pBulletBmp,
					  GBitmap *pShieldBmp, GBitmap *pBornBmp)
	{
		mpTankBmp = pTankBmp;
		mpShieldBmp = pShieldBmp;
		mBullet[0].attachBmpRes(pBulletBmp);
		mBullet[1].attachBmpRes(pBulletBmp);
		mBorn.attachBmpRes(pBornBmp);
	}
	
	// �ı�̹���˶�����
	void changeDirection(GDirection direct);

	// ����̹��
	void shield(DWORD time)
	{
		mIsShield = true;
		mShieldFrame = 1;
		mMaxShieldTime = time;
		mShieldTime = mFlickerTime = timeGetTime();
	}

	// �ƶ�̹��
	virtual bool move(DWORD time)
	{ 
		mFrame = !mFrame;		// �ı�ͼ��֡, �����˶�Ч��
		return GMoveableSprite::move(time); 
	}
};

// ���̹����
class GPlayerTank : public GTank
{
public:
	int			mLife;			// ����ֵ
	int			mScore;			// �÷�
	bool		mIsLocked;		// �Ƿ�����
	bool		mIsShow;		// ��������ʱ��˸
public:
	GPlayerTank();

	// �����û�����(������𷵻�true)
	bool processInput(WORD input, DWORD time);

	// ����
	bool fire();

	// ����
	void draw(HDC dc);

	// ������̹��
	void born();

	// ����
	void lock()
	{ 
		mIsLocked = true;
		mShieldTime = mFlickerTime = timeGetTime();
	}
};

// �з�̹��
class GEnemyTank : public GTank
{
public:
	bool		mIsBonus;		// �Ƿ�Ϊ�����к��������
	bool		mIsShowRed;		// �Ƿ��ں�ɫ��ǿ״̬
	DWORD		mRedTime;		// ��ɫʱ��
	int			mType;			// ̹������
public:
	GEnemyTank();

	// �ı䷽��
	void autoChangeDirection();

	// ����
	bool fire();

	// ����
	void born();

	// ����
	void draw(HDC dc);
};


#endif