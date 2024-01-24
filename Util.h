#pragma once

#include <Windows.h>
#include <TlHelp32.h>

#include <vector>

#include <numbers>
#include <cmath>



namespace Process {
	DWORD PID;
	HWND hwnd;
	int windowWeight,windowHeight;
	HANDLE processHandle;
	uintptr_t clientBaseAddres;
	uintptr_t engine2BaseAddres;

	HWND GetHwndByPid(DWORD dwProcessID)
	{
		HWND h = GetTopWindow(0);
		HWND retHwnd = NULL;
		while (h)
		{
			DWORD pid = 0;
			DWORD dwTheardId = GetWindowThreadProcessId(h, &pid);
			if (dwTheardId != 0)
			{
				if (pid == dwProcessID && GetParent(h) == NULL && ::IsWindowVisible(h))
				{
					retHwnd = h;    //会有多个相等值
					//    /*
					//char buf[MAX_PATH] = { 0 };
					//sprintf_s(buf, "%0x", h);
					//MessageBox(NULL, buf, "提示", MB_OK);
					//    */
				}
			}
			h = GetNextWindow(h, GW_HWNDNEXT);
		}
		return retHwnd;
	}


	/*
	取模块基质
	成功返回基质
	失败0
	*/
	uintptr_t GetModulBaseAddres(const char* modulName) {

		HANDLE snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, PID);

		if (snapshotHandle == INVALID_HANDLE_VALUE) return 0;//如果函数失败，它将返回 INVALID_HANDLE_VALUE

		MODULEENTRY32 moduleEntry32 = {};
		moduleEntry32.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(snapshotHandle, &moduleEntry32)) {
			do {
				if (_stricmp(modulName, moduleEntry32.szModule) == 0) {
					CloseHandle(snapshotHandle);
					return reinterpret_cast<uintptr_t>(moduleEntry32.modBaseAddr);
				}

			} while (Module32Next(snapshotHandle, &moduleEntry32));

		}
		CloseHandle(snapshotHandle);
		return 0;
	}


	/*
	* 附加进程
		如果获取失败就return 0;
		成功返回PID;
	*/
	bool AttachProcess(const char* processName) {
		HANDLE snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snapshotHandle == INVALID_HANDLE_VALUE) return 0;//如果函数失败，它将返回 INVALID_HANDLE_VALUE
		PROCESSENTRY32 processEntry32 = {};
		processEntry32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(snapshotHandle, &processEntry32)) {
			do {
				if (_stricmp(processName, processEntry32.szExeFile) == 0) {
					PID = processEntry32.th32ProcessID;
					processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, PID);
				}

			} while (Process32Next(snapshotHandle, &processEntry32));

		}
		CloseHandle(snapshotHandle);
		if (PID != 0) {
			hwnd = GetHwndByPid(PID);
			RECT rctA;
			GetWindowRect(hwnd, &rctA);
			windowWeight = rctA.right - rctA.left; windowHeight = rctA.bottom - rctA.top;
			return true;
		}
		return false;
	}


	char* memRed1024String(uintptr_t addres) {
		char temp[1024];
		ReadProcessMemory(processHandle, (LPCVOID)addres, &temp, sizeof(temp), 0);
		return temp;
	}
	template<typename tname>
	tname memRed(uintptr_t addres) {
		tname value = {};
		ReadProcessMemory(processHandle,(LPCVOID)addres,&value,sizeof(tname),0);
		return value;
	}
	template <typename ReadType>
	bool ReadMemory(DWORD64 Address, ReadType& Value)
	{
		if (ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(Address), &Value, sizeof(ReadType), 0))
			return true;
		return false;
	}
	template<typename tname>
	bool memWrite(uintptr_t addres,tname value) {
		return WriteProcessMemory(processHandle, (LPCVOID)addres, &value, sizeof(tname), 0);
	}

}
namespace Util {
	struct Vector2
	{
		int width, height;
	};
	struct view_matrix_t {
		float* operator[ ](int index) {
			return matrix[index];
		}
		float matrix[4][4];
	};
	struct Vector3 {
		constexpr Vector3(
			const float x = 0.f,
			const float y = 0.f,
			const float z = 0.f) noexcept :
			x(x), y(y), z(z) { }

		int Distance(Vector3 entity) {
			float _x = pow((x - entity.x), 2);
			float _y = pow((y - entity.y), 2);
			float _z = pow((z - entity.z), 2);
			return sqrt(_x + _y + _z);
		}
		constexpr const Vector3& operator-(const Vector3& other) const noexcept
		{
			return Vector3{ x - other.x, y - other.y, z - other.z };
		}

		constexpr const Vector3& operator+(const Vector3& other) const noexcept
		{
			return Vector3{ x + other.x, y + other.y, z + other.z };
		}

		constexpr const Vector3& operator/(const float factor) const noexcept
		{
			return Vector3{ x / factor, y / factor, z / factor };
		}

		constexpr const Vector3& operator*(const float factor) const noexcept
		{
			return Vector3{ x * factor, y * factor, z * factor };
		}

		constexpr const Vector3& ToAngle() const noexcept
		{
			return Vector3{
				std::atan2(-z, std::hypot(x, y)) * (180.0f / std::numbers::pi_v<float>),
				std::atan2(y, x) * (180.0f / std::numbers::pi_v<float>),
				0.0f
			};
		}


		Vector3 worldToScreen(view_matrix_t matrix) {
			float _x = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * z + matrix[0][3];
			float _y = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * z + matrix[1][3];

			float w = matrix[3][0] * x + matrix[3][1] * y + matrix[3][2] * z + matrix[3][3];

			float inv_w = 1.f / w;
			_x *= inv_w;
			_y *= inv_w;

			float screen_x = GetSystemMetrics(SM_CXSCREEN) * 0.5f;
			float screen_y = GetSystemMetrics(SM_CYSCREEN) * 0.5f;

			screen_x += 0.5f * _x * GetSystemMetrics(SM_CXSCREEN) + 0.5f;
			screen_y -= 0.5f * _y * GetSystemMetrics(SM_CYSCREEN) + 0.5f;

			return { screen_x, screen_y, w };
		}
		char* format() {
			char out[128];
			sprintf_s(out,sizeof(out), "[Vec3]X:{%f}\tY:{%f}\tZ:{%f}",x,y,z);
			return out;
		}
		bool worldToScreen(view_matrix_t Matrix,Vec2 & ToPos,Vec2 WindowSize) {
			float View = 0.f;
			float SightX = WindowSize.x / 2, SightY = WindowSize.y / 2;

			View = Matrix[3][0] * x + Matrix[3][1] * y + Matrix[3][2] * z + Matrix[3][3];

			if (View <= 0.01)
				return false;

			ToPos.x = SightX + (Matrix[0][0] * x + Matrix[0][1] *y + Matrix[0][2] * z + Matrix[0][3]) / View * SightX;
			ToPos.y = SightY - (Matrix[1][0] * x + Matrix[1][1] * y + Matrix[1][2] *z + Matrix[1][3]) / View * SightY;
			return true;
		}
		bool onScreen() {return z >= 0;}
		constexpr const bool IsZero() const noexcept
		{
			return x == 0.f && y == 0.f && z == 0.f;
		}

		float x, y, z;
	};
}