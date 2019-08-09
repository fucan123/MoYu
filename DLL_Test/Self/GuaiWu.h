#pragma once
#include "GameStruct.h"
#include "Magic.h"
#include <Windows.h>

class Game;

class GuaiWu
{
public:
	// ...
	GuaiWu(Game* p);

	// ��ʼ������
	void InitData();
	// ������Χ�Ƿ��й���
	bool HasInArea(DWORD cx, DWORD cy);
	// �����Ƿ������������� x,y=����Χ(IN),��������(OUT)
	bool IsInArea(const char* name, IN OUT DWORD& x, IN OUT DWORD& y);
	// �������
	bool Clear(const char* magic_name, DWORD cx, DWORD cy);
	// ��ʼ���������Ĺ���
	void InitAttack();
	// �������ڹ�������
	void SetAttack(GamePlayer* p);
	// �Ƿ�������ڹ�������
	bool IsIgnoreAttack();
	// ��ȡ���ﵱǰѪ��
	DWORD GetLife(GamePlayer* p, DWORD no);
	// ��ȡ����
	bool ReadGuaiWu();
public:
	// ��Ϸ��
	Game* m_pGame;

	// �Ƿ�������������
	bool m_bSearchName;
	// �����Ĺ�������
	char m_sSearChName[16];
	// �Ƿ����������
	bool m_bIsClear;
	// ������X�뾶
	DWORD m_dwCX;
	// ������Y�뾶
	DWORD m_dwCY;
	// ���ڹ����Ĺ���ָ��
	GamePlayer* m_pAttack;
	// ����ʱ��
	__int64     m_i64AttackTime;
};