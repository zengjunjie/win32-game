#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "gtankgame.h"

// ȫ�ֱ�������
HWND		hWnd = NULL;									// ���ھ��
TCHAR		szTitle[] = TEXT("Tank Game");					// ���ڱ���
TCHAR		szWindowClass[] = TEXT("TankGameApp");			// ��������
GTankGame	tankGame;										// ��Ϸ����

// ȫ�ֺ�������
ATOM				RegisterWndClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// ע�ᴰ��
	RegisterWndClass(hInstance);

	// ��ʼ��Ӧ�ó���
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	// ��ʼ������Ϸ
	if(!tankGame.init(hInstance, hWnd))
	{
		return -1;
	}

	return tankGame.run();
}

//  ע����Ϸ������
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

// ��ʼ��Ӧ�ó���
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

//  ���ڻص�����
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
