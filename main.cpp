#include <iostream>
#include <iomanip>
#include <thread>
#include <string>

#include "OS-ImGui/OS-ImGui.h"

#include "Util.h"
#include "CSGO.h"
#include "offsets.hpp"
#include "DataShare.h"

#define ull unsigned long long
#define ll long long


Util::view_matrix_t matrix;
C_CSPlayerPawn localPlayer;

using namespace std;
void init() {


	Process::AttachProcess("cs2.exe");

	Process::clientBaseAddres = Process::GetModulBaseAddres("client.dll");
	Process::engine2BaseAddres = Process::GetModulBaseAddres("engine2.dll");


	matrix = Process::memRed<Util::view_matrix_t>(Process::clientBaseAddres+client_dll::dwViewMatrix);
}
//更新数据
//	矩阵
void updata() {
	matrix = Process::memRed<Util::view_matrix_t>(Process::clientBaseAddres + client_dll::dwViewMatrix);

	ull CWORD = Process::memRed<ull>(Process::clientBaseAddres + client_dll::dwEntityList) + (ull)0x10;

	localPlayer = C_CSPlayerPawn(Process::memRed<ull>(Process::clientBaseAddres + client_dll::dwLocalPlayerPawn),matrix);//259b6402800
	Data::localPlayer = localPlayer;
	//Clear:
	Data::players.clear();
	Data::playersController.clear();

	for (int i = 1; i < 64; i++)
	{
		//PlayerPawn:
		ull addres = Process::memRed<ull>(Process::clientBaseAddres + ((ull)0x16E1C70 + (ull)(i * 0x10)));
		C_CSPlayerPawn player = C_CSPlayerPawn(addres,matrix);
		if (player.isReal()) {
			Data::players.push_back(player);
		}

		//PlayerControll:
		ull cbpcaddres = Process::memRed<ull>(CWORD) + (i * (ull)0x78);//78
		CBasePlayerController c = CBasePlayerController(cbpcaddres);
		if (c.isAvailable()) {
			Data::playersController.push_back(c);
		}
	}
	
}

//检查功能是否开启
void Detectionfunction() {
	//ESP:
	if (Capabilities::ESP) {
		//ESP config:
		static float ratio = 0.618;
		static float thickness = 2.5;
		static bool drawTeamate = false;
		ImGui::Begin("ESPConfig",&Capabilities::ESP);

		ImGui::Checkbox("DrawTeamate", &drawTeamate);//显示队友?
		ImGui::SliderFloat("Ratio", &ratio, 0.5f, 1.0f, "%.3f");//缩放比例
		ImGui::SliderFloat("Thickness",&thickness,0.0f,5.0f,"%.1f");
		
		ImGui::End();
		for (int i = 0; i < Data::players.size(); i++) {
			C_CSPlayerPawn player = Data::players[i];
			Util::Vector3 playerPosition0 = player.position;			
			Util::Vector3 playerPosition1 = player.position;

			playerPosition0.z += 73;

			Util::Vector3 screenpos_head=playerPosition0.worldToScreen(matrix);
			Util::Vector3 screenpos1_foot=playerPosition1.worldToScreen(matrix);

			float boxHeight = screenpos1_foot.y - screenpos_head.y;
			float boxWeight = boxHeight * ratio;

			Vec2 realPosition = {screenpos_head.x-boxWeight/2,screenpos_head.y};
			Vec2 realSize = { boxWeight,boxHeight };
			if (player.isLife()&& screenpos_head.onScreen()) {
				//绘制队友
				if (drawTeamate == true) {
					if (localPlayer.TeamNum == player.TeamNum) {
						Gui.Rectangle(realPosition, realSize, IM_COL32(0, 255, 0, 255), thickness);
					}
					else {
						Gui.Rectangle(realPosition, realSize, IM_COL32(255, 0, 0, 255), thickness);
					}
				}
				else {
					if (localPlayer.TeamNum != player.TeamNum) {
						Gui.Rectangle(realPosition, realSize, IM_COL32(255, 0, 0, 255), thickness);
					}
				}
				
			}
		}
	}
}

//绘制回调
void drawCallBack() {
	updata();
	Detectionfunction();
	ImGui::Begin("Menu");

	ImGui::Text(string("Player Cout:").append(to_string(Data::players.size())).c_str());

	Gui.MyCheckBox2("ESP", &Capabilities::ESP);

	if (ImGui::Button("Quit")) { 
		CloseHandle(Process::processHandle);
		Data::running = false;
		Gui.Quit();
	}

	ImGui::End();
}
int main() {
	init();
	Gui.AttachAnotherWindow(Process::PID, drawCallBack);
	while (Data::running) {
		//OnRunning
	}
	cout << "Finish" << endl;
}

