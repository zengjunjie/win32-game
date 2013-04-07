//:2013-3-8
//:

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "gbitmap.h"

// 全局变量定义
HWND	hWnd = NULL;								// 窗口句柄
TCHAR	szTitle[] = TEXT("Tank Game");				// 窗口标题
TCHAR	szWindowClass[] = TEXT("TankGameApp");		// 窗口类名

// 全局函数声明
ATOM				RegisterWndClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

HBITMAP	 hBmp;					        // 图像
HBITMAP hMaskBmp;			    	// 掩码图像
int		bmpWidth, bmpHeight;	// 图像宽高
HBITMAP hTankBmp; 

HBITMAP hMemBmp;		    	   //缓存图像
HDC     hMemDC;					   //缓存图像绘图句柄

int yPos[] = {290, 320};             //坦克图像绘制位置
int selIndex = 0 ;                      //当前选中下标

void UpdateFrame();                //

//  注册窗口类
ATOM RegisterWndClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);
	return RegisterClassEx(&wcex);
}

// 初始化应用程序
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hWnd = CreateWindow(szWindowClass, szTitle, 
						WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
						CW_USEDEFAULT, 
						CW_USEDEFAULT, 
						646, 495,
						NULL, 
						NULL, 
						hInstance, 
						NULL);
	if (!hWnd) return FALSE;

	//创建双缓存
	HDC hdc = GetDC(hWnd);						//声明一个双缓存
	hMemDC = CreateCompatibleDC(hdc);
	hMemBmp = CreateCompatibleBitmap(hdc, 650, 500);
	SelectObject(hMemDC, hMemBmp);
	DeleteObject(hdc);
	
	// 载入图像
	hBmp = (HBITMAP)LoadImage(
					hInstance,
					"graphics\\splash.bmp",
					IMAGE_BITMAP,
					0,
					0,
					LR_LOADFROMFILE | LR_CREATEDIBSECTION);

	

	DIBSECTION		ds;
	GetObject(hBmp, sizeof(ds), &ds);
	bmpWidth = ds.dsBmih.biWidth;
	bmpHeight = ds.dsBmih.biHeight;

	//载入掩码图
	hMaskBmp = (HBITMAP)LoadImage(
					hInstance,
					"graphics\\player_mask.bmp",
					IMAGE_BITMAP,
					0,
					0,
					LR_LOADFROMFILE | LR_CREATEDIBSECTION);
   //载入坦克原图
	hTankBmp = (HBITMAP)LoadImage(
					hInstance,
					"graphics\\player1.bmp",
					IMAGE_BITMAP,
					0,
					0,
					LR_LOADFROMFILE | LR_CREATEDIBSECTION);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	return TRUE;
}

//  窗口回调函数
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
    //case WM_PAINT:
	//	UpdateFrame();
	//	break;
	case WM_ERASEBKGND:
		return 0;

	case WM_KEYDOWN:
		if(wParam == VK_UP || wParam == VK_DOWN || wParam == 'W' || wParam == 'S') {
			selIndex = !selIndex;
			//InvalidateRect(hWnd, NULL, false);
		}
		break;

    case WM_LBUTTONDOWN:
		selIndex = !selIndex;
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;



	/*	PAINTSTRUCT ps;
		HDC hdc, bmpDC;
		HBRUSH hbr;
		RECT rc;
		int x, y;
		hdc = GetDC(hWnd);
		GetClientRect(hWnd, &rc);
		hbr = CreateSolidBrush(RGB(0, 0, 0));
		SelectObject(hMemDC, hbr);	//选择缓存写入图像
		FillRect(hMemDC, &rc, hbr);//将
		DeleteObject(hbr);

		bmpDC = CreateCompatibleDC(hdc);
		SelectObject(bmpDC, hBmp);
		// 将图像贴到窗口上
		x = (rc.right-bmpWidth) / 2;
		y = (rc.bottom-bmpHeight) / 2;
		BitBlt(hMemDC, x, y, bmpWidth, bmpHeight, 
				bmpDC, 0, 0, SRCCOPY);

		SelectObject(bmpDC, hTankBmp);
		x = 205;
		y = 290;
		BitBlt(hMemDC, x, y, 28, 28,
				bmpDC, 0, 28, SRCCOPY);

		//将图像缓存到屏幕上
		BitBlt(hdc, 0, 0, 640, 480,
				hMemDC, 0, 0, SRCCOPY);


		DeleteObject(bmpDC);
		DeleteObject(hdc);
		//EndPaint(hWnd, &ps);
		break ;*/
	

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG	msg;
    // 注册窗口
    RegisterWndClass(hInstance);
    // 初始化应用程序
    if (!InitInstance(hInstance, nCmdShow)) 
    {
        return FALSE;
    }
    // 消息循环
	while(TRUE) {
		if(PeekMessage(&msg, NULL, 0, 0 , PM_NOREMOVE)) {
			if(!GetMessage(&msg, NULL, 0, 0))
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			UpdateFrame();
			Sleep(1);
		}

	}
    /*while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg); 
        DispatchMessage(&msg);
    }*/
    return msg.wParam;
}

void UpdateFrame() {
	    HDC hdc, bmpDC;
		HBRUSH hbr;
		//PAINTSTRUCT ps;
		RECT rc;
		int x, y;
		hdc = GetDC(hWnd);
		GetClientRect(hWnd, &rc);
		hbr = CreateSolidBrush(RGB(0, 0, 0));
		SelectObject(hMemDC, hbr);	//选择缓存写入图像
		FillRect(hMemDC, &rc, hbr);//画刷填充矩形
		DeleteObject(hbr);

		bmpDC = CreateCompatibleDC(hdc);
		SelectObject(bmpDC, hBmp);
		// 将图像贴到窗口上
		x = (rc.right-bmpWidth) / 2;
		y = (rc.bottom-bmpHeight) / 2;
		BitBlt(hMemDC, x, y, bmpWidth, bmpHeight, 
				bmpDC, 0, 0, SRCCOPY);
		//贴掩码
		SelectObject(bmpDC, hMaskBmp);
		x = 205;
		//y = 290;
	    y = yPos[selIndex];
		BitBlt(hMemDC, x, y, 28, 28,
				bmpDC, 0, 28, SRCAND);

		//贴原图
		SelectObject(bmpDC, hTankBmp);
		x = 205;
		//y = 290;
	    y = yPos[selIndex];
		BitBlt(hMemDC, x, y, 28, 28,
				bmpDC, 0, 28, SRCPAINT);

		//将图像缓存到屏幕上
		BitBlt(hdc, 0, 0, 640, 480,
				hMemDC, 0, 0, SRCCOPY);


		DeleteObject(bmpDC);
		DeleteObject(hdc);
		//EndPaint(hWnd, &ps);
		//break ;

}

