#include "GameMain.h"
#include <stdio.h>
#include <time.h>

// 关闭
#define OP_CLOSE        0x00
// 有效的端口
#define OP_ACTIVE_PORT  0x01
// 发送数据
#define OP_SEND_DATA    0x02
// 发送子弹
#define OP_SEND_FIRE    0x03
// 发送命中鱼
#define OP_SEND_FISH    0x04

// 设置反汇编DLL路径
#define OP_SETDSDLLPATH 0x10

GameMain* GameMain::self = nullptr;

GameMain::GameMain()
{
	ZeroMemory(m_SocketDatas, sizeof(m_SocketDatas));
}

void GameMain::Connect(const char * host, USHORT port)
{
	m_Client.onconnect_error = OnConnectError;
	m_Client.onconect = OnConnect;
	m_Client.onread = OnRead;
	m_Client.onclose = OnClose;
	m_Client.onerror = OnError;

	m_Client.Connect(host, port);
}

void GameMain::ReConnect(const char * host, USHORT port)
{
	if (m_iReConnect++ == 500)
		return;

	Sleep(5000);
	//printf("重连服务器...\n");
	Connect(host, port);
}

bool GameMain::InSocketData(SOCKET s, sockaddr_in* remote, USHORT allowPort)
{
	//if (htons(remote->sin_port) != allowPort)
	//	return false;
	u_short remote_port = ntohs(remote->sin_port);
	for (int i = 0; i < MAX_SOCKETDATA_NUM; i++) {
		SocketData* p = &m_SocketDatas[i];
		if (p->socket == 0 && 
			(remote_port == 6371 || remote_port == 6074)) {
			struct sockaddr_in local;
			int len = sizeof(local);
			getsockname(s, (struct sockaddr*)&local, &len);

			m_SocketDatas[i].socket = s;
			m_SocketDatas[i].remote_addr = remote->sin_addr.S_un.S_addr;
			m_SocketDatas[i].remote_port = ntohs(remote->sin_port);
			m_SocketDatas[i].local_port = ntohs(local.sin_port);

			//printf("%d(%lld). 本地端口:%d -> 远程:%s:%d\n", i + 1, p->socket, ntohs(p->local_port), inet_ntoa(p->addr), p->remote_port);
			return true;
		}
		else {
			if (0 && p->socket && m_Socket == 0) {
				struct sockaddr_in localaddr;
				int len = sizeof(localaddr);
				getsockname(p->socket, (struct sockaddr*)&localaddr, &len);

				//printf("%d(%lld)存在. 本地端口:%d -> 远程:%08x:%d\n", i + 1, p->socket, ntohs(localaddr.sin_port), p->remote_addr, p->remote_port);
			}

			//printf("%d. 没有过期 (%lld - %lld)\n", i + 1, now_time, g_socket_data[i].time_stamp);
		}
	}
	return false;
}

void GameMain::CloseSocketData(SOCKET s)
{
	if (s == 0)
		return;

	int i; 
	for (i = 0; i < MAX_SOCKETDATA_NUM; i++) {
		if (m_SocketDatas[i].socket == s) {
			m_SocketDatas[i].socket = 0;
		}
	}
	for (i = 0; i < MAX_BRW_SOCKET; i++) {
		if (s == m_BrowerSockets[i]) {
			m_BrowerSockets[i] = 0;
			printf("关闭SOKCET:%lld - %d\n", s, m_iBrowerSocketCount);
			m_Client.ClearSendString();
			m_Client.SetInt(0, (int)s);
			char* buff = m_Client.MakeSendString(OP_CLOSE);
			int send_len = send(m_Socket, buff, m_Client.GetSendLength(), 0);

			m_iBrowerSocketCount--;
			for (int j = i; j < m_iBrowerSocketCount; j++) {
				m_BrowerSockets[j] = m_BrowerSockets[j + 1];
			}
			if (m_iBrowerSocketCount < 0)
				m_iBrowerSocketCount = 0;
		}
	}
}

SocketData* GameMain::GetSocketDataByNetInfo(ULONG addr, USHORT port)
{
	SocketData* sd = nullptr;
	for (int i = 0; i < MAX_SOCKETDATA_NUM; i++) {
		if (m_SocketDatas[i].socket) {
			struct sockaddr_in local;
			int len = sizeof(local);
			getsockname(m_SocketDatas[i].socket, (struct sockaddr*)&local, &len);

			//printf("比较：%08x:%d = %08x:%d\n", addr, port, m_SocketDatas[i].remote_addr, ntohs(local.sin_port));
			if (addr == m_SocketDatas[i].remote_addr && port == ntohs(local.sin_port)) {
				sd = &m_SocketDatas[i];
			}
			else {
				m_SocketDatas[i].socket = 0;
			}
		}
	}
	return sd;
}

void GameMain::CheckActivePort(ULONG addr, USHORT port)
{
	printf("获得端口号：%08x:%d\n", addr, port);
	SocketData* p = GetSocketDataByNetInfo(addr, port);
	if (p) {
		if (m_iBrowerSocketCount < MAX_BRW_SOCKET) {
			printf("找到对应浏览器SOKCET：%lld，端口：%d, %d\n", p->socket, port, m_iBrowerSocketCount);
			m_BrowerSockets[m_iBrowerSocketCount++] = p->socket;
		}
		else {
			printf("找到对应浏览器SOKCET，但连接超出最大值:%d\n", p->socket, m_iBrowerSocketCount);
			p = nullptr;
		}
	}

	m_Client.ClearSendString();
	m_Client.SetInt(0, port);
	m_Client.SetInt(4, p ? 1 : 0);
	m_Client.SetInt(8, p ? p->socket: 0);
	m_Client.SetInt(12, GetCurrentProcessId());
	char* buff = m_Client.MakeSendString(OP_ACTIVE_PORT);
	int send_len = send(m_Socket, buff, m_Client.GetSendLength(), 0);
	//printf("回应服务器：%d 错误码：%d\n", send_len, GetLastError());
}

void GameMain::SendData(int opcode, const char* data, int len)
{
	//printf("获得发送数据，长度：%d\n", len - 4);
	int send_len = 0;
	int browser_socket = Client::s_n2hi(&data[4]);
	if (browser_socket) {
		/*for (int i = 0; i < 16; i++) {
			printf("%02x ", data[4+i]&0xff);
		}
		printf("\n");*/
		if (opcode == OP_SEND_FIRE) {
			printf("发送子弹数据！\n");
		}
		else if (opcode == OP_SEND_FISH) {
			printf("发送命中鱼数据！\n");
		}
		send_len = send(browser_socket, &data[8], len - 8, 0);
	}
	else {
		send_len = -1;
	}
	// printf("发送到游戏服务器数据长度：%d 错误码：%d\n", send_len, GetLastError());

	m_Client.ClearSendString();
	m_Client.SetInt(0, Client::s_n2hi(&data[0]));
	m_Client.SetInt(4, browser_socket);
	m_Client.SetInt(8, send_len);
	char* buff = m_Client.MakeSendString(opcode);
	send_len = send(m_Socket, buff, m_Client.GetSendLength(), 0);
	//printf("回应服务器：%d\n", send_len);
}

void GameMain::Close()
{
	m_Client.ClearSendString();
	m_Client.SetInt(0, 0);
	char* buff = m_Client.MakeSendString(OP_CLOSE);
	send(m_Socket, buff, m_Client.GetSendLength(), 0);
}

void GameMain::OnConnectError(const char* host, USHORT port)
{
	//printf("连接不上服务器！\n");
	self->ReConnect(host, port);
}

void GameMain::OnConnect(const char * host, USHORT port, SOCKET s)
{
	printf("连接服务器成功！\n");
	self->m_Socket = s;
	self->m_iReConnect = 0;
}

void GameMain::OnRead(const char * host, USHORT port, int opcode, const char* data, int len)
{
	/*printf("开始读取，指令:%d 数据长度：%d\n", opcode, len);
	for (int i = 0; i < 4; i++) {
		printf("%02x ", data[i] & 0xff);
	}
	printf("\n");*/
	switch (opcode)
	{
	case OP_ACTIVE_PORT:
		self->CheckActivePort(Client::s_n2hi(&data[0]), Client::s_n2hi(&data[4]));
		break;
	case OP_SEND_FIRE:
	case OP_SEND_FISH:
	case OP_SEND_DATA:
		self->SendData(opcode, data, len);
		break;
	case OP_SETDSDLLPATH:
		if (self->m_DSDllPathFunc) {
			self->m_DSDllPathFunc((wchar_t*)data, len);
		}
		break;
	default:
		break;
	}
}

void GameMain::OnClose(const char * host, USHORT port)
{
	printf("服务器关闭！\n");
	self->m_Socket = 0;
	self->ReConnect(host, port);
}

void GameMain::OnError(const char * host, USHORT port)
{
	printf("连接发送错误！\n");
	self->m_Socket = 0;
}
