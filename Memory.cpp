#include "Memory.h"
#include <sstream>
#include <iomanip>
#include <Psapi.h>

//typedef uintptr_t(__stdcall* UniversalCall_t)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
//uintptr_t Memory::UniversalCall(uintptr_t functionAddress, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5) {
//	UniversalCall_t callFunction = (UniversalCall_t)functionAddress;
//	return callFunction(arg1, arg2, arg3, arg4, arg5);
//}

DWORD Memory::ReadDWORD(uintptr_t address) {
	return *reinterpret_cast<DWORD*>(address);
}

void Memory::WriteDWORD(uintptr_t address, DWORD value) {
	*reinterpret_cast<DWORD*>(address) = value;
}

uint64_t Memory::ReadQWORD(uintptr_t address) {
	return *reinterpret_cast<uint64_t*>(address);
}

void Memory::WriteQWORD(uintptr_t address, DWORD64 value)
{
	*reinterpret_cast<uint64_t*>(address) = value;
}

BYTE Memory::ReadBYTE(uintptr_t address) {
	return *reinterpret_cast<BYTE*>(address);
}

void Memory::WriteBYTE(uintptr_t address, BYTE value) {
	*reinterpret_cast<BYTE*>(address) = value;
}

float Memory::ReadFloat(uintptr_t address) {
	return *reinterpret_cast<float*>(address);
}

void Memory::WriteFloat(uintptr_t address, float value) {
	*reinterpret_cast<float*>(address) = value;
}

double Memory::ReadDouble(uintptr_t address) {
	return *reinterpret_cast<double*>(address);
}

void Memory::WriteDouble(uintptr_t address, double value) {
	*reinterpret_cast<double*>(address) = value;
}

bool Memory::ReadBool(uintptr_t address) {
	return *reinterpret_cast<bool*>(address);
}

void Memory::WriteBool(uintptr_t address, bool value) {
	*reinterpret_cast<bool*>(address) = value;
}

std::string Memory::ReadString(uintptr_t address, size_t maxLength) {
	const char* str = reinterpret_cast<const char*>(address);
	return std::string(str, strnlen_s(str, maxLength));
}

void Memory::WriteString(uintptr_t address, const std::string& str) {
	memcpy(reinterpret_cast<void*>(address), str.c_str(), str.length() + 1);
}

std::wstring Memory::ReadWString(uintptr_t address, size_t maxLength) {
	const wchar_t* wstr = reinterpret_cast<const wchar_t*>(address);
	return std::wstring(wstr, wcsnlen_s(wstr, maxLength));
}

void Memory::WriteWString(uintptr_t address, const std::wstring& str) {
	memcpy(reinterpret_cast<void*>(address), str.c_str(), (str.length() + 1) * sizeof(wchar_t));
}

uintptr_t Memory::ReadPtrChain(uintptr_t base, const std::vector<uintptr_t>& offsets) {
	uintptr_t addr = base;
	for (auto offset : offsets) {
		addr = *reinterpret_cast<uintptr_t*>(addr);
		addr += offset;
	}
	return addr;
}

std::wstring Memory::AnsiToUnicode(const std::string& ansiStr) {
	int len = MultiByteToWideChar(CP_ACP, 0, ansiStr.c_str(), -1, NULL, 0);
	std::wstring wstr(len, L'\0');
	MultiByteToWideChar(CP_ACP, 0, ansiStr.c_str(), -1, &wstr[0], len);
	return wstr;
}

std::string Memory::UnicodeToAnsi(const std::wstring& wideStr) {
	int len = WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), -1, NULL, 0, NULL, NULL);
	std::string str(len, '\0');
	WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), -1, &str[0], len, NULL, NULL);
	return str;
}

std::string Memory::DecToHex(uint64_t value) {
	std::stringstream ss;
	ss << std::hex << std::uppercase << value;
	return ss.str();
}

uint64_t Memory::HexToDec(const std::string& hexStr) {
	uint64_t value = 0;
	std::stringstream ss;
	ss << std::hex << hexStr;
	ss >> value;
	return value;
}
// 比较两个数组是否匹配（支持 ?? 通配符）
BOOL CompareArrays(const std::vector<BYTE>& feature, const BYTE* memory) {
	DWORD size = static_cast<DWORD>(feature.size());
	for (DWORD i = 0; i < size; ++i)
	{
		if (feature[i] != memory[i] && feature[i] != 0xFF)
		{
			return FALSE;
		}
	}
	return TRUE;
}
// 将字符串特征码转换成字节数组
std::vector<BYTE> ConvertStringToByteArray(const std::string& pattern) {
	std::vector<BYTE> byteArray;
	for (size_t i = 0; i < pattern.length(); ++i)
	{
		if (pattern[i] == ' ')
		{
			continue;
		}
		else if (pattern[i] == '?' && pattern[i + 1] == '?')
		{
			byteArray.push_back(0xFF); // 通配符
			++i; // 跳过第二个 ?
		}
		else
		{
			std::string byteStr = pattern.substr(i, 2);
			BYTE byte = static_cast<BYTE>(std::stoi(byteStr, nullptr, 16));
			byteArray.push_back(byte);
			++i; // 跳过第二个字符
		}
	}
	return byteArray;
}
std::pair<DWORD64, DWORD64> Memory::GetModuleAddressRange(HMODULE hModule) {
	MODULEINFO moduleInfo;
	if (GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo)))
	{
		DWORD_PTR baseAddress = reinterpret_cast<DWORD_PTR>(moduleInfo.lpBaseOfDll);
		DWORD moduleSize = moduleInfo.SizeOfImage;

		DWORD64 startAddress = static_cast<DWORD64>(baseAddress);
		DWORD64 endAddress = startAddress + static_cast<DWORD64>(moduleSize);

		return std::make_pair(startAddress, endAddress);
	}
	return std::make_pair(0, 0);
}

// 在内存中定位特征码
DWORD64 Memory::LocateSignature(const std::string& maskedPattern, DWORD64 start, DWORD64 end, int offset = 0) {
	std::vector<BYTE> byteArray = ConvertStringToByteArray(maskedPattern);
	DWORD size = static_cast<DWORD>(byteArray.size());
	BYTE* pStart = reinterpret_cast<BYTE*>(start);
	BYTE* pEnd = reinterpret_cast<BYTE*>(end);

	while (pStart < pEnd)
	{
		DWORD scanSize = (pEnd - pStart >= 4096) ? 4096 : static_cast<DWORD>(pEnd - pStart);
		for (DWORD i = 0; i <= scanSize - size; ++i)
		{
			if (CompareArrays(byteArray, pStart + i))
			{
				return reinterpret_cast<DWORD64>(pStart + i + offset);
			}
		}
		pStart += (4096 - size);
	}
	return 0;
}