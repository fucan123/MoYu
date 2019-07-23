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

	// �������
	bool Clear(MagicType magic, DWORD cx, DWORD cy);
	// ��ʼ���������Ĺ���
	void InitAttack();
	// �������ڹ�������
	void SetAttack(GameGuaiWu* p);
	// �Ƿ�������ڹ�������
	bool IsIgnoreAttack();
	// ��ȡ���ﵱǰѪ��
	DWORD GetLife(GameGuaiWu* p);
	// ��ȡ����
	bool ReadGuaiWu();
public:
	// ��Ϸ��
	Game* m_pGame;

	// �Ƿ����������
	bool m_bIsClear;
	// ������X�뾶
	DWORD m_dwCX;
	// ������Y�뾶
	DWORD m_dwCY;
	// ���ڹ����Ĺ���ָ��
	GameGuaiWu* m_pAttack;
	// ����ʱ��
	__int64     m_i64AttackTime;
};