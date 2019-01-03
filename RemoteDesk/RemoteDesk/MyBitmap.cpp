#include "stdafx.h"
#include "MyBitmap.h"


CMyBitmap::CMyBitmap()
{
}


CMyBitmap::~CMyBitmap()
{
}


/**
*	@brief	�õ�����ͼ�񣬼ǵ�delete
*
*	@return	HBITMAP	λͼͼ����
*/
HBITMAP CMyBitmap::GetDesktopBitMap()
{
	// �õ���������DC
	HWND hWndDesk = ::GetDesktopWindow();
	HDC hDcDesk = ::GetDC(hWndDesk);
	// ��������λͼ
	HBITMAP hBitMap = ::CreateCompatibleBitmap(hDcDesk, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	// ����λͼ��Ϣ
	BITMAP bmDesktop;
	::GetObject(hBitMap, sizeof(BITMAP), &bmDesktop);

	// ��������DC
	HDC hDcMem = ::CreateCompatibleDC(hDcDesk);
	// ��λͼѡ��DC
	HBITMAP hOldBmp = (HBITMAP)::SelectObject(hDcMem, hBitMap);
	// ������DC���Ƶ�����DC
	::BitBlt(hDcMem, 0, 0, bmDesktop.bmWidth, bmDesktop.bmHeight, hDcDesk, 0, 0, SRCCOPY);
	hBitMap = (HBITMAP)::SelectObject(hDcMem, hOldBmp);

	::DeleteObject(hOldBmp);
	::ReleaseDC(NULL, hDcMem);
	::ReleaseDC(NULL, hDcDesk);
	return hBitMap;
}


/**
*	@brief	�õ�����DC������ǵ�delete
*
*	@return	HDC	��������ͼ����豸�����ľ��
*/
HDC CMyBitmap::GetDesktopImageDC()
{
	HWND hWndDesk = ::GetDesktopWindow();
	HDC hDcDesk = ::GetDC(hWndDesk);
	HBITMAP hBitMap = ::CreateCompatibleBitmap(hDcDesk, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

	BITMAP bmDesktop;
	::GetObject(hBitMap, sizeof(BITMAP), &bmDesktop);

	HDC hdcTmp = ::CreateCompatibleDC(hDcDesk);
	::SelectObject(hdcTmp, hBitMap);
	::BitBlt(hdcTmp, 0, 0, bmDesktop.bmWidth, bmDesktop.bmHeight, hDcDesk, 0, 0, SRCCOPY);

	::DeleteObject(hBitMap);
	::ReleaseDC(NULL, hDcDesk);
	return hdcTmp;
}


/**
*	@brief	����λͼΪ�ļ�
*
*	@param	hBitmap		λͼ�ļ����
*	@param	FileName	�ļ���
*	@return	BOOL		�Ƿ�ɹ����ɹ�����TRUE����FALSE
*/
BOOL CMyBitmap::SaveBmp(HBITMAP hBitmap, WCHAR* lpszFileName)
{
	if (hBitmap == NULL || lpszFileName == NULL)
	{
		MessageBox(NULL, L"��������", L"Error", MB_OK);
		return false;
	}
	HDC hDC;
	//��ǰ�ֱ�����ÿ������ռ�ֽ���
	int iBits;
	//λͼ��ÿ������ռ�ֽ���
	WORD wBitCount;
	//�����ɫ���С�� λͼ�������ֽڴ�С ��λͼ�ļ���С �� д���ļ��ֽ��� 
	DWORD dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	//λͼ���Խṹ 
	BITMAP Bitmap;
	//λͼ�ļ�ͷ�ṹ
	BITMAPFILEHEADER bmfHdr;
	//λͼ��Ϣͷ�ṹ 
	BITMAPINFOHEADER bi;
	//ָ��λͼ��Ϣͷ�ṹ  
	LPBITMAPINFOHEADER lpbi;
	//�����ļ��������ڴ�������ɫ���� 
	HANDLE fh, hDib, hPal, hOldPal = NULL;
	//����λͼ�ļ�ÿ��������ռ�ֽ��� 
	hDC = ::CreateDC(L"DISPLAY", NULL, NULL, NULL);
	iBits = ::GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	::DeleteDC(hDC);
	if (iBits <= 1)  wBitCount = 1;
	else if (iBits <= 4)  wBitCount = 4;
	else if (iBits <= 8)  wBitCount = 8;
	else if (iBits <= 24) wBitCount = 24;
	else wBitCount = 32;
	::GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 0;

	dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	//Ϊλͼ���ݷ����ڴ� 
	hDib = ::GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock(hDib);
	*lpbi = bi;

	// �����ɫ��  
	hPal = ::GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		::RealizePalette(hDC);
	}

	// ��ȡ�õ�ɫ�����µ�����ֵ 
	::GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)
		+ dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);

	//�ָ���ɫ��  
	if (hOldPal)
	{
		::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		::RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}

	//����λͼ�ļ�  
	fh = ::CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)  return FALSE;

	// ����λͼ�ļ�ͷ 
	bmfHdr.bfType = 0x4D42; // "BM" 
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
	// д��λͼ�ļ�ͷ 
	::WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	// д��λͼ�ļ��������� 
	::WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	//���  
	::GlobalUnlock(hDib);
	::GlobalFree(hDib);
	::CloseHandle(fh);
	return TRUE;
}


/**
*	@brief	����λͼ�����а�
*
*	@param	hWnd	���ھ��
*	@param	hBitmap	λͼ���
*	@retuen	BOOL	�ɹ� TRUE, ʧ�� FALSE
*/
BOOL CMyBitmap::SaveToClipboard(HWND hWnd, HBITMAP hBitmap)
{
	if (OpenClipboard(hWnd))
	{
		//��ռ�����
		EmptyClipboard();
		//����Ļ����ճ������������,
		//hBitmap Ϊ�ղŵ���Ļλͼ���
		SetClipboardData(CF_BITMAP, hBitmap);
		//�رռ�����
		CloseClipboard();
		return TRUE;
	}
	return FALSE;
}


/**
*	@brief	����һ��λͼ���ǵ�delete
*
*	@param	hDc			��λͼ���ݵ�Dc���
*	@param	hSourceBmp	λͼ�����Ϊ����Դ
*	@return	HBITMAP		λͼ���
*/
HBITMAP CMyBitmap::CopyBitmap(HDC hDc, HBITMAP hSourceBmp)
{
	if (hSourceBmp == NULL)
	{
		MessageBox(NULL, L"Copy null error", L"Copy error", MB_OK);
		return NULL;
	}
	HDC hDcSource = CreateCompatibleDC(hDc);
	HDC hDcDest = CreateCompatibleDC(hDc);
	BITMAP bmp;
	GetObject(hSourceBmp, sizeof(bmp), &bmp);

	HBITMAP hResBmp = CreateCompatibleBitmap(hDc, bmp.bmWidth, bmp.bmHeight);
	HBITMAP hOldBmpSource = (HBITMAP)SelectObject(hDcSource, hSourceBmp);
	HBITMAP hOldBmpDest = (HBITMAP)SelectObject(hDcDest, hResBmp);
	BitBlt(hDcDest, 0, 0, bmp.bmWidth, bmp.bmHeight, hDcSource, 0, 0, SRCCOPY);

	hResBmp = (HBITMAP)SelectObject(hDcDest, hOldBmpDest);
	SelectObject(hDcSource, hOldBmpSource);

	DeleteDC(hDcSource);
	DeleteDC(hDcDest);
	DeleteObject(hOldBmpSource);
	DeleteObject(hOldBmpDest);
	return hResBmp;
}


/**
*	@brief	��ð���������λͼ���,�ǵ�delete

*	@param	hDc		��λͼ���ݵ�DC�ľ��
*	@param	hBitmap	λͼ���
*	@return	HBITMAP	λͼ���
*/
HBITMAP CMyBitmap::GetDarkBitmap(HDC hDc, HBITMAP hBitmap)
{
	if (hBitmap == NULL || hDc == NULL)
	{
		return NULL;
	}
	BITMAP bmp;
	BITMAPINFO bmpInfo;
	UINT* pData;
	HBITMAP hResBmp;

	GetObject(hBitmap, sizeof(bmp), &bmp);
	pData = new UINT[bmp.bmWidth * bmp.bmHeight];
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = bmp.bmWidth;
	bmpInfo.bmiHeader.biHeight = -bmp.bmHeight;	// ������
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	bmpInfo.bmiHeader.biBitCount = 32;

	GetDIBits(hDc, hBitmap, 0, bmp.bmHeight, pData, &bmpInfo, DIB_RGB_COLORS);
	UINT color, r, g, b;
	for (int i = 0; i < bmp.bmWidth * bmp.bmHeight; i++)
	{
		color = pData[i];
		b = (UINT)((color << 8 >> 24) * 0.6);
		g = (UINT)((color << 16 >> 24) * 0.6);
		r = (UINT)((color << 24 >> 24) * 0.6);
		//note   that   infact,   the   r   is   Blue,   b   =   Red,
		//r   =   0;//mask   the   blue   bits
		pData[i] = RGB(r, g, b);
	}
	hResBmp = CreateCompatibleBitmap(hDc, bmp.bmWidth, bmp.bmHeight);
	SetDIBits(hDc, hResBmp, 0, bmp.bmHeight, pData, &bmpInfo, DIB_RGB_COLORS);

	delete[] pData;
	return hResBmp;
}
