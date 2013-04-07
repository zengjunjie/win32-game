/*******************************************************************************
����ϷԴ�ڿ��ý�ѧ
*******************************************************************************/
#ifndef GBITMAP_H
#define GBITMAP_H

#include <windows.h>

// ͼ����
class GBitmap
{
private:
	HBITMAP		mBmp;				    // λͼ����
	HBITMAP		mMask;				// ����λͼ����
	int		mWidth, mHeight;	    // ͼ��ߴ�
	bool		mIsTrans;		            	// �Ƿ�͸��
	COLORREF	mTransColor;		// ͸����ɫ
	
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

	// ����λͼ
	bool	load(const char *fileName);

	// ����λͼ������͸��
	bool	loadTransparent(const char *fileName, COLORREF transCr);

	// ��������λͼ
	void	createMask();
	
	// ����ͼ��
	void	draw(HDC dc, int dx=0, int dy=0, int dw=0, int dh=0, int sx=0, int sy=0);
};

#endif