#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtankgame.h"

void PlaySoundEffect(EFFECT ect)
{
	switch(ect)
	{
	case EFFECT_FIRE:
		PlaySound("sound\\Gunfire.wav", NULL, SND_FILENAME | SND_ASYNC);
		break ;
	case EFFECT_EXPLODE:
		PlaySound("sound\\Bang.wav", NULL, SND_FILENAME | SND_ASYNC);
		break ;
	case EFFECT_HIT:
		PlaySound("sound\\hit.wav", NULL, SND_FILENAME | SND_ASYNC);
		break ;
	case EFFECT_PICK:
		PlaySound("sound\\Peow.wav", NULL, SND_FILENAME | SND_ASYNC);
		break ;
	case EFFECT_LIFE:
		PlaySound("sound\\Fanfare.wav", NULL, SND_FILENAME | SND_ASYNC);
	}
}

//----class GTankGame-----------------------------------------------------------
GTankGame::GTankGame()
{
	mHInst = NULL;
	mHWnd = NULL;
	mDC = NULL;
	mMemDC = NULL;
	mMemBmp = NULL;
	mGameState = GM_SPLASH;
	mLevel = 1;
	mIsSingle = true;
	mNumEnemys = 0;
	mMaxEnemys = 0;
	mEnemysLeft = 0;
	mIsEnemyLocked = false;
	mLockTime = 0;
	mIsActive = true;
	mKey = 0;
	mPlayerInput[0] = mPlayerInput[1] = 0;
}

GTankGame::~GTankGame()
{
	if (mDC) ReleaseDC(mHWnd, mDC);
	if (mMemDC) DeleteDC(mMemDC);
	if (mMemBmp) DeleteObject(mMemBmp);
}

bool GTankGame::init(HINSTANCE hInst, HWND hWnd)
{
	int i;

	srand(timeGetTime());

	mHInst = hInst;
	mHWnd = hWnd;

	// 创建双缓存
	mDC = GetDC(mHWnd);
	mMemDC = CreateCompatibleDC(mDC);
	mMemBmp = CreateCompatibleBitmap(mDC, SCREEN_W, SCREEN_H);
	SelectObject(mMemDC, mMemBmp);
	

	// 载入资源
	if (!loadResource()) return false;

	// 初始化玩家坦克
	mPlayer[0].init(28, 28, true);
	mPlayer[1].init(28, 28, true);

	// 初始化敌方坦克
	for(i=0; i<NUM_ENEMYS; i++)
	{
		mEnemy[i].init(28, 28, false);
		mEnemy[i].init(28, 28, false);
	}

	// 初始化奖励
	mBonus.init(30, 28, false);

	mLevel = 1;
	mIsSingle = true;
	mGameState = GM_SPLASH;
	mIsActive = true;

	return true;
}

bool GTankGame::loadResource()
{
	int i;
	COLORREF keyCr = RGB(0, 0, 0);

	// 载入地图瓦片图像
	if (!mBitmapList[0].loadTransparent("graphics\\tile.bmp", keyCr)) return false;
	mGameMap.attachBmpRes(&mBitmapList[0]);

	// 载入坦克生成时的闪烁图像
	if (!mBitmapList[1].loadTransparent("graphics\\bore.bmp", keyCr)) return false;

	// 载入保护罩图像
	if (!mBitmapList[2].loadTransparent("graphics\\shield.bmp", keyCr)) return false;

	// 载入子弹图像
	if (!mBitmapList[3].loadTransparent("graphics\\bullet.bmp", keyCr)) return false;

	// 载入玩家坦克图像
	if (!mBitmapList[4].loadTransparent("graphics\\player1.bmp", keyCr)) return false;
	if (!mBitmapList[5].loadTransparent("graphics\\player2.bmp", keyCr)) return false;

	// 关联玩家坦克图像资源
	mPlayer[0].attachBmpRes(&mBitmapList[4], &mBitmapList[3], &mBitmapList[2], &mBitmapList[1]);
	mPlayer[1].attachBmpRes(&mBitmapList[5], &mBitmapList[3], &mBitmapList[2], &mBitmapList[1]);

	// 载入爆炸图像
	if (!mBitmapList[6].loadTransparent("graphics\\explode1.bmp", keyCr)) return false;
	if (!mBitmapList[7].loadTransparent("graphics\\explode2.bmp", keyCr)) return false;
	for(i=0; i<NUM_EXPLODES; i++)
	{
		mExplode[i].attachBmpRes(&mBitmapList[6], &mBitmapList[7]);
	}

	// 载入奖励图像
	if (!mBitmapList[8].loadTransparent("graphics\\bonus.bmp", keyCr)) return false;
	mBonus.attachBmpRes(&mBitmapList[8]);

	// 载入敌方坦克图像
	if (!mBitmapList[9].loadTransparent("graphics\\enemy.bmp", keyCr)) return false;
	for(i=0; i<NUM_ENEMYS; i++)
	{
		mEnemy[i].attachBmpRes(&mBitmapList[9], &mBitmapList[3], &mBitmapList[2], &mBitmapList[1]);
	}

	// 载入启动封面
	if (!mBitmapList[10].load("graphics\\splash.bmp")) return false;

	// 载入游戏结束画面
	if (!mBitmapList[11].load("graphics\\gameover.bmp")) return false;
	

	return true;
}

void GTankGame::resetPlayer(int iplayer)
{
	int x, y;
	if (iplayer == 0)
	{
		x = PLAYER1_STARTX;
		y = PLAYER1_STARTY;
	}
	else
	{
		x = PLAYER2_STARTX;
		y = PLAYER2_STARTY;
	}
	mPlayer[iplayer].mX = (float)x;
	mPlayer[iplayer].mY = (float)y;
	mPlayer[iplayer].born();
	mPlayer[iplayer].mBullet[0].mIsActive = false;
	mPlayer[iplayer].mBullet[1].mIsActive = false;
}

bool GTankGame::initLevel()
{
	int i;
	char mapFileName[512];
	sprintf(mapFileName, "map\\level%d.map", mLevel);

	// 载入关卡地图
	if(!mGameMap.load(mapFileName))
	{
		return false;
	}

	// 设置玩家
	if (mPlayer[0].mLife > 0) resetPlayer(0);

	if (mIsSingle) mPlayer[1].mIsActive = false;
	else if (mPlayer[1].mLife > 0) resetPlayer(1);

	mIsEnemyLocked = false;
	mEnemysLeft = mNumEnemys = 20;
	for(i=0; i<NUM_ENEMYS; i++)
	{
		mEnemy[i].mBullet[0].mIsActive = false;
		mEnemy[i].mBullet[1].mIsActive = false;
		mEnemy[i].mIsActive = false;
	}

	mLastTime = timeGetTime();
	mBornEmyTm = mLastTime;

	return true;
}

void GTankGame::onKeyDown(UINT key)
{
	mKey = key;
	if (mGameState == GM_ACTIVE)
	{
		switch(key)
		{
		case PLAYER1_UP_KEY:
			mPlayerInput[0] |= KEY_UP;
			break;
		case PLAYER1_DOWN_KEY:
			mPlayerInput[0] |= KEY_DOWN;
			break;
		case PLAYER1_LEFT_KEY:
			mPlayerInput[0] |= KEY_LEFT;
			break;
		case PLAYER1_RIGHT_KEY:
			mPlayerInput[0] |= KEY_RIGHT;
			break;
		case PLAYER1_FIRE_KEY:
			mPlayerInput[0] |= KEY_FIRE;
			break;
		case PLAYER2_UP_KEY:
			mPlayerInput[1] |= KEY_UP;
			break;
		case PLAYER2_DOWN_KEY:
			mPlayerInput[1] |= KEY_DOWN;
			break;
		case PLAYER2_LEFT_KEY:
			mPlayerInput[1] |= KEY_LEFT;
			break;
		case PLAYER2_RIGHT_KEY:
			mPlayerInput[1] |= KEY_RIGHT;
			break;
		case PLAYER2_FIRE_KEY:
			mPlayerInput[1] |= KEY_FIRE;
			break;
		}
	}
}

void GTankGame::onKeyUp(UINT key)
{
	switch(key)
	{
	case PLAYER1_UP_KEY:
		mPlayerInput[0] &= ~KEY_UP;
		break;
	case PLAYER1_DOWN_KEY:
		mPlayerInput[0] &= ~KEY_DOWN;
		break;
	case PLAYER1_LEFT_KEY:
		mPlayerInput[0] &= ~KEY_LEFT;
		break;
	case PLAYER1_RIGHT_KEY:
		mPlayerInput[0] &= ~KEY_RIGHT;
		break;
	case PLAYER1_FIRE_KEY:
		mPlayerInput[0] &= ~KEY_FIRE;
		break;
	case PLAYER2_UP_KEY:
		mPlayerInput[1] &= ~KEY_UP;
		break;
	case PLAYER2_DOWN_KEY:
		mPlayerInput[1] &= ~KEY_DOWN;
		break;
	case PLAYER2_LEFT_KEY:
		mPlayerInput[1] &= ~KEY_LEFT;
		break;
	case PLAYER2_RIGHT_KEY:
		mPlayerInput[1] &= ~KEY_RIGHT;
		break;
	case PLAYER2_FIRE_KEY:
		mPlayerInput[1] &= ~KEY_FIRE;
		break;
	}
}

void GTankGame::updateFrame()
{
	switch(mGameState)
	{
	case GM_ACTIVE:
		if(gameProcess())
		{
			drawGame(mMemDC);
		}
		break ;
	case GM_SPLASH:
		menuProcess();
		drawMainMenu(mMemDC);
		break ;
	case GM_OVER:
		menuProcess();
		drawGameOver(mMemDC);
		break ;
	}

	// 将缓图像存贴到窗口上
	BitBlt(mDC, 0, 0, SCREEN_W, SCREEN_H, mMemDC, 0, 0, SRCCOPY);
}

void GTankGame::explode(const GSprite &sprite, bool isExplode)
{
	// 找出一个可用的爆炸对象, 使其爆炸
	for(int i=0; i<NUM_EXPLODES; i++)
	{
		if (!mExplode[i].mIsActive)
		{
			int x = ROUND(sprite.mX + sprite.mWidth/2);
			int y = ROUND(sprite.mY + sprite.mHeight/2);
			mExplode[i].mIsActive = true;
			mExplode[i].mX = x - mExplode[i].mWidth/2.0f;
			mExplode[i].mY = y - mExplode[i].mHeight/2.0f;
			mExplode[i].mLastTime = timeGetTime();
			mExplode[i].mIsExplode = isExplode;
			break;
		}
	}
}

bool GTankGame::bornEnemy()
{
	if (mEnemysLeft <= 0) return true;
	
	int where = rand() % 5;
	int i, j;
	float x, y;
	
	for(i=0; i<mMaxEnemys; i++)
	{
		if (!mEnemy[i].mIsActive)
		{
			switch(where)
			{
			case 0:
				x = 2; y = 2;
				break;
			case 1:
				x = 194; y = 2;
				break;
			case 2:
				x = 386; y = 2;
				break;
			case 3:
				x = 2; y = 160;
				break;
			case 4:
				x = 386; y = 160;
				break;
			}

			// 确保出生位置上没有其它坦克, 避免出生碰撞
			mEnemy[i].mX = x;
			mEnemy[i].mY = y;
			mEnemy[i].mDirection = DIR_DOWN;
			mEnemy[i].mIsActive = true;
			for(j=0; j<mMaxEnemys; j++)
			{
				if (i != j && mEnemy[j].mIsActive && mEnemy[i].collisionTest(mEnemy[j]))
				{
					mEnemy[i].mIsActive = false;
					return false;
				}
			}
			for(j=0; j<2; j++)
			{
				if (mEnemy[i].collisionTest(mPlayer[j]))
				{
					mEnemy[i].mIsActive = false;
					return false;
				}
			}

			int temp = rand() % 100;
			if (temp < 70) mEnemy[i].mType = 0;
			else if( temp < 80 ) mEnemy[i].mType = 1;
			else mEnemy[i].mType = 2;

			if (mEnemysLeft == 4 || mEnemysLeft == 11 || mEnemysLeft == 18 )
			{
				mEnemy[i].mIsBonus = true;
				mEnemy[i].mRedTime = 0;
			}
			else
				mEnemy[i].mIsBonus = false;
			
			mEnemy[i].born();
			
			mEnemysLeft --;
			return true;
		}
	}

	return false;
}

void GTankGame::bornBonus()
{
	mBonus.mIsActive = true;
	int temp = rand() % 100;
	if (temp < 10)
		mBonus.mType = BONUS_LIFE;
	else if( temp < 30 )
		mBonus.mType = BONUS_CLOCK;
	else if( temp < 50 )
		mBonus.mType = BONUS_SHOVEL;
	else if( temp < 65 )
		mBonus.mType = BONUS_BOMB;
	else if( temp < 85 )
		mBonus.mType = BONUS_STAR;
	else
		mBonus.mType = BONUS_HELMET;
	
	mBonus.mX = (float)(rand()%(416 - mBonus.mWidth));
	mBonus.mY = (float)(rand()%(416 - mBonus.mHeight));
	mBonus.mLastTime = mBonus.mFlickerTime = timeGetTime();
}

void GTankGame::eatBonus(GPlayerTank &player)
{
	PlaySoundEffect(EFFECT_PICK);
	
	int i;
	switch(mBonus.mType)
	{
	case BONUS_LIFE:
		player.mLife++;
		PlaySoundEffect(EFFECT_LIFE);
		break;
	case BONUS_CLOCK:
		mIsEnemyLocked = TRUE;
		mLockTime = timeGetTime();
		break;
	case BONUS_SHOVEL:
		mGameMap.concreteProtect();
		break;
	case BONUS_BOMB:
		for(i=0; i<mMaxEnemys; i++)
		{
			if (mEnemy[i].mIsActive && !mEnemy[i].mIsBorning )
			{
				explode(mEnemy[i], true);
				mEnemy[i].mIsActive = false;
				mNumEnemys --;
			}
		}
		PlaySoundEffect(EFFECT_EXPLODE);
		break;
	case BONUS_STAR:
		if( ++player.mGrade > 3 )
			player.mGrade = 3;
		break;
	case BONUS_HELMET:
		player.shield(10000);
		break;
	}
	mBonus.mIsActive = false;
}

void GTankGame::playerBeenShooted(int iplayer)
{
	GPlayerTank &player = mPlayer[iplayer];
	if (!player.mIsActive) return;
	
	explode(player, true);
	PlaySoundEffect(EFFECT_EXPLODE);
	if (--player.mLife <= 0)
	{
		player.mIsActive = FALSE;
		return;
	}
	
	resetPlayer(iplayer);
	player.mGrade = 0;
}

void GTankGame::drawMainMenu(HDC dc)
{
	// 刷新背景
	RECT rc;
	rc.left = 0;	rc.top = 0;
	rc.right = SCREEN_W;	rc.bottom = SCREEN_H;
	HBRUSH br = CreateSolidBrush(RGB(0, 0, 0));
	FillRect(mMemDC, &rc, br);
	DeleteObject(br);

	mBitmapList[10].draw(dc, 135, 130);

	mBitmapList[4].draw(dc, 200, 300+30*(!mIsSingle), 28, 28, 0, 28);
}

void GTankGame::drawGameOver(HDC dc)
{
	// 刷新背景
	RECT rc;
	rc.left = 0;	rc.top = 0;
	rc.right = SCREEN_W;	rc.bottom = SCREEN_H;
	HBRUSH br = CreateSolidBrush(RGB(0, 0, 0));
	FillRect(mMemDC, &rc, br);
	DeleteObject(br);
	
	mBitmapList[11].draw(dc, 200, 130);
}

void GTankGame::drawGame(HDC dc)
{
	int i;
	// 刷新背景
	RECT rc;
	rc.left = 0;	rc.top = 0;
	rc.right = SCREEN_W;	rc.bottom = SCREEN_H;
	HBRUSH br = CreateSolidBrush(RGB(0, 0, 0));
	FillRect(mMemDC, &rc, br);
	DeleteObject(br);

	// 绘制地图
	mGameMap.draw(dc);

	// 绘制玩家坦克
	mPlayer[0].draw(dc);
	if (!mIsSingle) mPlayer[1].draw(dc);

	// 绘制敌方坦克
	for(i=0; i<mMaxEnemys; i++)
	{
		mEnemy[i].draw(dc);
	}

	// 绘制树木
	mGameMap.drawTree(dc);

	// 绘制爆炸效果
	for(i=0; i<NUM_EXPLODES; i++)
	{
		mExplode[i].draw(dc);
	}

	// 绘制奖励
	mBonus.draw(dc);
}

void GTankGame::menuProcess()
{
	if (mGameState == GM_SPLASH)
	{
		if (mKey == VK_UP || mKey == VK_DOWN)
		{
			mIsSingle = !mIsSingle;
		}
		else if(mKey == VK_RETURN)
		{
			mLevel = 1;
			if (mIsSingle) mMaxEnemys = 4;
			else mMaxEnemys = 6;
			initLevel();
			mPlayer[0].mLife = 3;
			mPlayer[1].mLife = 3;
			mGameState = GM_ACTIVE;
			mIsActive = true;
		}
	}
	else if (mGameState == GM_OVER)
	{
		if (mKey == VK_RETURN)
		{
			mGameState = GM_SPLASH;
		}
	}

	mKey = 0;
}

bool GTankGame::gameProcess()
{
	int i, j, k;
	float x, y;
	int target, tileType;
	GDirection dir;
	GBullet *pBullet;

	DWORD thisTime = timeGetTime();
	DWORD deltaTime = thisTime - mLastTime;
	// 每隔5毫秒处理一次
	if(deltaTime < 5) return false;

	if(deltaTime > 20)
	{
 		deltaTime = 20;
	}
	
	mLastTime = thisTime;

	// 判断是否暂停游戏
	if (mIsActive)
	{
		if (mKey == VK_PAUSE) mIsActive = false;
	}
	else
	{
		if (mKey == VK_PAUSE) mIsActive = true;
	}

	// 用户选择关卡
	if (mKey == VK_F3 || mKey == VK_F4)
	{
		if(mKey == VK_F3)
		{
			mLevel++;
			if(mLevel > 20) mLevel = 1;
		}
		else
		{
			mLevel--;
			if(mLevel < 1) mLevel = 20;
		}
		for(i=0; i<mMaxEnemys; i++)
		{
			mEnemy[i].mIsActive = false;
			mEnemy[i].mBullet[0].mIsActive = false;
		}
		initLevel();
	}

	mKey = 0;
	if (!mIsActive) return true;

	// 处理玩家坦克
	for(i=0; i<2; i++)
	{
		if (!mPlayer[i].mIsActive) continue ;

		// 先记录坦克原来的位置
		x = mPlayer[i].mX;
		y = mPlayer[i].mY;
		dir = mPlayer[i].mDirection;

		if (!mPlayer[i].mIsBorning && mGameState != GM_OVER)
		{
			mPlayer[i].processInput(mPlayerInput[i], deltaTime);
		}

		// 检测处理输入后坦克是否与地图冲突
		tileType = mGameMap.collisionTest(mPlayer[i]);
		if(tileType != OBJ_NULL && tileType != OBJ_TREE)
		{
			mPlayer[i].mX = x;
			mPlayer[i].mY = y;
			mPlayer[i].mDirection = dir;
		}

		// 检测玩家坦克是否与敌方坦克碰撞
		for(j=0; j<mMaxEnemys; j++)
		{
			if(mPlayer[i].collisionTest(mEnemy[j]))
			{
				mPlayer[i].mX = x;
				mPlayer[i].mY = y;
				mPlayer[i].mDirection = dir;
			}
		}

		// 如果是双玩家, 检测玩家坦克是否相撞
		if (!mIsSingle && mPlayer[i].collisionTest(mPlayer[!i]))
		{
			mPlayer[i].mX = x;
			mPlayer[i].mY = y;
			mPlayer[i].mDirection = dir;
		}

		// 检查玩家是否吃到奖励
		if (mPlayer[i].collisionTest(mBonus))
		{
			eatBonus(mPlayer[i]);
		}

		// 处理玩家子弹
		for(j=0; j<2; j++)
		{
			pBullet = &mPlayer[i].mBullet[j];
			if (!pBullet->mIsActive) continue;
			
			if (!pBullet->move(deltaTime))
			{
				pBullet->mIsActive = false;
				continue;
			}

			target = mGameMap.shootObj(*pBullet, mPlayer[i].mGrade >= 3);
			if (target == OBJ_BRICK)
			{
				pBullet->mIsActive = false;
				explode(*pBullet);
			}
			else if (target == OBJ_CONCRETE)
			{
				pBullet->mIsActive = false;
				explode(*pBullet);
				PlaySoundEffect(EFFECT_HIT);
			}
			else if (target == OBJ_HAWK)
			{
				pBullet->mIsActive = false;
				explode(*pBullet, true);
				PlaySoundEffect(EFFECT_EXPLODE);
				mGameState = GM_OVER;
			}

			// 检查是否击中另一个玩家
			if (!mIsSingle && pBullet->collisionTest(mPlayer[!i]))
				
			{
				// 让另一个玩家坦克处于锁定状态
				pBullet->mIsActive = false;
				if(!mPlayer[!i].mIsShield) 	mPlayer[!i].lock();
			}

			// 检查是否击中敌方坦克和敌方子弹
			for(k=0; k<mMaxEnemys; k++)
			{
				if (!mEnemy[k].mIsActive) continue ;

				if (!mEnemy[k].mIsShield && !mEnemy[k].mIsBorning &&
					pBullet->collisionTest(mEnemy[k]))
				{
					pBullet->mIsActive = false;
					explode(*pBullet);					
					if (mEnemy[k].mIsBonus)
					{
						mEnemy[k].mIsBonus = false;
						bornBonus();
					}
					
					if (mEnemy[k].mType == 2)
					{
						if (--mEnemy[k].mGrade < 0 )
							mEnemy[k].mIsActive = false;
					}
					else
						mEnemy[k].mIsActive = false;
					
					if (!mEnemy[k].mIsActive)
					{
						explode(mEnemy[k], true);
						PlaySoundEffect(EFFECT_EXPLODE);
						mNumEnemys --;
						mPlayer[i].mScore += (mEnemy[i].mType+1) * 100;
					}
					break;
				}
				else if (mEnemy[k].mBullet[0].mIsActive)
				{
					if (pBullet->collisionTest(mEnemy[k].mBullet[0]))
					{
						pBullet->mIsActive = false;
						mEnemy[k].mBullet[0].mIsActive = false;
						break;
					}
				}
			}
		}
	}

	// 处理敌方坦克
	for(i=0; i<mMaxEnemys; i++)
	{
		if (!mEnemy[i].mIsActive) continue ;

		if (!mIsEnemyLocked && !mEnemy[i].mIsBorning)
		{
			x = mEnemy[i].mX;
			y = mEnemy[i].mY;
			dir = mEnemy[i].mDirection;
			
			if (rand() % 200 == 0 ||
				!mEnemy[i].move(deltaTime) )
			{
				mEnemy[i].autoChangeDirection();
			}
			
			// 检测敌方坦克是否与地图发生碰撞
			target = mGameMap.collisionTest(mEnemy[i]);
			if (target == OBJ_BRICK)	// 如果是砖墙, 考虑转向或者射击
			{
				if (rand() % 100 < 30)
				{
					mEnemy[i].autoChangeDirection();
					target = mGameMap.collisionTest(mEnemy[i]);
				}
				else
					mEnemy[i].fire();
			}
			else if(target == OBJ_CONCRETE || target == OBJ_RIVER )	// 如果是不可穿越障碍转向
			{
				mEnemy[i].autoChangeDirection();
				target = mGameMap.collisionTest(mEnemy[i]);
			}

			// 检测是否与玩家坦克碰撞, 如果是开火
			if (mEnemy[i].collisionTest(mPlayer[0]) ||
				mEnemy[i].collisionTest(mPlayer[1]) )
			{
				mEnemy[i].mX = x;
				mEnemy[i].mY = y;
				mEnemy[i].mDirection = dir;
				mEnemy[i].fire();
			}
			else if(target != OBJ_NULL && target != OBJ_TREE)
			{
				mEnemy[i].mX = x;
				mEnemy[i].mY = y;
				mEnemy[i].mDirection = dir;
			}

			// 检测是否与其它敌方坦克相撞
			for(j=0; j<mMaxEnemys; j++)
			{
				if (i != j && mEnemy[i].collisionTest(mEnemy[j]))
				{
					mEnemy[i].changeDirection(mEnemy[j].mDirection);
					mEnemy[j].changeDirection(dir);
					if( mEnemy[i].collisionTest(mEnemy[j]))
					{
						mEnemy[i].mX = x;
						mEnemy[i].mY = y;
					}
					break;
				}
			}
			
			// 敌方坦克吃掉奖励
			//if (mEnemy[i].collisionTest(mBonus))
			//	eatBonus(mEnemy[i]);	
			switch(mEnemy[i].mType)
			{
			case 0:		
				if (rand() % 200 == 0) mEnemy[i].fire();
				break ;
			case 1:
				if (rand() % 400 == 0) mEnemy[i].fire();
				break ;
			case 2:
				if (rand() % 800 == 0) mEnemy[i].fire();
				break ;
			}
			
		}

		// 处理敌方坦克子弹
		pBullet = &mEnemy[i].mBullet[0];
		if (pBullet->mIsActive)
		{
			if (!pBullet->move(deltaTime)) pBullet->mIsActive = false;
	
			target = mGameMap.shootObj(*pBullet);
			if (target == OBJ_BRICK || target == OBJ_CONCRETE)
			{
				pBullet->mIsActive = false;
				explode(*pBullet);
			}
			else if(target == OBJ_HAWK)
			{
				pBullet->mIsActive = false;
				explode(*pBullet, true);
				PlaySoundEffect(EFFECT_EXPLODE);
				mGameState = GM_OVER;
			}

			for(j=0; j<2; j++)
			{
				if (pBullet->collisionTest(mPlayer[j]))
				{
					// 敌方子弹击中玩家坦克
					pBullet->mIsActive = false;
					if (!mPlayer[j].mIsShield)
					{
						explode(*pBullet);
						playerBeenShooted(j);
						PlaySoundEffect(EFFECT_EXPLODE);
					}
				}
			}
		}
	}

	// 生成敌方坦克
	DWORD time = thisTime - mBornEmyTm;
	if(time > 3000)
	{
		if(bornEnemy())
		{
			mBornEmyTm = thisTime;
		}
	}

	// 如果锁定时间到, 解锁
	if (mIsEnemyLocked && thisTime - mLockTime > 10000)
	{
		mIsEnemyLocked = false;
	}

	// 如果剩余敌人为0, 胜利
	if (mNumEnemys <= 0 && mGameState == GM_ACTIVE)
	{
		mLevel++;
		if(mLevel > 20) mLevel = 1;
		initLevel();
	}
	
	// 如果玩家坦克全部死亡, 失败
	if (!mPlayer[0].mIsActive && !mPlayer[1].mIsActive)
	{
		mGameState = GM_OVER;
	}

	return true;
}

int GTankGame::run()
{
	MSG	msg;
	
    while(TRUE)
    {
		// 检查消息队列中是否有新消息
        if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
			// 获取消息
            if (!GetMessage(&msg, NULL, 0, 0))
                break;
			
			// 派遣处理消息
            TranslateMessage(&msg); 
            DispatchMessage(&msg);
        }
        else
        {
			// 没有消息到来, 绘制帧
			updateFrame();
        }
    }

	return msg.wParam;
}