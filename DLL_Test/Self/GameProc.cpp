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

#include <My/Common/mystring.h>
#include <My/Common/func.h>
#include <My/Driver/KbdMou.h>
#include <psapi.h>
#include <stdio.h>
#include <time.h>

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
	m_bPause = true;
	m_bReStart = false;
	m_i64SearchKairi = 0;

	ZeroMemory(&m_stLastStepInfo, sizeof(m_stLastStepInfo));
	m_pGame->m_pMove->InitData();
	m_pGame->m_pItem->InitData();
	m_pGame->m_pTalk->InitData();
	m_pGame->m_pGuaiWu->InitData();
}

// ����
void GameProc::Run()
{
	//return;
	//Game::KeyDown('I');
	//Sleep(100);
	//Game::KeyUp('I');
	char log[64];
	//INLOGVARN(32, "����%.2f", asin(0.5f));
#if RUNRUN == 0
	return;
#endif
	m_bPause = true;
	printf("ѡ�������ļ���A:ħ�򸱱�����.txt B:ħ�򸱱�����2.txt\n");
	while (m_bPause) {
		Sleep(500);
	}
	m_pGame->m_pGameConf->ReadConf();
	if (!m_pGameStep->InitSteps(m_iChkFile)) {
		printf("��ʼ������ʧ�ܣ��޷��������У�����\n");
		return;
	}
start:
	InitData();
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

	DWORD dwPetNo[] = { 0, 1, 2 };
	m_pGame->m_pPet->PetOut(m_pGame->m_pGameConf->m_stPetOut.No, m_pGame->m_pGameConf->m_stPetOut.Length, true);

	printf("��ʼ\n");
	//Drv_MouseMovAbsolute(m_pGame->m_GameWnd.Rect.left, m_pGame->m_GameWnd.Rect.top);
	//return;
	//INLOGVARN(32, "Step:%08x Stop:%d", step, m_bStop);
	int start_time = time(nullptr);
	_step_* step;
	while (step = m_pGameStep->Current()) {
		m_pStep = step;

		switch (step->OpCode)
		{
		case OP_MOVE:
			printf("����->�����ƶ�:%d.%d\n", step->X, step->Y);
			Move();
			break;
		case OP_NPC:
			printf("����->NPC:%s\n", step->NPCName);
			NPC();
			break;
		case OP_SELECT:
			printf("����->ѡ��:%d\n", step->SelectNo);
			Select();
			break;
		case OP_MAGIC:
			printf("����->����:%s\n", step->Magic);
			Magic();
			break;
		case OP_MAGIC_PET:
			printf("����->���＼��:%s\n", step->Magic);
			MagicPet();
			break;
		case OP_CRAZY:
			Crazy();
			break;
		case OP_CLEAR:
			Clear();
			break;
		case OP_PICKUP:
			printf("����->��ʰ��Ʒ\n");
			PickUp();
			break;
		case OP_CHECKIN:
			printf("����->������Ʒ\n");
			CheckIn();
			break;
		case OP_USEITEM:
			printf("����->ʹ����Ʒ\n");
			UseItem();
			break;
		case OP_SELL:
			printf("����->������Ʒ\n");
			SellItem();
			break;
		case OP_WAIT:
			printf("����->�ȴ�:%d %s\n", step->WaitMs/1000, step->Magic);
			Wait();
			break;
		default:
			break;
		}

		do {
			do {
				if (m_bStop || m_bReStart)
					goto end;
				if (m_bPause)
					Sleep(500);
			} while (m_bPause);
				
			Sleep(250);
			__int64 now_ms = getmillisecond();
			if (IsNeedAddLife()) {
				AddLife();
			}
			do {
				// ����һЩҩƷ
				m_pGame->m_pItem->DropSelfItemByType(��Ч����ҩˮ, 6);
				m_pGame->m_pItem->DropSelfItemByType(��Ч���ư�,   8);
				if (m_pStep->OpCode != OP_MOVE) {
					// ��������û��Ѫ������
					m_pGame->m_pPet->Revive();
				}
				if (m_pStep->OpCode != OP_SELECT && m_dwKairi == 1 
					&& ((now_ms - m_i64SearchKairi) >= 1000)) { // �������
					DWORD krX = 12, krY = 12;
					if (m_pGame->m_pGuaiWu->IsInArea("ΰ��Ŀ���", krX, krY)) {
						if (m_pGame->m_pMagic->CheckCd("Ӱ����Լ")) {
							m_pGame->m_pMagic->UseMagic("Ӱ����Լ", krX, krY);
						}
						else if (m_pGame->m_pMagic->CheckCd("����")) {
							m_pGame->m_pMagic->UseMagic("����", krX, krY);
						}

						m_dwKairi = 2;
						m_i64SearchKairi = now_ms;
						Wait(10);
					}
				}
				
			} while (false);

			if (StepIsComplete()) { // ����ɴ˲���
				if (m_pStep->OpCode != OP_NPC && m_pStep->OpCode != OP_SELECT && m_pStep->OpCode != OP_WAIT) {
					if (m_pGame->m_pTalk->NPCTalkStatus()) // �Ի����Ǵ򿪵�
						m_pGame->m_pTalk->NPCTalk(0xff);   // �ص���
				}
				if (m_pGame->m_pTalk->TipBoxStatus())  // ��ʾ���Ƿ��
					m_pGame->m_pTalk->CloseTipBox();   // �ص���

#if 1
				if (m_pStep->X == 425 && m_pStep->Y == 395) { // ��ʼɨ��ΰ��Ŀ���
					//m_dwKairi = 1;
				}
#endif
				
				m_pGameStep->CompleteExec();
				Sleep(200);
				break;
			}
		} while (true);
	}
end:
	int second = time(nullptr) - start_time;
	printf("��ɣ�����ʱ:%02d��%02d��\n", second/60, second%60);
	if (m_bReStart) {
		m_pGameStep->m_Step.Release(1);
		printf("GameProc::Run���¼���...\n");
		Run();
	}
	else {
		m_pGameStep->ResetStep();
		m_bPause = true;
		goto start;
	}
}

// �����Ƿ���ִ�����
bool GameProc::StepIsComplete()
{
	if (m_bIsCrazy) {
		m_pGame->m_pGuaiWu->Clear(m_CrazyMagic, 8, 8); // ���5*5��Χ�ڵĹ���
	}

	bool result = false;
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
		if (m_pGame->m_pMove->IsMoveEnd()) { // �ѵ�ָ��λ��
			result = true;
			goto end;
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
		m_pGame->m_pTalk->NPC(m_stLastStepInfo.NPCId);
	}
	else {
		if (strcmp(m_pStep->NPCName, "ħ���ϱ�") == 0) {
			m_dwKairi = 0;
		}
		m_stLastStepInfo.NPCId = m_pGame->m_pTalk->NPC(m_pStep->NPCName);
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
		printf("��(%d)ѡ��\n", i);
		m_pGame->m_pTalk->NPCTalk(m_pStep->SelectNo);

		if (i > 1) {
			if (IsNeedAddLife()) {
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
		GameMagic* pMagic = m_pGame->m_pMagic->ReadMagic(m_pStep->Magic);
		if (!pMagic)
			return;
		while (!m_pGame->m_pMagic->CheckCd(pMagic->Id)) {
			Sleep(100);
		}
	}

	DWORD x = m_pStep->X, y = m_pStep->Y;
	if (!x || !y) { // �ͷŵ�λ������
		m_pGame->ReadCoor(&x, &y);
	}
	m_pGame->m_pMagic->UseMagic(m_pStep->Magic, x, y);
	strcpy(m_stLastStepInfo.Magic, m_pStep->Magic);
	Sleep(100);
}

// ����-����
void GameProc::MagicPet()
{
	DWORD x = m_pStep->X, y = m_pStep->Y;
	if (!x || !y) { // �ͷŵ�λ������
		m_pGame->ReadCoor(&x, &y);
	}
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
	for (int i = 0; i < 3; i++) {
		printf("��%d�μ���\n", i);
		m_pGame->m_pItem->PickUpItem(m_pGame->m_pGameConf->m_stPickUp.PickUps, 
			m_pGame->m_pGameConf->m_stPickUp.Length);
		Sleep(1000);
	}
}

// ����
void GameProc::CheckIn()
{
	HWND child, parent;

	m_pGame->FindButtonWnd(BUTTON_ID_ROLE, child, parent);
	SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_ROLE, BN_CLICKED), (LPARAM)child);    // ������ﰴť
	Sleep(1000);
	m_pGame->FindButtonWnd(BUTTON_ID_VIP, child, parent);
	SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_VIP, BN_CLICKED), (LPARAM)child);     // ���VIP��ť
	Sleep(1000);
	m_pGame->FindButtonWnd(BUTTON_ID_CHECKIN, child, parent);
    SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_CHECKIN, BN_CLICKED), (LPARAM)child); // �����Ʒ�ֿⰴť
	if (!parent) {
		printf("�򿪲ֿ�ʧ��\n");
		return;
	}

	Sleep(1500);
	m_pGame->m_pItem->CheckInSelfItem(m_pGame->m_pGameConf->m_stCheckIn.CheckIns,
		m_pGame->m_pGameConf->m_stCheckIn.Length);

	m_pGame->FindButtonWnd(BUTTON_ID_CLOSECKIN, child, parent);
	SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_CLOSECKIN, BN_CLICKED), (LPARAM)child); // ����رղֿⰴť
}

// ʹ����Ʒ
void GameProc::UseItem()
{
	m_pGame->m_pItem->UseSelfItem(m_pStep->Name, m_pStep->X, m_pStep->Y);
}

// ������Ʒ
void GameProc::SellItem()
{
	if (!m_stLastStepInfo.NPCId) {
		printf("û���ҵ�NPC �޷�������Ʒ\n");
		return;
	}

	m_pGame->m_pItem->SellSelfItem(m_pGame->m_pGameConf->m_stSell.Sells,
		m_pGame->m_pGameConf->m_stSell.Length);

	HWND child, parent;
	m_pGame->FindButtonWnd(BUTTON_ID_CLOSESHOP, child, parent);
	SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_CLOSESHOP, BN_CLICKED), (LPARAM)child); // ����ر��̵갴ť
}

// �ȴ�
void GameProc::Wait()
{
	if (strlen(m_pStep->Magic)) { // �ȴ�������ȴ
		GameMagic* pMagic = m_pGame->m_pMagic->ReadMagic(m_pStep->Magic);
		if (!pMagic)
			return;

		while (!m_pGame->m_pMagic->CheckCd(pMagic->Id, m_pStep->WaitMs)) {
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
			printf("�ȴ�%d�룬��ʣ%d��\n", ms / 1000, ls);
		}
		if (c <= 0) {
			break;
		}
		n = ls;
		Sleep(100);
	}
}

// �Ƿ��ڸ���
bool GameProc::IsInFB()
{
	ReadCoor();
	return (m_iCoorX >= 200 && m_iCoorX <= 930) && (m_iCoorY >= 30 && m_iCoorY <= 1120);
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
bool GameProc::IsNeedAddLife()
{
	if (getmillisecond() < (m_i64AddLifeTime + 500)) // 1���ڲ��ظ���
		return false;
	if (!ReadLife())
		return false;
	if (m_iLife == 0)
		return false;

	return (m_iLife + 2500) <= m_iLifeMax;
}

// ��Ѫ
void GameProc::AddLife()
{
	bool add = false;
	DWORD count = m_pGame->m_pItem->GetSelfItemCountByType(��Ч����ҩˮ);
	if (count > 0) { // ��ҩˮ
		m_pGame->m_pItem->UseSelfItemByType(��Ч����ҩˮ);
		add = true;
		count--;
	}
	if (count == 0) { // û��ҩˮ��
		if (m_pGame->m_pItem->GetSelfItemCountByType(��Ч���ư�) > 0) {
			if (add) {
				Sleep(200);
			}	
			m_pGame->m_pItem->UseSelfItemByType(��Ч���ư�); // ʹ��ҩ������ҩˮ
			if (!add) {
				Sleep(200);
				m_pGame->m_pItem->UseSelfItemByType(��Ч����ҩˮ);
				add = true;
			}
		}
	}
	if (add == true) {
		printf("��Ѫ.\n");
		m_i64AddLifeTime = getmillisecond();
	}	
}

// ֹͣ
void GameProc::Stop(bool v)
{
	INLOGVARN(32, "Set Stop:%d", v);
	m_bStop = v;
}


