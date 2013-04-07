#ifndef GGAME_MAP_H
#define GGAME_MAP_H

#include <windows.h>
#include "gbitmap.h"
#include "gsprite.h"

// 地图宽度(列数)
#define MAP_W		13
// 地图高度(行数)
#define MAP_H		13
// 瓦片宽度(像素)
#define TILE_W		32
// 瓦片高度(像素)
#define TILE_H		32

// 瓦片类型
enum GTileType
{
	OBJ_NULL = -1,
	OBJ_BRICK = 0,
	OBJ_CONCRETE = 1,
	OBJ_TREE = 2,
	OBJ_RIVER = 3,
	OBJ_HAWK = 5,
	OBJ_STONE
};

// 瓦片数据结构
struct GTile
{
	char	mObj;			// 类型
	WORD	mMask;			// 掩码
};

// 地图类
class GGameMap
{
private:
	GTile		mMap[MAP_H][MAP_W];			// 瓦片数组	
	bool		mRiverToggle;				// River触发变量
	DWORD		mTime;						// 时间
	DWORD		mProtectTime;				// 保护时间
	bool		mIsProtected;				// 是否受保护
	GBitmap		*mpTileBmp;					// 瓦片图像指针

protected:
	// 检测精灵矩形rcSprite是否与给定网格[rw, col]冲突
	// 如果检查整个块a, b设为-1, 如果要检查特定子块a, b为待查编号(0, 1, 2, 3)
	bool collisionTest(int row, int col, RECT &rcSprite, int a, int b);

	// 判断是否可以被射击
	bool canShoot(int obj)
	{
		return obj==OBJ_BRICK || obj==OBJ_CONCRETE;
	}
	
	// 判断是否可以被摧毁
	bool canDestroy(int row, int col, bool canDestroyConcrete)
	{
		return (mMap[row][col].mObj == OBJ_BRICK ||
			mMap[row][col].mObj == OBJ_CONCRETE && canDestroyConcrete);
	}	

public:
	GGameMap();
	virtual ~GGameMap();

	// 关联位图资源
	void attachBmpRes(GBitmap *pBmp)
	{ mpTileBmp = pBmp; }

	// 使用水泥墙保护皇冠
	void concreteProtect();

	// 使用砖墙保护皇冠
	void brickProtect();

	// 裸露皇冠
	void bare();

	// 从文件中读取关卡地图并构造Map
	bool load(const char* fileName);

	// 绘制除树木外的其它瓦片数据
	void draw(HDC dc);

	// 绘制树木
	void drawTree(HDC dc);

	// 检测移动精灵是否与地图Tile发生冲突
	// 如果没有冲突返回OBJ_NULL或OBJ_TREE
	int collisionTest(const GMoveableSprite &sprite);

	// 子弹射击Tile对象, isShovel为true允许摧毁水泥墙
	int shootObj(const GBullet &bullet, bool isPowerful=false);
};

#endif