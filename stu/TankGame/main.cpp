#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "gtankgame.h"

// 全局变量定义
HWND		hWnd = NULL;									// 窗口句柄
TCHAR		szTitle[] = TEXT("Tank Game");					// 窗口标题
TCHAR		szWindowClass[] = TEXT("TankGameApp");			// 窗口类名
GTankGame	tankGame;										// 游戏对象

// 全局函数声明
ATOM				RegisterWndClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// 注册窗口
	RegisterWndClass(hInstance);

	// 初始化应用程序
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	// 开始进行游戏
	if(!tankGame.init(hInstance, hWnd))
	{
		return -1;
	}

	return tankGame.run();
}

//  注册游戏窗口类
ATOM RegisterWndClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon (NULL, IDI_APPLICATION);
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

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//  窗口回调函数
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_KEYDOWN:
		tankGame.onKeyDown(wParam);
		break ;
	case WM_KEYUP:
		tankGame.onKeyUp(wParam);
		break ;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
