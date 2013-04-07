#include "gbitmap.h"

GBitmap::GBitmap() :
			mBmp(NULL),
			mMask(NULL),
			mWidth(0),
			mHeight(0),
			mIsTrans(false),
			mTransColor(0)
{
}

GBitmap::GBitmap(const char *fileName, bool isTrans, COLORREF transCr):
			mBmp(NULL),
			mMask(NULL),
			mWidth(0),
			mHeight(0),
			mIsTrans(isTrans),
			mTransColor(transCr)
{
	load(fileName);
	if(mIsTrans) createMask();
}

GBitmap::~GBitmap()
{
	clear();
}

void GBitmap::clear()
{
	if(mBmp) DeleteObject(mBmp);
	mBmp = NULL;
	if(mMask) DeleteObject(mMask);
	mMask = NULL;
}

bool GBitmap::load(const char *fileName)
{
	clear();

	mBmp = (HBITMAP)LoadImage(
					NULL,
					fileName,
					IMAGE_BITMAP,
					0,
					0,
					LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if(!mBmp) return false;

	DIBSECTION		ds;
	GetObject(mBmp, sizeof(ds), &ds);
	mWidth = ds.dsBmih.biWidth;
	mHeight = ds.dsBmih.biHeight;

	return true;
}

bool GBitmap::loadTransparent(const char *fileName, COLORREF transCr)
{
	if(!load(fileName)) return false;

	mIsTrans = true;
	mTransColor = transCr;

	createMask();

	return true;
}

void GBitmap::createMask()
{
	if(!mBmp) return ;

	if(!mMask) DeleteObject(mMask);
	mMask = NULL;
	// 创建单色位图
	mMask = CreateBitmap(mWidth, mHeight, 1, 1, NULL);
	HDC bmpDC = CreateCompatibleDC(0);
	HDC maskDC = CreateCompatibleDC(0);
	SelectObject(bmpDC, mBmp);
	SelectObject(maskDC, mMask);
	
	SetBkColor(bmpDC, mTransColor);
	// 将Bmp图像拷贝到Mask
	BitBlt(maskDC, 0, 0, mWidth, mHeight, bmpDC, 0, 0, SRCCOPY);
	SetBkColor(bmpDC, RGB(0, 0, 0));
	SetTextColor(bmpDC, RGB(255, 255, 255));
	// 将Bmp图像和Mask进行按位与
	BitBlt(bmpDC, 0, 0, mWidth, mHeight, maskDC, 0, 0, SRCAND);
	
	DeleteDC(bmpDC);
	DeleteDC(maskDC);
}

void GBitmap::draw(HDC dc, int dx, int dy, int dw, int dh, int sx, int sy)
{
	if(!dw) dw = mWidth;
	if(!dh) dh = mHeight;
	
	HDC bmpDC = CreateCompatibleDC(dc);
	SelectObject(bmpDC, mBmp);
	
	if(mIsTrans)
	{
		HDC maskDC = CreateCompatibleDC(dc);
		SelectObject(maskDC, mMask);
		
		BitBlt(dc, dx, dy, dw, dh, maskDC, sx, sy, SRCAND);
		BitBlt(dc, dx, dy, dw, dh, bmpDC, sx, sy, SRCPAINT);
		
		DeleteDC(maskDC);
	}
	else BitBlt(dc, dx, dy, dw, dh, bmpDC, sx, sy, SRCCOPY);
	
	DeleteDC(bmpDC);
}
