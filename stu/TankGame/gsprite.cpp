#include "gsprite.h"
#include "gtankgame.h"

//----class GSprite-------------------------------------------------------------
GSprite::GSprite()
{
	mX = mY = 0;
	mWidth = mHeight = 0;
}

bool GSprite::collisionTest(const GSprite& sprite)
{
	if (!mIsActive || !sprite.mIsActive)
		return FALSE;
	
	RECT rc1, rc2, temp;
	
	rc1.left = ROUND(mX);
	rc1.right = rc1.left + mWidth;
	rc1.top = ROUND(mY);
	rc1.bottom = rc1.top + mHeight;
	
	rc2.left = (int)sprite.mX;
	rc2.right = rc2.left + sprite.mWidth;
	rc2.top = (int)sprite.mY;
	rc2.bottom = rc2.top + sprite.mHeight;
	
	// 检测rc1与rc2是否相交
	return IntersectRect(&temp, &rc1, &rc2) == 0 ? false : true;
}

//----class GBonus--------------------------------------------------------------
GBonus::GBonus()
{
	mType = BONUS_LIFE;
	mpBmp = NULL;
	mLastTime = 0;
	mFlickerTime = 0;
}

void GBonus::draw(HDC dc)
{
	static BOOL bShow;
	
	if (!mIsActive) return;
	
	// 每隔200毫秒闪烁一次
	if (timeGetTime() - mFlickerTime > 200)
	{
		mFlickerTime = timeGetTime();
		bShow = !bShow;
	}	
	
	if (bShow)
	{
		int sx = (int)mType * mWidth;
		int x = ROUND(mX+OFFSETX);
		int y = ROUND(mY+OFFSETY);
		mpBmp->draw(dc, x, y, mWidth, mHeight, sx, 0);
	}
	
	// 持续10秒钟
	if(timeGetTime() - mLastTime > 10000)
		mIsActive = false;
}

//----class GExplode------------------------------------------------------------
GExplode::GExplode()
{
	mpBmps[0] = NULL;
	mpBmps[1] = NULL;

	mIsExplode = false;
	mLastTime = 0;
}

void GExplode::draw(HDC dc)
{
	if (!mIsActive) return;
	
	DWORD time = timeGetTime() - mLastTime;
	int x = ROUND(mX - 20 + OFFSETX);
	int y = ROUND(mY - 20 + OFFSETY);
	
	if (mIsExplode)
	{
		if(time < 50 || time > 150)
			mpBmps[0]->draw(dc, x, y, mWidth, mHeight, 0, 0);
		else mpBmps[1]->draw(dc, x, y, mWidth, mHeight, 0, 0);
		if( time > 200 ) mIsActive = false;
	}
	else
	{
		mpBmps[0]->draw(dc, x, y, mWidth, mHeight, 0, 0);
		if (time > 70) mIsActive = false;
	}
}

//----class GBorn---------------------------------------------------------------
GBorn::GBorn()
{
	mpBmp = NULL;
	mIsActive = false;
	mIsAdvance = false;
	mFrame = 0;
	mLastTime = 0;
}

void GBorn::born()
{
	mIsActive = true;
	mIsAdvance = true;
	mFrame = 0;
	mLastTime = timeGetTime();
}

void GBorn::draw(HDC dc)
{
	if (!mIsActive) return;
	
	int sx = mFrame * mWidth;
	int x = ROUND(mX + OFFSETX);
	int y = ROUND(mY + OFFSETY);
	
	mpBmp->draw(dc, x, y, mWidth, mHeight, sx, 0);
	
	if (timeGetTime() - mLastTime > 100)
	{
		mLastTime = timeGetTime();
		if (mIsAdvance)		// 渐大阶段
		{
			if (++mFrame > 3)
			{
				mFrame = 2;
				mIsAdvance = false;
			}
		}
		else				// 渐小阶段
		{
			if (--mFrame < 0)
			{
				mFrame = 1;
				mIsAdvance = true;
			}
		}
	}
}

//----class GMoveableSprite-----------------------------------------------------
GMoveableSprite::GMoveableSprite()
{
	mDirection = DIR_UP;
	mSpeed = 0;
}

bool GMoveableSprite::move(DWORD time)
{
	if (!mIsActive) return false;
	
	float delta = time * mSpeed / 20.0f;
	switch(mDirection)
	{
	case DIR_UP:		mY -= delta;		break;
	case DIR_DOWN:		mY += delta;		break;
	case DIR_LEFT:		mX -= delta;		break;
	case DIR_RIGHT:		mX += delta;		break;
	}
	
	if (mX >= 0 && mX+mWidth <= MAX_X &&
		mY >= 0 && mY+mHeight <= MAX_Y)
		return true;
	
	if (mX < 0) mX = 0;
	else if (mX+mWidth > MAX_X) mX = (float)(MAX_X-mWidth);
	
	if (mY < 0) mY = 0;
	else if (mY+mHeight > MAX_Y) mY = (float)(MAX_Y-mHeight);
	
	return false;
}

//----class GBullet-------------------------------------------------------------
GBullet::GBullet()
{
	mpBmp = NULL;
}

void GBullet::draw(HDC dc)
{
	if (!mIsActive) return ;

	int sx = mDirection * mWidth;
	int x = ROUND(mX + OFFSETX);
	int y = ROUND(mY + OFFSETY);

	mpBmp->draw(dc, x, y, mWidth, mHeight, sx, 0);
}

//----class GTank---------------------------------------------------------------
GTank::GTank()
{
	mpTankBmp = NULL;
	mpShieldBmp = NULL;
	mFrame = 0;
	mGrade = 0;
	mIsShield = false;
	mShieldFrame = 1;
	mLastTime = 0;
	mShieldTime = 0;
	mMaxShieldTime = 0;
	mFlickerTime = 0;
	mIsBorning = false;

	mBullet[0].init(BULLET_W, BULLET_H, false);
	mBullet[1].init(BULLET_W, BULLET_H, false);	
}

void GTank::changeDirection(GDirection direct)
{
	if (mDirection != direct)
	{
		int x = ROUND(mX);
		int y = ROUND(mY);
		int row = y / TILE_H;
		int col = x / TILE_W;
		int xoffset = x % TILE_W;
		int yoffset = y % TILE_H;

		// 下面代码确保转弯后坦克仍然在轨道上运行(轨道为地图整网格和半网格)
		if ((mDirection == DIR_LEFT || mDirection == DIR_RIGHT) && (direct == DIR_UP || direct == DIR_DOWN))
		{
			if (xoffset <= 10)
				mX = col * TILE_W + 2.0f;
			else if (xoffset < TILE_W - 6 )
				mX = col * TILE_W + 18.0f;
			else mX = col * TILE_W + 34.0f;
		}

		if ((mDirection == DIR_UP || mDirection == DIR_DOWN) && (direct == DIR_LEFT || direct == DIR_RIGHT))
		{
			if (yoffset <= 10)
				mY = row * TILE_H + 2.0f;
			else if (yoffset < TILE_H - 6)
				mY = row * TILE_H + 18.0f;
			else mY = row * TILE_H + 34.0f;
		}

		mDirection = direct;
	}
}

void GTank::drawShield(HDC dc)
{
	int sy = mShieldFrame * 32;
	int x = ROUND(mX - 2 + OFFSETX);
	int y = ROUND(mY - 2 + OFFSETY);
	
	mpShieldBmp->draw(dc, x, y, 32, 32, 0, sy);
	
	if (timeGetTime() - mFlickerTime > 50)
	{
		mShieldFrame = !mShieldFrame;
		mFlickerTime = timeGetTime();
	}

	if (timeGetTime() - mShieldTime > mMaxShieldTime)
	{
		mIsShield = false;
	}
}

//----class GPlayerTank---------------------------------------------------------
GPlayerTank::GPlayerTank()
{
	mLife = 3;
	mScore = 0;
	mIsLocked = false;
	mIsShow = true;
}

void GPlayerTank::born()
{ 
	mIsActive = true;
	mIsShield = true;
	mIsBorning = true;
	mIsLocked = false;
	mBorn.mX = mX - 2;
	mBorn.mY = mY - 2;
	mBorn.born();
	mShieldTime = timeGetTime();
	mDirection = DIR_UP;
	mSpeed = 1.5f;
}

bool GPlayerTank::processInput(WORD input, DWORD time)
{
	if (!mIsLocked)
	{
		GDirection dir;
		if (input & KEY_UP)
			dir = DIR_UP;
		else if (input & KEY_DOWN)
			dir = DIR_DOWN;
		else if (input & KEY_LEFT)
			dir = DIR_LEFT;
		else if (input & KEY_RIGHT)
			dir = DIR_RIGHT;
		
		if (input & KEY_DIRECTION)
		{
			if (mDirection == dir)
				move(time);
			else changeDirection(dir);
		}
	}
	
	if (input & KEY_FIRE)
		return fire();
	
	return false;
}

bool GPlayerTank::fire()
{
	int i;
	float speed;
	// 计算子弹发射速度
	switch (mGrade)
	{
	case 0:
		speed = 3.0f;
		break;
	case 1:
		speed = 4.0f;
		break;
	case 2:
	case 3:
		speed = 5.0f;
		break;
	}
	
	// 开火间隙大于等于200毫秒
	if (timeGetTime() - mLastTime < 200)
		return false;
	
	// 寻找可用子弹
	for(i=0; i<2; i++)
	{
		if (!mBullet[i].mIsActive) break;
	}

	// 最多允许有2颗子弹同时运行, 且坦克级别小于2时只能允许1颗子弹运行
	if (i >= 2 || i != 0 && mGrade < 2)
		return false;
	
	mLastTime = timeGetTime();
	mBullet[i].mIsActive = true;
	mBullet[i].mSpeed = speed;
	mBullet[i].mDirection = mDirection;
	// 计算子弹初始位置
	switch(mDirection)
	{
	case DIR_UP:
		mBullet[i].mX = mX + mWidth/2 - 4;
		mBullet[i].mY = mY + 4;
		break;
	case DIR_DOWN:
		mBullet[i].mX = mX + mWidth/2 - 4;
		mBullet[i].mY = mY + mHeight - 12;
		break;
	case DIR_LEFT:
		mBullet[i].mX = mX + 4;
		mBullet[i].mY = mY + mHeight/2 - 4;
		break;
	case DIR_RIGHT:
		mBullet[i].mX = mX + mWidth - 12;
		mBullet[i].mY = mY + mHeight/2 - 4;
	}

	PlaySoundEffect(EFFECT_FIRE);
	return TRUE;
}

void GPlayerTank::draw(HDC dc)
{
	if (!mIsActive) return;
	
	// 如果正在生成, 显示生成闪烁图像
	if (mIsBorning)
	{
		mBorn.draw(dc);
		if (timeGetTime() - mShieldTime > 800)
		{
			mIsBorning = false;
			// 生成后3秒以内处于保护状态
			shield(2000);
		}
		return;
	}
	
	// 如果坦克被锁定
	if (mIsLocked)
	{
		if (timeGetTime() - mFlickerTime > 200)
		{
			mIsShow = !mIsShow;
			mFlickerTime = timeGetTime();
		}
		else if (timeGetTime() - mShieldTime > 5000)
			mIsLocked = false;
	}
	
	if (!mIsLocked || mIsShow)
	{
		int sx = (mGrade * 2 + mFrame) * mWidth;
		int sy = (int)mDirection * mHeight;
		int x = ROUND(mX + OFFSETX);
		int y = ROUND(mY + OFFSETY);
		
		mpTankBmp->draw(dc, x, y, mWidth, mHeight, sx, sy);
	}
	
	if (mIsShield)
	{
		drawShield(dc);
	}
	mBullet[0].draw(dc);
	mBullet[1].draw(dc);
}

//----class GEnemyTank----------------------------------------------------------
GEnemyTank::GEnemyTank()
{
	mIsBonus = false;
	mIsShowRed = false;
	mRedTime = 0;
	mType = 0;
}

void GEnemyTank::autoChangeDirection()
{
	GDirection dir;
	do 
	{
		dir = (GDirection)(rand() % 4);
	} while (dir == mDirection);
	GTank::changeDirection(dir);
}

bool GEnemyTank::fire()
{
	// 敌方坦克只允许一发子弹运行, 且发射间隔大于150毫秒
	if (mBullet[0].mIsActive || timeGetTime() - mLastTime < 150)
		return false;
	
	mLastTime = timeGetTime();
	mBullet[0].mIsActive = true;
	mBullet[0].mSpeed = 3.0f;
	mBullet[0].mDirection = mDirection;
	
	switch(mDirection)
	{
	case DIR_UP:
		mBullet[0].mX = mX + mWidth/2 - 4;
		mBullet[0].mY = mY;
		break;
	case DIR_DOWN:
		mBullet[0].mX = mX + mWidth/2 - 4;
		mBullet[0].mY = mY + mHeight - 8;
		break;
	case DIR_LEFT:
		mBullet[0].mX = mX;
		mBullet[0].mY = mY + mHeight/2 - 4;
		break;
	case DIR_RIGHT:
		mBullet[0].mX = mX + mWidth - 8;
		mBullet[0].mY = mY + mHeight/2 - 4;
	}

	PlaySoundEffect(EFFECT_FIRE);
	return true;
}

void GEnemyTank::born()
{ 
	mIsActive = true;
	mIsShield = false;
	mIsBorning = true;
	mBorn.mX = mX - 2;
	mBorn.mY = mY - 2;
	mBorn.born();

	switch(mType)
	{
	case 0:
		mSpeed = 0.7f;		mGrade = 0;
		break;
	case 1:
		mSpeed = 1.0f;		mGrade = 0;
		break;
	case 2:
		mSpeed = 0.4f;		mGrade = 2;
		break;
	}
	mShieldTime = timeGetTime();
}

void GEnemyTank::draw(HDC dc)
{
	if (!mIsActive) return;
	
	if (mIsBorning)
	{
		mBorn.draw(dc);
		if (timeGetTime() - mShieldTime > 800)
			mIsBorning = FALSE;
		return;
	}
	
	if (mIsBonus && timeGetTime() - mRedTime > 80)
	{	
		mRedTime = timeGetTime();
		mIsShowRed = !mIsShowRed;
	}
	
	int x = ROUND(mX + OFFSETX);
	int y = ROUND(mY + OFFSETY);
	int sx, sy;

	if (mIsBonus && mIsShowRed)
	{
		switch (mType)
		{
		case 0:
		case 1:
			sy = (int)mDirection * mHeight;
			sx = (2 + mFrame + 4*mType) * mWidth;
			break;
		case 2:
			sy = ((int)mDirection + 4) * mHeight;
			sx = 6 * mWidth;
			break;
		}
	}
	else
	{
		switch(mType)
		{
		case 0:
		case 1:
			sy = (int)mDirection * mHeight;
			sx = (mType * 4 + mFrame) * mWidth;
			break;
		case 2:
			sx = ((2-mGrade)*2 + mFrame) * mWidth;
			sy = ((int)mDirection + 4) * mHeight;
			break;
		}
	}
	mpTankBmp->draw(dc, x, y, mWidth, mHeight, sx, sy);
	
	if (mIsShield)
	{
		drawShield(dc);
	}
	mBullet[0].draw(dc);
}