// dllmain.cpp : 定义 DLL 应用程序的入口点。
// #include "pch.h"
#include <Windows.h>
#include <clocale>
#include <CommCtrl.h>
#include <string>
#include <Richedit.h>

#include "resource.h"
#include "Memory.h"
#include "ConstantTable.h"

// 全局变量:
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

HINSTANCE hInst;                                // 当前实例
WNDPROC lpPrevWndFunc;							// 旧窗口过程

HWND hMain, hPicBox[7], hEdit[5], hCheckBOx[2], hRadio[2], hCheckBox[2];	// 控件句柄
HWND clienthWnd;			// 游戏客户端窗口句柄

BOOL START, HERO, dt;		// 是否切牌 | 是否大招切牌 | 是否二段大招
pickInfo pickKey;			// 按键信息结构
DWORD64 userObject, baseSkill;	// 人物对象 | 技能对象+偏移
HANDLE hThread;					// 线程句柄
BYTE tragentCard;		// 目标颜色
int destinyNum = 1;		// 大招段数
bool getUserName() {
	//USER_BASE;	
	userObject = Memory::ReadQWORD(USER_BASE);
	// int nameLen = Memory::ReadDWORD(userObject + 0x42F8);
	std::string heroName = Memory::ReadString(userObject + 0x42E8, 11);
	PrintToConsole(L"HeroName: %S", heroName.c_str());
	return  heroName == "TwistedFate";
}

// 存放使用者对输入内容产生何种效果的指引图标
void InitPicBox() {
	// 选牌技能施法按键图标
	hPicBox[0] = CreateWindowExW(
		0, WC_STATICW, NULL,
		WS_VISIBLE | WS_CHILD | SS_BITMAP, // 样式：可见、子窗口、位图样式
		28, 26, 32, 32,
		hMain,
		0, hInst, NULL
	);
	// 加载位图资源
	HBITMAP hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_PICKACARD));
	// 将位图设置到Static控件
	SendMessageW(hPicBox[0], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

	// 命运技能施法按键图标
	hPicBox[1] = CreateWindowExW(
		0, WC_STATICW, NULL,
		WS_VISIBLE | WS_CHILD | SS_BITMAP,
		144, 28, 32, 32,
		hMain, 0, hInst, NULL
	);

	hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_DESTINY));
	SendMessageW(hPicBox[1], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

	// 金牌切牌技能施法按键图标
	hPicBox[2] = CreateWindowExW(
		0, WC_STATICW, NULL,
		WS_VISIBLE | WS_CHILD | SS_BITMAP,
		28, 75, 32, 32,
		hMain, 0, hInst, NULL
	);
	hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_GOLD));
	SendMessageW(hPicBox[2], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

	// 蓝牌切牌技能施法按键图标
	hPicBox[3] = CreateWindowExW(
		0, WC_STATICW, NULL,
		WS_VISIBLE | WS_CHILD | SS_BITMAP,
		144, 75, 32, 32,
		hMain, 0, hInst, NULL
	);
	hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_BLUE));
	SendMessageW(hPicBox[3], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

	// 红牌切牌技能施法按键图标
	hPicBox[4] = CreateWindowExW(
		0, WC_STATICW, NULL,
		WS_VISIBLE | WS_CHILD | SS_BITMAP,
		260, 75, 32, 32,
		hMain, 0, hInst, NULL
	);

	hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_RED));
	SendMessageW(hPicBox[4], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

	// 大招一段图标
	hPicBox[5] = CreateWindowExW(
		0, WC_STATICW, NULL,
		WS_VISIBLE | WS_CHILD | SS_BITMAP,
		144, 122, 32, 32,
		hMain, 0, hInst, NULL
	);

	hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_DESTINY));
	SendMessageW(hPicBox[5], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

	// 大招二段图标
	hPicBox[6] = CreateWindowExW(
		0, WC_STATICW, NULL,
		WS_VISIBLE | WS_CHILD | SS_BITMAP,
		260, 122, 32, 32,
		hMain, 0, hInst, NULL
	);

	hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_GATE));
	SendMessageW(hPicBox[6], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
}

// 存放使用者想要使用的技能快捷键 例如 M W E 3 R
void InitEdit() {
	// 选牌技能施法按键
	hEdit[0] = CreateWindowExW(
		0, WC_EDITW, L"W",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER | ES_UPPERCASE | ES_MULTILINE,
		86, 26, 32, 32,
		hMain, (HMENU)IDC_EDIT_PICK, hInst, NULL);
	// 命运技能施法按键
	hEdit[1] = CreateWindowExW(
		0, WC_EDITW, L"R",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER | ES_UPPERCASE | ES_MULTILINE,
		202, 26, 32, 32,
		hMain, (HMENU)IDC_EDIT_DESTINY, hInst, NULL);
	// 金牌切牌技能施法按键
	hEdit[2] = CreateWindowExW(
		0, WC_EDITW, L"W",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER | ES_UPPERCASE | ES_MULTILINE,
		86, 75, 32, 32,
		hMain, (HMENU)IDC_EDIT_GOLD, hInst, NULL);
	// 蓝牌切牌技能施法按键
	hEdit[3] = CreateWindowExW(
		0, WC_EDITW, L"E",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER | ES_UPPERCASE | ES_MULTILINE,
		202, 75, 32, 32,
		hMain, (HMENU)IDC_EDIT_BLUE, hInst, NULL);
	// 红牌切牌技能施法按键
	hEdit[4] = CreateWindowExW(
		0, WC_EDITW, L"3",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER | ES_UPPERCASE | ES_MULTILINE,
		319, 75, 32, 32,
		hMain, (HMENU)IDC_EDIT_RED, hInst, NULL);
	for (size_t i = 0; i < 5; i++)
	{
		// 设置编辑框的文本限制为1个字符
		SendMessage(hEdit[i], EM_SETLIMITTEXT, 1, 0);
	}
}

// 选择大招一段或者二段进行自动切牌
void InitRadio() {
	// 选中此处后大招一段(开启)切牌(需勾选英雄登场)
	hRadio[0] = CreateWindowExW(
		0, WC_BUTTONW, L"",
		WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
		202, 122, 32, 32,
		hMain, (HMENU)IDC_RADIO_ONE, hInst, NULL);
	// 选中此处后大招二段(传送)切牌(需勾选英雄登场)
	hRadio[1] = CreateWindowExW(
		0, WC_BUTTONW, L"",
		WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
		319, 122, 32, 32,
		hMain, (HMENU)IDC_RADIO_TWO, hInst, NULL);
	// 置选中
	SendDlgItemMessage(hMain, IDC_RADIO_TWO, BM_SETCHECK, BST_CHECKED, 0);
}

void InithCheckBox() {
	// 是否开启切牌
	hCheckBox[0] = CreateWindowExW(
		0, WC_BUTTONW, L"掌控命运",
		WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
		260, 28, 91, 32,
		hMain, (HMENU)IDC_CHECKBOX_START, hInst, NULL);
	// 是否开启大招切牌 - 此功能暂未实现
	hCheckBox[1] = CreateWindowExW(
		0, WC_BUTTONW, L"英雄登场",
		WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
		28, 122, 91, 32,
		hMain, (HMENU)IDC_CHECKBOX_HERO, hInst, NULL);
	// 设置选中状态
	SendMessage(hCheckBox[0], BM_SETCHECK, BST_UNCHECKED, 0); // 未选中
	SendMessage(hCheckBox[1], BM_SETCHECK, BST_CHECKED, 0);  // 选中
}

// 模拟按键
void SimulateKeyPress() {
	// keybd_event 会触发NewProc中的按键消息 当技能键和切牌键相同时，会导致切牌错误

	//keybd_event(pickKey.pickKey, pickKey.pickMek, 0, KEYEVENTF_EXTENDEDKEY);       // 按下
	//Sleep(170);
	//keybd_event(pickKey.pickKey, pickKey.pickMek, KEYEVENTF_KEYUP, KEYEVENTF_EXTENDEDKEY); // 释放
	//return;

	INPUT input[2] = { 0 };
	// 设置 KEYEVENTF_SCANCODE 标志以根据扫描代码定义键盘输入。 
	// 无论当前使用的是哪种键盘，这都可用于模拟物理击键。 
	// 如果扫描代码是扩展密钥，还可以传递 KEYEVENTF_EXTENDEDKEY 标志。 
	// 键的虚拟键值可能会根据当前键盘布局或按下的其他键而更改，但扫描代码将始终相同。
	// 按下键
	input[0].type = INPUT_KEYBOARD;
	// input[0].ki.wVk = vkCode;
	input[0].ki.wScan = pickKey.pickMek;
	input[0].ki.dwFlags = 0;
	input[0].ki.dwExtraInfo = KEYEVENTF_EXTENDEDKEY;
	// 释放键
	input[1].type = INPUT_KEYBOARD;
	// input[1].ki.wVk = vkCode;
	input[1].ki.wScan = pickKey.pickMek;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;
	input[1].ki.dwExtraInfo = KEYEVENTF_EXTENDEDKEY;
	SendInput(2, input, sizeof(INPUT));

}

BYTE getKName() {
	if (baseSkill == NULL)
	{
		DWORD64 P1, P2, P3;
		//PrintToConsole(L"userObject:0x%llx", userObject);
		P1 = Memory::ReadQWORD(userObject + 0x2C60);
		//PrintToConsole(L"P1:0x%llx", P1);

		P2 = Memory::ReadQWORD(P1 + 0x2A0);
		//PrintToConsole(L"P2:0x%llx", P2);

		P3 = Memory::ReadQWORD(P2 + 0xAF0);
		//PrintToConsole(L"P3:0x%llx", BYET_BASE);
		baseSkill = P3;
	}
	DWORD64 P4, P5;
	P4 = Memory::ReadQWORD(baseSkill + 0x128);
	//PrintToConsole(L"P4:0x%llx", P4);

	P5 = Memory::ReadQWORD(P4 + 0x28);
	return Memory::ReadBYTE(P5);
	// PrintToConsole(L"BYET_BASE:0x%llx", BYET_BASE);
}

DWORD WINAPI FindCard(LPVOID lpParam) {
	// 按下的不是选牌按键
	if (pickKey.cKey != pickKey.pickKey)
	{
		//PrintToConsole(L"按下一次选牌按键  %S", pickKey.tragentCard);
		SimulateKeyPress();
	}
	Sleep(168);
	// 获取现在的牌名字
	BYTE currentCard = getKName();
	//PrintToConsole(L"当前牌：%S", currentCard);
	// PickACard
	while (currentCard != 'P') {
		currentCard = getKName();
		//PrintToConsole(L"当前牌：%S", currentCard);
		//PrintToConsole(L"目标牌：%S", tragentCard);
		if (currentCard == tragentCard)
		{
			SimulateKeyPress();
			break;
		}
		Sleep(300);
	}
	CloseHandle(hThread);
	hThread = NULL;
	return 0;
}

// 这是中间层吗
void FCard() {
	if (hThread == NULL)
	{
		hThread = CreateThread(NULL, NULL, FindCard, NULL, NULL, NULL);
		return;
	}
}

// 接管窗口消息
LRESULT APIENTRY NewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//https://learn.microsoft.com/zh-cn/windows/win32/inputdev/virtual-key-codes 虚拟键码
	if (!START)
	{
		// 开启切牌才会接收键盘消息
		return CallWindowProc(lpPrevWndFunc, hWnd, uMsg, wParam, lParam);
	}
	// 按下键盘
	if (uMsg == WM_KEYDOWN)
	{
		if (wParam == pickKey.goldKey)
		{
			pickKey.cKey = wParam;
			tragentCard = 'G';
			// GoldCardLock
			FCard();
			return 0;
		}
		else if (wParam == pickKey.blueKey)
		{
			pickKey.cKey = wParam;
			tragentCard = 'B';
			//  BlueCardLock
			FCard();
			return 0;
		}
		else if (wParam == pickKey.redKey)
		{
			pickKey.cKey = wParam;
			tragentCard = 'R';
			// RedCardLock
			FCard();
			return 0;
		}
		//else if (wParam == pickKey.destinyKey && HERO)
		//{
		//	// 二段大招
		//	if (dt)
		//	{

		//	}
		//	else
		//	{

		//	}
		//}
	}
	return CallWindowProc(lpPrevWndFunc, hWnd, uMsg, wParam, lParam);
}

// 初始化应用程序
void InitApplication(HWND hWnd) {
	hMain = hWnd;
	InitPicBox();
	InitEdit();
	InitRadio();
	InithCheckBox();
	// Windows 10版本 2004 中引入 WDA_EXCLUDEFROMCAPTURE
	// 窗口仅显示在监视器上。 在其他任何位置，窗口根本不显示
	SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE);
	Sleep(3344);
	while (clienthWnd == NULL)//如果窗口句柄是NULL就继续循环
	{
		//寻找窗口句柄
		clienthWnd = FindWindowW(L"RiotWindowClass", NULL);//"RiotWindowClass","League of Legends (TM) Client"
		// clienthWnd = FindWindowW(L"asmtest", NULL);//"RiotWindowClass","League of Legends (TM) Client"
		Sleep(1314);
	}
	lpPrevWndFunc = (WNDPROC)SetWindowLongPtrW(clienthWnd, GWLP_WNDPROC, (LRESULT)NewProc);
	if (lpPrevWndFunc == NULL)
	{
		//无法接管消息 输出错误结果
		PrintToConsole(L"[错误] SetWindowLong-  错误代码：%d", GetLastError());
		//return 0;
	}
}

// 从编辑框获取按键字符并转换为虚拟键码
int GetKeyFromEdit(HWND hEdit) {
	WCHAR buffer[2] = { 0 };
	GetWindowTextW(hEdit, buffer, 2);

	if (buffer[0] == 0) {
		return 0;
	}

	// 转换为大写字母的虚拟键码
	return toupper(buffer[0]);
}

static HBRUSH hBackgroundBrush = NULL;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		hBackgroundBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
		InitApplication(hWnd);
	}
	break;
	case WM_CTLCOLORSTATIC:
	{
		HDC hdcStatic = (HDC)wParam;
		SetTextColor(hdcStatic, GetSysColor(COLOR_WINDOWTEXT)); // 设置文字颜色
		SetBkMode(hdcStatic, TRANSPARENT); // 设置透明背景模式
		return (LRESULT)hBackgroundBrush; // 返回背景画刷
	}
	case WM_CTLCOLORBTN:
	{
		HDC hdcButton = (HDC)wParam;
		SetBkMode(hdcButton, TRANSPARENT);
		return (LRESULT)hBackgroundBrush;
	}
	break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_CHECKBOX_START:
		{
			if (HIWORD(wParam) == BN_CLICKED)
			{
				UINT state = IsDlgButtonChecked(hWnd, IDC_CHECKBOX_START);
				START = (state == BST_CHECKED);

				if (START)
				{
					if (!getUserName())
					{
						SendMessage(hCheckBox[0], BM_SETCHECK, BST_UNCHECKED, 0);
						break;
					}

					// 获取编辑框输入的内容转换成VK_CODE
					pickKey.pickKey = GetKeyFromEdit(hEdit[0]);
					pickKey.destinyKey = GetKeyFromEdit(hEdit[1]);

					pickKey.goldKey = GetKeyFromEdit(hEdit[2]);
					pickKey.blueKey = GetKeyFromEdit(hEdit[3]);
					pickKey.redKey = GetKeyFromEdit(hEdit[4]);
					pickKey.pickMek = MapVirtualKey(pickKey.pickKey, 0);
					//PrintToConsole(L"pickKey.pickKey:%d", pickKey.pickKey);
					//PrintToConsole(L"pickKey.destinyKey:%d", pickKey.destinyKey);
					//PrintToConsole(L"pickKey.goldKey:%d", pickKey.goldKey);
					//PrintToConsole(L"pickKey.blueKey:%d", pickKey.blueKey);
					//PrintToConsole(L"pickKey.redKey:%d", pickKey.redKey);
				}
				// PrintToConsole(L"START == :%d", START);
				for (size_t i = 0; i < 5; i++)
				{
					// SendMessage(hEdit[i], EM_SETBKGNDCOLOR, 0, RGB(6, 1, 8));
					SendMessage(hEdit[i], EM_SETREADONLY, (WPARAM)START, 0);
				}
			}
		}
		break;
		case IDC_CHECKBOX_HERO:
		{
			// 大招黄牌
			if (HIWORD(wParam) == BN_CLICKED)
			{
				UINT state = IsDlgButtonChecked(hWnd, IDC_CHECKBOX_HERO);
				HERO = (state == BST_CHECKED);
			}
		}
		case IDC_EDIT_PICK:
		case IDC_EDIT_DESTINY:
		case IDC_EDIT_GOLD:
		case IDC_EDIT_BLUE:
		case IDC_EDIT_RED:
		{
			// 全选编辑框
			HWND hEdit = GetDlgItem(hWnd, LOWORD(wParam));
			SendMessage(hEdit, EM_SETSEL, 0, -1);
			return 0;
		}
		break;
		default:
			break;
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		if (lpPrevWndFunc && clienthWnd) {
			// 还原窗口过程
			SetWindowLongPtr(clienthWnd, GWLP_WNDPROC, (LONG_PTR)lpPrevWndFunc);
		}
		if (hBackgroundBrush)
		{
			// 清理画笔
			DeleteObject(hBackgroundBrush);
		}
		for (int i = 0; i < 7; ++i)
		{
			// 清理位图资源
			HBITMAP hBmp = (HBITMAP)SendMessage(hPicBox[i], STM_GETIMAGE, IMAGE_BITMAP, 0);
			if (hBmp) DeleteObject(hBmp);
		}
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_M));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDS_APP_CLASS);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_M));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance()
{
	// 预期客户区大小
	RECT rc = { 0, 0, 388, 208 };

	// 根据样式计算窗口尺寸
	AdjustWindowRectEx(&rc,
		WS_OVERLAPPED |
		WS_CAPTION |
		WS_SYSMENU |
		WS_MINIMIZEBOX,
		FALSE, 0);

	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;
	// 创建窗口
	HWND hWnd = CreateWindowExW(
		WS_EX_TOPMOST, szWindowClass, szTitle,
		WS_OVERLAPPED |
		WS_CAPTION |
		WS_SYSMENU |
		WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		NULL, NULL, hInst, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, SW_SHOW);
	// UpdateWindow(hWnd);

	return TRUE;
}

DWORD WINAPI ShowMainWindow(LPVOID lpParam)
{
	hInst = (HINSTANCE)lpParam; // 使用DLL的实例句柄
	// TODO: 在此处放置代码。

	// 初始化全局字符串
	LoadStringW(hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInst, IDS_APP_CLASS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInst);

	// 执行应用程序初始化:
	if (!InitInstance())
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDS_APP_CLASS));

	MSG msg;

	// 主消息循环:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	//禁用DLL_THREAD_ATTACH和DLL_THREAD_DETACH通知，减小程序的工作集大小
	DisableThreadLibraryCalls(hModule);
	setlocale(LC_ALL, ".utf8");
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		// 入口
		CreateThread(NULL, NULL, ShowMainWindow, hModule, NULL, NULL);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

