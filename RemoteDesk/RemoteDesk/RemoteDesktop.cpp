// RemoteDesk.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <stdio.h>
#include "RemoteDesktop.h"
#include "RemoteDeskControl.h"
#include <CommCtrl.h>
#include "LanRemoteDesktopControl.h"
#include "PublicString.h"

#define MAX_LOADSTRING 100
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400

#define ID_EDIT_ID	2000
#define ID_EDIT_PW	2001
#define ID_BTN_OK	2002

#define COLOR_EDIT RGB(18, 32, 61)
#define COLOR_BTN RGB(255,102,0)


// ȫ�ֱ���: 
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

HWND g_hWnd;
HWND g_hRemoteWnd;
HWND g_hEditId;
HWND g_hEditPw;
HWND g_hBtnOk;

TCHAR szRemoteWindowClass[MAX_LOADSTRING] = L"MyRemoteWindow";
CRemoteDeskControl* g_pRemoteControl = NULL;
char g_szPassword[8] = { 0 };
int g_nId = 0;
bool g_bIsLan = false;
bool g_bIsControling = false;


CLanRemoteDesktopControl* g_pLanControl = NULL;

// �˴���ģ���а����ĺ�����ǰ������: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

// �Զ��庯������
ATOM				MyRegisterRemoteWindowClass(HINSTANCE hInstance);
LRESULT CALLBACK	RemoteWndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO:  �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_REMOTEDESK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	MyRegisterRemoteWindowClass(hInstance);

	// ִ��Ӧ�ó����ʼ��: 
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REMOTEDESK));

	// ����Ϣѭ��: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  ����:  MyRegisterClass()
//
//  Ŀ��:  ע�ᴰ���ࡣ
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REMOTEDESK));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCE(IDC_REMOTEDESK);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


ATOM MyRegisterRemoteWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = RemoteWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REMOTEDESK));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCE(IDC_REMOTEDESK);
	wcex.lpszClassName = szRemoteWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   ����:  InitInstance(HINSTANCE, int)
//
//   Ŀ��:  ����ʵ�����������������
//
//   ע��: 
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW^WS_THICKFRAME^WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

VOID InitRemoteControl()
{
	//g_pRemoteControl = new CRemoteDeskControl("119.29.160.95", 18888);
	g_pRemoteControl = new CRemoteDeskControl("192.168.226.1", 18888);
	g_pLanControl = new CLanRemoteDesktopControl();
	g_pLanControl->SetJpegQuality(70);
}

VOID DrawUserInfo()
{
	HDC hDc = GetDC(g_hWnd);
	SetBkMode(hDc, TRANSPARENT);
	SetTextColor(hDc, RGB(0, 0, 0));
	if (g_nId != 0)
	{
		char Buf[20];
		_itoa_s(g_nId, Buf, 10);
		SetTextColor(hDc, RGB(255, 255, 255));
		TextOutA(hDc, 10, 10, Buf, strlen(Buf));
		TextOutA(hDc, 10, 30, g_szPassword, strlen(g_szPassword));
	}
	else
	{
		// TextOutW(hDc, 10, 10, L"�����������", 7);
	}
}

void ThreadFuncGetUserInfo(void *)
{
	while (true)
	{
		if (g_nId != 0)
		{
			break;
		}
		g_pRemoteControl->SendOnlineData();
		g_nId = g_pRemoteControl->GetId();
		strcpy_s(g_szPassword, g_pRemoteControl->GetPassword());
		Sleep(100);
	}
	DrawUserInfo();
}

// ��ʼ���ؼ�
void InitControl()
{
	DWORD dwStyle = ES_LEFT  | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER;
	DWORD dwExStyle = WS_TABSTOP;
	g_hEditId = ::CreateWindowEx(
		dwExStyle,			//dwExStyle ��չ��ʽ
		L"Edit",			//lpClassName ��������
		NULL,				//lpWindowName ���ڱ���
		dwStyle,			//dwStyle ������ʽ
		199,				//x ���λ��
		160,				//y ����λ��
		200,				//nWidth ���
		25,					//nHeight �߶�
		g_hWnd,				//hWndParent �����ھ��
		(HMENU)ID_EDIT_ID,	//hMenu �˵����
		NULL,				//hInstance Ӧ�ó�����
		NULL				//lpParam ���Ӳ���
		);
	dwStyle = ES_LEFT | ES_PASSWORD | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER;
	g_hEditPw = ::CreateWindowEx(
		dwExStyle,			//dwExStyle ��չ��ʽ
		L"Edit",			//lpClassName ��������
		NULL,				//lpWindowName ���ڱ���
		dwStyle,			//dwStyle ������ʽ
		199,				//x ���λ��
		200,				//y ����λ��
		200,				//nWidth ���
		25,					//nHeight �߶�
		g_hWnd,				//hWndParent �����ھ��
		(HMENU)ID_EDIT_PW,	//hMenu �˵����
		NULL,				//hInstance Ӧ�ó�����
		NULL				//lpParam ���Ӳ���
	);
	dwStyle = BS_PUSHBUTTON | BS_TEXT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE;
	dwExStyle = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR;
	TCHAR szWindowName[MAX_LOADSTRING] = L"ȷ��";
	g_hBtnOk = ::CreateWindowEx(
		dwExStyle,			//dwExStyle ��չ��ʽ
		L"Button",			//lpClassName ��������
		szWindowName,		//lpWindowName ���ڱ���
		dwStyle,			//dwStyle ������ʽ
		239,				//x ���λ��
		250,				//y ����λ��
		120,				//nWidth ���
		25,					//nHeight �߶�
		g_hWnd,				//hWndParent �����ھ��
		(HMENU)ID_BTN_OK,	//hMenu �˵����
		NULL,				//hInstance Ӧ�ó�����
		NULL				//lpParam ���Ӳ���
		);
	SetWindowLong(g_hBtnOk, GWL_STYLE, GetWindowLong(g_hBtnOk, GWL_STYLE) | BS_OWNERDRAW | BS_FLAT);

	SetWindowTextA(g_hEditId, "192.168.226.133");
	SetWindowTextA(g_hEditPw, "666");
}

//
//  ����:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��:    ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		g_hWnd = hWnd;
		InitControl();
		InitRemoteControl();
		_beginthread(ThreadFuncGetUserInfo, 0, NULL);
		break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		// TODO:  �ڴ���������ͼ����...
		HBITMAP hBitMap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		HDC hMemDc = CreateCompatibleDC(hdc);
		SelectObject(hMemDc, hBitMap);
		BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hMemDc, 0, 0, SRCCOPY);
		DeleteDC(hMemDc);
		DeleteObject(hBitMap);
		//DrawUserInfo();
		EndPaint(hWnd, &ps);
	}
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case ID_BTN_OK:
			if (g_bIsControling)
			{
				break;
			}
			char szBufId[30], szBufPw[8];
			GetWindowTextA(g_hEditId, szBufId, 30);
			GetWindowTextA(g_hEditPw, szBufPw, 7);
			if (strlen(szBufPw) == 0 || strlen(szBufId) == 0)
			{
				MessageBox(hWnd, L"���벻�Ϸ�", L"��ʾ", MB_OK);
				break;
			}
			if (IsValidIp(szBufId))
			{
				g_bIsLan = true;
				g_hRemoteWnd = CreateWindow(szRemoteWindowClass, L"Զ������",
					WS_OVERLAPPEDWINDOW, 500, 500, 500, 500, NULL, NULL, NULL, 0);
				g_pLanControl->SetDrawHwnd(g_hRemoteWnd);
				g_pLanControl->ControlLanRemoteDesk(szBufId, szBufPw);
				g_bIsControling = true;
			}
			else if (IsValidId(szBufId))
			{
				g_bIsLan = false;
				g_hRemoteWnd = CreateWindow(szRemoteWindowClass, L"Զ������",
					WS_OVERLAPPEDWINDOW, 500, 500, 500, 500, NULL, NULL, NULL, 0);
				g_pRemoteControl->SetDrawTargetWindow(g_hRemoteWnd);
				g_pRemoteControl->ControlRemoteClient(atoi(szBufId), szBufPw);
				g_bIsControling = true;
				
				/*g_pLanControl->SetDrawHwnd(g_hRemoteWnd);
				g_pLanControl->ControlLanRemoteDesk("192.168.226.133", "666");*/
			}
			else
			{
				MessageBox(hWnd, L"���벻�Ϸ�", L"��ʾ", MB_OK);
				break;
				break;
			}
			//g_pRemoteControl->ControlRemoteClient(atoi(szBufId), szBufPw);
			break;
		default:
			break;
		}
		break;

	case WM_CTLCOLOREDIT:
	{
		HWND hWndParam = (HWND)lParam;
		if (hWndParam == g_hEditId || hWndParam == g_hEditPw)
		{
			HDC hdc = (HDC)wParam;
			HBRUSH hbrush;
			/* ������ˢ */
			hbrush = CreateSolidBrush(COLOR_EDIT);
			SetBkColor(hdc, COLOR_EDIT);
			SetTextColor(hdc, RGB(255, 255, 255));
			/*
			** ����WM_CTLCOLOREDIT��Ϣ���뷵��һ����ˢ
			** windowsʹ�������ˢ����edit�ؼ��ı���
			** ע��������������ֱ�����ɫ��ͬ
			*/
			return (LRESULT)hbrush;
		}
	}
		break;
	case WM_CTLCOLORBTN:
	{
		HWND hWndParam = (HWND)lParam;
		if (hWndParam == g_hBtnOk)
		{
			HDC hdc = (HDC)wParam;
			HBRUSH hbrush;
			RECT rc;
			TCHAR text[64];
			GetWindowText(g_hBtnOk, text, 63);
			GetClientRect(g_hBtnOk, &rc);
			SetTextColor(hdc, RGB(255, 255, 255));
			SetBkMode(hdc, TRANSPARENT);
			DrawText(hdc, text, _tcslen(text), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			SetBkColor(hdc, COLOR_BTN);
			hbrush = CreateSolidBrush(COLOR_BTN);
			return (LRESULT)hbrush;
		}
	}
		break;
	case WM_DESTROY:
		//delete g_pRemoteControl;
		g_pLanControl->EndClientConnection();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Զ�̿��ƴ��ڵ���Ϣ������
LRESULT CALLBACK RemoteWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	INPUT input = { 0 };

	switch (message)
	{
	case WM_CREATE:
		g_hRemoteWnd = hWnd;
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO:  �ڴ���������ͼ����...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		if (g_bIsLan)
		{
			g_pLanControl->EndClientConnection();
			g_bIsControling = false;
		}
		else
		{

		}
		DestroyWindow(hWnd);
		break;
	case WM_KEYDOWN:
		g_pLanControl->SendKeyDown((int)wParam);
		break;
	case WM_KEYUP:
		g_pLanControl->SendKeyUp((int)wParam);
		break;
	case WM_MOUSEMOVE:
		g_pLanControl->SendMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONDOWN:
		g_pLanControl->SendLeftButtonDown();
		break;
	case WM_LBUTTONUP:
		g_pLanControl->SendLeftButtonUp();
		break;
	case WM_LBUTTONDBLCLK:
		g_pLanControl->SendLeftButtonDoubleClick();
		break;
	case WM_RBUTTONDOWN:
		g_pLanControl->SendRightButtonDown();
		break;
	case WM_RBUTTONUP:
		g_pLanControl->SendRightButtonUp();
		break;
	case WM_RBUTTONDBLCLK:
		g_pLanControl->SendRightButtonDoubleClick();
		break;
	case WM_MBUTTONDOWN:
		g_pLanControl->SendMiddleButtonDown();
		break;
	case WM_MBUTTONUP:
		g_pLanControl->SendMiddleButtonUp();
		break;
	case WM_MBUTTONDBLCLK:
		g_pLanControl->SendMiddleButtonDoubleClick();
		break;
	case WM_MOUSEWHEEL:
		g_pLanControl->SendMouseWheel((short)HIWORD(wParam));
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


















// �����ڡ������Ϣ�������
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
