#include "stdafx.h"
#include "LanRemoteDesktopControl.h"
#include <process.h>
#include "CxImage\include\ximage.h"
#include "MyBitmap.h"

#pragma comment(lib, "CxImage/lib/cximage.lib")

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

CLanRemoteDesktopControl::CLanRemoteDesktopControl()
{
	InitSocket();
	StartWaitClient();
	strcpy_s(m_szPassword, PASSWORD);
	m_nJpegQuality = 70;
}


CLanRemoteDesktopControl::~CLanRemoteDesktopControl()
{
	EndServerConnection();
	DestorySocket();
	DestoryClientSocket();
}

// �����
void CLanRemoteDesktopControl::InitSocket()
{
	m_bIsSocketReady = false;
	int ret;
	ret = WSAStartup(MAKEWORD(2, 2), &m_wsad);
	if (ret != 0)
	{
		return;
	}
	m_socketServer = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socketServer == INVALID_SOCKET)
	{
		WSACleanup();
		return;
	}
	m_addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	m_addrServer.sin_family = AF_INET;
	m_addrServer.sin_port = htons(SERVER_PORT);
	ret = bind(m_socketServer, (SOCKADDR*)&m_addrServer, sizeof(m_addrServer));
	if (ret == SOCKET_ERROR)
	{
		WSACleanup();
		return;
	}
	ret = listen(m_socketServer, MAX_SERVER_LISTEN);
	if (ret == SOCKET_ERROR)
	{
		closesocket(m_socketServer);
		WSACleanup();
	}
	m_bIsSocketReady = true;
}

void CLanRemoteDesktopControl::DestorySocket()
{
	if (m_bIsSocketReady)
	{
		closesocket(m_socketServer);
		WSACleanup();
	}
}

// ��ʼ��Ϊû�к��κοͻ��˽��лỰ
void CLanRemoteDesktopControl::InitServer()
{
	m_bIsControlledByClient = false;
}

void CLanRemoteDesktopControl::EndServerConnection()
{
	closesocket(m_socketCurrentConn);
	InitServer();
}

// �ȴ�һ�οͻ�������
void CLanRemoteDesktopControl::WaitClient()
{
	SOCKET socketConnection;
	SOCKADDR_IN addrClient;
	int nLen = sizeof(SOCKADDR_IN);
	//MessageBoxA(NULL, "�ȵȵ�", "warning", MB_OK);
	socketConnection = accept(m_socketServer, (SOCKADDR*)&addrClient, &nLen);
	//MessageBoxA(NULL, "������", "warning", MB_OK);

	m_socketCurrentConn = socketConnection;
	m_listAddrClients.push_back(addrClient);

	while (true)
	{
		int ret = recv(socketConnection, m_szRecvBufServer, MAX_RECV_BUF, 0);
		if (ret == 0)
		{
			InitServer();
			m_listAddrClients.pop_back();
			//MessageBoxA(NULL, "���ӶϿ�", "warning", MB_OK);
			return;		// ����û��Ͽ��ˣ��ȴ���һ����Ե��
		}
		else if (ret == SOCKET_ERROR)
		{
			InitServer();
			//MessageBoxA(NULL, "socket error", "warning", MB_OK);
			return;
		}
		else
		{
			//MessageBoxA(NULL, "get it", "warning", MB_OK);
			int flag = ProcessServerData(m_szRecvBufServer, ret);
			if (flag == MSGTYPE_WP) // �������
			{
				InitServer();
				m_listAddrClients.pop_back();
				MessageBoxA(NULL, "�Է��������㣬���ǰ��Ų���", "error", MB_OK);
				return;		// ����û����Ų��ԣ���Ե�޷�
			}
			else if (flag == SOCKET_ERROR)
			{
				// ������Ҫ�˳�
				InitServer();
				m_listAddrClients.pop_back();
				return;
			}
			else if (flag == MSGTYPE_OK)
			{
				int ans  = ProcessCommandData();	// ר�Ž��������������
				if (ans == SOCKET_ERROR)	// �ر�������
				{
					InitServer();
					return;
				}
			}
		}
	}
	
}

void ThreadFuncWaitClient(void* p)
{
	CLanRemoteDesktopControl* pLrdc = (CLanRemoteDesktopControl*)p;
	if (pLrdc == NULL)
	{
		return;
	}
	while (true)
	{
		pLrdc->WaitClient();
	}
}

void CLanRemoteDesktopControl::StartWaitClient()
{
	_beginthread(ThreadFuncWaitClient, 0, this);
}

// ���������յ�������
int CLanRemoteDesktopControl::ProcessServerData(const char* lpszData, int nLen)
{
	LanMsgHeader lanHeader;
	size_t nSize = sizeof(lanHeader);
	if (nLen < (int)nSize)
	{
		return -1;
	}
	memcpy_s(&lanHeader, nSize, lpszData, nSize);

	if (lanHeader.MsgType == MSGTYPE_LAN && m_bIsControling == false && GetServerStatus() == false)		// ��Ϊ�ͻ��˿��Ʊ���ing
	{
		//MessageBoxA(NULL, "password okkkkkk", "tt", MB_OK);
		char szPw[PASSWORD_LEN];
		memcpy_s(szPw, PASSWORD_LEN, lpszData + nSize, lanHeader.nDataSize);
		szPw[lanHeader.nDataSize] = '\0';
		if (strcmp(szPw,m_szPassword) == 0)
		{
			LanMsgHeader lanHeader;
			lanHeader.MsgType = MSGTYPE_OK;
			lanHeader.nHeaderSize = sizeof(lanHeader);
			lanHeader.nDataSize = 0;
			lanHeader.nPackageSize = lanHeader.nHeaderSize + lanHeader.nDataSize;

			char* pBuf = new char[sizeof(lanHeader)];
			memcpy_s(pBuf, sizeof(lanHeader), &lanHeader, sizeof(lanHeader));
			send(m_socketCurrentConn, pBuf, sizeof(lanHeader), 0);
			delete[] pBuf;

			// ��ʼ����ing�������̣߳���ͼ��
			m_bIsControlledByClient = true;
			StartSendImage();
			return MSGTYPE_OK;
		}
		else
		{
			LanMsgHeader lanHeader;
			lanHeader.MsgType = MSGTYPE_WP;
			lanHeader.nHeaderSize = sizeof(lanHeader);
			lanHeader.nDataSize = 0;
			lanHeader.nPackageSize = lanHeader.nHeaderSize + lanHeader.nDataSize;

			char* pBuf = new char[sizeof(lanHeader)];
			memcpy_s(pBuf, sizeof(lanHeader), &lanHeader, sizeof(lanHeader));
			send(m_socketCurrentConn, pBuf, sizeof(lanHeader), 0);
			delete[] pBuf;
			return MSGTYPE_WP;
		}
	}
	return 0;
}

// һֱ���տ�������Ϣ���ͻ��˷�����ֻ�������ֱ࣬�ӽ����ṹ��INPUT
int CLanRemoteDesktopControl::ProcessCommandData()
{
	int nInputSize = sizeof(INPUT);
	char szBuf[sizeof(INPUT)];
	int nRealRecv = 0;
	INPUT input;
	while (true)
	{
		int nRecv = recv(m_socketCurrentConn, szBuf + nRealRecv, nInputSize - nRealRecv, 0);
		if (nRecv == SOCKET_ERROR)
		{
			return SOCKET_ERROR;
		}
		nRealRecv += nRecv;
		if (nRealRecv == nInputSize)
		{
			nRealRecv = 0;
			memcpy_s(&input, nInputSize, szBuf, nInputSize);
			SendInput(1, &input, sizeof(INPUT));
		}
	}
}

void CLanRemoteDesktopControl::SplitSendImage(BYTE* pImgData, int nSize)
{
	size_t nPackageNum = nSize / MAX_TCP_IMG_BUF;
	size_t nLastPackageSize = nSize % MAX_TCP_IMG_BUF;
	if (nLastPackageSize != 0)
	{
		nPackageNum++;
	}

	size_t nTotalSize = MAX_TCP_IMG_BUF + sizeof(LanMsgHeader);
	LanMsgHeader lanHeader;
	lanHeader.MsgType = MSGTYPE_IMG;
	lanHeader.nImageSize = nSize;
	// ������Ϣ��ʵû��
	lanHeader.nHeaderSize = sizeof(lanHeader);
	lanHeader.nDataSize = nSize;
	lanHeader.nPackageSize = lanHeader.nDataSize + lanHeader.nHeaderSize;
	char* pBuf = new char[MAX_RECV_BUF];

	// �ȷ�ͷ�����
	memcpy_s(pBuf, MAX_RECV_BUF, HEAD_MARK, sizeof(HEAD_MARK));
	send(m_socketCurrentConn, pBuf, sizeof(HEAD_MARK), 0);

	// ���ṹ��
	memcpy_s(pBuf, MAX_RECV_BUF, &lanHeader, sizeof(lanHeader));
	send(m_socketCurrentConn, pBuf, sizeof(lanHeader), 0);

	// ��ͼƬ����
	//send(m_socketCurrentConn, (char*)pImgData, nSize, 0);
	
	for (size_t i = 0; i < nPackageNum; ++i)
	{
		size_t nDataSize = MAX_TCP_IMG_BUF;
		if (nLastPackageSize != 0 && i == nPackageNum - 1)
		{
			nDataSize = nLastPackageSize;
		}
		size_t nOffset = i * MAX_TCP_IMG_BUF;
		memcpy_s(pBuf, nTotalSize, pImgData + nOffset, nDataSize);
		send(m_socketCurrentConn, pBuf, nDataSize, 0);
	}
	delete[] pBuf;
}

void CLanRemoteDesktopControl::SnedImage()
{
	// ��������н�ͼ
	HBITMAP hBitmapDesk = CMyBitmap::GetDesktopBitMap();
	CxImage *pxImage = new CxImage();
	pxImage->CreateFromHBITMAP(hBitmapDesk);

	// תΪjpeg
	BYTE* pDataBuffer = NULL;
	long nSize = 0;
	pxImage->SetJpegQuality(this->GetJpegQuality());
	pxImage->Encode(pDataBuffer, nSize, CXIMAGE_FORMAT_JPG);

	SplitSendImage(pDataBuffer, nSize);

	DeleteObject(hBitmapDesk);
	pxImage->FreeMemory(pDataBuffer);
	delete pxImage;
}

void ThreadFuncSendImageTcpLan(void *p)
{
	CLanRemoteDesktopControl* pLrdc = (CLanRemoteDesktopControl*)p;
	if (pLrdc == NULL)
	{
		return;
	}
	bool tmp = false;
	while (pLrdc->GetServerStatus())
	{
		if (!tmp)
		{
			//MessageBoxA(NULL, "Ҫ��ʼ����ͼ����", "ע��", MB_OK);
			tmp = true;
		}
		pLrdc->SnedImage();
		Sleep(20);
	}
	//MessageBoxA(NULL, "����ͼ�����", "ע��", MB_OK);
}

void CLanRemoteDesktopControl::StartSendImage()
{
	_beginthread(ThreadFuncSendImageTcpLan, 0, this);
}

bool CLanRemoteDesktopControl::GetServerStatus()
{
	return m_bIsControlledByClient;
}

//////////////////////////////////////////////////////////////////////////
//									�ֽ���
//////////////////////////////////////////////////////////////////////////

// �ͻ���

bool CLanRemoteDesktopControl::GetIsControling()
{
	return m_bIsControling;
}

void CLanRemoteDesktopControl::InitClientSocket()
{
	m_socketClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	m_bIsEnd = false;
	m_nCurrentSize = 0;
	m_nImageSize = 0;
	m_bIsStartRecvImage = false;
	m_nRemoteScreenWidth = 0;
	m_nRemoteScreenHeight = 0;
	m_nDrawOriginX = 0;
	m_nDrawOriginY = 0;
}

void CLanRemoteDesktopControl::DestoryClientSocket()
{
	if (m_socketClient != INVALID_SOCKET)
	{
		closesocket(m_socketClient);
	}
}

int CLanRemoteDesktopControl::ClientConnectServer(const char* lpszServerIp, u_short nServerPort)
{
	SOCKADDR_IN addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(nServerPort);
	addrServer.sin_addr.S_un.S_addr = inet_addr(lpszServerIp);

	return connect(m_socketClient, (SOCKADDR*)&addrServer, sizeof(addrServer));
}

void CLanRemoteDesktopControl::ClientRecvServer()
{
	memset(m_szRecvBufClient, 0, sizeof(m_szRecvBufClient));
	int ret = recv(m_socketClient, m_szRecvBufClient, MAX_TCP_IMG_BUF, 0);
	if (ret == 0)
	{
		// ���Ӷ��ˣ������ɸ���
		EndClientConnection();
		return;
	}
	else if (ret == SOCKET_ERROR)
	{
		// ��Ϣ���������գ�ĳ���ط��������⡣Ҫ��ҪҲ�Ͽ�����
		EndClientConnection();
	}
	else
	{
		// ��Ϣ�������յ���
		ProcessClientData(m_szRecvBufClient, ret);
	}
	DestoryClientSocket();
	DestroyWindow(m_hWndDraw);
}

void ThreadFuncRecvServerMsg(void *p)
{
	CLanRemoteDesktopControl* pLrdc = (CLanRemoteDesktopControl*)p;
	if (pLrdc == NULL)
	{
		return;
	}

	while (pLrdc->GetClientIsEnd() == false)
	{
		pLrdc->ClientRecvServer();
	}
	//MessageBoxA(NULL, "��������", "e", MB_OK);
}

void CLanRemoteDesktopControl::StartRecvServerMsg()
{
	_beginthread(ThreadFuncRecvServerMsg, 0, this);
}

int CLanRemoteDesktopControl::ProcessClientData(const char* lpszData, int nLen)
{
	// ˵��û��ͷ����ʶ������ͼƬ���ݣ���һ��ֻ��һ���ṹ��
	LanMsgHeader lanHeader;
	size_t nHeaderSize = sizeof(lanHeader);


	/*
	// ����㷨,����
	if (m_bIsStartRecvImage)
	{
		if (m_nImageSize > MAX_RECV_BUF)
		{
			//MessageBoxA(NULL, "holyshit", "a", MB_OK);
			return 0;
		}
		if (m_nImageSize != 0 && m_nImageSize < MAX_RECV_BUF)
		{
			int nTmpSize = m_nCurrentSize + nLen;
			if (nTmpSize > m_nImageSize)
			{
				int nSubSize = nTmpSize - m_nImageSize;			// ������ͼƬ�����ֽ�
				int nNeedSize = m_nImageSize - m_nCurrentSize;	// ����Ҫ�����ֽ��γ����ڵ�ͼƬ
				memcpy_s(m_byImageData + m_nCurrentSize, MAX_RECV_BUF, lpszData, nNeedSize);
				DrawImageToWindow();
				m_nCurrentSize = 0;
				if (nSubSize > nHeaderSize)		// �������ֹ���ͷ��Ϣ
				{
					LanMsgHeader lanTmpHeader;
					memcpy_s(&lanTmpHeader, nHeaderSize, lpszData + nNeedSize, nHeaderSize);
					m_nImageSize = lanTmpHeader.nImageSize;

					// �������־���ͼƬ������
					int nTmpExceed = nSubSize - nHeaderSize;
					memcpy_s(m_byImageData + m_nCurrentSize, MAX_RECV_BUF, lpszData + nNeedSize + nHeaderSize, nTmpExceed);
					m_nCurrentSize += nTmpExceed;
				}
				else if (nSubSize == nHeaderSize)
				{
					LanMsgHeader lanTmpHeader;
					memcpy_s(&lanTmpHeader, nHeaderSize, lpszData + nNeedSize, nSubSize);
					m_nImageSize = lanTmpHeader.nImageSize;
				}
				else
				{
					// ʣ�ಿ�ֲ���ͷ��
					char* pTmpBufRecv = new char[nHeaderSize + 2];
					char* pTmpBufSave = new char[nHeaderSize + 2];
					int nTmpHeadNeed = nHeaderSize - nSubSize;
					int nRecvLen = recv(m_socketClient, pTmpBufRecv, nTmpHeadNeed, 0);
					if (nRecvLen == nTmpHeadNeed)
					{
						memcpy_s(pTmpBufSave, nHeaderSize, lpszData + nNeedSize, nSubSize);
						memcpy_s(pTmpBufSave + nSubSize, nHeaderSize, pTmpBufRecv, nTmpHeadNeed);
						LanMsgHeader lanTmpHeader;
						memcpy_s(&lanTmpHeader, nHeaderSize, pTmpBufSave, nHeaderSize);
						m_nImageSize = lanTmpHeader.nImageSize;
					}
					delete[] pTmpBufSave;
					delete[] pTmpBufRecv;
				}
			}
			else if (nTmpSize == m_nImageSize)
			{
				m_nCurrentSize = 0;
				DrawImageToWindow();
			}
			else
			{
				// ֱ�ӷŽ�ȥ�ͺ�
				memcpy_s(m_byImageData + m_nCurrentSize, MAX_RECV_BUF, lpszData, nLen);
				m_nCurrentSize += nLen;
			}
		}
		return 0;
	}
	*/
	

	// �ͻ����յ���Ϣ���ǶԷ���ͼƬ����ʼ׼��������Ϣ
	if (nLen < nHeaderSize)
	{
		return -1;
	}
	memcpy_s(&lanHeader, nHeaderSize, lpszData, nHeaderSize);

	if (lanHeader.MsgType == MSGTYPE_OK)
	{
		//MessageBoxA(NULL, "������ȷ", "a", MB_OK);
		m_bIsControling = true;
		m_bIsStartRecvImage = true;
		ShowWindow(m_hWndDraw, SW_SHOW);
		// ��ʼ���պʹ���ͼ��
		ProcessImageData();
		// �ı�Ϊ������ģʽ
		//unsigned long nMode = 1;
		//int ret = ioctlsocket(m_socketClient, FIONBIO, &nMode);
		//if (ret == SOCKET_ERROR)
		//{
		//	closesocket(m_socketClient);
		//	MessageBoxA(NULL, "tcp client iocktl failed", "error", MB_OK);
		//}
		// ���̣߳�������
		return MSGTYPE_OK;
	}
	else if (lanHeader.MsgType == MSGTYPE_WP)
	{
		EndClientConnection();
		MessageBoxA(NULL, "password incorrect, connectiong close", "error", MB_OK);
		return MSGTYPE_WP;
	}
	return 0;
}

void CLanRemoteDesktopControl::ProcessImageData()
{
	static bool bIsShouldExit = false;
	// �ж��ǲ���ͼ����Ϣ�����ж���ľ��ͷ����ʶ
	const size_t nHeadMarkSize = sizeof(HEAD_MARK);
	char szTmpDataBuf[nHeadMarkSize];
	char szHeadMark[nHeadMarkSize];
	// ȡ��ʶͷ
	int nLen = recv(m_socketClient, szTmpDataBuf, nHeadMarkSize, 0);
	if (nLen == SOCKET_ERROR)
	{
		EndClientConnection();
		return;
	}
	if (nLen < nHeadMarkSize)
	{
		return;
	}
	memcpy_s(szHeadMark, sizeof(szHeadMark), szTmpDataBuf, nHeadMarkSize);
	// ���յ���һ��ͼƬ�ı�ʶͷ
	if (strcmp(szHeadMark, HEAD_MARK) == 0)
	{
		if (!m_bIsStartRecvImage)
		{
			return;
		}

		while (this->GetClientIsEnd() == false)
		{
			// ���ˣ���ȷ�õ���ʶͷ����ʼ�õ��ṹ��
			int nRealRecv = 0;
			int nHeaderSize = sizeof(LanMsgHeader);
			char szHeaderBuf[sizeof(LanMsgHeader) + 1];
			while (this->GetClientIsEnd() == false)
			{
				int nRecvLen = recv(m_socketClient, m_szRecvBufClient, nHeaderSize - nRealRecv, 0);
				if (nRecvLen == SOCKET_ERROR)
				{
					EndClientConnection();
					return;
				}
				memcpy_s(szHeaderBuf + nRealRecv, nHeaderSize, m_szRecvBufClient, nRecvLen);
				nRealRecv += nRecvLen;
				if (nRealRecv == nHeaderSize)
				{
					nRealRecv = 0;
					break;
				}
			}
			LanMsgHeader tmpLanHeader;
			memcpy_s(&tmpLanHeader, nHeaderSize, szHeaderBuf, nHeaderSize);
			m_nImageSize = tmpLanHeader.nImageSize;

			// �ṹ��ɹ��õ�����ʼ�õ�ͼƬ
			memset(m_szRecvBufClient, 0, sizeof(m_szRecvBufClient));
			nRealRecv = 0;
			while (this->GetClientIsEnd() == false)
			{
				int nRecvLen = recv(m_socketClient, m_szRecvBufClient, m_nImageSize - nRealRecv, 0);
				if (nRecvLen == SOCKET_ERROR)
				{
					EndClientConnection();
					return;
				}
				memcpy_s(m_byImageData + m_nCurrentSize, MAX_RECV_BUF - m_nCurrentSize, m_szRecvBufClient, nRecvLen);
				m_nCurrentSize += nRecvLen;
				nRealRecv += nRecvLen;
				if (nRealRecv == m_nImageSize)
				{
					break;
				}
			}
			// �õ�ͼƬ��ͻ���������
			DrawImageToWindow();
			m_nCurrentSize = 0;

			// �õ�����ͼƬ�ı�ʶͷ
			memset(m_szRecvBufClient, 0, sizeof(m_szRecvBufClient));
			nRealRecv = 0;
			while (this->GetClientIsEnd() == false)
			{
				int nRecvLen = recv(m_socketClient, m_szRecvBufClient, nHeadMarkSize - nRealRecv, 0);
				if (nRecvLen == SOCKET_ERROR)
				{
					EndClientConnection();
					return;
				}
				memcpy_s(szHeadMark + nRealRecv, nHeadMarkSize, m_szRecvBufClient, nRecvLen);
				nRealRecv += nRecvLen;
				if (nRealRecv == nHeadMarkSize)
				{
					nRealRecv = 0;
					if (strcmp(szHeadMark, HEAD_MARK) == 0)
					{
						break;
					}
				}
			}
		}
		return;
	}
}

void CLanRemoteDesktopControl::DrawImageToWindow()
{
	CxImage *px = new CxImage((BYTE*)this->m_byImageData, m_nImageSize, CXIMAGE_FORMAT_JPG);
	m_nRemoteScreenWidth = px->GetWidth();
	m_nRemoteScreenHeight = px->GetHeight();

	HDC hdc = GetDC(m_hWndDraw);
	HDC hMemDc = CreateCompatibleDC(hdc);
	HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, m_nRemoteScreenWidth, m_nRemoteScreenHeight);
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDc, hMemBmp);
	px->Draw(hMemDc);

	RECT rect;
	GetClientRect(m_hWndDraw, &rect);
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;
	m_nDrawOriginX = m_nRemoteScreenWidth < w ? (w - m_nRemoteScreenWidth) / 2 : 0;
	m_nDrawOriginY = m_nRemoteScreenHeight < h ? (h - m_nRemoteScreenHeight) / 2 : 0;
	BitBlt(hdc, m_nDrawOriginX, m_nDrawOriginY, m_nRemoteScreenWidth, m_nRemoteScreenHeight, hMemDc, 0, 0, SRCCOPY);
	SelectObject(hMemDc, hOldBmp);

	DeleteObject(hMemBmp);
	DeleteObject(hOldBmp);
	DeleteDC(hMemDc);
	::delete px;
}

bool CLanRemoteDesktopControl::GetClientIsEnd()
{
	// �������յ��߳�
	return m_bIsEnd;
}

// ��������,�������
void CLanRemoteDesktopControl::EndClientConnection()
{
	m_bIsEnd = true;
	m_bIsControling = false;
}

// ��ʼ����,�ͻ������
void CLanRemoteDesktopControl::ControlLanRemoteDesk(const char* lpszServerIp, const char* lpszPassword)
{
	if (m_bIsControling)
	{
		return;
	}
	InitClientSocket();
	int ret = ClientConnectServer(lpszServerIp, SERVER_PORT);
	if (ret == SOCKET_ERROR)
	{
		MessageBoxA(NULL, "connect failed", "error", MB_OK);
		DestoryClientSocket();
		return;
	}
	StartRecvServerMsg();

	LanMsgHeader lanHeader;
	lanHeader.MsgType = MSGTYPE_LAN;
	lanHeader.nHeaderSize = sizeof(lanHeader);
	lanHeader.nDataSize = strlen(lpszPassword);
	lanHeader.nImageSize = 0;
	lanHeader.nPackageSize = lanHeader.nHeaderSize + lanHeader.nDataSize;

	char* pBuf = new char[lanHeader.nPackageSize + 1];
	memcpy_s(pBuf, lanHeader.nPackageSize + 1, &lanHeader, lanHeader.nHeaderSize);
	memcpy_s(pBuf + lanHeader.nHeaderSize, lanHeader.nPackageSize + 1 - sizeof(lanHeader.nHeaderSize), lpszPassword, lanHeader.nDataSize);
	ret = send(m_socketClient, pBuf, lanHeader.nPackageSize, 0);
	delete[] pBuf;
}

// �������ͼ����¼�
int CLanRemoteDesktopControl::SendCommand(const INPUT& inputData)
{
	if (m_bIsControling == false)
	{
		return -1;
	}
	int nInputSize = sizeof(inputData);
	return send(m_socketClient, (char*)&inputData, nInputSize, 0);
}

BYTE CLanRemoteDesktopControl::GetJpegQuality()
{
	return m_nJpegQuality;
}

void CLanRemoteDesktopControl::SetJpegQuality(BYTE nQuality)
{
	if (nQuality > 0 && nQuality <= 100)
	{
		m_nJpegQuality = nQuality;
	}
}

void CLanRemoteDesktopControl::SetDrawHwnd(HWND hWnd)
{
	m_hWndDraw = hWnd;
}

// ���͸�������
void CLanRemoteDesktopControl::SendKeyDown(WORD wVk)
{
	INPUT input;
	memset(&input, 0, sizeof(input));
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = wVk;
	this->SendCommand(input);
}

void CLanRemoteDesktopControl::SendKeyUp(WORD wVk)
{
	INPUT input;
	memset(&input, 0, sizeof(input));
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = wVk;
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	this->SendCommand(input);
}

void CLanRemoteDesktopControl::SendMouseMove(DWORD dx, DWORD dy)
{
	INPUT input;
	memset(&input, 0, sizeof(input));
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE;
	input.mi.dx = (dx - m_nDrawOriginX) * 65535 / m_nRemoteScreenWidth;
	input.mi.dy = (dy - m_nDrawOriginY) * 65535 / m_nRemoteScreenHeight;
	this->SendCommand(input);
}

void CLanRemoteDesktopControl::SendLeftButtonDown()
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	this->SendCommand(input);
}

void CLanRemoteDesktopControl::SendLeftButtonUp()
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	this->SendCommand(input);
}

void CLanRemoteDesktopControl::SendLeftButtonDoubleClick()
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	this->SendCommand(input);
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	this->SendCommand(input);
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	this->SendCommand(input);
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	this->SendCommand(input);
}

void CLanRemoteDesktopControl::SendRightButtonDown()
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	this->SendCommand(input);
}

void CLanRemoteDesktopControl::SendRightButtonUp()
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	this->SendCommand(input);
}

void CLanRemoteDesktopControl::SendRightButtonDoubleClick()
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	this->SendCommand(input);;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	this->SendCommand(input);
	input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	this->SendCommand(input);;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	this->SendCommand(input);
}

void CLanRemoteDesktopControl::SendMiddleButtonDown()
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
	this->SendCommand(input);
}

void CLanRemoteDesktopControl::SendMiddleButtonUp()
{

	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
	this->SendCommand(input);
}

void CLanRemoteDesktopControl::SendMiddleButtonDoubleClick()
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
	this->SendCommand(input);
	input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
	this->SendCommand(input);
	input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
	this->SendCommand(input);
	input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
	this->SendCommand(input);
}

void CLanRemoteDesktopControl::SendMouseWheel(DWORD mouseData)
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_WHEEL;
	input.mi.mouseData = mouseData;
	this->SendCommand(input);
}
