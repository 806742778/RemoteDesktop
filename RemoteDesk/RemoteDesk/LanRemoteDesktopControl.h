#pragma once
#include <WinSock2.h>
#include <list>

#pragma comment(lib,"ws2_32.lib")

#define PASSWORD "666"

// ����ͼƬʱ������ݱ�ʶͷ  һ��ͼƬ�ķ��Ͱ���  ����ʶͷ + �ṹ��ͷ  +  ͼ������
#define HEAD_MARK "TEA"

#define MAX_SERVER_LISTEN 10

const size_t MAX_RECV_BUF = 1024 * 1024;
const size_t MAX_TCP_IMG_BUF = 1400;
const u_short SERVER_PORT = 17788;
const size_t PASSWORD_LEN = 10;

enum MSGTYPE
{
	MSGTYPE_CMD,	// ����������,��� INPUT �ṹ��
	MSGTYPE_IMG,	// ������ͼƬ
	MSGTYPE_LAN,	// �������ӣ��������
	MSGTYPE_WP,		// �������
	MSGTYPE_OK,		// ������ȷ
	MSGTYPE_BYE		// �ͻ��˸��߷���˶Ͽ�
};

typedef struct _LanMsgHeader
{
	unsigned int nPackageSize;	// �����ܴ�С = ��ͷ + ����
	unsigned int nHeaderSize;	// ��ͷ��С
	unsigned int nDataSize;		// ���ݴ�С
	unsigned int nImageSize;	// ͼ���ܴ�С
	MSGTYPE MsgType;			// ����������ʲô����
}LanMsgHeader;


class CLanRemoteDesktopControl
{
public:
	CLanRemoteDesktopControl();
	virtual ~CLanRemoteDesktopControl();

	void WaitClient();
	void ClientRecvServer();
	void EndClientConnection();	// �ͻ��������Ͽ�����
	void EndServerConnection();	// ����������Ͽ�����
	void SnedImage();
	int SendCommand(const INPUT&);
	bool GetServerStatus();	// true ��ʾ���ںͿͻ��˻Ự

	void ControlLanRemoteDesk(const char* lpszServerIp, const char* lpszPassword);

	bool GetClientIsEnd();
	BYTE GetJpegQuality();
	void SetJpegQuality(BYTE);
	void SetDrawHwnd(HWND hWnd);
	
	// ���͸�������������
	void SendKeyDown(WORD wVk);
	void SendKeyUp(WORD wVk);
	void SendMouseMove(DWORD dx, DWORD dy);
	void SendLeftButtonDown();
	void SendLeftButtonUp();
	void SendLeftButtonDoubleClick();
	void SendRightButtonDown();
	void SendRightButtonUp();
	void SendRightButtonDoubleClick();
	void SendMiddleButtonDown();
	void SendMiddleButtonUp();
	void SendMiddleButtonDoubleClick();
	void SendMouseWheel(DWORD mouseData);		// �����ֹ���
private:
	char m_szPassword[PASSWORD_LEN];
	char m_szRecvBufServer[MAX_RECV_BUF];	// ����˽��յ������ݴ�����
	char m_szRecvBufClient[MAX_RECV_BUF];	// �ͻ��˵Ĵ�����
	BYTE m_nJpegQuality;
	int m_nRemoteScreenWidth;
	int m_nRemoteScreenHeight;
	int m_nDrawOriginX;
	int m_nDrawOriginY;

	// server
	WSADATA m_wsad;
	SOCKET m_socketServer;
	SOCKET m_socketCurrentConn;
	SOCKADDR_IN m_addrServer;
	std::list<SOCKADDR_IN> m_listAddrClients;
	bool m_bIsControlledByClient;

	bool m_bIsSocketReady;
	void InitSocket();
	void DestorySocket();
	void InitServer();			// ����ǰ�����״̬
	void StartWaitClient();
	int ProcessServerData(const char* lpszData, int nLen);
	int ProcessCommandData();
	void StartSendImage();
	void SplitSendImage(BYTE* pImgData, int nSize);

	// client
	SOCKET m_socketClient;
	bool m_bIsClientSocketReady;
	bool m_bIsEnd;	// �Ƿ��н����ź�
	bool m_bIsControling;

	BYTE m_byImageData[MAX_RECV_BUF];
	bool m_bIsStartRecvImage;
	size_t m_nCurrentSize;
	size_t m_nImageSize;
	HWND m_hWndDraw;

	void InitClientSocket();
	void DestoryClientSocket();
	int ClientConnectServer(const char* lpszServerIp, u_short nServerPort);
	void StartRecvServerMsg();
	int ProcessClientData(const char* lpszData, int nLen);
	void ProcessImageData();		// ����ͼ������
	void DrawImageToWindow();
	bool GetIsControling();
};

