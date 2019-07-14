#pragma once
#include "GameStruct.h"
#include <Windows.h>

class Game;

class GuaiWu
{
public:
	// ...
	GuaiWu(Game* p);

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

	// 正在攻击的怪物指针
	GameGuaiWu* m_pAttack;
	// 攻击时间
	__int64     m_i64AttackTime;
};