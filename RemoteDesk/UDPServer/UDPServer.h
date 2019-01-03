// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� UDPSERVER_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// UDPSERVER_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef UDPSERVER_EXPORTS
#define UDPSERVER_API __declspec(dllexport)
#else
#define UDPSERVER_API __declspec(dllimport)
#endif

#include <WinSock2.h>
#include <process.h>

#pragma comment(lib,"ws2_32.lib")

#define BUF_LEN 548

typedef void(*PTR_THREAD_FUN)(void*);
typedef VOID(*PTR_THREAD_FUN)(PVOID);
typedef void(*PTR_ON_RECV_FUN)(const char*);

// �����Ǵ� UDPServer.dll ������
class UDPSERVER_API CUDPServer {
public:
	CUDPServer(USHORT unPort, PTR_ON_RECV_FUN pOnRevcFun);
	~CUDPServer();
	// TODO:  �ڴ�������ķ�����
	BOOL StartRecv();
	int OnRecv();
	int SendToClient(const char* lpszSendBuf);


private:
	char m_szRecvBuf[BUF_LEN];
	PTR_ON_RECV_FUN m_pOnRevcFun;

	WSADATA m_wsad;
	SOCKET m_socketServer;
	SOCKADDR_IN m_addrServer;
	SOCKADDR_IN m_addrClient;

	BOOL m_bIsReady;
	BOOL m_bIsStart;
};
