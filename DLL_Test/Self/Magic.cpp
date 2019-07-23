#include "Magic.h"
#include "Game.h"
#include <My/Common/mystring.h>
#include <stdio.h>

// ...
Magic::Magic(Game* p)
{
	m_pGame = p;
	m_dwAttackGWId = 0;

	m_dwMagicInfoCount = 0;
	AddMagicInfo(生命祈祷, MCT_GWID, 100);
	AddMagicInfo(神圣复苏, MCT_GWID, 80000);
	AddMagicInfo(星陨,    MCT_POS,  31000);
	AddMagicInfo(影魂契约, MCT_POS,  56000);
	AddMagicInfo(诸神裁决, MCT_POS,  500);
	AddMagicInfo(虚无空间, MCT_POS,  31000);
}

// 使用技能
bool Magic::UseMagic(MagicType type, DWORD v, DWORD v2)
{
	MagicInfo* p = GetMagicInfo(type);
	if (p == nullptr)
		return false;
	if (!CheckCd(type))
		return false;

	p->UseTime = getmillisecond();  // 使用时间
	p->CdTime = p->UseTime + p->Cd; // 冷却时间
	//printf("UseMagic:%d %d,%d %d\n", type, v, v2, p->CallType);
	//return false;
	if (p->CallType == MCT_POS) {
		Game::Call_Magic(p->Type, v, v2);
		return true;
	}
	if (p->CallType == MCT_GWID) {
		Game::Call_Magic(p->Type, v);
		return true;
	}
	return false;
}

// 检查冷却时间[allow_ms=允许最大还剩此冷却]
bool Magic::CheckCd(MagicType type, DWORD allow_ms)
{
	MagicInfo* p = GetMagicInfo(type);
	if (p == nullptr)
		return true;
	if (!p->CdTime)
		return true;

	__int64 t = p->CdTime - getmillisecond();
	return t <= (__int64)allow_ms;
}

// 获取技能信息
MagicInfo* Magic::GetMagicInfo(MagicType type)
{
	for (DWORD i = 0; i < m_dwMagicInfoCount; i++) {
		if (m_ListMagicInfo[i].Type == type)
			return &m_ListMagicInfo[i];
	}
	return nullptr;
}

// 添加技能信息
void Magic::AddMagicInfo(MagicType type, MagicCallType call_type, DWORD cd)
{
	MagicInfo* p = &m_ListMagicInfo[m_dwMagicInfoCount];
	p->Type = type;
	p->CallType = call_type;
	p->Cd = cd;
	p->UseTime = 0;
	p->CdTime = 0;
	m_dwMagicInfoCount++;
}
