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

	// 清理怪物
	bool Clear(MagicType magic, DWORD cx, DWORD cy);
	// 初始化被攻击的怪物
	void InitAttack();
	// 设置正在攻击怪物
	void SetAttack(GameGuaiWu* p);
	// 是否忽略正在攻击怪物
	bool IsIgnoreAttack();
	// 获取怪物当前血量
	DWORD GetLife(GameGuaiWu* p);
	// 读取怪物
	bool ReadGuaiWu();
public:
	// 游戏类
	Game* m_pGame;

	// 是否开启清理怪物
	bool m_bIsClear;
	// 攻击的X半径
	DWORD m_dwCX;
	// 攻击的Y半径
	DWORD m_dwCY;
	// 正在攻击的怪物指针
	GameGuaiWu* m_pAttack;
	// 攻击时间
	__int64     m_i64AttackTime;
};