#pragma once
#include <Windows.h>

#define ToDwordPtr(v) ((DWORD*)(v))  // ת��DWORDָ��
#define PtrToDword(v) (*(DWORD*)(v)) // ת��DWORD��ֵ
#define PtrVToDwordPtr(v) ToDwordPtr(PtrToDword(v)) // ��ȡ�˵�ַ��ֵ�ٰ�ֵת��DWORDָ��

// Game::Call_Talk(v, 2)
#define INLOG(v) (v)
#define INLOGVARP(p,...) { sprintf_s(p,__VA_ARGS__);INLOG(p); }
#define INLOGVARN(n,...) {char _s[n]; INLOGVARP(_s,__VA_ARGS__); }

#define WND_TITLE           L"��ħ��"
#define MOD_3drole          L"3drole.dll"
#define MOD_3dgamemap       L"3dgamemap.dll"
#define MOD_sound           L"sound.dll"
#define ADDR_ACCOUNT_NAME   0x11BD5E4     // ��¼�ʺ�����
#define ADDR_ROLE_NAME      0x11B8EDC     // ��Ϸ��ɫ����
#define ADDR_COOR_X_OFFSET  0x11EC164     // X�����ַ��ģ�������ƫ��[MOD_3drole]
#define ADDR_COOR_Y_OFFSET  0x11EC168     // Y�����ַ��ģ�������ƫ��[MOD_3drole]
#define ADDR_MOV_STA_OFFSET 0x4A7A60      // �����ƶ�״̬��ģ������ƫ��[MOD_sound]
#define ADDR_TALKBOX_PTR    0x10A97C8     // �Ի����״̬��ַָ��[Soul.exe+CA97C8]
// mov eax,[edi+00005394] << EDI=05B7D020
#define ADDR_TALKBOX_REAL   (PtrToDword(ADDR_TALKBOX_PTR)+0x5394)
#define CHD_TALBOX_STATUS   0x00         // 0-�Ի���û�д� 1-��
#define CHD_TALBOX_STRING   0x1144       // �Ի�����������ڶԻ���ƫ��

//02C5965B:?MagicAttack@CHero@@QAEXIIHH@Z
//02C55656: ? MagicAttack@CHero@@QAEXIUC3_POS@@HH@Z
#define BASE_DS_OFFSET      0xF1A518               // ��Ϸ����ƫ�� mov ecx, dword ptr ds : [0xF07500]
// F1C730 2218-21F4=24 6B8
#define CALLTALK_DS_COMMON  (BASE_DS_OFFSET-0x04)   // ����-����Ƶ��   CHero::Talk.6
#define CALLTALK_DS_TEAM    (BASE_DS_OFFSET-0x08)   // ����-����
#define CALLNPC_DS          (BASE_DS_OFFSET+0x2218) // NPC�Ի�        CHero::ActiveNpc.2
#define CALLUSEITEM_DS      (BASE_DS_OFFSET+0x0D48) // ʹ����Ʒ        CHero::UseItem.5
#define CALLDROPITEM_DS     (BASE_DS_OFFSET+0x159C) // ������Ʒ        CHero::DropItem.3
#define CALLPICKUPITEM_DS   (BASE_DS_OFFSET+0x23C4) // ��ʰ��Ʒ        CHero::PickUpItem.3
#define CALLMAGIC_DS        (BASE_DS_OFFSET+0x2288) // ʹ�ü���-����ID  CHero::MagicAttack.4
#define CALLMAGIC_XY_DS     (BASE_DS_OFFSET+0x1778) // ʹ�ü���-XY����  CHero::MagicAttack.5
#define CALLPETOUT_DS       (BASE_DS_OFFSET+0x06B8) // �������        CHero::CallEudenmon
#define CALLPETIN_DS        (BASE_DS_OFFSET+0x06B0) // �����ٻ�        CHero::KillEudenmon 
#define CALLPETFUCK_DS      (BASE_DS_OFFSET+0x1564) // �������        CHero::AttachEudemon
#define CALLPETUNFUCK_DS    (BASE_DS_OFFSET+0x06BC) // �������        CHero::UnAttachEudemon

#define GUAIWU_MAX          100          // ����ȡ��������

// CALLƫ��
enum CALL_DATA_OFFSET {
	RUN_3drole = 0x621379,          // �����ƶ����� CHero::run(x, y, 0)
	NPCTALK_EAX_3drole = 0x1327858, // NPC�����Ի�EAX��ֵ
	NPCTALK_EDI_3drole = 0xEF1E68,  // NPC�����Ի�EDI��ֵ
};
// 0001933E 000193EC
// ��Ϸģ���ַ
typedef struct game_mod_addr
{
	DWORD Mod3DRole;     // 3drole.dll
	DWORD Mod3DGameMap;  // 3dgamemap.dll
} GameModAddr;

// ��ϷCALL��ַ
typedef struct game_call
{
	DWORD Run;              // ��Ϸ�ƶ���ַ    CHero::Run(x, y, 0);
	DWORD ActiveNpc;        // NPC�Ի�        CHero::ActiveNpc.2
	DWORD UseItem;          // ʹ����Ʒ        CHero::UseItem.5
	DWORD DropItem;         // ������Ʒ        CHero::DropItem.3
	DWORD PickUpItem;       // ��ʰ��Ʒ        CHero::PickUpItem.3
	DWORD MagicAttack_GWID; // ʹ�ü���-����ID  CHero::MagicAttack.4
	DWORD MagicAttack_XY;   // ʹ�ü���-XY����  CHero::MagicAttack.5
	DWORD CallEudenmon;     // �������        CHero::CallEudenmon
	DWORD KillEudenmon;     // �����ٻ�        CHero::KillEudenmon 
	DWORD AttachEudemon;    // �������        CHero::AttachEudemon
	DWORD UnAttachEudemon;  // �������        CHero::UnAttachEudemon
	DWORD SetRealLife;      // ��������Ѫ��     CPlayer::SetRealLife 
	DWORD CloseTipBox;      // �ر���ʾ��      ����������
} GameCall;

// 029B0208 - 23c0000 = 5F0208
// ��Ϸ��һЩֵ�ĵ�ַ
typedef struct game_addr
{
	DWORD Bag;            // ����
	DWORD Life;           // Ѫ��
	DWORD LifeMax;        // ���Ѫ��
	DWORD CoorX;          // X����
	DWORD CoorY;          // Y����
	DWORD MovSta;         // �ƶ�״̬
	DWORD TalKBoxSta;     // �Ի���״̬
	DWORD TipBoxSta;      // ��ʾ��״̬
	DWORD QuickKeyNum;    // ��ݼ�������Ʒ����(F1��һ��)
	DWORD QuickKey2Num;   // ��ݼ�������Ʒ����(1 ��һ��)
	DWORD QuickKeyType;   // ��ݼ�������Ʒ����(F1��һ��)
	DWORD QuickKey2Type;  // ��ݼ�������Ʒ����(1 ��һ��)
	DWORD ItemPtr;        // �����Ϊ������Ʒָ���׵�ַ *Itemptr=������Ʒ��ַ�׵�ַ *(Itemptr+4)=������Ʒ��ַĩ��ַ
	DWORD CallNpcTalkEsi; // NPC�����Ի�ESI�Ĵ�����ֵ
	DWORD PetPtr;         // �����б����ַ ���������
} GameAddr;

// ��Ϸ������Ϣ
typedef struct game_wnd
{
	RECT Rect;   // ������Ϣ
	int  Width;  // ���ڿ��
	int  Height; // ���ڸ߶�
} GameWnd;

// ��Ϸ��Ʒ����
enum ITEM_TYPE
{
	��Ч���ư�        =  0x000B5593, // ���Կ�����ƿ��Ч����ҩˮ
	�������ҩˮ      = 0x000B783C,
	��Ч����ҩˮ      = 0x000F6982, // +2500����ֵ
	��Чʥ����ҩ      = 0x000F943E, // �������
	��ʮ��������Ƭ��1  = 0x000F90E4,
	��ʮ��������Ƭ��2  = 0x000F90E5,
	ħ�꾧ʯ          = 0x000FD35E,
	��꾧ʯ          = 0x000FD368,
	��ħ��ʯ          = 0x000FD372,
};

// ��Ϸ�Լ�ӵ����Ʒ��Ϣ
typedef struct game_self_item
{
	DWORD Fix;  // Ӧ���ǹ̶�ֵ
	DWORD Id;   // ID
	DWORD Fix2; // Ӧ���ǹ̶�ֵ
	DWORD Type; // ����
} GameSelfItem;

// ��Ϸ��Ʒ��Ϣ
typedef struct game_ground_item
{
	DWORD Id;     // ID
	DWORD Type;   // ����
	DWORD Zero;   // δ֪
	DWORD X;      // X���� 
	DWORD Y;      // Y����
} GameGroundItem;

// ����
typedef struct game_guaiwu
{
	BYTE  UnKnow[0xB4];  // δ֪
	DWORD X;             // X����
	DWORD Y;             // Y����
	BYTE  UnKnow2[0x40]; // δ֪
	DWORD Id;            // ID
	DWORD Type;          // ����
} GameGuaiWu;

// ����ִ��CALL����
enum CallStepType
{
	CST_NOTHING = 0x00, // û����ʲô
	CST_RUN     = 0x01, // ������·
	CST_NPC     = 0x02, // ���ڴ�NPC
	CST_NPCTALK = 0x03, // ����ѡ��NPC�Ի���
	CST_DROP    = 0x04, // ��������Ʒ
	CST_PICKUP  = 0x05, // ���ڷż���
	CST_MAGIC   = 0x06, // ���ڷż���
};

// ����ִ��CALL���� 
typedef struct call_step
{
	union {
		struct {
			DWORD v1;
			DWORD v2;
		};
		struct {
			DWORD MvX; // �ƶ�����X����
			DWORD MvY; // �ƶ�����Y����
		};
		struct {
			DWORD ItemId;  // ����Ʒ��ID
		};
	};
	CallStepType Type; // ����ִ������
	time_t CallTime;   // ִ��ʱ��
} CallStep;

// ����������
typedef struct sea_code
{
	DWORD Code; // ������
	DWORD Flag; // ��ʲô��ʽ�Ƚ�
} SeaCode;

enum SearchCodeType
{
	SeaCd_Jump = 0x00000000, // ��������֤
	SeaCd_NDEQ = 0x00010000, // ��Ҫ���
	SeaCd_NoZo = 0x00020000, // ������0
	SeaCd_EQIX = 0x00040000, // ��Ҫ����ָ������[��2�ֽ�ָ������ ��0x0004000B��ʾ��ֵ��Ҫ������Bֵ���]
	SeaCd_EQLB = 0x00080000, // �ͼ����ֽ���Ҫ���[��2�ֽ�ָ���ֽ��� ��0x00080002��ʾ��2���ֽ���Ҫ���]
	SeaCd_EQHB = 0x00080000, // �߼����ֽ���Ҫ���[��2�ֽ�ָ���ֽ��� ��0x00080002��ʾ��2���ֽ���Ҫ���]
};

// ����
enum MagicType
{
	δ֪����   = 0x00,
	������   = 0x7D6,  // Call_Magic(., pet_id) ���������
	��ʥ����   = 0x7D7,  // Call_Magic(., pet_id) �������г���
	����      = 0x839,  // Call_Magic(., x, y)
	Ӱ����Լ  = 0x91F,  // Call_Magic(., guaiwu_id) 0x0F8EDE��֪���ǲ��ǵ��� �˼���guaiwu_id�̶��������
	����þ�  = 0x983,  // Call_Magic(., x, y)
	���޿ռ�  = 0x9C9,  // Call_Magic(., x, y)
	����     = 0xBB8,  // Call_Magic(., x, y)
	�����   = 0xBBE,  // Call_Magic(., guaiwu_id)
	�������� = 0x5272, // Call_Magic(., x, y) ���˼Ӹ���
};