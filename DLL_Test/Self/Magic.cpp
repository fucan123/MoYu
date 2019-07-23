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
	AddMagicInfo(������, MCT_GWID, 100);
	AddMagicInfo(��ʥ����, MCT_GWID, 80000);
	AddMagicInfo(����,    MCT_POS,  31000);
	AddMagicInfo(Ӱ����Լ, MCT_POS,  56000);
	AddMagicInfo(����þ�, MCT_POS,  500);
	AddMagicInfo(���޿ռ�, MCT_POS,  31000);
}

// ʹ�ü���
bool Magic::UseMagic(MagicType type, DWORD v, DWORD v2)
{
	MagicInfo* p = GetMagicInfo(type);
	if (p == nullptr)
		return false;
	if (!CheckCd(type))
		return false;

	p->UseTime = getmillisecond();  // ʹ��ʱ��
	p->CdTime = p->UseTime + p->Cd; // ��ȴʱ��
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

// �����ȴʱ��[allow_ms=�������ʣ����ȴ]
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

// ��ȡ������Ϣ
MagicInfo* Magic::GetMagicInfo(MagicType type)
{
	for (DWORD i = 0; i < m_dwMagicInfoCount; i++) {
		if (m_ListMagicInfo[i].Type == type)
			return &m_ListMagicInfo[i];
	}
	return nullptr;
}

// ��Ӽ�����Ϣ
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
