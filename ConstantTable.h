#pragma once

#define IDC_RADIO_ONE			1001
#define IDC_RADIO_TWO			1002

#define IDC_CHECKBOX_START		1003
#define IDC_CHECKBOX_HERO		1004

#define IDC_EDIT_PICK			1005
#define IDC_EDIT_DESTINY		1006

#define IDC_EDIT_GOLD			1007
#define IDC_EDIT_BLUE			1008
#define IDC_EDIT_RED			1009

#define MAX_LOADSTRING			66

// 人物基址
#define USER_BASE				0x141BC4A88

struct pickInfo {
	int pickKey;	// 选牌技能施法按键
	int pickMek;	// 选牌技能虚拟键码
	int destinyKey;	// 命运技能施法按键

	int goldKey;	// 金牌切牌技能施法按键
	int blueKey;	// 蓝牌切牌技能施法按键
	int redKey;		// 红牌切牌技能施法按键
	int cKey;		// 按下的按键
};

/**
 * @brief 打印宽字符串到控制台
 * @param format 带L前缀的格式化字符串
 * @param ... 可变参数
 * @note 自动处理控制台缓冲区大小
 */
void PrintToConsole(const wchar_t* format, ...)
{
	// 创建一个可变参数列表
	va_list args;
	va_start(args, format);

	// 计算格式化字符串后的长度
	int length = _vscwprintf(format, args) + 5 + 1; // 添加 5 个字符的长度用于存储标记

	// 分配缓冲区来保存格式化后的字符串
	wchar_t* buffer = new wchar_t[length];

	// 格式化字符串并添加标记
	swprintf(buffer, length, L"[pickFase] ");
	vswprintf(buffer + 5, static_cast<size_t>(length) - 5, format, args);

	// 将格式化后的字符串输出到控制台
	wprintf(L"%s\n", buffer);

	// 释放缓冲区和可变参数列表
	delete[] buffer;
	va_end(args);
}
