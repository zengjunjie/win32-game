//:2013-3-8
//:

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "gbitmap.h"

// ȫ�ֱ�������
HWND	hWnd = NULL;								// ���ھ��
TCHAR	szTitle[] = TEXT("Tank Game");				// ���ڱ���
TCHAR	szWindowClass[] = TEXT("TankGameApp");		// ��������

// ȫ�ֺ�������
ATOM				RegisterWndClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

HBITMAP	 hBmp;					        // ͼ��
HBITMAP hMaskBmp;			    	// ����ͼ��
int		bmpWidth, bmpHeight;	// ͼ����
HBITMAP hTankBmp; 

HBITMAP hMemBmp;		    	   //����ͼ��
HDC     hMemDC;					   //����ͼ���ͼ���

int yPos[] = {290, 320};             //̹��ͼ�����λ��
int selIndex = 0 ;                      //��ǰѡ���±�

void UpdateFrame();                //

//  ע�ᴰ����
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
	if (!hWnd) return FALSE;

	//����˫����
	HDC hdc = GetDC(hWnd);						//����һ��˫����
	hMemDC = CreateCompatibleDC(hdc);
	hMemBmp = CreateCompatibleBitmap(hdc, 650, 500);
	SelectObject(hMemDC, hMemBmp);
	DeleteObject(hdc);
	
	// ����ͼ��
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

	//��������ͼ
	hMaskBmp = (HBITMAP)LoadImage(
					hInstance,
					"graphics\\player_mask.bmp",
					IMAGE_BITMAP,
					0,
					0,
					LR_LOADFROMFILE | LR_CREATEDIBSECTION);
   //����̹��ԭͼ
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

//  ���ڻص�����
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
		SelectObject(hMemDC, hbr);	//ѡ�񻺴�д��ͼ��
		FillRect(hMemDC, &rc, hbr);//��
		DeleteObject(hbr);

		bmpDC = CreateCompatibleDC(hdc);
		SelectObject(bmpDC, hBmp);
		// ��ͼ������������
		x = (rc.right-bmpWidth) / 2;
		y = (rc.bottom-bmpHeight) / 2;
		BitBlt(hMemDC, x, y, bmpWidth, bmpHeight, 
				bmpDC, 0, 0, SRCCOPY);

		SelectObject(bmpDC, hTankBmp);
		x = 205;
		y = 290;
		BitBlt(hMemDC, x, y, 28, 28,
				bmpDC, 0, 28, SRCCOPY);

		//��ͼ�񻺴浽��Ļ��
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
    // ע�ᴰ��
    RegisterWndClass(hInstance);
    // ��ʼ��Ӧ�ó���
    if (!InitInstance(hInstance, nCmdShow)) 
    {
        return FALSE;
    }
    // ��Ϣѭ��
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
		SelectObject(hMemDC, hbr);	//ѡ�񻺴�д��ͼ��
		FillRect(hMemDC, &rc, hbr);//��ˢ������
		DeleteObject(hbr);

		bmpDC = CreateCompatibleDC(hdc);
		SelectObject(bmpDC, hBmp);
		// ��ͼ������������
		x = (rc.right-bmpWidth) / 2;
		y = (rc.bottom-bmpHeight) / 2;
		BitBlt(hMemDC, x, y, bmpWidth, bmpHeight, 
				bmpDC, 0, 0, SRCCOPY);
		//������
		SelectObject(bmpDC, hMaskBmp);
		x = 205;
		//y = 290;
	    y = yPos[selIndex];
		BitBlt(hMemDC, x, y, 28, 28,
				bmpDC, 0, 28, SRCAND);

		//��ԭͼ
		SelectObject(bmpDC, hTankBmp);
		x = 205;
		//y = 290;
	    y = yPos[selIndex];
		BitBlt(hMemDC, x, y, 28, 28,
				bmpDC, 0, 28, SRCPAINT);

		//��ͼ�񻺴浽��Ļ��
		BitBlt(hdc, 0, 0, 640, 480,
				hMemDC, 0, 0, SRCCOPY);


		DeleteObject(bmpDC);
		DeleteObject(hdc);
		//EndPaint(hWnd, &ps);
		//break ;

}

