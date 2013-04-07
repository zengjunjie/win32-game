/*******************************************************************************
本游戏源于课堂教学
*******************************************************************************/
#ifndef GBITMAP_H
#define GBITMAP_H

#include <windows.h>

// 图像类
class GBitmap
{
private:
	HBITMAP		mBmp;				    // 位图对象
	HBITMAP		mMask;				// 掩码位图对象
	int		mWidth, mHeight;	    // 图像尺寸
	bool		mIsTrans;		            	// 是否透明
	COLORREF	mTransColor;		// 透明颜色
	
public:
	GBitmap();
	GBitmap(const char *fileName, bool isTrans=false, COLORREF transCr=RGB(0, 0, 0));
	~GBitmap();

	void clear();
	
	HBITMAP		getBitmap()		{ return mBmp; }
	int			getWidth()		{ return mWidth; }
	int			getHeight()		{ return mHeight; }
	bool		isTrans()		{ return mIsTrans; }
	COLORREF	getTransColor() { return mTransColor; }

	void		setTrans(bool isTrans)		{ mIsTrans = isTrans; }
	void		setTransColor(COLORREF cr)	{ mTransColor = cr; }

	// 载入位图
	bool	load(const char *fileName);

	// 载入位图并设置透明
	bool	loadTransparent(const char *fileName, COLORREF transCr);

	// 创建掩码位图
	void	createMask();
	
	// 绘制图像
	void	draw(HDC dc, int dx=0, int dy=0, int dw=0, int dh=0, int sx=0, int sy=0);
};

#endif