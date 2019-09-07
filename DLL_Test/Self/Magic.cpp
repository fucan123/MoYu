#include "Magic.h"
#include "Game.h"
#include <My/Common/mystring.h>
#include <My/Driver/KbdMou.h>
#include <stdio.h>

// ...
Magic::Magic(Game* p)
{
	m_pGame = p;
	m_dwAttackGWId = 0;

	m_dwMagicNum = 0;
	m_dwMagicInfoCount = 0;
}

// ʹ�ü���
bool Magic::UseMagic(const char* name, DWORD x, DWORD y, DWORD gwid)
{
	MagicInfo* pInfo = GetMagicInfo(name);
	if (!pInfo) {
		_Magic_ magic;
		if (!ReadMagic(name, &magic)) {
			printf("�Ҳ�������:%s\n", name);
			return false;
		}

		printf("��Ӽ���:%s ��ȴʱ��:%d��\n", name, magic.Cd);
		pInfo = AddMagicInfo(magic.Name, magic.Id, magic.Cd);
	}
	
	if (!CheckCd(pInfo)) {
		printf("����δ��ȴ:%s\n", name);
		//return false;
	}
		
	pInfo->UseTime = getmillisecond();          // ʹ��ʱ��
	pInfo->CdTime = pInfo->UseTime + pInfo->Cd; // ��ȴʱ��
	//printf("UseMagic:%d %d,%d %d\n", type, v, v2, p->CallType);
	//return false;
	if (MagicIsPos(name)) {
		if (strstr(name, "��������")) { // �˼������ͣ�Ĵ���
			SetForegroundWindow(m_pGame->m_hWnd);
			int sx = m_pGame->m_GameWnd.Width / 2, sy = m_pGame->m_GameWnd.Height / 2;
			m_pGame->GetGameCenterPos(sx, sy);
			sy -= 20;
			printf("����ƶ�:%d,%d\n", sx, sy);
			//m_pGame->MouseMove(sx, sy);
			mouse_event(MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE, sx, sy, 0, 0);//���
			
		}
		m_pGame->Call_Magic(pInfo->Id, x, y);
		return true;
	}
	else {
		m_pGame->Call_Magic(pInfo->Id, gwid);
		return true;
	}
	return false;
}

// �����ȴʱ��[allow_ms=�������ʣ����ȴ]
bool Magic::CheckCd(const char* name, DWORD allow_ms, DWORD* magic_id)
{
	MagicInfo* pInfo = GetMagicInfo(name);
	return CheckCd(pInfo, allow_ms);
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

// �����ȴʱ��[allow_ms=�������ʣ����ȴ]
bool Magic::CheckCd(MagicInfo* p, DWORD allow_ms)
{
	if (p == nullptr)
		return true;
	if (!p->CdTime)
		return true;

	__int64 t = p->CdTime - getmillisecond();
	return t <= (__int64)allow_ms;
}

// ��ȡ������Ϣ
MagicInfo* Magic::GetMagicInfo(const char* name)
{
	for (DWORD i = 0; i < m_dwMagicInfoCount; i++) {
		if (strstr(m_ListMagicInfo[i].Name, name) != nullptr)
			return &m_ListMagicInfo[i];
	}
	return nullptr;
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
int Magic::ReadMagicLength(bool flag)
{
	if (m_dwMagicNum)
		return m_dwMagicNum;
	// CHero::GetMagic����Ѱ��
#if 0
	__asm
	{
		mov ecx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [ecx]
		add ecx, 0x28FC
		mov eax, dword ptr ds : [ecx + 0x8]
		sub eax, dword ptr ds : [ecx + 0x4]
		sar eax, 0x2
	}
#else
	DWORD p = 0, n = 0, n2 = 0;
	if (m_pGame->ReadDwordMemory(g_pObjHero, p)) {
		p += 0x2914;
		if (m_pGame->ReadDwordMemory(p + 0x08, n)
			&& m_pGame->ReadDwordMemory(p + 0x04, n2)) {
			n = (n - n2) >> 2;
		}
	}
	return n;
#endif
}

// ��ȡ����
bool Magic::ReadMagic(const char* name, _Magic_* save, bool flag)
{
	// 4F60 51E6
	// CHero::GetMagic����Ѱ��
	int length = ReadMagicLength(flag);
	if (name == nullptr) {
		printf("��������:%d[%08X]\n", length, length);
	}
	if (length > 100) {
		printf("������������%d, �����д���\n");
		return false;
	}
	for (int i = 0; i < length; i++) {
#if 0
		GameMagic* pMagic;
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
#else
		DWORD p = 0, n = 0;
		if (m_pGame->ReadDwordMemory(g_pObjHero, p)) {
			p += 0x2914;
			if (m_pGame->ReadDwordMemory(p + 0x04, p)) {
				if (m_pGame->ReadDwordMemory(p + (i * 4), p)) {
					m_pGame->ReadDwordMemory(p + 0x28, p);
				}
			}
			
		}
		if (!p)
			break;
#endif
		if (save) {
			bool is_ok = false;
			char* p_name;
			if (m_pGame->ReadDwordMemory(p + 0x0C, (DWORD&)p_name)) {
				if (m_pGame->ReadMemory(p_name, save->Name, sizeof(save->Name))) {
					save->Name[31] = 0;
					if (strstr(save->Name, name)) {
						is_ok = true;
					}
				}
			}
			if (is_ok) {
				if (!m_pGame->ReadDwordMemory(p + 0x04, save->Id))
					return false;
				if (!m_pGame->ReadDwordMemory(p + 0x5C, save->Cd))
					return false;

				return true;
			}
		}
		else {
			GameMagic* pMagic  = (GameMagic*)p;
			printf("%08X.%s:%08X ��ȴʱ��:%d\n", pMagic, pMagic->Name, pMagic->Id, pMagic->Cd);
			AddMagicInfo(pMagic->Name, pMagic->Id, pMagic->Cd);
		}
	}
	return false;
}

// ��Ӽ�����Ϣ
MagicInfo* Magic::AddMagicInfo(CHAR* name, DWORD id, DWORD cd)
{
	MagicInfo* p = &m_ListMagicInfo[m_dwMagicInfoCount];
	strcpy(p->Name, name);
	p->Id = id;
	p->Cd = (cd + 1) * 1000;
	p->UseTime = 0;
	p->CdTime = 0;
	m_dwMagicInfoCount++;

	return p;
}
