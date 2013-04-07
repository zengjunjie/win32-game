#include <stdio.h>
#include "gmap.h"
#include "gtankgame.h"

GGameMap::GGameMap()
{
	mRiverToggle = true;
	mTime = 0;
	mIsProtected = false;
	mpTileBmp = NULL;
}

GGameMap::~GGameMap()
{

}

void GGameMap::concreteProtect()
{
	mMap[12][5].mObj = OBJ_CONCRETE;
	mMap[12][5].mMask = 0x3333;
	mMap[11][5].mObj = OBJ_CONCRETE;
	mMap[11][5].mMask = 0x0033;
	mMap[11][6].mObj = OBJ_CONCRETE;
	mMap[11][6].mMask = 0x00ff;
	mMap[11][7].mObj = OBJ_CONCRETE;
	mMap[11][7].mMask = 0x00cc;
	mMap[12][7].mObj = OBJ_CONCRETE;
	mMap[12][7].mMask = 0xcccc;
	
	mProtectTime = timeGetTime();
	mIsProtected = true;
}


void GGameMap::brickProtect()
{
	mMap[12][5].mObj = OBJ_BRICK;
	mMap[12][5].mMask = 0x3333;
	mMap[11][5].mObj = OBJ_BRICK;
	mMap[11][5].mMask = 0x0033;
	mMap[11][6].mObj = OBJ_BRICK;
	mMap[11][6].mMask = 0x00ff;
	mMap[11][7].mObj = OBJ_BRICK;
	mMap[11][7].mMask = 0x00cc;
	mMap[12][7].mObj = OBJ_BRICK;
	mMap[12][7].mMask = 0xcccc;
	mIsProtected = false;
}


void GGameMap::bare()
{
	mMap[12][5].mObj = OBJ_NULL;
	mMap[12][5].mMask = 0;
	mMap[11][5].mObj = OBJ_NULL;
	mMap[11][5].mMask = 0;
	mMap[11][6].mObj = OBJ_NULL;
	mMap[11][6].mMask = 0;
	mMap[11][7].mObj = OBJ_NULL;
	mMap[11][7].mMask = 0;
	mMap[12][7].mObj = OBJ_NULL;
	mMap[12][7].mMask = 0;
	mIsProtected = false;
}

bool GGameMap::load(const char* fileName)
{
	int i, j;

	FILE* fp = fopen(fileName, "rb");
	if(!fp) return false;
	
	for(i=0; i<MAP_H; i++)
	{
		for(j=0; j<MAP_W; j++)
		{
			mMap[i][j].mObj = (char)fgetc(fp);
			mMap[i][j].mObj --;
			mMap[i][j].mMask = 0xffff;
			if (mMap[i][j].mObj == OBJ_BRICK ||
				mMap[i][j].mObj == OBJ_CONCRETE)
			{
				// 墙可以有空缺, map文件中后一个字节给出具体类型, 形态见下面注释
				switch((char)fgetc(fp))
				{
				case 0:									// 1 1 1 1
					mMap[i][j].mMask = 0xff00;			// 1 1 1 1
					break;								// 0 0 0 0
														// 0 0 0 0

				case 1:									// 0 0 0 0
					mMap[i][j].mMask = 0x00ff;			// 0 0 0 0
					break;								// 1 1 1 1
														// 1 1 1 1

				case 2:									// 1 1 0 0
					mMap[i][j].mMask = 0xcccc;			// 1 1 0 0
					break;								// 1 1 0 0
														// 1 1 0 0

				case 3:									// 0 0 1 1
					mMap[i][j].mMask = 0x3333;			// 0 0 1 1
					break;								// 0 0 1 1
				}										// 0 0 1 1
			}
			else if (mMap[i][j].mObj == OBJ_NULL)
				mMap[i][j].mMask = 0;
		}
	}
		
	fclose(fp);
	
	brickProtect();
	return TRUE;
}

void GGameMap::draw(HDC dc)
{
	int i, j, k, x, y, sx, sy;

	for(i=0; i<MAP_H; i++)
	{
		y = i * TILE_H + OFFSETY;
		for(j=0; j<MAP_W; j++)
		{
			x = j * TILE_W + OFFSETX;
			int obj = mMap[i][j].mObj;
			WORD mask = mMap[i][j].mMask;
			if (obj == OBJ_NULL || obj == OBJ_TREE) continue;
			
			sx = obj * TILE_W;
			
			if (mask == 0xffff)
			{
				if (obj == OBJ_RIVER && !mRiverToggle)
				{
					sx += TILE_W;
				}

				mpTileBmp->draw(dc, x, y, TILE_W, TILE_H, sx, 0);
			}
			else
			{
				for(k=0; k<16; k++)
				{
					if (mask & 0x8000)
					{	
						sx = obj * TILE_W + k % 4 * 8;
						sy = k / 4 * 8;
						mpTileBmp->draw(dc, x + k%4*8, y + sy, 8, 8, sx, sy);
					}
					mask <<= 1;
				}
			}
		}
	}

	if (timeGetTime() - mTime > 500)
	{
		mRiverToggle = !mRiverToggle;
		mTime = timeGetTime();
	}
	
	if (mIsProtected &&
		timeGetTime() - mProtectTime > 20000 )
	{
		brickProtect();
	}
}

void GGameMap::drawTree(HDC dc)
{
	int i, j;
	for(i=0; i<MAP_H; i++)
	{
		for(j=0; j<MAP_W; j++)
		{
			int obj = mMap[i][j].mObj;
			if (obj == OBJ_TREE)
			{
				int x = j * TILE_W + OFFSETX;
				int y = i * TILE_H + OFFSETY;
				int sx = obj * TILE_W;
				mpTileBmp->draw(dc, x, y, TILE_W, TILE_H, sx, 0);
			}
		}
	}
}

bool GGameMap::collisionTest(int row, int col, RECT &rcSprite, int a, int b)
{
	char obj = mMap[row][col].mObj;
	if (obj == OBJ_NULL || obj == OBJ_TREE)
		return false;
	
	WORD mask = mMap[row][col].mMask;
	WORD flag;
	for(int i=0; i<4; i++)
	{
		if ((i == a || i == b) || (a == -1 && b == -1))
		{
			RECT rc, temp;
			rc.left = col * TILE_W + i % 2 * 16;
			rc.right = rc.left + 16;
			rc.top = row * TILE_H + i / 2 * 16;
			rc.bottom = rc.top + 16;
			switch( i )
			{
			case 0:
				flag = 0xcc00;
				break;
			case 1:
				flag = 0x3300;
				break;
			case 2:
				flag = 0x00cc;
				break;
			case 3:
				flag = 0x0033;
			}
			if ((mask & flag) && 
				IntersectRect(&temp, &rcSprite, &rc))
				return true;
		}
	}
	return false;
}

int GGameMap::collisionTest(const GMoveableSprite &sprite)
{
	int x1 = ROUND(sprite.mX);
	int y1 = ROUND(sprite.mY);
	int x2 = x1 + sprite.mWidth;
	int y2 = y1 + sprite.mHeight;
	int x = x1 + sprite.mWidth/2;
	int y = y1 + sprite.mHeight/2;
	int row, col;
	RECT rc;
	rc.left = x1;
	rc.right = x2;
	rc.top = y1;
	rc.bottom = y2;
	
	if (sprite.mDirection == DIR_UP || sprite.mDirection == DIR_DOWN)
	{
		col = x / TILE_W;
		if (sprite.mDirection == DIR_UP)
			row = y1 / TILE_H;
		else row = y2 / TILE_H;
		if (x % TILE_W)
		{
			if (collisionTest(row, col, rc, -1, -1))
				return mMap[row][col].mObj;
		}
		else
		{
			if (collisionTest(row, col-1, rc, 1, 3))
				return mMap[row][col-1].mObj;
			if (collisionTest(row, col, rc, 0, 2 ))
				return mMap[row][col].mObj;
		}
	}
	else
	{
		row = y / TILE_H;
		if (sprite.mDirection == DIR_LEFT)
			col = x1 / TILE_W;
		else col = x2 / TILE_W;
		if (y % TILE_H)
		{
			if( collisionTest( row, col, rc, -1, -1 ) )
				return mMap[row][col].mObj;
		}
		else
		{
			if (collisionTest(row-1, col, rc, 2, 3))
				return mMap[row-1][col].mObj;
			if (collisionTest(row, col, rc, 0, 1))
				return mMap[row][col].mObj;
		}
	}
	return OBJ_NULL;
}

int GGameMap::shootObj(const GBullet &bullet, bool isPowerful)
{
	int x = ROUND(bullet.mX + bullet.mWidth/2);
	int y = ROUND(bullet.mY + bullet.mHeight/2);
	int row = y / TILE_H;
	int col = x / TILE_W;

	int shift;
	WORD mask, flag;
	int target = OBJ_NULL;
	
	switch(bullet.mDirection)
	{
	case DIR_UP:
	case DIR_DOWN:
		// 强力弹摧毁墙的1/2, 普通弹摧毁1/4
		// 计算子弹掩码, 普通弹为1111, 强力弹为1111 / 1111
		if (!isPowerful)
			shift = (3 - (y - row*TILE_H) / 8) * 4;	
		else
			shift = (1 - (y - row*TILE_H) / 16) * 8;
		
		if (x % TILE_W)		// 子弹仅在col列内
		{
			if (!isPowerful)
				flag = 0xf << shift;
			else
				flag = 0xff << shift;
			if (mMap[row][col].mMask & flag)	// 如果子弹掩码与Tile掩码相交, 认为摧毁该Tile
			{
				target = mMap[row][col].mObj;
				if (canDestroy(row, col, isPowerful))
					mMap[row][col].mMask &= ~flag;
			}
		}
		else				// 子弹正好位于col和col-1两个列中间
		{
			if (!isPowerful)
				flag = 0xc << shift;
			else
				flag = 0xcc << shift;
			if (mMap[row][col].mMask & flag)
			{
				target = mMap[row][col].mObj;
				if (canDestroy(row, col, isPowerful))
					mMap[row][col].mMask &= ~flag;
			}

			if (!isPowerful)
				flag = 0x3 << shift;
			else
				flag = 0x33 << shift;
			if(col >= 1 && mMap[row][col-1].mMask & flag)
			{
				if (!canShoot(target))
					target = mMap[row][col-1].mObj;
				if (canDestroy(row, col-1, isPowerful))
					mMap[row][col-1].mMask &= ~flag;
			}
		}
		if( col >= 1 && mMap[row][col-1].mMask == 0 )
			mMap[row][col-1].mObj = OBJ_NULL;
		break;

	case DIR_LEFT:
	case DIR_RIGHT:
		if (!isPowerful)
			shift = (x - col*TILE_W) / 8;
		else
			shift = (x - col*TILE_W) / 16;
		if (y % TILE_H)
		{
			if (!isPowerful)
			{
				if( shift == 0 )
					mask = 0x8888;
				else if( shift == 1 )
					mask = 0x4444;
				else if( shift == 2 )
					mask = 0x2222;
				else
					mask = 0x1111;
			}
			else
			{
				if( shift == 0 )
					mask = 0xcccc;
				else
					mask = 0x3333;
			}
			if (mMap[row][col].mMask & mask)
			{
				target = mMap[row][col].mObj;
				if (canDestroy(row, col, isPowerful))
					mMap[row][col].mMask &= ~mask;
			}
		}
		else
		{
			if( !isPowerful )
			{
				if( shift == 0 )
					mask = 0x8800;
				else if( shift == 1 )
					mask = 0x4400;
				else if( shift == 2 )
					mask = 0x2200;
				else
					mask = 0x1100;
			}
			else
			{
				if( shift == 0 )
					mask = 0xcc00;
				else
					mask = 0x3300;
			}
			if (mMap[row][col].mMask & mask)
			{
				target = mMap[row][col].mObj;
				if (canDestroy(row, col, isPowerful))
					mMap[row][col].mMask &= ~mask;
			}
			mask >>= 8;

			if (row >= 1 && mMap[row-1][col].mMask & mask )
			{
				if (!canShoot(target))
					target = mMap[row-1][col].mObj;
				if (canDestroy(row-1, col, isPowerful))
					mMap[row-1][col].mMask &= ~mask;
			}
		}
		if( row >= 1 && mMap[row-1][col].mMask == 0 )
			mMap[row-1][col].mObj = OBJ_NULL;
		break;
	}
	if (mMap[row][col].mMask == 0 )
		mMap[row][col].mObj = OBJ_NULL;

	if (target == OBJ_HAWK)
		mMap[12][6].mObj = OBJ_STONE;

	return target;
}