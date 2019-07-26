#pragma once
#include <Windows.h>

#define ToDwordPtr(v) ((DWORD*)(v))  // 转成DWORD指针
#define PtrToDword(v) (*(DWORD*)(v)) // 转成DWORD数值
#define PtrVToDwordPtr(v) ToDwordPtr(PtrToDword(v)) // 先取此地址的值再把值转成DWORD指针

// Game::Call_Talk(v, 2)
#define INLOG(v) (v)
#define INLOGVARP(p,...) { sprintf_s(p,__VA_ARGS__);INLOG(p); }
#define INLOGVARN(n,...) {char _s[n]; INLOGVARP(_s,__VA_ARGS__); }

#define WND_TITLE           L"【魔域】"
#define MOD_3drole          L"3drole.dll"
#define MOD_3dgamemap       L"3dgamemap.dll"
#define MOD_sound           L"sound.dll"
#define ADDR_ACCOUNT_NAME   0x11BD5E4     // 登录帐号名称
#define ADDR_ROLE_NAME      0x11B8EDC     // 游戏角色名称
#define ADDR_COOR_X_OFFSET  0x11EC164     // X坐标地址在模块里面的偏移[MOD_3drole]
#define ADDR_COOR_Y_OFFSET  0x11EC168     // Y坐标地址在模块里面的偏移[MOD_3drole]
#define ADDR_MOV_STA_OFFSET 0x4A7A60      // 人物移动状态在模块里面偏移[MOD_sound]
#define ADDR_TALKBOX_PTR    0x10A97C8     // 对话框打开状态地址指针[Soul.exe+CA97C8]
// mov eax,[edi+00005394] << EDI=05B7D020
#define ADDR_TALKBOX_REAL   (PtrToDword(ADDR_TALKBOX_PTR)+0x5394)
#define CHD_TALBOX_STATUS   0x00         // 0-对话框没有打开 1-打开
#define CHD_TALBOX_STRING   0x1144       // 对话框内容相对于对话框偏移

//02C5965B:?MagicAttack@CHero@@QAEXIIHH@Z
//02C55656: ? MagicAttack@CHero@@QAEXIUC3_POS@@HH@Z
#define BASE_DS_OFFSET      0xF1A518               // 游戏常用偏移 mov ecx, dword ptr ds : [0xF07500]
// F1C730 2218-21F4=24 6B8
#define CALLTALK_DS_COMMON  (BASE_DS_OFFSET-0x04)   // 喊话-公共频道   CHero::Talk.6
#define CALLTALK_DS_TEAM    (BASE_DS_OFFSET-0x08)   // 喊话-队伍
#define CALLNPC_DS          (BASE_DS_OFFSET+0x2218) // NPC对话        CHero::ActiveNpc.2
#define CALLUSEITEM_DS      (BASE_DS_OFFSET+0x0D48) // 使用物品        CHero::UseItem.5
#define CALLDROPITEM_DS     (BASE_DS_OFFSET+0x159C) // 丢弃物品        CHero::DropItem.3
#define CALLPICKUPITEM_DS   (BASE_DS_OFFSET+0x23C4) // 捡拾物品        CHero::PickUpItem.3
#define CALLMAGIC_DS        (BASE_DS_OFFSET+0x2288) // 使用技能-怪物ID  CHero::MagicAttack.4
#define CALLMAGIC_XY_DS     (BASE_DS_OFFSET+0x1778) // 使用技能-XY坐标  CHero::MagicAttack.5
#define CALLPETOUT_DS       (BASE_DS_OFFSET+0x06B8) // 宠物出征        CHero::CallEudenmon
#define CALLPETIN_DS        (BASE_DS_OFFSET+0x06B0) // 宠物召回        CHero::KillEudenmon 
#define CALLPETFUCK_DS      (BASE_DS_OFFSET+0x1564) // 宠物合体        CHero::AttachEudemon
#define CALLPETUNFUCK_DS    (BASE_DS_OFFSET+0x06BC) // 宠物解体        CHero::UnAttachEudemon

#define GUAIWU_MAX          100          // 最大读取怪物数量

// CALL偏移
enum CALL_DATA_OFFSET {
	RUN_3drole = 0x621379,          // 人物移动函数 CHero::run(x, y, 0)
	NPCTALK_EAX_3drole = 0x1327858, // NPC二级对话EAX数值
	NPCTALK_EDI_3drole = 0xEF1E68,  // NPC二级对话EDI数值
};
// 0001933E 000193EC
// 游戏模块地址
typedef struct game_mod_addr
{
	DWORD Mod3DRole;     // 3drole.dll
	DWORD Mod3DGameMap;  // 3dgamemap.dll
} GameModAddr;

// 游戏CALL地址
typedef struct game_call
{
	DWORD Run;              // 游戏移动地址    CHero::Run(x, y, 0);
	DWORD ActiveNpc;        // NPC对话        CHero::ActiveNpc.2
	DWORD UseItem;          // 使用物品        CHero::UseItem.5
	DWORD DropItem;         // 丢弃物品        CHero::DropItem.3
	DWORD PickUpItem;       // 捡拾物品        CHero::PickUpItem.3
	DWORD MagicAttack_GWID; // 使用技能-怪物ID  CHero::MagicAttack.4
	DWORD MagicAttack_XY;   // 使用技能-XY坐标  CHero::MagicAttack.5
	DWORD CallEudenmon;     // 宠物出征        CHero::CallEudenmon
	DWORD KillEudenmon;     // 宠物召回        CHero::KillEudenmon 
	DWORD AttachEudemon;    // 宠物合体        CHero::AttachEudemon
	DWORD UnAttachEudemon;  // 宠物解体        CHero::UnAttachEudemon
	DWORD SetRealLife;      // 设置真是血量     CPlayer::SetRealLife 
	DWORD CloseTipBox;      // 关闭提示框      搜索特征码
} GameCall;

// 029B0208 - 23c0000 = 5F0208
// 游戏里一些值的地址
typedef struct game_addr
{
	DWORD Bag;            // 背包
	DWORD Life;           // 血量
	DWORD LifeMax;        // 最大血量
	DWORD CoorX;          // X坐标
	DWORD CoorY;          // Y坐标
	DWORD MovSta;         // 移动状态
	DWORD TalKBoxSta;     // 对话框状态
	DWORD TipBoxSta;      // 提示框状态
	DWORD QuickKeyNum;    // 快捷键上面物品数量(F1那一排)
	DWORD QuickKey2Num;   // 快捷键上面物品数量(1 那一排)
	DWORD QuickKeyType;   // 快捷键上面物品类型(F1那一排)
	DWORD QuickKey2Type;  // 快捷键上面物品类型(1 那一排)
	DWORD ItemPtr;        // 保存的为地面物品指针首地址 *Itemptr=地面物品地址首地址 *(Itemptr+4)=地面物品地址末地址
	DWORD CallNpcTalkEsi; // NPC二级对话ESI寄存器数值
	DWORD PetPtr;         // 宠物列表基地址 详见宠物类
} GameAddr;

// 游戏窗口信息
typedef struct game_wnd
{
	RECT Rect;   // 矩形信息
	int  Width;  // 窗口宽度
	int  Height; // 窗口高度
} GameWnd;

// 游戏物品类型
enum ITEM_TYPE
{
	速效治疗包        =  0x000B5593, // 可以开出几瓶速效治疗药水
	神恩治疗药水      = 0x000B783C,
	速效治疗药水      = 0x000F6982, // +2500生命值
	速效圣兽灵药      = 0x000F943E, // 复活宝宝的
	三十星神兽碎片加1  = 0x000F90E4,
	三十星神兽碎片加2  = 0x000F90E5,
	魔魂晶石          = 0x000FD35E,
	灵魂晶石          = 0x000FD368,
	幻魔晶石          = 0x000FD372,
};

// 游戏自己拥有物品信息
typedef struct game_self_item
{
	DWORD Fix;  // 应该是固定值
	DWORD Id;   // ID
	DWORD Fix2; // 应该是固定值
	DWORD Type; // 类型
} GameSelfItem;

// 游戏物品信息
typedef struct game_ground_item
{
	DWORD Id;     // ID
	DWORD Type;   // 类型
	DWORD Zero;   // 未知
	DWORD X;      // X坐标 
	DWORD Y;      // Y坐标
} GameGroundItem;

// 怪物
typedef struct game_guaiwu
{
	BYTE  UnKnow[0xB4];  // 未知
	DWORD X;             // X坐标
	DWORD Y;             // Y坐标
	BYTE  UnKnow2[0x40]; // 未知
	DWORD Id;            // ID
	DWORD Type;          // 类型
} GameGuaiWu;

// 正在执行CALL类型
enum CallStepType
{
	CST_NOTHING = 0x00, // 没有做什么
	CST_RUN     = 0x01, // 正在走路
	CST_NPC     = 0x02, // 正在打开NPC
	CST_NPCTALK = 0x03, // 正在选择NPC对话框
	CST_DROP    = 0x04, // 正在扔物品
	CST_PICKUP  = 0x05, // 正在放技能
	CST_MAGIC   = 0x06, // 正在放技能
};

// 正在执行CALL数据 
typedef struct call_step
{
	union {
		struct {
			DWORD v1;
			DWORD v2;
		};
		struct {
			DWORD MvX; // 移动到的X坐标
			DWORD MvY; // 移动到的Y坐标
		};
		struct {
			DWORD ItemId;  // 捡物品的ID
		};
	};
	CallStepType Type; // 正在执行类型
	time_t CallTime;   // 执行时间
} CallStep;

// 搜索特征码
typedef struct sea_code
{
	DWORD Code; // 搜索码
	DWORD Flag; // 以什么方式比较
} SeaCode;

enum SearchCodeType
{
	SeaCd_Jump = 0x00000000, // 跳过不验证
	SeaCd_NDEQ = 0x00010000, // 需要相等
	SeaCd_NoZo = 0x00020000, // 不等于0
	SeaCd_EQIX = 0x00040000, // 需要等于指定索引[低2字节指定索引 如0x0004000B表示此值需要与索引B值相等]
	SeaCd_EQLB = 0x00080000, // 低几个字节需要相等[低2字节指定字节数 如0x00080002表示低2个字节需要相等]
	SeaCd_EQHB = 0x00080000, // 高几个字节需要相等[低2字节指定字节数 如0x00080002表示高2个字节需要相等]
};

// 技能
enum MagicType
{
	未知技能   = 0x00,
	生命祈祷   = 0x7D6,  // Call_Magic(., pet_id) 复活单个宠物
	神圣复苏   = 0x7D7,  // Call_Magic(., pet_id) 复活所有宠物
	星陨      = 0x839,  // Call_Magic(., x, y)
	影魂契约  = 0x91F,  // Call_Magic(., guaiwu_id) 0x0F8EDE不知道是不是地面 此技能guaiwu_id固定这个参数
	诸神裁决  = 0x983,  // Call_Magic(., x, y)
	虚无空间  = 0x9C9,  // Call_Magic(., x, y)
	凤珠     = 0xBB8,  // Call_Magic(., x, y)
	电击术   = 0xBBE,  // Call_Magic(., guaiwu_id)
	最终审判 = 0x5272, // Call_Magic(., x, y) 免伤加高伤
};