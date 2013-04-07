#ifndef GGAME_MAP_H
#define GGAME_MAP_H

#include <windows.h>
#include "gbitmap.h"
#include "gsprite.h"

// ��ͼ���(����)
#define MAP_W		13
// ��ͼ�߶�(����)
#define MAP_H		13
// ��Ƭ���(����)
#define TILE_W		32
// ��Ƭ�߶�(����)
#define TILE_H		32

// ��Ƭ����
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

// ��Ƭ���ݽṹ
struct GTile
{
	char	mObj;			// ����
	WORD	mMask;			// ����
};

// ��ͼ��
class GGameMap
{
private:
	GTile		mMap[MAP_H][MAP_W];			// ��Ƭ����	
	bool		mRiverToggle;				// River��������
	DWORD		mTime;						// ʱ��
	DWORD		mProtectTime;				// ����ʱ��
	bool		mIsProtected;				// �Ƿ��ܱ���
	GBitmap		*mpTileBmp;					// ��Ƭͼ��ָ��

protected:
	// ��⾫�����rcSprite�Ƿ����������[rw, col]��ͻ
	// ������������a, b��Ϊ-1, ���Ҫ����ض��ӿ�a, bΪ������(0, 1, 2, 3)
	bool collisionTest(int row, int col, RECT &rcSprite, int a, int b);

	// �ж��Ƿ���Ա����
	bool canShoot(int obj)
	{
		return obj==OBJ_BRICK || obj==OBJ_CONCRETE;
	}
	
	// �ж��Ƿ���Ա��ݻ�
	bool canDestroy(int row, int col, bool canDestroyConcrete)
	{
		return (mMap[row][col].mObj == OBJ_BRICK ||
			mMap[row][col].mObj == OBJ_CONCRETE && canDestroyConcrete);
	}	

public:
	GGameMap();
	virtual ~GGameMap();

	// ����λͼ��Դ
	void attachBmpRes(GBitmap *pBmp)
	{ mpTileBmp = pBmp; }

	// ʹ��ˮ��ǽ�����ʹ�
	void concreteProtect();

	// ʹ��שǽ�����ʹ�
	void brickProtect();

	// ��¶�ʹ�
	void bare();

	// ���ļ��ж�ȡ�ؿ���ͼ������Map
	bool load(const char* fileName);

	// ���Ƴ���ľ���������Ƭ����
	void draw(HDC dc);

	// ������ľ
	void drawTree(HDC dc);

	// ����ƶ������Ƿ����ͼTile������ͻ
	// ���û�г�ͻ����OBJ_NULL��OBJ_TREE
	int collisionTest(const GMoveableSprite &sprite);

	// �ӵ����Tile����, isShovelΪtrue����ݻ�ˮ��ǽ
	int shootObj(const GBullet &bullet, bool isPowerful=false);
};

#endif