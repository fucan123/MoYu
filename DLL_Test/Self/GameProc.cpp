#include "GameClient.h"
#include "GameProc.h"
#include "Game.h"
#include "GameConf.h"
#include "GameStep.h"
#include "Move.h"
#include "Item.h"
#include "GuaiWu.h"
#include "Talk.h"
#include "Magic.h"
#include "Pet.h"

#include <ShlObj_core.h>
#include <My/Common/mystring.h>
#include <My/Common/func.h>
#include <My/Driver/KbdMou.h>
#include <psapi.h>
#include <stdio.h>
#include <time.h>

#if 1
#if 0
#define SMSG_D(v) m_pGame->m_pClient->SendMsg2(v)
#else
#define SMSG_D(v) WriteLog(v);
#endif
#define SMSG_DP(p,...) { sprintf_s(p,__VA_ARGS__);SMSG_D(p); }
#else
#define SMSG_D(v) 
#define SMSG_DP(v) 
#endif
#define SMSG(v) SendMsg(v)
#define SMSG_P(p,...) { sprintf_s(p,__VA_ARGS__);SMSG(p); }
#define SMSG_N(n,...) {char _s[n]; SMSG_P(_s,__VA_ARGS__); }

// !!!
GameProc::GameProc(Game* pGame)
{
	m_pGame = pGame;
	m_pGameStep = new GameStep;
}

// ��ʼ������
void GameProc::InitData()
{
	m_bStop = false;
	m_bPause = false;
	m_bReStart = false;
	m_i64SearchKairi = 0;
	m_bLockGoFB = false;
	m_bAtFB = false;
	m_iNeedYao = 6;
	m_iNeedBao = 6;
	m_pStepCopy = nullptr;

	ZeroMemory(&m_stLastStepInfo, sizeof(m_stLastStepInfo));
	m_pGame->m_pMove->InitData();
	m_pGame->m_pItem->InitData();
	m_pGame->m_pTalk->InitData();
	m_pGame->m_pGuaiWu->InitData();
	m_pGame->m_pPet->InitData();
}

// ȡ���������İ�ť
void GameProc::CancelAllButton()
{
	if (!m_pGame->IsBig()) {
		for (int i = 1; i <= 10; i++) {
			if (Button(BUTTON_ID_TEAMFLAG)) {
				Sleep(2000);
				Button(BUTTON_ID_CANCEL, 1500);
			}
			else {
				break;
			}
		}
	}
	
	Button(BUTTON_ID_CANCEL, 1500);
	Button(BUTTON_ID_CLOSEMENU, 1500);
}

// ִ��
void GameProc::Exec()
{
	bool have_team = m_pGame->Call_IsHaveTeam();
	GoLeiMing();
	CancelAllButton();
	if (m_pGame->IsInFB()) { // �ڸ�������
		if (have_team) { // �ж���, ����ˢ
			m_bAtFB = true;
			return;
		}
		else {
			GoOutFB("�����Ǳ�������", false);
		}
	}

	if (m_pGame->IsBig()) {
		SellItem();

		if (!have_team) {
			m_pGame->Call_TeamCreate(); // ��Ŵ�������
			Sleep(1000);
		}
		int has = m_pGame->Call_IsHaveTeam();
		int leader = m_pGame->Call_IsTeamLeader();
		printf("�Ƿ��ж���:%d �ӳ�:%d\n", has, leader);
	}
	else {
		printf("�Ƿ���������:%d\n", m_pGame->m_Account.IsGetXL);
		if (!m_pGame->m_Account.IsGetXL) {
			printf("ȥ������\n");
			GoGetXiangLian(); // ��ȡ����
		}
	}
		
	GoFBDoor(); // ȥ�����ſ�
	if (!have_team) {
		// ���߿��������
		m_pGame->m_pClient->Send(SCK_CANTEAM, true);
	}
	else {
		m_pGame->m_pClient->Send(SCK_CANINFB, true);
	}
}

// ����
void GameProc::Run()
{
	//return;
	//m_pGame->KeyDown('I');
	//Sleep(100);
	//m_pGame->KeyUp('I');
	char log[64];
	//INLOGVARN(32, "����%.2f", asin(0.5f));
#if RUNRUN == 0
	return;
#endif
#if 0
	m_bPause = true;
	printf("ѡ�������ļ���A:ħ�򸱱�����.txt B:ħ�򸱱�����2.txt\n");
	while (m_bPause) {
		Sleep(500);
	}
#endif
	m_pGame->m_pGameConf->ReadConf();
	if (!m_pGameStep->InitSteps(m_pGame->m_pGameConf->m_Setting.FBFile)) {
		printf("��ʼ������ʧ�ܣ��޷��������У�����\n");
		return;
	}
start:
	InitData();
	//m_bPause = true;
	if (m_bPause) {
		printf("��C��ʼ\n");
	}
	while (m_bPause) {
		Sleep(500);
	}
	//ReadQuickKey2Num();
	//INLOGVARP(log, "��ݼ�����Ʒ����:%d %d", m_QuickKey2Nums[0], m_QuickKey2Nums[1]);
	m_dwKairi = 0;
	int n = 0;
	int exec_time = time(nullptr) + 2;
	while (true) {
		int c = exec_time - time(nullptr);
		if (c >= 0 && n != c) {
			printf("׼����ʼִ��[%02d��]\n", c);
		}
		n = c;
		if (c < 0) {
			break;
		}
		Sleep(500);
	}

	printf("��ʼִ������\n");
	m_pGame->m_pPet->PetOut(m_pGame->m_pGameConf->m_stPetOut.No, m_pGame->m_pGameConf->m_stPetOut.Length, true);
	Exec();
	//Drv_MouseMovAbsolute(m_pGame->m_GameWnd.Rect.left, m_pGame->m_GameWnd.Rect.top);
	//INLOGVARN(32, "Step:%08x Stop:%d", step, m_bStop);
	bool send_at = false, send_out = true;
	int start_time = time(nullptr);
	while (true) {
		if (m_bAtFB || m_pGame->IsInFB()) {
			if (!send_at) {
				m_pGame->m_pClient->Send(SCK_ATFB, true); // ֪ͨ��������ڸ�����
				send_at = true;
			}
			
			InitData();
			ExecInFB(); // ִ�и�������
		}
		else {
			send_at = false;
		}
		Sleep(1000);
	}
	//GoLeiMing();
	//while (ExecStep(m_pGameStep->m_Step));
end:
	int second = time(nullptr) - start_time;
	printf("��ɣ�����ʱ:%02d��%02d��\n", second/60, second%60);
	if (m_bReStart) {
		printf("GameProc::Run���¼���...\n");
		Run();
	}
	else {
		m_pGameStep->ResetStep();
		m_bPause = true;
		//goto start;
	}
}

// �������
bool GameProc::ViteInTeam(const char* name)
{
	printf("�������:%s���\n", name);
	for (int i = 0; i < 10; i++) {
		DWORD id = m_pGame->m_pTalk->GetNPCId(name);
		if (id) {
			m_pGame->Call_TeamInvite(id);
			printf("���������:%s���\n", name);
			return true;
		}
		Sleep(1000);
	}
	printf("���:%s�Ҳ���\n", name);
	return false;
}

// ���
void GameProc::InTeam()
{
	for (int i = 1; i <= 10; i++) {
		printf("��%d��Ѱ��������İ�ť\n", i);
		if (Button(BUTTON_ID_TEAMFLAG))
			break;

		Sleep(1000);
	}

	Sleep(2000);
	Button(BUTTON_ID_INTEAM,    1500, "ͬ��");
	Button(BUTTON_ID_CLOSEMENU, 0);
	printf("ͬ��������\n");
	SendMsg("ͬ�����");
}

// ���������
void GameProc::ViteInFB()
{
	printf("���������\n");
	Wait(2 * 1000);
	m_pGame->Call_CheckTeam();
	Wait(1 * 1000);
	for (int i = 0; i < 5; i++) {
		if (Button(BUTTON_ID_SURE))
			break;
		Sleep(1000);
	}

	m_bAtFB = true;
	SendMsg("���������");
}

// ͬ�������
void GameProc::AgreeInFB()
{
	if (m_bAtFB) {
		SendMsg("�Ѿ��ڸ���, ����ͬ��");
		return;
	}
	
	int now_time = time(nullptr);
	while (m_bLockGoFB) {
		if ((time(nullptr) - now_time) > 60)
			break;

		printf("�ȴ�ͬ����븱��\n");
		Sleep(1500);
	}
	printf("ͬ����븱��\n");
	SendMsg("ͬ����븱��");
	m_pGame->m_Account.IsMeOpenFB = false;
	for (int i = 0; i < 3; i++) {
		Sleep(1000);
		if (Button(BUTTON_ID_INFB, 0, "ͬ��")) {
			Wait(5 * 1000);
			m_bAtFB = true;
			printf("��ͬ��\n");
			return;
		}	
	}
	printf("�Ҳ���ͬ�ⰴť\n");
	SendMsg("�Ҳ���ͬ���������ť");
}

// ���ȥ������½����
void GameProc::GoLeiMing()
{
	if (!m_pGame->IsInMap("���")) {
		printf("�����������\n");
		return;
	}

	SendMsg("ȥ������½");
	printf("���������\n");
	while (ExecStep(m_pGameStep->m_GoLeiMingStep));
	m_pGameStep->ResetStep();
}

// ȥ��ȡ����
void GameProc::GoGetXiangLian()
{
	int x = 250, y = 500;
	if (m_pGame->IsGetXL())
		goto end;

	SendMsg("ȥ������");
	if (m_pGame->IsInArea(x, y))
		goto get;

	for (int j = 0; j < 5; j++) {
		if (!m_pGame->m_pItem->UseSelfItem("�ǳ�֮��", x, y, 10)) {
			SendMsg("û���ǳ�֮��, �޷�������");
			goto end;
		}
		for (int i = 0; i < 5; i++) {
			Sleep(1000);
			if (m_pGame->IsInArea(x, y))
				goto get;
		}
	}

get:
	m_pGame->m_pTalk->NPC("�ó�ʦ������");
	Wait(2 * 1000);
	m_pGame->m_pTalk->NPCTalk(0x03); // ��ȡ���ƾ�����

	// ÿ����3������
	m_pGame->m_pTalk->NPC("�ó�ʦ������");
	Wait(2 * 1000);
	m_pGame->m_pTalk->NPCTalk(0x04);

	Wait(1 * 1000);
	m_pGame->m_pTalk->NPCTalk(0xff);

	m_pGame->m_Account.IsGetXL = 1;
	m_pGame->m_Account.GetXLTime = time(nullptr);

end:
	if (m_pGame->m_Account.GetXLLogout) { // �����������˳�
		SendMsg("����ȡ����");
		Sleep(500);
		m_pGame->Close(false);
	}	
}

// ȥ�����ſ�
void GameProc::GoFBDoor()
{
	SendMsg("ȥ�����ſ�");
	int x = 865, y = 500;
	if (m_pGame->IsInArea(x, y))
		return;
	for (int j = 0; j < 50; j++) {
		if (!m_pGame->m_pItem->UseSelfItem("�ǳ�֮��", x, y, 10)) {
			printf("û���ǳ�֮��\n");
			break;
		}
		for (int i = 0; i < 5; i++) {
			Sleep(1000);
			if (m_pGame->IsInArea(x, y))
				return;
		}
	}

	SendMsg("δ��ȥ�����ſ�, ��û���ǳ�֮��(865,500)");
}

// ������
bool GameProc::GoInFB()
{
	if (m_pGame->m_Account.IsMeOpenFB) {
		printf("�ȴ��ٴν��븱��\n");
		SendMsg("30���������");
		Wait(30 * 1000);
	}

	int select_no = 0x01;
	int select_count = 1000;
	if (m_pGame->IsBig()) { // ��Ž���
		SYSTEMTIME stLocal;
		::GetLocalTime(&stLocal);
		if (stLocal.wHour >= 20 && stLocal.wHour <= 23) { // �����ʱ�������
			select_no = 0x00; // ��һ��ѡ��
			select_count = 3; // ������
			if (!m_pGame->m_Account.IsMeOpenFB) {
				Wait(30 * 1000);
			}
		}
	}

	SendMsg("��������");
	while (true) {
		for (int j = 0; j < select_count; j++) {
			m_pGame->m_pTalk->NPC("�����Ǳ�������");
			Wait(1 * 1000);
			m_pGame->m_pTalk->NPCTalk(select_no);
			Wait(1 * 1000);
			m_pGame->m_pTalk->NPCTalk(0x00);
			Wait(5 * 1000);

			if (m_pGame->IsInFB()) {
				Sleep(1000);
				ViteInFB();
				m_pGame->m_Account.IsMeOpenFB = true;
				return true;
			}
			Sleep(1000);
		}
		if (select_no == 0x00) {
			select_no = 0x01;
			select_count = 1000;
		}
		else {
			break;
		}
	}
	
	return false;
}

// ������
void GameProc::GoOutFB(const char* name, bool notify)
{
	printf("GoOutFB:%d\n", m_pGame->IsBig());

	if (!m_pGame->IsBig()) { // С����Ҫ����
		m_pGame->Call_ReBoren(); // ����
		while (IsNeedAddLife() == -1) {
			Sleep(500);
			m_pGame->Call_ReBoren(); // ����
			Sleep(500);
		}
	}
	if (!m_pGame->IsInFB())
		goto end;

	for (int i = 0; true; i++) { // �����
		if (!m_pGame->IsInFB()) { // ��ȥ��
			break;
		}

		printf("���µ�%d�ζԻ�������\n", i + 1);
		m_pGame->m_pTalk->NPC(name);
		Wait(1 * 1000);
		m_pGame->m_pTalk->NPCTalk(0x00);
		Wait(1 * 1000);
		m_pGame->m_pTalk->NPCTalk(0x00);
		Wait(1 * 1000);
		m_pGame->m_pTalk->NPCTalk(0x00);
		Wait(1 * 1000);
	}
end:
	m_bAtFB = false;
	if (notify && !m_bSendOut) {
		SendMsg("������");
		m_bSendOut = true;
		m_pGame->m_pClient->Send(SCK_OUTFB, true); // ֪ͨ������
	}	
}

// ִ�и�������
void GameProc::ExecInFB()
{
	m_bSendOut = false;
	if (m_pGame->IsBig()) {
		printf("ִ�и�������\n");
		SendMsg("��ʼˢ����");
		int start_time = time(nullptr);
		
		// ��������
		m_pGame->m_pPet->PetOut(m_pGame->m_pGameConf->m_stPetOut.No, m_pGame->m_pGameConf->m_stPetOut.Length, true);
		// ��ǮǮ
		SaveMoney();

		while (ExecStep(m_pGameStep->m_Step)); // ���ˢ����
		m_bLockGoFB = false;

		int second = time(nullptr) - start_time;
		printf("ִ�и���������ɣ�����ʱ:%02d��%02d��\n", second / 60, second % 60);
		char log[64];
		sprintf_s(log, "���ˢ����������ʱ:%02d��%02d��", second / 60, second % 60);
		SendMsg(log);

		m_pGameStep->ResetStep(0);
	}
}

// ִ������
bool GameProc::ExecStep(vector<_step_*>& link)
{
	m_pStep = m_pGameStep->Current(link);
	if (!m_pStep)
		return false;

	_step_* m_pTmpStep = m_pStep; // ��ʱ��
	if (m_pStepCopy) { // �Ѿ�ִ�е��Ĳ���
		if (m_pStep == m_pStepCopy) {
			m_pStepCopy = nullptr;
		}
		else {
			if (m_pStep->OpCode != OP_MOVE && m_pStep->OpCode != OP_MAGIC) { // �����ƶ���ż���
				return m_pGameStep->CompleteExec(link) != nullptr;
			}
		}
	}

	if (m_pStep->OpCode != OP_NPC && m_pStep->OpCode != OP_SELECT) {
		SMSG_D("�������");
		// ��������û��Ѫ������
		if (m_pGame->m_pPet->Revive()) {
			Wait(1 * 1000);
		}
		SMSG_D("�������->���");
#if 0
		// ����һЩҩƷ
		m_pGame->m_pItem->DropSelfItemByType(��Ч����ҩˮ, m_iNeedYao);
		m_pGame->m_pItem->DropSelfItemByType(��Ч���ư�, m_iNeedBao);
		SMSG_D("������Ʒ->���");
#endif
	}

	char msg[128];
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
		printf("����->�ƶ�:%d.%d\n", m_pStep->X, m_pStep->Y);
		SMSG_DP(msg, "����->�ƶ�.%d,%d", m_pStep->X, m_pStep->Y);
		Move();
		break;
	case OP_MOVEFAR:
		printf("����->����:%d.%d\n", m_pStep->X, m_pStep->Y);
		SMSG_DP(msg, "����->����.%d,%d", m_pStep->X, m_pStep->Y);
		Move();
		break;
	case OP_NPC:
		printf("����->NPC:%s\n", m_pStep->NPCName);
		SMSG_DP(msg, "����->NPC.%s ����{%d,%d}", m_pStep->NPCName, m_stLastStepInfo.MvX, m_stLastStepInfo.MvY);
		NPC();
		break;
	case OP_SELECT:
		printf("����->ѡ��:%d\n", m_pStep->SelectNo);
		SMSG_DP(msg, "����->ѡ��:%d", m_pStep->SelectNo);
		Select();
		break;
	case OP_MAGIC:
		printf("����->����:%s\n", m_pStep->Magic);
		SMSG_DP(msg, "����->����.%s", m_pStep->Magic);
		Magic();
		break;
	case OP_MAGIC_PET:
		printf("����->���＼��:%s\n", m_pStep->Magic);
		SMSG_DP(msg, "����->���＼��:%s", m_pStep->Magic);
		MagicPet();
		break;
	case OP_CRAZY:
		SMSG_D("����->���");
		Crazy();
		break;
	case OP_CLEAR:
		SMSG_D("����->����");
		Clear();
		break;
	case OP_PICKUP:
		printf("����->��ʰ��Ʒ\n");
		SMSG_D("����->��ʰ��Ʒ");
		PickUp();
		break;
	case OP_CHECKIN:
		printf("����->������Ʒ\n");
		SMSG_D("����->������Ʒ");
		CheckIn();
		break;
	case OP_USEITEM:
		printf("����->ʹ����Ʒ\n");
		SMSG_D("����->ʹ����Ʒ");
		UseItem();
		break;
	case OP_DROPITEM:
		printf("����->������Ʒ:%s\n", m_pStep->Name);
		SMSG_DP(msg, "����->������Ʒ:%s", m_pStep->Name);
		DropItem();
		break;
	case OP_SELL:
		printf("����->������Ʒ\n");
		SMSG_D("����->������Ʒ");
		SellItem();
		break;
	case OP_BUTTON:
		printf("����->��ť:%08X\n", m_pStep->ButtonId);
		SMSG_D("����->��ť");
		Button();
		break;
	case OP_WAIT:
		printf("����->�ȴ�:%d %s\n", m_pStep->WaitMs / 1000, m_pStep->Magic);
		SMSG_DP(msg, "����->�ȴ�:%d", m_pStep->WaitMs / 1000);
		Wait();
		break;
	default:
		SMSG_D("����->δ֪");
		break;
	}

	SMSG_D("����->ִ�����");

	int drop_i = 0;
	int move_far_i = 0;
	do {
		do {
			if (m_bStop || m_bReStart)
				return false;
			if (m_bPause)
				Sleep(500);
		} while (m_bPause);

		Sleep(500);
		__int64 now_ms = getmillisecond();

		if (m_pStep->OpCode != OP_DROPITEM) {
			SMSG_D("�жϼ�Ѫ");
			int life_flag = IsNeedAddLife();
			if (life_flag == 1) { // ��Ҫ��Ѫ
				SMSG_D("��Ѫ");
				AddLife();
				SMSG_D("��Ѫ->���");
			}
			else if (life_flag == -1) { // ��Ҫ����
				ReBorn(); // ����
				m_pStepCopy = m_pStep; // ���浱ǰִ�и���
				m_pGameStep->ResetStep(0); // ���õ���һ��
				printf("���õ���һ��\n");
				return true;
			}
			SMSG_D("�жϼ�Ѫ->���");
		}
		
		//SMSG_D("�ж�����");
		bool complete = StepIsComplete();
		//SMSG_D("�ж�����->���");
		if (m_pStep->OpCode == OP_MOVEFAR) {
			if (++move_far_i == 300) {
				printf("���ͳ�ʱ\n");
				complete = true;
			}
		}

		if (!m_pStep) {
			SMSG_D("!m_pStep");
			printf("!m_pStep\n");
			while (true) Sleep(1000);
		}
		if (m_pStep != m_pTmpStep) {
			SMSG_D("m_pStep != m_pTmpStep");
			printf("m_pStep != m_pTmpStep\n");
			while (true) Sleep(1000);
		}

		if (complete) { // ����ɴ˲���
			SMSG_D("����->����ɴ˲���");
			if (m_pStep->OpCode != OP_NPC && m_pStep->OpCode != OP_SELECT && m_pStep->OpCode != OP_WAIT) {
				if (m_pGame->m_pTalk->NPCTalkStatus()) // �Ի����Ǵ򿪵�
					m_pGame->m_pTalk->NPCTalk(0xff);   // �ص���
			}

			_step_* next = m_pGameStep->CompleteExec(link);
			if (m_pStep->OpCode != OP_WAIT && next) {
				if (next->OpCode == OP_MOVE) {
					Sleep(100);
				}
				else {
					Sleep(100);
				}
			}
			return next != nullptr;
		}
	} while (true);

	return false;
}

// �����Ƿ���ִ�����
bool GameProc::StepIsComplete()
{
	bool result = false;
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
		SMSG_D("�ж��Ƿ��ƶ����յ�&�Ƿ��ƶ�", true);
		if (m_pGame->m_pMove->IsMoveEnd()) { // �ѵ�ָ��λ��
			result = true;
			goto end;
		}
		SMSG_D("�ж��Ƿ��ƶ����յ�->���", true);
		if (!m_pGame->m_pMove->IsMove()) {   // �Ѿ�ֹͣ�ƶ�
			m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y);
		}
		SMSG_D("�ж��Ƿ��ƶ�->���", true);
		break;
	case OP_MOVEFAR:
		if (m_pGame->IsNotInArea(m_pStep->X, m_pStep->Y, 50)) { // �Ѳ��ڴ�����
			result = true;
			goto end;
		}
		if (m_pGame->m_pMove->IsMoveEnd()) { // �ѵ�ָ��λ��
			m_pGame->m_pMove->Run(m_pStep->X - 5, m_pStep->Y);
		}
		if (!m_pGame->m_pMove->IsMove()) {   // �Ѿ�ֹͣ�ƶ�
			m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y);
		}
		break;
	case OP_NPC:
		result = m_pGame->m_pTalk->NPCTalkStatus();
		result = true;
		if (!result) { // �Ի���û�д�
			m_pGame->m_pTalk->NPC(m_pStep->NPCName);
		}
		break;
	case OP_SELECT:
		result = true;
		break;
	case OP_MAGIC:
		result = true;
		break;
	case OP_CRAZY:
		result = true;
		break;
	case OP_CLEAR:
		printf("���:%08X\n", m_pStep->Magic);
		result = m_pGame->m_pGuaiWu->Clear(m_pStep->Magic, 8, 8); // ���5*5��Χ�ڵĹ���
		break;
	case OP_PICKUP:
		result = true;
		break;
	case OP_WAIT:
		result = true;
		break;
	default:
		result = true;
		break;
	}
end:
	return result;
}

// �ƶ�
void GameProc::Move()
{
	if (m_pStep->X == 260 && m_pStep->Y == 54) { // ֪ͨ��ȥ
		m_bSendOut = true;
		m_pGame->m_pClient->Send(SCK_OUTFB, true); // ֪ͨ������
	}

	m_stLastStepInfo.MvX = m_pStep->X;
	m_stLastStepInfo.MvY = m_pStep->Y;
	m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y);
}

// NPC
void GameProc::NPC()
{
	// 500 - 930
	// 560 - 1120
	// https://31d7f5.link.yunpan.360.cn/lk/surl_yL2uvtfBesv#/-0
	if (0
		|| strcmp(m_pStep->NPCName, "̰������") == 0
		|| strcmp(m_pStep->NPCName, "Թ�޸�����") == 0
		|| strcmp(m_pStep->NPCName, "�Իø�����") == 0
		|| strcmp(m_pStep->NPCName, "�밮������") == 0) {
		printf("NPC����:%s\n", m_pStep->NPCName);
		char gw_name[16];
		int length = strstr(m_pStep->NPCName, "������") - m_pStep->NPCName;
		
		if (m_dwKairi == 0) {
			//m_dwKairi = 1;
		}
		if (0 && length > 0) {
			DWORD x = 8, y = 8;
			memset(gw_name, 0, sizeof(gw_name));
			memcpy(gw_name, m_pStep->NPCName, length);
			strcat(gw_name, "֮��");
			printf("Ҫ�����Ĺ���:%s %d,%d\n", gw_name, x, y);
			__int64 t = getmillisecond();
			while (!m_pGame->m_pGuaiWu->IsInArea(gw_name, x, y)) { // ����δ��
				if ((getmillisecond() - t) > 10000) {
					printf("10�뻹δ��ȡ������, ����\n");
					break;
				}
				Sleep(100);
			}
			m_pGame->m_pMagic->UseMagic(m_stLastStepInfo.Magic, x, y);
			Sleep(100);
		}
	}
	else if (strcmp(m_pStep->NPCName, "��һ��") == 0) { // �Ի���һ�����˻���NPC
		m_iNeedYao = 3;
		m_iNeedBao = 2;
		m_pGame->m_pTalk->NPC(m_stLastStepInfo.NPCId);
	}
	else {
		if (strcmp(m_pStep->NPCName, "������") == 0) {
			m_bLockGoFB = true; // �ȴ�����������
			printf("�Ի�.������[NPC]\n");
			GoOutFB("������");
			return;
		}
		m_bLastBoss = strcmp(m_pStep->NPCName, "�������Թ��") == 0;
		m_stLastStepInfo.NPCId = m_pGame->m_pTalk->NPC(m_pStep->NPCName);
		strcpy(m_stLastStepInfo.NPCName, m_pStep->NPCName);
	}
}

// ѡ��
void GameProc::Select()
{
	if (!m_stLastStepInfo.NPCId) // û����NPC�Ի�, û��Ҫ��ѡ��
		return;

	for (DWORD i = 1; i <= m_pStep->OpCount; i++) {
		if (1 || i < m_pStep->OpCount) { // �������һ��
			if (!m_pGame->m_pTalk->WaitTalkBoxOpen()) {        // �ȴ��Ի���򿪳�ʱ
				printf("���¶Ի�NPC\n");
				m_pGame->m_pTalk->NPC(m_stLastStepInfo.NPCId); // ������NPC�Ի�
				if (m_pGame->m_pTalk->WaitTalkBoxOpen()); // NPC��Ч
					//i--;
			}
		}
		printf("��(%d)��ѡ��\n", i);
		m_pGame->m_pTalk->NPCTalk(m_pStep->SelectNo);

		if (i > 1) {
			if (IsNeedAddLife() == 1) {
				AddLife();
			}
			Sleep(500);
		}
	}
}

// ����
void GameProc::Magic()
{
	if (m_pStep->WaitMs) { // ��Ҫ�ȴ���ȴ
		while (!m_pGame->m_pMagic->CheckCd(m_pStep->Magic)) {
			Sleep(100);
		}
	}

	DWORD x = m_pStep->X, y = m_pStep->Y;
	if (!x || !y) { // �ͷŵ�λ������
		m_pGame->ReadCoor(&x, &y);
	}
	m_pGame->m_pMagic->UseMagic(m_pStep->Magic, x, y);
	strcpy(m_stLastStepInfo.Magic, m_pStep->Magic);
	Sleep(50);
}

// ����-����
void GameProc::MagicPet()
{
	m_pGame->m_pPet->Magic(m_pStep->Magic[0]);
}

// ��˦
void GameProc::Crazy()
{
	m_bIsCrazy = strstr(m_pStep->Magic, "����") ? false : true;
	strcpy(m_CrazyMagic, m_pStep->Magic);
	if (m_bIsCrazy) {
		printf("������˦ģʽ\n");
	}
	else {
		printf("��˦ģʽ����\n");
	}
}

// ���
void GameProc::Clear()
{
}

// ����
void GameProc::PickUp()
{
	for (int i = 1; i <= 2; i++) {
		printf("��%d�μ���\n", i);
		m_pGame->m_pItem->PickUpItem(m_pGame->m_pGameConf->m_stPickUp.PickUps, 
			m_pGame->m_pGameConf->m_stPickUp.Length);
		Sleep(1000);
	}
}

// ��Ǯ
void GameProc::SaveMoney()
{
	Button(BUTTON_ID_ROLE, 1000);           // ������ﰴť  
	Button(BUTTON_ID_VIP, 1000);            // ���VIP��ť   
	if (Button(BUTTON_ID_CHECKIN, 1500)) {  // �����Ʒ�ֿⰴť
		m_pGame->Call_SaveMoney(30000000);
	}
}

// ����
DWORD GameProc::CheckIn(bool in)
{
	HWND child, parent;

	Button(BUTTON_ID_ROLE, 1000);           // ������ﰴť  
	Button(BUTTON_ID_VIP, 1000);            // ���VIP��ť   
	if (!Button(BUTTON_ID_CHECKIN, 1500)) { // �����Ʒ�ֿⰴť
		printf("�򿪲ֿ�ʧ��\n");
		return 0;
	}

	DWORD count = 0;
	if (in) {
		count = m_pGame->m_pItem->CheckInSelfItem(m_pGame->m_pGameConf->m_stCheckIn.CheckIns,
			m_pGame->m_pGameConf->m_stCheckIn.Length);
	}
	else {
		count = m_pGame->m_pItem->CheckOutItem(m_pGame->m_pGameConf->m_stSell.Sells,
			m_pGame->m_pGameConf->m_stSell.Length);
	}
	
	Button(BUTTON_ID_CLOSECKIN); // ����رղֿⰴť
	return count;
}

// ʹ����Ʒ
void GameProc::UseItem()
{
	m_pGame->m_pItem->UseSelfItem(m_pStep->Name, m_pStep->X, m_pStep->Y, m_pStep->Extra[0]);
}

// ����Ʒ
void GameProc::DropItem()
{
	int live_count = m_pStep->Extra[0];
	if (live_count == 0) {
		if (strcmp("��Ч����ҩˮ", m_pStep->Name) == 0)
			live_count = m_iNeedYao;
		else if (strcmp("��Ч���ư�", m_pStep->Name) == 0)
			live_count = m_iNeedBao;
	}
	m_pGame->m_pItem->DropSelfItemByName(m_pStep->Name, 6);
	Sleep(1000);
	m_pGame->m_pItem->ReadYaoBao(0);
	m_pGame->m_pItem->ReadYaoBao(1);
}

// ������Ʒ
void GameProc::SellItem()
{
	int x = 295, y = 490;
	if (m_pGame->IsInArea(x, y))
		goto sell;
	for (int j = 0; j < 5; j++) {
		m_pGame->m_pItem->UseSelfItem("�ǳ�֮��", x, y, 10);
		for (int i = 0; i < 5; i++) {
			Sleep(1000);
			if (m_pGame->IsInArea(x, y))
				goto sell;
		}
	}

sell:
	// �Ի�NPC������
	printf("������\n");
	SendMsg("������");

	m_pGame->m_pTalk->NPC("ά������");
	Wait(2 * 1000);
	m_pGame->m_pTalk->NPCTalk(0x00, true);

	m_pGame->m_pItem->SellSelfItem(m_pGame->m_pGameConf->m_stSell.Sells,
		m_pGame->m_pGameConf->m_stSell.Length);

	while (true) {
		DWORD count = m_pGame->m_pItem->CheckOutItem(m_pGame->m_pGameConf->m_stSell.Sells,
			m_pGame->m_pGameConf->m_stSell.Length);
		if (count == 0)
			break;

		m_pGame->m_pItem->SellSelfItem(m_pGame->m_pGameConf->m_stSell.Sells,
			m_pGame->m_pGameConf->m_stSell.Length);
	}

	Button(BUTTON_ID_CLOSESHOP, 500, "CLOSE"); // ����ر��̵갴ť

	m_bLockGoFB = false;
}

// ��ť
void GameProc::Button()
{
	Button(m_pStep->ButtonId);
}

// ��ť
bool GameProc::Button(int button_id, DWORD sleep_ms, const char* name)
{
	HWND child, parent;
	if (!m_pGame->FindButtonWnd(button_id, child, parent, name))
		return false;

	LRESULT result = SendMessageW(parent, WM_COMMAND, MAKEWPARAM(button_id, BN_CLICKED), (LPARAM)child); // �����ť
	if (sleep_ms) {
		Sleep(sleep_ms);
	}
	return result != 0;
}

// �ȴ�
void GameProc::Wait()
{
	if (strlen(m_pStep->Magic)) { // �ȴ�������ȴ
		while (!m_pGame->m_pMagic->CheckCd(m_pStep->Magic, m_pStep->WaitMs)) {
			Sleep(100);
		}
		strcpy(m_stLastStepInfo.Magic, m_pStep->Magic);
	}
	else { // �ȴ�
		Wait(m_pStep->WaitMs);
	}
}

// �ȴ�
void GameProc::Wait(DWORD ms)
{
	if (ms < 1000) {
		printf("�ȴ�%d����\n", ms);
		Sleep(ms);
		return;
	}

	int n = 0;
	__int64 now_time = getmillisecond();
	while (true) {
		int c = getmillisecond() - now_time;
		c = (int)ms - c;
		int ls = c / 1000;

		if (c >= 0 && n != ls) {
			printf("�ȴ�%02d�룬��ʣ%02d��\n", ms / 1000, ls);
		}
		if (c <= 0) {
			break;
		}
		n = ls;
		Sleep(100);
	}
}

// ����
void GameProc::ReBorn()
{
	printf("û��Ѫ����, �ȴ�����\n");
	SMSG_N(64, "��Ҫ����, λ��:%d,%d", m_pStep->X, m_pStep->Y);
	Wait(25 * 1000);
	m_pGame->m_pClient->Send(SCK_REBORN, true); // ֪ͨ����˸�����
	m_pGame->Call_ReBoren();

	Sleep(1000);
	// ��������
	m_pGame->m_pPet->PetOut(m_pGame->m_pGameConf->m_stPetOut.No, m_pGame->m_pGameConf->m_stPetOut.Length, true);
}

// �Ƿ��ڸ���
bool GameProc::IsInFB()
{
	return m_pGame->IsInFB();
}

// ��ȡ��������
bool GameProc::ReadCoor()
{
	return m_pGame->ReadCoor(&m_iCoorX, &m_iCoorY);
}

// ��ȡ����Ѫ��
bool GameProc::ReadLife()
{
	return m_pGame->ReadLife(m_iLife, m_iLifeMax);
}

// ��ȡ��ݼ�������Ʒ����
bool GameProc::ReadQuickKey2Num()
{
	return m_pGame->ReadQuickKey2Num(m_QuickKey2Nums, sizeof(m_QuickKey2Nums));
}

// ��ȡ������Ʒ
bool GameProc::ReadBag()
{
	return m_pGame->ReadBag(m_dwBag, sizeof(m_dwBag));
}

// �Ƿ���Ҫ��Ѫ��
int GameProc::IsNeedAddLife()
{
	if (getmillisecond() < (m_i64AddLifeTime + 350)) // 1���ڲ��ظ���
		return 0;
	if (!ReadLife())
		return 0;
	if (m_iLife == 0)
		return -1;
		
	return (m_iLife + 5000) <= m_iLifeMax ? 1 : 0;
}

// ��Ѫ
void GameProc::AddLife()
{
	if (m_pGame->m_pItem->UseYao()) {
		printf("��Ѫ.\n");
		m_i64AddLifeTime = getmillisecond();
	}
	else {
		printf("û��ҩ\n");
	}
}

// ������Ϣ�������
void GameProc::SendMsg(const char * v, const char * v2)
{
	m_pGame->m_pClient->SendMsg(v, v2);
}

// ֹͣ
void GameProc::Stop(bool v)
{
	INLOGVARN(32, "Set Stop:%d", v);
	m_bStop = v;
}

// ���ռ��ļ�
void GameProc::OpenLogFile()
{
	char logfile[255];
	SHGetSpecialFolderPathA(0, logfile, CSIDL_DESKTOPDIRECTORY, 0);
	strcat(logfile, "\\MoYu\\�ռ�.txt");

	m_LogFile.open(logfile);
	printf("�ռ�:%s\n", logfile);
}

// д���ռ�
void GameProc::WriteLog(const char* log, bool flush)
{
	return;
	if (!m_pGame->IsBig())
		return;

	m_LogFile << log << endl;
	if (flush) {
		m_LogFile.flush();
	}
}


