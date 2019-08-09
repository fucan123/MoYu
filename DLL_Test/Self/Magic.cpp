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
}

// ʹ�ü���
bool Magic::UseMagic(const char* name, DWORD x, DWORD y, DWORD gwid)
{
	GameMagic* pMagic = ReadMagic(name);
	if (!pMagic) {
		printf("�Ҳ�������:%s\n", name);
		return false;
	}
	
	MagicInfo* pInfo = GetMagicInfo(pMagic->Id);
	if (!pInfo) {
		printf("��Ӽ���:%s ��ȴʱ��:%d��\n", name, pMagic->Cd);
		pInfo = AddMagicInfo(pMagic->Id, pMagic->Cd);
	}
	else if (!CheckCd(pMagic->Id)) {
		printf("����δ��ȴ:%s\n", name);
		//return false;
	}
		
	pInfo->UseTime = getmillisecond();          // ʹ��ʱ��
	pInfo->CdTime = pInfo->UseTime + pInfo->Cd; // ��ȴʱ��
	//printf("UseMagic:%d %d,%d %d\n", type, v, v2, p->CallType);
	//return false;
	if (MagicIsPos(name)) {
		Game::Call_Magic(pMagic->Id, x, y);
		return true;
	}
	else {
		Game::Call_Magic(pMagic->Id, gwid);
		return true;
	}
	return false;
}

// �����ȴʱ��[allow_ms=�������ʣ����ȴ]
bool Magic::CheckCd(const char* name, DWORD allow_ms, DWORD* magic_id)
{
	GameMagic* pMagic = ReadMagic(name);
	if (!pMagic)
		return true;
	if (magic_id)
		*magic_id = pMagic->Id;

	return CheckCd(pMagic->Id);
}

// �����ȴʱ��[allow_ms=�������ʣ����ȴ]
bool Magic::CheckCd(DWORD id, DWORD allow_ms)
{
	MagicInfo* p = GetMagicInfo(id);
	if (p == nullptr)
		return true;
	if (!p->CdTime)
		return true;

	__int64 t = p->CdTime - getmillisecond();
	return t <= (__int64)allow_ms;
}

// ��ȡ������Ϣ
MagicInfo* Magic::GetMagicInfo(DWORD id)
{
	for (DWORD i = 0; i < m_dwMagicInfoCount; i++) {
		if (m_ListMagicInfo[i].Id == id)
			return &m_ListMagicInfo[i];
	}
	return nullptr;
}

// �����Ƿ�Ϊ���꼼��
bool Magic::MagicIsPos(const char* name)
{
	return strstr(name, "������") == nullptr
		&& strstr(name, "��ʥ����") == nullptr;
}

// ��ȡ��������
int Magic::ReadMagicLength()
{
	// CHero::GetMagic����Ѱ��
	__asm
	{
		mov ecx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [ecx]
		add ecx, 0x28FC
		mov eax, dword ptr ds : [ecx + 0x8]
		sub eax, dword ptr ds : [ecx + 0x4]
		sar eax, 0x2
	}
}

// ��ȡ����
GameMagic* Magic::ReadMagic(const char* name)
{
	// 4F60 51E6
	// CHero::GetMagic����Ѱ��
	int length = ReadMagicLength();
	if (name == nullptr) {
		printf("��������:%d[%08X]\n", length, length);
	}
	for (int i = 0; i < length; i++) {
		GameMagic* p;
		__asm
		{
			mov ecx, dword ptr ds : [BASE_DS_OFFSET]
			mov ecx, dword ptr ds : [ecx]
			add ecx, 0x28FC
			mov eax, dword ptr ds : [ecx + 0x4]
			mov ecx, dword ptr [i]
			lea eax, dword ptr ds : [eax + ecx * 4]
			mov eax, dword ptr ds : [eax]
			mov eax, dword ptr ds : [eax + 0x28]
			mov dword ptr [p], eax 
			//mov eax, dword ptr ds : [eax + 0x4]       // ������Ƿż��ܺ���Ҫ���Ĳ���
			//mov dword ptr [magic], eax
		}
		if (name) {
			if (strstr(p->Name, name)) {
				return p;
			}
		}
		else {
			printf("%08X.%s:%08X ��ȴʱ��:%d\n", p, p->Name, p->Id, p->Cd);
		}
	}
	return nullptr;
}

// ��Ӽ�����Ϣ
MagicInfo* Magic::AddMagicInfo(DWORD id, DWORD cd)
{
	MagicInfo* p = &m_ListMagicInfo[m_dwMagicInfoCount];
	p->Id = id;
	p->Cd = (cd + 1) * 1000;
	p->UseTime = 0;
	p->CdTime = 0;
	m_dwMagicInfoCount++;

	return p;
}
