#pragma once
#include "GameStruct.h"
#include <Windows.h>

class Game;

class Move
{
public:
	// ...
	Move(Game* p);
	// ����ƶ�����
	void ClearMove();
	// �����ƶ�λ��
	void SetMove(DWORD x, DWORD y);
	// �Ƿ�ﵽ�յ�
	bool IsMoveEnd();
	// �Ƿ��ƶ�
	bool IsMove();
public:
	// ��Ϸ��
	Game* m_pGame;
	// ��ǰλ��X
	DWORD m_dwX;
	// ��ǰλ��Y
	DWORD m_dwY;
	// �ϴ�λ��X
	DWORD m_dwLastX;
	// �ϴ�λ��Y
	DWORD m_dwLastY;
	// �ƶ�λ��X
	DWORD m_dwMvX;
	// �ƶ�λ��Y
	DWORD m_dwMvY;
	// �ƶ�ʱ��
	__int64 m_i64MvTime;
};