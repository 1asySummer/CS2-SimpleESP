#pragma once



#include "Util.h"


#define ull unsigned long long
#define ll long long

class C_CSPlayerPawn {
private:
	uintptr_t addres;
	Util::view_matrix_t matrix;
	void readPlayerInfo() {
		HP = Process::memRed <int>(addres + 0x32C);

		MaxHp = Process::memRed <int>(addres + 0x328);

		TeamNum = Process::memRed <int>(addres + 0x3BF);
		//生命状态
		LifeState = Process::memRed <int>(addres + 0x330);
		//位置
		position = Process::memRed<Util::Vector3>(addres + 0x1224);
		//所在位置
		LastPlace = Process::memRed1024String(addres+ 0x16F4);
		uintptr_t GameSceneNode = 0;
		uintptr_t BoneArrayAddress = 0;

		Process::ReadMemory(addres + (ull)0x310, GameSceneNode);
		Process::ReadMemory(addres + (ull)0x1E0, BoneArrayAddress);


		//for (int i = 0; i < 30; i++)
		//{
		//	Vec2 ScreenPos;
		//	bool IsVisible = false;
		//	if (BoneArray[i].Pos.worldToScreen(matrix, ScreenPos, { (float)windowsize.width,(float)windowsize.height }))
		//		IsVisible = true;
		//	BonePosList.push_back({ {BoneArray[i].Pos.x,BoneArray[i].Pos.y,BoneArray[i].Pos.z} ,ScreenPos,IsVisible });
		//}

	}
public:
	int HP, TeamNum, LifeState,MaxHp;
	char* LastPlace;
	Util::Vector3 position;
	C_CSPlayerPawn(){}
	C_CSPlayerPawn(uintptr_t addres, Util::view_matrix_t matrix) {
		
		this->addres = addres;
		readPlayerInfo();
	}
	
	void printInfo() {
		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(handle, FOREGROUND_GREEN);
		printf("\n[PlayerPawn]---|HP:%d---Team:%d---%s|---Position:{|X:%f|Y:%f|Z:%f|}\n", HP, TeamNum, LastPlace, position.x, position.y, position.z);
	}
	void printInfo(int id) {
		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(handle, FOREGROUND_GREEN);
		printf("\n[PlayerPawnId:%d]---|HP:%d---Team:%d---Lifestae:%d|---Position:{|X:%f|Y:%f|Z:%f|}\n", id,HP, TeamNum, LifeState, position.x, position.y, position.z);
	}
	bool isReal() {
		if (addres!=0&&LifeState==256|| LifeState == 257 || LifeState == 258) {
			return true;
		}
		return false;
	}
	bool isLife() {
		return LifeState == 256;
	}
	uintptr_t getAddres() {
		return addres;
	}
	void update() {
		readPlayerInfo();
	}
};
class CBasePlayerController {
private:
	UINT64 addres;
	void readInfo() {
		name =Process::memRed1024String(addres+ 0x640);
		DWORD64 MoneyServices;
		/*
			DWORD MoneyServices = 0x700;
			DWORD Account = 0x40;
			DWORD TotalCashSpent = 0x48;
			DWORD CashSpentThisRound = 0x4C;
		*/
		MoneyServices = Process::memRed<DWORD64>(addres + 0x700);
		Money= Process::memRed<int>(MoneyServices + 0x40);
		CashSpent = Process::memRed<int>(MoneyServices + 0x48);
		CashSpentTotal = Process::memRed<int>(MoneyServices + 0x4C);
	}
public:
	std::string name="unKnow";
	int Money = 0;
	int CashSpent = 0;
	int CashSpentTotal = 0;

	CBasePlayerController(UINT64 addres) {
		this->addres = Process::memRed<UINT64>(addres);
		readInfo();
	}
	void updata() { readInfo(); }
	bool isAvailable() { return addres != 0; }
	uintptr_t getAddres() {
		return addres;
	}
};