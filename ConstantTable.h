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

// �����ַ
#define USER_BASE				0x141BC4A88

struct pickInfo {
	int pickKey;	// ѡ�Ƽ���ʩ������
	int pickMek;	// ѡ�Ƽ����������
	int destinyKey;	// ���˼���ʩ������

	int goldKey;	// �������Ƽ���ʩ������
	int blueKey;	// �������Ƽ���ʩ������
	int redKey;		// �������Ƽ���ʩ������
	int cKey;		// ���µİ���
};

/**
 * @brief ��ӡ���ַ���������̨
 * @param format ��Lǰ׺�ĸ�ʽ���ַ���
 * @param ... �ɱ����
 * @note �Զ��������̨��������С
 */
void PrintToConsole(const wchar_t* format, ...)
{
	// ����һ���ɱ�����б�
	va_list args;
	va_start(args, format);

	// �����ʽ���ַ�����ĳ���
	int length = _vscwprintf(format, args) + 5 + 1; // ��� 5 ���ַ��ĳ������ڴ洢���

	// ���仺�����������ʽ������ַ���
	wchar_t* buffer = new wchar_t[length];

	// ��ʽ���ַ�������ӱ��
	swprintf(buffer, length, L"[pickFase] ");
	vswprintf(buffer + 5, static_cast<size_t>(length) - 5, format, args);

	// ����ʽ������ַ������������̨
	wprintf(L"%s\n", buffer);

	// �ͷŻ������Ϳɱ�����б�
	delete[] buffer;
	va_end(args);
}
