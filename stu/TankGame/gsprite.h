#ifndef GSPRITE_H
#define GSPRITE_H

#include <windows.h>
#include "gbitmap.h"

// 方向枚举
enum GDirection
{
	DIR_UP,
	DIR_RIGHT,
	DIR_DOWN,
	DIR_LEFT
};

// 奖励枚举
enum GBonusType
{
	BONUS_LIFE,			// 生命
	BONUS_CLOCK,		// 锁定
	BONUS_SHOVEL,		// 铁铲
	BONUS_BOMB,			// 炸弹
	BONUS_STAR,			// 升级
	BONUS_HELMET		// 盔甲
};

// 精灵类
class GSprite  
{
public:
	bool	mIsActive;				// 是否为激活状态
	float	mX;						// X坐标
	float	mY;						// Y坐标
	int		mWidth;					// 宽度
	int		mHeight;				// 高度

public:
	GSprite();

	// 初始化
	virtual void init(int w, int h, bool active)
	{
		mWidth = w;
		mHeight = h; 
		mIsActive = active;
	}

	// 碰撞检测
	virtual bool collisionTest(const GSprite& sprite);	

	// 绘制
	virtual void draw(HDC dc) {};
};

// 奖励类
class GBonus : public GSprite
{
public:
	GBitmap		*mpBmp;			// 位图资源
	GBonusType	mType;			// 奖励类型
	DWORD		mLastTime;		// 记录出现时间
	DWORD		mFlickerTime;	// 记录闪烁时间

public:
	GBonus();

	// 关联位图资源
	void attachBmpRes(GBitmap *pBmp)
	{ mpBmp = pBmp; }

	// 绘制
	virtual void draw(HDC dc);
};

// 爆炸效果类
class GExplode: public GSprite
{
public:
	GBitmap		*mpBmps[2];		// 位图资源
	bool		mIsExplode;		// 是否产生大爆炸
	DWORD		mLastTime;		// 爆炸时间
public:
	GExplode();

	// 关联位图资源(小图, 大图)
	void attachBmpRes(GBitmap *pBmp1, GBitmap *pBmp2)
	{
		mpBmps[0] = pBmp1;
		mpBmps[1] = pBmp2;
	}

	// 绘制
	virtual void draw(HDC dc);
};

// 坦克生成时的闪烁类
class GBorn : public GSprite
{
public:
	GBitmap		*mpBmp;			// 位图资源
	int			mFrame;			// 绘制帧数
	DWORD		mLastTime;
	bool		mIsAdvance;

public:
	GBorn();

	// 生成初始状态
	void born();

	// 关联位图资源
	void attachBmpRes(GBitmap *pBmp)
	{ mpBmp = pBmp; }

	// 绘制
	virtual void draw(HDC dc);
};

// 可移动精灵类
class GMoveableSprite: public GSprite
{
public:
	GDirection		mDirection;		// 移动方向
	float			mSpeed;			// 移动速度

public:
	GMoveableSprite();

	// 移动(如果移动到有效范围之外返回false)
	virtual bool move(DWORD time);
};

// 子弹类
class GBullet : public GMoveableSprite
{
public:
	GBitmap		*mpBmp;			// 位图资源

public:
	GBullet();
	
	// 关联位图资源
	void attachBmpRes(GBitmap *pBmp)
	{ mpBmp = pBmp; }

	// 绘制
	virtual void draw(HDC dc);
};

// 坦克基类
class GTank : public GMoveableSprite
{
public:
	GBitmap		*mpTankBmp;				// 坦克图像
	GBitmap		*mpShieldBmp;			// 保护罩图像
	GBullet		mBullet[2];				// 子弹对象
	DWORD		mLastTime;				// 上一次开火时间
	int			mFrame;					// 帧
	int			mGrade;					// 等级
	bool		mIsShield;				// 是否处于保护状态
	int			mShieldFrame;			// 保护图像帧号
	DWORD		mMaxShieldTime;			// 最大保护时长
	DWORD		mShieldTime;			// 启动保护时刻
	DWORD		mFlickerTime;			// 闪烁时刻
	bool		mIsBorning;				// 是否处于产生状态
	GBorn		mBorn;					// 生成闪烁对象

protected:
	// 绘制保护罩
	void drawShield(HDC dc);

public:
	GTank();

	// 初始化坦克对象
	virtual void init(int w, int h, bool isActive)
	{
		GMoveableSprite::init(w, h, isActive);
		mBorn.init(32, 32, true);
	}	

	// 关联位图资源
	void attachBmpRes(GBitmap *pTankBmp, GBitmap *pBulletBmp,
					  GBitmap *pShieldBmp, GBitmap *pBornBmp)
	{
		mpTankBmp = pTankBmp;
		mpShieldBmp = pShieldBmp;
		mBullet[0].attachBmpRes(pBulletBmp);
		mBullet[1].attachBmpRes(pBulletBmp);
		mBorn.attachBmpRes(pBornBmp);
	}
	
	// 改变坦克运动方向
	void changeDirection(GDirection direct);

	// 保护坦克
	void shield(DWORD time)
	{
		mIsShield = true;
		mShieldFrame = 1;
		mMaxShieldTime = time;
		mShieldTime = mFlickerTime = timeGetTime();
	}

	// 移动坦克
	virtual bool move(DWORD time)
	{ 
		mFrame = !mFrame;		// 改变图像帧, 产生运动效果
		return GMoveableSprite::move(time); 
	}
};

// 玩家坦克类
class GPlayerTank : public GTank
{
public:
	int			mLife;			// 生命值
	int			mScore;			// 得分
	bool		mIsLocked;		// 是否被锁定
	bool		mIsShow;		// 用于锁定时闪烁
public:
	GPlayerTank();

	// 处理用户输入(如果开火返回true)
	bool processInput(WORD input, DWORD time);

	// 开火
	bool fire();

	// 绘制
	void draw(HDC dc);

	// 生成新坦克
	void born();

	// 锁定
	void lock()
	{ 
		mIsLocked = true;
		mShieldTime = mFlickerTime = timeGetTime();
	}
};

// 敌方坦克
class GEnemyTank : public GTank
{
public:
	bool		mIsBonus;		// 是否为被击中后产生奖励
	bool		mIsShowRed;		// 是否处于红色增强状态
	DWORD		mRedTime;		// 红色时间
	int			mType;			// 坦克类型
public:
	GEnemyTank();

	// 改变方向
	void autoChangeDirection();

	// 开火
	bool fire();

	// 生成
	void born();

	// 绘制
	void draw(HDC dc);
};


#endif