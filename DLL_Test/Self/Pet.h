#pragma once
#include "GameStruct.h"

// ����ṹ��
typedef struct game_pet {
	DWORD Id;   // ID
	DWORD Life; // Ѫ��
} GamePet;

class Game;

// ������
class Pet
{
public:
	// ...
	Pet(Game* p);

	// �������
	bool  PetOut(DWORD* nums, DWORD length, bool fuck=true);
	// ��������û��Ѫ������
	DWORD Revive();
	// ��ȡӵ�г���
	DWORD ReadPetList(DWORD life=1);
public:
	// ��Ϸ��
	Game* m_pGame;
	
	// ӵ�г�������
	DWORD   m_dwPetCount;
	// ����ṹ�б�
	GamePet m_GamePet[8];
};