#pragma once
#include "../UDPClient/UDPClient.h"
#include "../UDPServer/UDPServer.h"
#include "ImageDataFormat.h"

#pragma comment(lib,"../Debug/UDPClient.lib")
#pragma comment(lib,"../Debug/UDPServer.lib")

#define CONTROL_DATA_HEART		"HEART"
#define CONTROL_DATA_ONLINE		"DPC"
#define CONTROL_DATA_BYE		"BYE"
#define CONTROL_DATA_CONTROL	"C"
#define CONTROL_DATA_WRONG_PW	"WP"
#define CONTROL_DATA_GO			"GO"
#define CONTROL_DATA_ID			"ID"
#define CONTROL_DATA_OK			"OK"
#define CONTROL_DATA_IMG		"IMG"
#define CONTROL_DATA_CMD		"CMD"
#define CONTROL_DATA_LAN		"LAN"

#define HEARTBEAT_INTERVAL 3000

#define MAX_BUF_LEN 1250

#define LAN_PORT 11223


class CRemoteDeskControl
{
public:
	CRemoteDeskControl(const char* lpszServerIP, const u_short& uServerPort);
	virtual ~CRemoteDeskControl();

	void SendHeartbeatData();

	int GetId();
	char* GetPassword();

	int OnRecv();
	int SendOnlineData();
	void ControlRemoteClient(const int& m_nId, const char* lpszPassword);
	void SendImage();
	void SetDrawTargetWindow(HWND hWnd);
	void EndControl();

	bool GetIsEnd()const;
	bool GetIsOk()const;
	bool GetIsEndRecvServer()const;

	void SetJpegQuality(BYTE q);
	BYTE GetJpegQuality()const;
private:
	char m_szServerIp[20];
	u_short m_nServerPort;
	char m_szTargetIp[20];
	u_short m_nTargetPort;

	char m_szMac[20];		// ����MAC��ַ���� 1e-30-7d-96-1e-2d
	int m_nId;				// ID ��132456789
	char m_szPassword[8];	// ������λ��
	bool m_bIsReady;		// �Ƿ�׼�����������Ƿ�get����ID
	bool m_bIsControler;	// �ǲ��ǿ�����

	bool m_bIsGo;			// �Ƿ��Ѿ��յ���go���Ƿ���׼���Ự�׶�
	bool m_bIsOk;			// �Ƿ�ɶԷ��ͻ�����ͨ�����磬��ͨ��ſ�ʼ��������
	bool m_bIsEnd;			// ���ƽ����������ٷ���ͼ�������
	bool m_bIsEndRecvServer;// �Ƿ�������շ���˵���Ϣ

	HWND m_hWnd;			// Ҫ�������������

	WSADATA m_wsad;
	SOCKET m_socketClient;
	SOCKADDR_IN m_addrServer;
	//SOCKADDR_IN m_addrTarget;
	bool m_bIsSocketOk;
	bool m_bIsStartRecv;
	char m_szRecvBuf[MAX_BUF_LEN];
	char m_szSendTmpBuf[MAX_BUF_LEN];

	BYTE m_byJpegQuality;						// jpeg��ʽͼƬ��������0-100
	char m_szImageDataBuf[MAX_IMG_BUFFER_SIZE];	// �ݴ�ͼ������
	size_t m_nCurrentImageSize;					// ��ǰ�Ѿ������˶���ͼ��
	size_t m_nCurrentIndex;

	unsigned int m_nImageCount;
	bool m_bIsGetAck;							// �Ƿ��յ���ȷ���ź�

	void InitFlag();							// ÿ�λỰ��Ҫ��ʼ���ı���
	void InitSocket(const char* lpszServerIP, 
		const u_short& unServerPort);			// ��ʼ��socket

	void StartRecvMsg();						// ����������Ϣ���߳�
	void OnRecvMsg(const char*lpszData);		// �����յ�����Ϣ
	bool SendP2PMsg();							// ��ͣ�ķ���
	void StartSendImage();						// ��ʼ��������ͼ��
	void SplitSendImage(BYTE* pImgData, int nSize);
	void ProcessImageData();
	void ProcessCommandData();
	void SaveImageData(const char *lpszData);	// ����ͼ�����ݵ�����
	void DrawImageToWindow(size_t nSize);		// �����յ���ͼ�񵽴���


	CUDPClient* m_pUdpHeartbeat;
	void StartHeartbeat();
};
