#pragma once
#include "GameStruct.h"

typedef LRESULT(CALLBACK *Game_KbdProc)(int nCode, WPARAM wParam, LPARAM lParam);
typedef LRESULT(CALLBACK *Game_MouseProc)(int nCode, WPARAM wParam, LPARAM lParam);

// ����ģ���з���������Ҫ����Ϣ
typedef struct search_mod_func_msg
{
	const char*   Name;    // ����������
	const char*   Substr;  // �����а����򲻰������ַ���
	DWORD         Flag;    // 0-������ 1-����
	DWORD*        Save;    // �����ַ
	const char*   Remark;  // ˵��
} SearchModFuncMsg;

typedef struct _account_
{
	char SerBig[32];   // ��Ϸ����
	char SerSmall[32]; // ��ϷС��
	char Name[32];     // �ʺ�
	char Pwd[32];      // ����
	char Role[16];     // ��ɫ����
	int  RoleNo;       // ѡ���ĸ���ɫ��¼
	int  IsGetXL;      // �Ƿ��ȡ������
	int  IsBig;        // �Ƿ���
	int  IsLogin;      // �Ƿ��¼��
	int  LoginTime;    // ��¼ʱ��
	int  IsOffLine;    // ������
	int  GetXLTime;    // ��ȡ����ʱ��
	int  GetXLLogout;  // ��ȡ�������Ƿ��˳�
	bool IsReady;      // �Ƿ���׼����
	bool IsMeOpenFB;   // �Ƿ����Լ���������
} Account;

// ��Ϸg_pObjHeroȫ�ֱ���(CHero��thisָ��)
extern DWORD g_pObjHero;
// ��Ϸg_objPlayerSetȫ�ֱ���
extern DWORD g_objPlayerSet;

class GameClient;
class GameConf;
class GameProc;
class Item;
class Talk;
class Move;
class GuaiWu;
class Magic;
class Pet;

class Game
{
public:
	// ...
	Game();
	// >>>
	~Game();
	// �ر���Ϸ
	void Close(bool leave_team=true);
	// ���ӹ������
	void Connect(const char* host, USHORT port);
	// ��¼��Ϸ�ʺ�
	void Login();
	// �����ʺ�����
	void InputUserPwd(bool input_user=true);
	// ��ʼ��
	bool Init(DWORD hook_tid=0, HOOKPROC hook_key_proc=NULL);
	// �ȴ���Ϸ��ʼ�����
	void WaitGameInit(int wait_s);
	// �����Ƿ��Ǵ��
	void SetAccount(const char* ser_big, const char* ser_small, const char* name, const char* pwd, int role_no, int getxl, int big, int getxl_logout);
	// �����Ƿ����
	void SetOffLine(int v);
	// �����Ƿ���ħ��
	bool IsMoYu();
	// �Ƿ��¼��
	bool IsLogin();
	// �Ƿ������ 
	bool IsOffLine();
	// �Ƚϵ�¼�ʺ�
	bool CmpLoginAccount(const char* name);
	// �Ƿ���
	bool IsBig();
	// �Ƿ��ڸ���
	bool IsInFB();
	// �Ƿ���ָ����ͼ
	bool IsInMap(const char* name);
	// �Ƿ���ָ���������� allow=���
	bool IsInArea(int x, int y, int allow=10);
	// �Ƿ���ָ���������� allow=���
	bool IsNotInArea(int x, int y, int allow = 10);
	// �Ƿ��ѻ�ȡ������
	bool IsGetXL();
	// ����
	void Run();
	// ֹͣ
	void Stop(bool v = true);
	// ��ȡ��Ϸ����λ������Ļ�ϵ�����
	void GetGameCenterPos(int& x, int& y);
	// �����Ϸ����
	HWND FindGameWnd();
	// ��ȡ����ģ���ַ
	void FindAllModAddr();
	// ��ȡ��Ϸ����CALL
	void FindAllCall();
	// ��ȡNPC�Ի�����
	DWORD FindNPCTalkCall();
	// ��ȡ�����������ѡ�����
	DWORD FindTeamChkCall();
	// ���ְ�������
	DWORD FindKeyNumCall();
	// ��ȡ�ر���ʾ����
	DWORD FindCloseTipBoxCall();
	// ��ȡ��ȡNPC����ַ����
	DWORD FindGetNpcBaseAddr();
	// ��ȡģ���ַ
	DWORD FindModAddr(LPCWSTR name);
	// ��ȡ����
	bool ReadCoor(DWORD* x=NULL, DWORD* y=NULL);
	// ��ȡ����ֵ
	bool ReadLife(int& life, int& life_max);
	// ��ȡҩ������
	bool ReadQuickKey2Num(int* nums, int length);
	// ��ȡ������Ʒ
	bool ReadBag(DWORD* bag, int length);
	// ��ô��ھ��
	bool FindButtonWnd(int button_id, HWND& hwnd, HWND& parent, const char* text=nullptr);
	// ��ȡ�����ַ
	bool FindCoorAddr();
	// ��ȡ�ƶ�״̬��ַ
	bool FindMoveStaAddr();
	// ��ȡ�Ի���״̬��ַ
	bool FindTalkBoxStaAddr();
	// ��ȡ�Ƿ�ѡ���������״̬��ַ
	bool FindTeamChkStaAddr();
	// ��ȡ��ʾ��״̬��ַ
	bool FindTipBoxStaAddr();
	// ��ȡ������ַ
	bool FindLifeAddr();
	// ��ȡ��������
	bool FindBagAddr();
	// ��õ�����Ʒ��ַ�ı����ַ
	bool FindItemPtr();
	// ��ȡNPC�����Ի�ESI�Ĵ�����ֵ
	bool FindCallNPCTalkEsi();
	// ��ȡ�����б����ַ
	bool FindPetPtrAddr();
	// ��ȡ��ͼ���Ƶ�ַ
	bool FindMapName();
	// ��ĳ��ģ������������
	DWORD SearchFuncInMode(SearchModFuncMsg* info, HANDLE hMod);
	// ��ĳ��ģ����������
	DWORD SearchInMod(LPCTSTR name, DWORD* codes, DWORD length, DWORD* save, DWORD save_length = 1, DWORD step = 1);
	// ����������
	DWORD SearchCode(DWORD* codes, DWORD length, DWORD* save, DWORD save_length=1, DWORD step=4);
	// ��ȡ���ֽ�����
	bool ReadDwordMemory(DWORD addr, DWORD& v);
	// ��ȡ�ڴ�
	bool ReadMemory(PVOID addr, PVOID save, DWORD length);
	// ��ȡ��Ϸ�ڴ�
	bool ReadGameMemory(DWORD flag=0x01);
	// ��ӡ�ռ�
	void InsertLog(char* text);
	// ��ӡ�ռ�
	void InsertLog(wchar_t* text);
	// ��ȡ�����������
	void GetSerBigClickPos(int& x, int& y);
	// ��ȡ�����������
	void GetSerSmallClickPos(int& x, int& y);
public:
	DWORD m_dwHookTid;
	HOOKPROC m_HookKeyProc;
	// ��Ϸ����
	HWND  m_hWnd = NULL;
	// ��Ϸ����2
	HWND  m_hWnd2 = NULL;
	// ��Ϸ���洰��
	HWND  m_hWndPic = NULL;
	// ��Ϸ����ID
	DWORD m_dwPid = 0;
	// ��Ϸ��������
	FLOAT m_fScale;
	// ��ϷȨ�޾��
	HANDLE   m_hGameProcess = NULL; 
	// ��Ϸģ���ַ
	GameModAddr m_GameModAddr;
	// ��Ϸ��ַ
	GameAddr m_GameAddr;
	// ��ϷCALL
	GameCall m_GameCall;
	// ��Ϸ������Ϣ
	GameWnd m_GameWnd;

	DWORD   m_Ecx;

	// ��ǰ��ɫ��Ϣ
	Account m_Account;
	// ��ǰX����
	DWORD   m_dwX;
	// ��ǰY����
	DWORD   m_dwY;
	// ��ǰY����
	DWORD   m_dwLifeMax = 0;
	// ��ȡ�����ʱ��
	int     m_iGetPosTime = 0;
	// �ϴ����겻һ��ʱ��
	int     m_iFlagPosTime = 0;

	// ��������
	DWORD        m_dwGuaiWuCount;

	// �Ƿ��ȡ���
	bool  m_bIsReadEnd;
	// ��ȡ�ڴ��Ĵ�С
	DWORD m_dwReadSize;
	// ��ȡ����ַ
	DWORD m_dwReadBase;
	// ��ȡ������ʱ�ڴ�
	BYTE* m_pReadBuffer;

	// ���ӷ�������
	GameClient* m_pClient;
	// ��Ϸ��������
	GameConf* m_pGameConf;
	// ��Ϸ���̴�����
	GameProc* m_pGameProc;
	// ��Ʒ��
	Item*     m_pItem;
	// �Ի���
	Talk*     m_pTalk;
	// �ƶ���
	Move*     m_pMove;
	// ������
	GuaiWu*   m_pGuaiWu;
	// ������
	Magic*    m_pMagic;
	// ������
	Pet*      m_pPet;

	// ����
	static Game* self;

	CHAR m_Test[0x1000];
public:
	// ���̹���
	static LRESULT CALLBACK CldKeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
	// ö�ٴ���
	static BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam);
	// ö���Ӵ���
	static BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);
	// ����
	void Keyborad(int key, HWND hwnd=NULL);
	// ����
	void LeftClick(int x, int y, HWND hwnd = NULL);
	// ����ƶ�
	void MouseMove(int x, int y);
	// ���︴��
	void Call_ReBoren();
	// �����ƶ�����
	void Call_Run(int x, int y);
	// ����CALL
	void Call_Talk(const char* msg, int type=0);
	// NPC�Ի�
	void Call_NPC(int npc_id);
	// NPC�����Ի�
	void Call_NPCTalk(int no, bool close=true);
	// �ر���ʾ��
	void Call_CloseTipBox(int close=1);
	// ��ȡ��Ʒָ��
	DWORD Call_GetPackageItemByIndex(int index);
	// ʹ����Ʒ
	void Call_UseItem(int item_id);
	// ����Ʒ
	void Call_DropItem(int item_id);
	// ����Ʒ
	void Call_PickUpItem(DWORD id, DWORD x, DWORD y);
	// ������
	void Call_SellItem(int item_id);
	// ��Ǯ
	void Call_SaveMoney(int money);
	// ����Զ�ֿ̲�
	void Call_CheckInItem(int item_id);
	// ȡ���ֿ���Ʒ
	void Call_CheckOutItem(int item_id);
	// ʹ�ÿɴ�����Ʒ
	void Call_TransmByMemoryStone(int item_id);
	// �ż���
	void Call_Magic(int magic_id, int guaiwu_id);
	// �ż���
	void Call_Magic(int magic_id, int x, int y);
	// �������
	void Call_PetOut(int pet_id);
	// �����ٻ�
	void Call_PetIn(int pet_id);
	// �������
	void Call_PetFuck(int pet_id);
	// �������
	void Call_PetUnFuck(int pet_id);
	// ���＼�� key_no=�������� 1=0 2=1 ...
	void Call_PetMagic(int key_no);
	// ��ȡԶ������������Ϣ
	DWORD Call_QueryRemoteTeam(int no);
	// �Ƿ��ж���
	bool Call_IsHaveTeam();
	// �Ƿ��Ƕӳ�
	bool Call_IsTeamLeader();
	// ��������
	void Call_TeamCreate();
	// �뿪����[�ӳ�]
	void Call_TeamDismiss();
	// �뿪����[��Ա]
	void Call_TeamLeave();
	// �������
	void Call_TeamInvite(int player_id);
	// �Զ����
	void Call_TeamAutoJoin(int open=1);
	// �Ƿ�ѡ���������
	void Call_CheckTeam(int v=1);
	// ��ȡ��ַ[��֪��ʲô���Ի�ȡ]
	DWORD Call_GetBaseAddr(int index, DWORD _ecx);
};