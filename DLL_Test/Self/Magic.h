#pragma once
#include "GameStruct.h"

#define MAX_MAGIC_COUNT 0x10

// 技能调用哪个CALL
enum MagicCallType{
	MCT_POS  = 0x00, // 调用传坐标CALL
	MCT_GWID = 0x01, // 调用传怪物IDCALL
};
typedef struct magic_info
{
	MagicType     Type;       // 技能
	MagicCallType CallType;   // CALL类型 0-调用
	DWORD         Cd;         // 所需冷却时间[毫秒]
	__int64       UseTime;    // 使用时间[毫秒]
	__int64       CdTime;     // 冷却时间
	
} MagicInfo;

class Game;

class Magic
{
public:
	// ...
	Magic(Game* p);
	// 使用技能
	bool UseMagic(MagicType type, DWORD v, DWORD v2);
	// 检查冷却时间[allow_ms=允许最大还剩此冷却]
	bool CheckCd(MagicType type, DWORD allow_ms=0);
	// 获取技能信息
	MagicInfo* GetMagicInfo(MagicType type);
private:
	// 添加技能信息
	void AddMagicInfo(MagicType type, MagicCallType call_type, DWORD cd);
public:
	// 游戏类
	Game* m_pGame;
	// 技能信息列表数量
	DWORD m_dwMagicInfoCount;
	// 技能信息列表
	MagicInfo m_ListMagicInfo[MAX_MAGIC_COUNT];
	// 正在攻击的怪物Id
	DWORD m_dwAttackGWId;
};