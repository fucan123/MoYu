#include "GuaiWu.h"
#include "Game.h"
#include <stdio.h>
#include <My/Common/mystring.h>

// ...
GuaiWu::GuaiWu(Game * p)
{
	m_pGame = p;
	InitAttack();
}

// 初始化被攻击的怪物
void GuaiWu::InitAttack()
{
	m_pAttack = nullptr;
	m_i64AttackTime = 0;
}

// 设置被攻击怪物
void GuaiWu::SetAttack(GameGuaiWu * p)
{
	m_pAttack = p;
	m_i64AttackTime = getmillisecond();
}

// 是否忽略正在攻击怪物
bool GuaiWu::IsIgnoreAttack()
{
	try {
		if (!m_pAttack->Id || !m_pAttack->X || !m_pAttack->Y) { // 信息无效
			throw "...";
		}
		if (!GetLife(m_pAttack)) { // 无血量
			throw "...";
		}
	}
	catch (void*) {
		InitAttack();
		return true;
	}
}

// 读取怪物
bool GuaiWu::ReadGuaiWu()
{
	// 037F3C05
	// 037D81B0
	// 4:* 4:0x00 4:0xFFFFFFFF 4:0x01 4:0x00 4:0x00 4:* 4:0x00
	DWORD codes[] = {
		0x123469F0, 0x00000000, 0xFFFFFFFF, 0x00000001,
		0x00000000, 0x00000000, 0x1234F320, 0x00000000
	};

	//偏移15CC处+0C是血量地址

	DWORD address[16];
	DWORD count = m_pGame->SearchCode(codes, sizeof(codes) / sizeof(DWORD), address, 16);

	if (count) {
		m_pGame->ReadCoor();
		//printf("\n---------------------------\n");
		//printf("[%d]怪物数量：%d(%d)\n", (int)time(nullptr), count, m_dwGuaiWuCount + count);
		DWORD num = 0, near_index = 0xff, near_dist = 0;
		for (DWORD i = 0; i < count; i++) {
			//if (address[i] != ((DWORD)m_pGuaiWus[i + m_dwGuaiWuCount])) {
				//printf("不相等%d!=%d\n", i, i + m_dwGuaiWuCount);
			//}
			GameGuaiWu* pGuaiWu = (GameGuaiWu*)address[i];
			//printf("怪物地址:%08X\n", pGuaiWu);
			//continue;
			if (pGuaiWu->X > 0 && pGuaiWu->Y > 0 && pGuaiWu->Type && pGuaiWu->Type != 0x6E) {
				char* name = (char*)((DWORD)address[i] + 0x520);
				DWORD life = GetLife(pGuaiWu);
				
				printf("%02d[%08X].%s[%08X]: x:%X[%d] y:%X[%d] 类型:%X 血量:%d\n", i + 1, pGuaiWu, name, pGuaiWu->Id, pGuaiWu->X, pGuaiWu->X, pGuaiWu->Y, pGuaiWu->Y, pGuaiWu->Type, life);

#if 0
				if (m_pGame->m_dwX && y) {
					int cx = m_dwX - pGuaiWu->X, cy = y - pGuaiWu->Y;
					DWORD cxy = abs(cx) + abs(cy);
					if (near_index == 0xff || cxy < near_dist) {
						near_index = i;
						near_dist = cxy;
					}
				}
#endif
				num++;
			}
		}

	}

#if 0
	m_dwGuaiWuCount += count;
	if (m_dwGuaiWuCount > GUAIWU_MAX) {
		printf("怪物数量达到上限:%d\n", m_dwGuaiWuCount);
		m_dwGuaiWuCount = GUAIWU_MAX;
	}
#endif

	return count > 0;
}

// 获取怪物当前血量
DWORD GuaiWu::GetLife(GameGuaiWu* p)
{
	__asm {
		mov eax, dword ptr[p]
		add eax, 0x15CC // 偏移0x15CC处为血量指针-0x0C
		mov eax, [eax]
		add eax, 0x0C   // 保存血量指针的指针
		mov eax, [eax]  // 血量指针
		mov eax, [eax]  // 血量
	}
}
