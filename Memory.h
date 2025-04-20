#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <vector>

class Memory
{
public:
	// -------- 基础读写 --------

	// static uintptr_t UniversalCall(uintptr_t functionAddress, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);

	// 基础类型读写
	static DWORD     ReadDWORD(uintptr_t address);
	static void      WriteDWORD(uintptr_t address, DWORD value);

	static DWORD64   ReadQWORD(uintptr_t address);
	static void      WriteQWORD(uintptr_t address, DWORD64 value);

	static BYTE      ReadBYTE(uintptr_t address);
	static void      WriteBYTE(uintptr_t address, BYTE value);

	static float     ReadFloat(uintptr_t address);
	static void      WriteFloat(uintptr_t address, float value);

	static double    ReadDouble(uintptr_t address);
	static void      WriteDouble(uintptr_t address, double value);

	static bool      ReadBool(uintptr_t address);
	static void      WriteBool(uintptr_t address, bool value);

	// 字符串读写
	static std::string  ReadString(uintptr_t address, size_t maxLength = 256);
	static void         WriteString(uintptr_t address, const std::string& str);

	static std::wstring ReadWString(uintptr_t address, size_t maxLength = 256);
	static void         WriteWString(uintptr_t address, const std::wstring& wstr);

	// 多级指针偏移
	static uintptr_t ReadPtrChain(uintptr_t base, const std::vector<uintptr_t>& offsets);

	// 读取多级偏移后返回最终数据
	// DWORD64 hp = Memory::ReadPtrChainValue<DWORD64>(base, { 0x10, 0x20 });
	// DWORD level = Memory::ReadPtrChainValue<DWORD>(base, { 0x18, 0x30 });
	// float speed = Memory::ReadPtrChainValue<float>(base, { 0x28, 0x38 });
	template<typename T>
	static T ReadPtrChainValue(uintptr_t base, const std::vector<uintptr_t>& offsets)
	{
		uintptr_t addr = base;
		for (auto offset : offsets)
		{
			addr = *reinterpret_cast<uintptr_t*>(addr);
			addr += offset;
		}
		return *reinterpret_cast<T*>(addr);
	}

	// 编码转换
	static std::wstring AnsiToUnicode(const std::string& ansiStr);
	static std::string  UnicodeToAnsi(const std::wstring& wideStr);

	// 进制转换
	static std::string  DecToHex(uint64_t value);
	static uint64_t     HexToDec(const std::string& hexStr);

	//获得模块起始地址和结束地址
	// std::pair<DWORD64, DWORD64> addressRange = method::GetModuleAddressRange(GetModuleHandleW(nullptr));
	// startAddress = addressRange.first;
	// endAddress = addressRange.second;
	static std::pair<DWORD64, DWORD64> GetModuleAddressRange(HMODULE hModule);

	// 搜索AOB特征码
	static DWORD64 LocateSignature(const std::string& maskedPattern, DWORD64 start, DWORD64 end, int offset);

};
