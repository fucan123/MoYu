#pragma once
#include "GameStruct.h"
#include <Windows.h>

class Game;

class GuaiWu
{
public:
	// ...
	GuaiWu(Game* p);

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

	// ���ڹ����Ĺ���ָ��
	GameGuaiWu* m_pAttack;
	// ����ʱ��
	__int64     m_i64AttackTime;
};