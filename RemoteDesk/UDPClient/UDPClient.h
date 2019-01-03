// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� UDPCLIENT_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// UDPCLIENT_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef UDPCLIENT_EXPORTS
#define UDPCLIENT_API __declspec(dllexport)
#else
#define UDPCLIENT_API __declspec(dllimport)
#endif

#include <WinSock2.h>
#include <process.h>

#pragma comment(lib,"ws2_32.lib")

#define BUF_LEN 548

typedef void(*PTR_ON_RECV_FUN)(const char*);


// �����Ǵ� UDPClient.dll ������
class UDPCLIENT_API CUDPClient {
public:

	CUDPClient(const char* lpszServerIP, const u_short& unServerPort, PTR_ON_RECV_FUN pOnRevcFun);
	CUDPClient(const char* lpszServerIP, const u_short& unServerPort, void* pOnRevcFun);
	virtual ~CUDPClient();
	// TODO:  �ڴ�������ķ�����
	void SetServerIP(const char* lpszServerIP);
	void SetServerPort(const u_short& unServerPort);
	BOOL StartRecv();
	int OnRecv();
	int SendToServer(const char* lpszSendBuf);
	
private:
	char m_szRecvBuf[BUF_LEN];
	PTR_ON_RECV_FUN m_pOnRevcFun;

	WSADATA m_wsad;
	SOCKET m_socketClient;
	SOCKADDR_IN m_addrServer;

	BOOL m_bIsReady;
	BOOL m_bIsStart;

protected:
};
