#pragma once
#include "GameStruct.h"

// ����ṹ��
typedef struct game_pet {
	DWORD Id;   // ID
	DWORD Life; // Ѫ��
} GamePet;

typedef struct pet_record {
	DWORD Life; // Ѫ��
	int   Time; // ��¼ʱ��
} PetRecord;

class Game;
// ������
class Pet
{
public:
	// ...
	Pet(Game* p);

	// ��ʼ������
	void InitData();
	// �������
	bool  PetOut(DWORD* nums, DWORD length, bool fuck=true);
	// �������
	bool  PetFuck(DWORD* nums, DWORD length);
	// �������
	bool  PetUnFuck(DWORD* nums, DWORD length);
	// ���＼��
	void  Magic(char key);
	// ��������û��Ѫ������
	DWORD Revive();
	// �Ƿ���Ҫ����
	bool IsNeedRevive(int index, DWORD life, DWORD life_max);
	// ��ȡӵ�г���
	DWORD ReadPetList(DWORD life=1);
public:
	// ��Ϸ��
	Game* m_pGame;
	
	// ӵ�г�������
	DWORD   m_dwPetCount;
	// ����ṹ�б�
	GamePet m_GamePet[8];
	// ����Ѫ����¼
	PetRecord m_PetRecord[3];

	CHAR m_Test[0x1000];
};