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
#define ADDR_COOR_X_OFFSET  0x11E7F24     // X坐标地址在模块里面的偏移[MOD_3drole]
#define ADDR_COOR_Y_OFFSET  0x11E7F28     // Y坐标地址在模块里面的偏移[MOD_3drole]
#define ADDR_MOV_STA_OFFSET 0x4A7A60      // 人物移动状态在模块里面偏移[MOD_sound]
#define ADDR_TALKBOX_PTR    0x10A97C8     // 对话框打开状态地址指针[Soul.exe+CA97C8]
// mov eax,[edi+00005394] << EDI=05B7D020
#define ADDR_TALKBOX_REAL   (PtrToDword(ADDR_TALKBOX_PTR)+0x5394)
#define CHD_TALBOX_STATUS   0x00         // 0-对话框没有打开 1-打开
#define CHD_TALBOX_STRING   0x1144       // 对话框内容相对于对话框偏移

#define BASE_DS_OFFSET      0x0F07500     // 游戏常用偏移 mov ecx, dword ptr ds : [0xF07500]

#define GUAIWU_MAX          100          // 最大读取怪物数量

// CALL偏移
enum CALL_DATA_OFFSET {
	RUN_3drole = 0x621379,          // 人物移动函数 CHero::run(x, y, 0)
	NPCTALK_EAX_3drole = 0x13235B8, // NPC二级对话EAX数值
	NPCTALK_EDI_3drole = 0xEEDDD8,  // NPC二级对话EDI数值
};
// 0001933E 000193EC
// 游戏模块地址
typedef struct game_mod_addr
{
	DWORD Mod3DRole;     // 3drole.dll
	DWORD Mod3DGameMap;  // 3dgamemap.dll
} GameModAddr;

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
	DWORD QuickKeyNum;    // 快捷键上面物品数量(F1那一排)
	DWORD QuickKey2Num;   // 快捷键上面物品数量(1 那一排)
	DWORD QuickKeyType;   // 快捷键上面物品类型(F1那一排)
	DWORD QuickKey2Type;  // 快捷键上面物品类型(1 那一排)
	DWORD ItemPtr;        // 保存的为地面物品指针首地址 *Itemptr=地面物品地址首地址 *(Itemptr+4)=地面物品地址末地址
	DWORD CallNpcTalkEsi; // NPC二级对话ESI寄存器数值
} GameAddr;

// 游戏CALL地址
typedef struct game_call
{
	DWORD Run; // 游戏移动地址 CHero::run(x, y, 0);
} GameCall;

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
	DWORD Flag[8];     // 特征码搜索
	DWORD UnKnow[37];  // 未知
	DWORD X;           // X坐标
	DWORD Y;           // Y坐标
	DWORD UnKnow2[16]; // 未知
	DWORD Id;          // ID
	DWORD Type;        // 类型
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
	未知技能  = 0x00,
	星陨     = 0x839,  // Call_Magic(., x, y)
	影魂契约 = 0x91F, // Call_Magic(., guaiwu_id) 0x0F8EDE不知道是不是地面 此技能guaiwu_id固定这个参数
	诸神裁决 = 0x983, // Call_Magic(., x, y)
	虚无空间 = 0x9C9, // Call_Magic(., x, y)
	凤珠    = 0xBB8, // Call_Magic(., x, y)
	电击术  = 0xBBE, // Call_Magic(., guaiwu_id)
};