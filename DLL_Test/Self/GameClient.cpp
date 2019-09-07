#include "GameClient.h"
#include "Game.h"
#include "GameProc.h"
#include "Item.h"
#include <stdio.h>

#define RECV_NAME(var,data,len)  char var[32]; ZeroMemory(var,sizeof(var)); memcpy(var,data,len); 
#define RECV_ROLE(var,data,len)  char var[16]; ZeroMemory(var,sizeof(var)); memcpy(var,data,len); 

// ����ָ��
GameClient* GameClient::self = nullptr;

// ...
GameClient::GameClient(Game* p)
{
	m_pGame = p;
}

// ��������ָ��
void GameClient::SetSelf(GameClient* p)
{
	self = p;
}

void GameClient::Connect(const char* host, USHORT port)
{
	m_Client.onconnect_error = OnConnectError;
	m_Client.onconect = OnConnect;
	m_Client.onread = OnRead;
	m_Client.onclose = OnClose;
	m_Client.onerror = OnError;

	m_Client.Connect(host, port);
}

void GameClient::ReConnect(const char * host, USHORT port)
{
	Sleep(5000);
	printf("����������...\n");
	Connect(host, port);
}

// Ҫ��¼���ʺ�
void GameClient::Account(const char * data, int len)
{
	char ser_big[32], ser_small[32], name[32], pwd[32];
	ZeroMemory(ser_big, sizeof(name));
	ZeroMemory(ser_small, sizeof(name));
	ZeroMemory(name, sizeof(name));
	ZeroMemory(pwd, sizeof(pwd));

	const char* tmp = data;
	int ser_big_len = P2INT(tmp);
	memcpy(ser_big, tmp + 4, ser_big_len);
	tmp += 4 + ser_big_len;

	int ser_small_len = P2INT(tmp);
	memcpy(ser_small, tmp + 4, ser_small_len);
	tmp += 4 + ser_small_len;

	int name_len = P2INT(tmp);
	memcpy(name, tmp + 4, name_len);
	tmp += 4 + name_len;

	int pwd_len = P2INT(tmp);
	memcpy(pwd, tmp + 4, pwd_len);
	tmp += 4 + pwd_len;

	self->m_pGame->SetAccount(ser_big, ser_small, name, pwd, P2INT(tmp), P2INT(tmp + 4), P2INT(tmp + 8), P2INT(tmp + 0x0C));
	self->Send(SCK_LOGIN, true); // ���Ϳ��Ե�¼
}

// �Ѿ��������
void GameClient::CanTeam(const char* data, int len)
{
	int index = P2INT(data); // ������ʺ��б�����
	RECV_ROLE(role, data + 4, len - 4);
	int result = m_pGame->m_pGameProc->ViteInTeam(role);

	// ���߷���˷���������
	m_Client.ClearSendString();
	m_Client.SetInt(index); // ԭ�����ظ���
	m_Client.SetContent(&result, 4);
	Send(SCK_INTEAM, false);
}

// ���������
void GameClient::InTeam()
{
	m_pGame->m_pGameProc->InTeam();

	// ���߷������׼���ÿ��Խ�������
	Send(SCK_CANINFB, true);
}

// ���Խ�������
void GameClient::CanInFB()
{
}

// ��������
void GameClient::OpenFB()
{
	m_pGame->m_pGameProc->GoFBDoor();
	if (m_pGame->m_pGameProc->GoInFB()) {
		Send(SCK_INFB, true);
	}
}

// ���븱��
void GameClient::InFB()
{
	m_pGame->m_pGameProc->AgreeInFB();
}

// ��ȥ����
void GameClient::OutFB()
{
	if (!m_pGame->IsBig())
		m_pGame->m_pGameProc->GoOutFB("�����Ǳ�������");
}

// ��ȡ��������������
void GameClient::GetXL()
{
	// ��ȡ����
	m_pGame->m_pGameProc->GoGetXiangLian();

	int count = m_pGame->m_pItem->GetSelfItemCountByName("����������");
	printf("ӵ����������:%d\n", count);
	m_Client.ClearSendString();
	m_Client.SetInt(count);
	Send(SCK_GETXL, false);
}

// �ر���Ϸ
void GameClient::CloseGame(bool leave_team)
{
	m_pGame->Close(false);
}

// ������Ϣ
int GameClient::SendMsg(const char * v, const char * v2)
{
	m_Client.ClearSendString();
	m_Client.SetContent((void*)v, strlen(v));
	if (v2) {
		m_Client.SetContent((void*)"->", 2);
		m_Client.SetContent((void*)v2, strlen(v2));
	}
	return Send(SCK_MSG, false);
}

// ������Ϣ
int GameClient::SendMsg2(const char * v)
{
	m_Client.ClearSendString();
	m_Client.SetContent((void*)v, strlen(v));
	return Send(SCK_MSG2, false);
}

// ֪ͨ����Ϸ���ڴ�
int GameClient::SendOpen(float scale, HWND wnd, RECT& rect)
{
	m_Client.ClearSendString();
	m_Client.SetContent(&scale, sizeof(float));
	m_Client.SetContent(&wnd, sizeof(HWND));
	m_Client.SetContent(&rect, sizeof(RECT));
	return Send(SCK_OPEN, false);
}

// ֪ͨ��Ϸ������
int GameClient::SendInGame(const char* name, const char* role)
{
	int name_len = strlen(name);
	int role_len = strlen(role);

	m_Client.ClearSendString();
	m_Client.SetInt(name_len);
	m_Client.SetContent((void*)name, name_len);
	m_Client.SetInt(role_len);
	m_Client.SetContent((void*)role, role_len);
	return Send(SCK_INGAME);
}

// ���ͼ���ʲô��Ʒ
int GameClient::SendPickUpItem(const char * name)
{
	m_Client.ClearSendString();
	m_Client.SetContent((void*)name, strlen(name));
	return Send(SCK_PICKUPITEM, false);
}

// ��������
int GameClient::Send()
{
	int ret = send(m_Socket, m_Client.GetSendString(), m_Client.GetSendLength(), 0);
	//printf("����:%s(%d) %d\n", m_Client.GetSendString(), m_Client.GetSendLength(), ret);
	return ret;
}

// ��������
int GameClient::Send(SOCKET_OPCODE opcode, bool clear)
{
	if (clear) {
		m_Client.ClearSendString();
	}
	m_Client.MakeSendString(opcode);
	return Send();
}


void GameClient::OnConnectError(const char* host, USHORT port)
{
	//printf("���Ӳ��Ϸ�������\n");
	//self->ReConnect(host, port);
}

void GameClient::OnConnect(const char* host, USHORT port, SOCKET s)
{
	printf("���ӷ������ɹ�[%s:%d]\n", host, port);
	self->m_Socket = s;
}

void GameClient::OnRead(const char* host, USHORT port, int opcode, const char* data, int len)
{
	if (opcode != SCK_PING)
		printf("����>>ָ��:%d ���ݳ��ȣ�%d\n", opcode, len);
	/*for (int i = 0; i < 4; i++) {
		printf("%02x ", data[i] & 0xff);
	}
	printf("\n");*/
	switch (opcode)
	{
	case SCK_PING:
		self->Send(SCK_PING, true); // ��Ӧ�����
		break;
	case SCK_ACCOUNT:
		self->Account(data, len);
		break;
	case SCK_CANTEAM: // ���������
		self->CanTeam(data, len);
		break;
	case SCK_INTEAM:  // ���������
		self->InTeam();
		break;
	case SCK_CANINFB: // ���Խ�ȥ����
		self->CanInFB();
		break;
	case SCK_OPENFB:  // ��������
		self->OpenFB();
		break;
	case SCK_INFB:    // ��ȥ����
		self->InFB();
		break;
	case SCK_OUTFB:   // ��ȥ����
		self->OutFB();
		break;
	case SCK_GETXL:   // ��ȡ����
		self->GetXL();
		break;
	case SCK_CLOSE:   // �ر���Ϸ
		self->CloseGame(len == 0);		
		break;
	default:
		break;
	}
}

void GameClient::OnClose(const char* host, USHORT port)
{
	printf("�������ر�[%s:%d]��\n", host, port);
	self->m_Socket = 0;
	//self->ReConnect(host, port);
}

void GameClient::OnError(const char* host, USHORT port)
{
	printf("���ӷ��ʹ���[%s:%d]��\n", host, port);
	self->m_Socket = 0;
}