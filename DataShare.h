#pragma once

#include <vector>


#include "CSGO.h"
namespace Data {
	bool running = true;
	C_CSPlayerPawn localPlayer;
	std::vector<C_CSPlayerPawn> players = std::vector<C_CSPlayerPawn>();
	std::vector<CBasePlayerController> playersController = std::vector<CBasePlayerController>();
}

namespace Capabilities {
	static bool ESP;
}