#pragma once
#include <My/Socket/SocketString.h>
#include <My/Socket/Client.h>
#include "Game.h"
#include "Item.h"

#define MAX_SOCKETDATA_NUM 32
#define MAX_BRW_SOCKET     16

typedef void (*disasmdllpath_func)(const wchar_t* path, int len);

typedef struct struct_socket_data
{
	SOCKET socket;       // SOCKET
	ULONG  remote_addr;         // ��ַ
	USHORT remote_port;  // Զ�̶˿�
	USHORT local_port;   // ���ض˿�
	time_t time_stamp;   // ʱ���
} SocketData;

class GameMain: public Game
{
public:
	GameMain();
	void Connect(const char* host, USHORT port);
	void ReConnect(const char* host, USHORT port);
	bool InSocketData(SOCKET s, struct sockaddr_in*, USHORT);
	void CloseSocketData(SOCKET s);
	SocketData* GetSocketDataByNetInfo(ULONG addr, USHORT port);

	void CheckActivePort(ULONG addr, USHORT port);
	void SendData(int op, const char* data, int len);
	void Close();

	static void OnConnectError(const char*, USHORT);
	static void OnConnect(const char*, USHORT, SOCKET);
	static void OnRead(const char*, USHORT, int, const char*, int);
	static void OnClose(const char*, USHORT);
	static void OnError(const char*, USHORT);
	
public:
	// ��������
	int m_iReConnect = 0;
	// Client��
	Client m_Client;
	// ������Socket
	SOCKET m_Socket = 0;
	// �����socket��Ŀ
	int m_iBrowerSocketCount = 0;
	// �����Socket
	SOCKET m_BrowerSockets[MAX_BRW_SOCKET];
	// �����ӵ�Socket
	SocketData m_SocketDatas[MAX_SOCKETDATA_NUM];

	disasmdllpath_func m_DSDllPathFunc = nullptr;

	// CCCָ��
	static GameMain* self;
};
