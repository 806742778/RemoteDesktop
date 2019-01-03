#include "stdafx.h"
#include "RemoteDeskControl.h"
#include "Adapter.h"
#include "PublicString.h"
#include "MyBitmap.h"
#include "CxImage/include/ximage.h"

#pragma comment(lib,"CxImage/lib/cximage.lib")

//#ifdef _DEBUG
//#ifdef _UNICODE
//#pragma comment(lib,"CxImage/lib/cximagedu.lib")
//#else
//#pragma comment(lib,"CxImage/lib/cximaged.lib")
//#endif // _UNICODE
//#else
//#ifdef _UNICODE
//#pragma comment(lib,"CxImage/lib/cximageu.lib")
//#else
//#pragma comment(lib,"CxImage/lib/cximage.lib")
//#endif // _UNICODE
//#endif	// _DEBUG


CRemoteDeskControl::CRemoteDeskControl(const char* lpszServerIP, const u_short& uServerPort)
{
	this->m_nId = 0;
	this->m_bIsControler = false;	//	Ĭ���Ǳ��˿��Ƶ�
	memset(this->m_szPassword, 0, sizeof(this->m_szPassword));
	this->m_bIsReady = false;
	this->m_byJpegQuality = 30;
	this->m_nCurrentImageSize = 0;
	this->m_nCurrentIndex = 0;
	this->m_bIsEnd = false;
	this->m_bIsEndRecvServer = true;

	InitFlag();

	memset(m_szTargetIp, 0, sizeof(m_szTargetIp));
	m_nTargetPort = 0;

	strcpy_s(this->m_szServerIp, lpszServerIP);
	this->m_nServerPort = uServerPort;
	InitSocket(lpszServerIP, uServerPort);

	std::string strMac = GetMacAddress();
	strcpy_s(m_szMac, strMac.c_str());

	StartRecvMsg();
	SendOnlineData();
	StartHeartbeat();
}



CRemoteDeskControl::~CRemoteDeskControl()
{
	if (m_bIsSocketOk)
	{
		WSACleanup();
		closesocket(m_socketClient);
	}
}

void CRemoteDeskControl::SendHeartbeatData()
{
	char bufMsg[20];
	sprintf_s(bufMsg, "#%s#%d", CONTROL_DATA_HEART, this->m_nId);
	this->m_pUdpHeartbeat->SendToServer(bufMsg);
}

int CRemoteDeskControl::GetId()
{
	return this->m_nId;
}

char* CRemoteDeskControl::GetPassword()
{
	return this->m_szPassword;
}

// ����Լ������Ϣ��ʽ�ı�ʾ���ߵ���Ϣ
int CRemoteDeskControl::SendOnlineData()
{
	char buf[30];
	sprintf_s(buf, "#%s#%s", CONTROL_DATA_ONLINE, m_szMac);
	return sendto(m_socketClient, buf, strlen(buf), 0, (SOCKADDR*)&m_addrServer, sizeof(SOCKADDR));
}

void CRemoteDeskControl::ControlRemoteClient(const int& m_nId, const char* lpszPassword)
{
	char bufMsg[30];
	sprintf_s(bufMsg, "#%s#%d#%d#%s", CONTROL_DATA_CONTROL, this->m_nId, m_nId, lpszPassword);
	sendto(m_socketClient, bufMsg, strlen(bufMsg), 0, (SOCKADDR*)&m_addrServer, sizeof(SOCKADDR));
	this->m_bIsControler = true;
}

void CRemoteDeskControl::SetDrawTargetWindow(HWND hWnd)
{
	this->m_hWnd = hWnd;
}

void CRemoteDeskControl::EndControl()
{
	char szBufMsg[5];
	sprintf_s(szBufMsg, "#%s", CONTROL_DATA_BYE);
	sendto(m_socketClient, szBufMsg, strlen(szBufMsg), 0, (SOCKADDR*)&m_addrServer, sizeof(SOCKADDR));
	this->m_bIsEnd = true;
	this->m_bIsGo = false;
	this->m_bIsControler = false;
}

bool CRemoteDeskControl::GetIsEnd()const
{
	return this->m_bIsEnd;
}

bool CRemoteDeskControl::GetIsOk()const
{
	return m_bIsOk;
}

bool CRemoteDeskControl::GetIsEndRecvServer() const
{
	return this->m_bIsEndRecvServer;
}

void CRemoteDeskControl::SetJpegQuality(BYTE q)
{
	if (q >=0 && q <= 100)
	{
		this->m_byJpegQuality = q;
	}
}

BYTE CRemoteDeskControl::GetJpegQuality() const
{
	return this->m_byJpegQuality;
}

void CRemoteDeskControl::InitFlag()
{
	this->m_bIsGo = false;
	this->m_bIsEnd = false;
	this->m_bIsOk = false;
}

void CRemoteDeskControl::InitSocket(const char* lpszServerIP, const u_short& uServerPort)
{
	m_bIsSocketOk = false;
	m_bIsSocketOk = false;
	int ret;
	ret = WSAStartup(MAKEWORD(2, 2), &m_wsad);
	if (ret != 0)
	{
		WSACleanup();
		return;
	}
	m_socketClient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socketClient == INVALID_SOCKET)
	{
		WSACleanup();
		return;
	}
	m_addrServer.sin_addr.S_un.S_addr = inet_addr(lpszServerIP);
	m_addrServer.sin_family = AF_INET;
	m_addrServer.sin_port = htons(uServerPort);

	// �ı�Ϊ������ģʽ
	unsigned long nMode = 1;
	ret = ioctlsocket(m_socketClient, FIONBIO, &nMode);
	if (ret == SOCKET_ERROR)
	{
		WSACleanup();
		closesocket(m_socketClient);
	}

	m_bIsSocketOk = true;
}

// ���յ���Ϣ�͸���һ����Ա��������
int CRemoteDeskControl::OnRecv()
{
	memset(m_szRecvBuf, 0, sizeof(m_szRecvBuf));
	int nLen = sizeof(SOCKADDR_IN);
	int ret = recvfrom(m_socketClient, m_szRecvBuf, sizeof(m_szRecvBuf), 0, (SOCKADDR*)&m_addrServer, &nLen);
	if (ret != SOCKET_ERROR)
	{
		OnRecvMsg(m_szRecvBuf);
	}
	Sleep(20);
	return ret;
}

// �̺߳��������ڲ��ϵ��ý�����Ϣ�ĺ���
void ThreadFuncRecvMsg(void *p)
{
	CRemoteDeskControl* pRdc = (CRemoteDeskControl*)p;
	if (pRdc == NULL)
	{
		return;
	}
	while (true)
	{
		pRdc->OnRecv();
	}
}

void CRemoteDeskControl::StartRecvMsg()
{
	if (m_bIsSocketOk && !m_bIsStartRecv)
	{
		_beginthread(ThreadFuncRecvMsg, 0, this);
		m_bIsStartRecv = true;
	}
}

// �������������ﴦ��
void CRemoteDeskControl::OnRecvMsg(const char* lpszData)
{
	std::string strData(lpszData);
	std::vector<std::string> segs = SplitString(strData, '#');
	if (segs[0] == CONTROL_DATA_ID && segs.size() == 3)
	{
		// ���յ���ID��Ϣ
		this->m_nId = atoi(segs[1].c_str());
		strcpy_s(this->m_szPassword, segs[2].c_str());
	}
	else if (segs[0] == CONTROL_DATA_GO && segs.size() == 3)
	{
		// ׼�����ƻ��߱�����
		this->m_bIsGo = true;
		this->m_bIsEnd = false;
		this->m_bIsGetAck = false;
		strcpy_s(this->m_szTargetIp, segs[1].c_str());
		this->m_nTargetPort = atoi(segs[2].c_str());

		bool res = SendP2PMsg();
		if (res)
		{
			// �������ģʽ�µĽ������ݲ���
			if (m_bIsControler)
			{
				// һֱ����ͼ��������

			}
			else
			{
				// ������ͼ�����ݵ��̣߳�������������
				StartSendImage();
				ProcessCommandData();
			}
		}
	}
	else if(segs[0] == CONTROL_DATA_BYE && segs.size() == 1)
	{
		// ���ƽ���
		this->EndControl();
	}
	else if (segs[0] == CONTROL_DATA_WRONG_PW && segs.size() == 1)
	{
		// �������
		this->EndControl();
		MessageBoxA(NULL, "�������", "��ʾ", MB_OK);
	}
}

bool CRemoteDeskControl::SendP2PMsg()
{
	// ���� OK
	SOCKADDR_IN addrTarget;
	addrTarget.sin_addr.S_un.S_addr = inet_addr(m_szTargetIp);
	addrTarget.sin_family = AF_INET;
	addrTarget.sin_port = htons(m_nTargetPort);
	char szBuf[5] = "#";
	strcat_s(szBuf, CONTROL_DATA_OK);

	char szRecvBuf[sizeof(CONTROL_DATA_OK) + 2];
	int nLen = sizeof(SOCKADDR_IN);
	int nCnt = 0;

	while (nCnt++ < 100)
	{
		int nSend = sendto(m_socketClient, szBuf, strlen(szBuf) + 1, 0, (SOCKADDR*)&addrTarget, sizeof(SOCKADDR));
		int ret = recvfrom(m_socketClient, szRecvBuf, sizeof(szRecvBuf), 0, (SOCKADDR*)&addrTarget, &nLen);
		if (ret != SOCKET_ERROR)
		{
			std::string strData(szRecvBuf);
			std::vector<std::string> segs = SplitString(strData, '#');
			if (segs[0] == CONTROL_DATA_OK && segs.size() == 1)
			{
				MessageBoxA(NULL, "�򶴳ɹ�", "t", MB_OK);
				return true;
			}
		}
		Sleep(100);
	}
	MessageBoxA(NULL, "��ʧ��", "t", MB_OK);
	return false;
}


void ThreadFuncSendImage(void *p)
{
	CRemoteDeskControl* pRdc = (CRemoteDeskControl*)p;
	if (pRdc == NULL)
	{
		return;
	}
	while (pRdc->GetIsEnd() == false)
	{
		pRdc->SendImage();
	}
}

void CRemoteDeskControl::StartSendImage()
{
	_beginthread(ThreadFuncSendImage, 0, this);
}

void CRemoteDeskControl::SplitSendImage(BYTE* pImgData, int nSize)
{
	size_t nPackageNum = nSize / MAX_IAMGE_DATA_SIZE;
	size_t nLastDataSize = nSize % MAX_IAMGE_DATA_SIZE;
	if (nLastDataSize != 0)
	{
		nPackageNum++;
	}

	SOCKADDR_IN addrTarget;
	addrTarget.sin_addr.S_un.S_addr = inet_addr(m_szTargetIp);
	addrTarget.sin_family = AF_INET;
	addrTarget.sin_port = htons(m_nTargetPort);
	int nLen = sizeof(SOCKADDR_IN);

	char szBuf[MAX_IAMGE_DATA_SIZE + sizeof(ImageMsgHeader) + 5];
	ImageMsgHeader imgHeader;
	imgHeader.nImageSize = nSize;
	imgHeader.nImageSize = MAX_IAMGE_DATA_SIZE;
	memcpy_s(szBuf, sizeof(szBuf), CONTROL_DATA_IMG, strlen(CONTROL_DATA_IMG));
	for (size_t i = 0; i < nPackageNum; ++i)
	{
		if (nLastDataSize != 0 && i == nPackageNum - 1)
		{
			imgHeader.nDataSize = nLastDataSize;
		}
		imgHeader.nOffset = i * MAX_IAMGE_DATA_SIZE;
	}
}

void CRemoteDeskControl::ProcessCommandData()
{
	while (this->GetIsEnd() == false)
	{

	}
}

void CRemoteDeskControl::SendImage()
{
	// ��������н�ͼ
	HBITMAP hBitmapDesk = CMyBitmap::GetDesktopBitMap();
	CxImage pxImage;
	pxImage.CreateFromHBITMAP(hBitmapDesk);

	// תΪjpeg
	BYTE* pDataBuffer = NULL;
	long nSize = 0;
	pxImage.SetJpegQuality(this->GetJpegQuality());
	pxImage.Encode(pDataBuffer, nSize, CXIMAGE_FORMAT_JPG);

	SplitSendImage(pDataBuffer, nSize);

	pxImage.FreeMemory(pDataBuffer);
}

// ͼ��ָ��㷨�����ְ�����
void ProcessImage(CRemoteDeskControl* pRdc, BYTE* pImgData, size_t nSize)
{
	size_t nPackageNum = nSize / MAX_IAMGE_DATA_SIZE;
	size_t nLastDataSize = nSize % MAX_IAMGE_DATA_SIZE;
	if (nLastDataSize != 0)
	{
		nPackageNum++;
	}

	BYTE* pBuffer = new BYTE[MAX_IMG_BUFFER_SIZE];
	char szMark[5] = "#";
	strcat_s(szMark, 5, CONTROL_DATA_IMG);
	size_t nLen = strlen(szMark);
	memcpy_s(pBuffer, MAX_IMG_BUFFER_SIZE, szMark, nLen);	//	#IMG

	ImagePackageHeader pkgHeader;
	size_t nHeaderSize = sizeof(ImagePackageHeader);
	pkgHeader.uMarkSize = nLen;
	pkgHeader.uHeaderSize = nHeaderSize;
	pkgHeader.uImageSize = nSize;
	pkgHeader.uPackageNum = nPackageNum;
	pkgHeader.uDataSize = MAX_IAMGE_DATA_SIZE;

	for (size_t nIndex = 0; nIndex < nPackageNum; ++nIndex)
	{
		if (nLastDataSize != 0 && nIndex == nPackageNum - 1)
		{
			pkgHeader.uDataSize = nLastDataSize;
		}
		pkgHeader.uPackageIndex = nIndex;
		pkgHeader.uDataOffset = nIndex * MAX_IAMGE_DATA_SIZE;

		memcpy_s(pBuffer + nLen, MAX_IMG_BUFFER_SIZE - nLen, &pkgHeader, nHeaderSize);
		memcpy_s(pBuffer + nLen + nHeaderSize, MAX_IMG_BUFFER_SIZE - nLen - nHeaderSize,
			pImgData + pkgHeader.uDataOffset, pkgHeader.uDataSize);

		//pRdc->SendToServer((char*)pBuffer, pkgHeader.uMarkSize + pkgHeader.uHeaderSize + pkgHeader.uDataSize);
		Sleep(50);
	}
	delete[] pBuffer;
}

// ��ѭ����ͣ����ͼ��ֱ����⵽END=true
void SendImage(CRemoteDeskControl* pRdc)
{
	// ��������н�ͼ
	HBITMAP hBitmapDesk = CMyBitmap::GetDesktopBitMap();
	CxImage pxImage;
	pxImage.CreateFromHBITMAP(hBitmapDesk);

	// תΪjpeg
	BYTE* pDataBuffer = NULL;
	long nSize = 0;
	pxImage.SetJpegQuality(pRdc->GetJpegQuality());
	pxImage.Encode(pDataBuffer, nSize, CXIMAGE_FORMAT_JPG);

	// �ָ�ͼ����д���
	ProcessImage(pRdc, pDataBuffer, nSize);

	DeleteObject(hBitmapDesk);
	pxImage.FreeMemory(pDataBuffer);

	Sleep(50);
}

void CRemoteDeskControl::SaveImageData(const char *lpszData)
{
	ImagePackageHeader pkgHeader;
	size_t nHeaderSize = sizeof(ImagePackageHeader);
	size_t nLen = strlen(CONTROL_DATA_IMG) + 1;		// + #

	memcpy_s(&pkgHeader, nHeaderSize, lpszData + nLen, nHeaderSize);
	
	if (pkgHeader.uPackageIndex == m_nCurrentIndex)
	{
		this->m_nCurrentIndex++;
		this->m_nCurrentImageSize += pkgHeader.uDataSize;

		memcpy_s(this->m_szImageDataBuf + pkgHeader.uDataOffset, MAX_IMG_BUFFER_SIZE,
			lpszData + nLen + nHeaderSize, pkgHeader.uDataSize);
		if (this->m_nCurrentImageSize == pkgHeader.uImageSize && m_nCurrentIndex + 1 == pkgHeader.uPackageNum)
		{
			this->DrawImageToWindow(this->m_nCurrentImageSize);
			this->m_nCurrentIndex = 0;
			this->m_nCurrentImageSize = 0;
		}
	}
	else
	{
		this->m_nCurrentIndex = 0;
		this->m_nCurrentImageSize = 0;
	}
}

void CRemoteDeskControl::DrawImageToWindow(size_t nSize)
{
	CxImage *px = new CxImage((BYTE*)this->m_szImageDataBuf, nSize, CXIMAGE_FORMAT_JPG);

	HDC hdc = GetDC(m_hWnd);
	HDC hMemDc = CreateCompatibleDC(hdc);
	HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, px->GetWidth(), px->GetHeight());
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDc, hMemBmp);
	px->Draw(hMemDc);

	BitBlt(hdc, 0, 0, px->GetWidth(), px->GetHeight(), hMemDc, 0, 0, SRCCOPY);
	SelectObject(hMemDc, hOldBmp);

	DeleteObject(hMemBmp);
	DeleteObject(hOldBmp);
	DeleteDC(hMemDc);
	delete px;
}


// heartbeat
void ThreadFuncHeartMsg(const char * lpszData)
{
}

void SendHeartbeatDataThreadFun(void* ptr)
{
	CRemoteDeskControl* pRdc = (CRemoteDeskControl*)ptr;
	while (true)
	{
		pRdc->SendHeartbeatData();
		Sleep(HEARTBEAT_INTERVAL);
	}
}

void CRemoteDeskControl::StartHeartbeat()
{
	this->m_pUdpHeartbeat = new CUDPClient(this->m_szServerIp, this->m_nServerPort,ThreadFuncHeartMsg);
	_beginthread(SendHeartbeatDataThreadFun, 0, this);
}
