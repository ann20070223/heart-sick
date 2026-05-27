#include <windows.h>
#include <tchar.h>

#define IDM_SINGLE   1001
#define IDM_DOUBLE   1002
#define IDM_MULTI    1003
#define IDM_HELP     1004

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void AddMenus(HWND hwnd);

struct CardPoint {
    float x_pct;
    float y_pct;
    bool is_inverted;
};

// 1 ~ 10 點數分佈（恢復你最原始、最習慣的配置，只靠繪圖邏輯修正倒立問題）
const CardPoint suitLayouts[10][10] = {
    { {0.5f, 0.5f, false} }, // A 
    { {0.5f, 0.22f, false}, {0.5f, 0.78f, true} }, // 2
    { {0.5f, 0.22f, false}, {0.5f, 0.5f, false}, {0.5f, 0.78f, true} }, // 3
    { {0.25f, 0.22f, false}, {0.75f, 0.22f, false}, {0.25f, 0.78f, true}, {0.75f, 0.78f, true} }, // 4
    { {0.25f, 0.22f, false}, {0.75f, 0.22f, false}, {0.5f, 0.5f, false}, {0.25f, 0.78f, true}, {0.75f, 0.78f, true} }, // 5

    // 6 點：你圈起來的右側中間 (0.75, 0.5) 是 true（必須倒立）
    {
        {0.25f, 0.22f, false}, {0.75f, 0.22f, false},
        {0.25f, 0.50f, false}, {0.75f, 0.50f, false},
        {0.25f, 0.78f, true},  {0.75f, 0.78f, true}
    },

    // 7 點：你圈起來的右側中間 (0.75, 0.5) 是 true（必須倒立）
    {
        {0.25f, 0.22f, false}, {0.75f, 0.22f, false},
        {0.25f, 0.50f, false}, {0.75f, 0.50f, false},
        {0.50f, 0.36f, false},
        {0.25f, 0.78f, true},  {0.75f, 0.78f, true}
    },

    // 8 點：你圈起來的右側中間 (0.75, 0.5) 以及中軸線下半 (0.5, 0.64) 都是 true（必須倒立）
    {
        {0.25f, 0.22f, false}, {0.75f, 0.22f, false},
        {0.25f, 0.50f, false}, {0.75f, 0.50f, false},
        {0.50f, 0.36f, false}, {0.50f, 0.64f, true},
        {0.25f, 0.78f, true},  {0.75f, 0.78f, true}
    },

    // 9 點：完全不變動
    {
        {0.25f, 0.22f, false}, {0.75f, 0.22f, false},
        {0.25f, 0.45f, false}, {0.75f, 0.45f, false},
        {0.25f, 0.58f, true},  {0.75f, 0.58f, true},
        {0.25f, 0.78f, true},  {0.75f, 0.78f, true},
        {0.5f,  0.5f,  false}
    },

    // 10 點：完全不變動
    {
        {0.25f, 0.22f, false}, {0.75f, 0.22f, false},
        {0.25f, 0.4f,  false}, {0.75f, 0.4f,  false},
        {0.25f, 0.6f,  true},  {0.75f, 0.6f,  true},
        {0.25f, 0.78f, true},  {0.75f, 0.78f, true},
        {0.5f,  0.31f, false}, {0.5f,  0.69f, true}
    }
};

const int suitCounts[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
    static TCHAR szAppName[] = TEXT("PokerFinalMatchFixedY");
    HWND         hwnd;
    MSG          msg;
    WNDCLASS     wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass)) return 0;

    hwnd = CreateWindow(szAppName, TEXT("心臟病(組別-學號...)"),
        WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, 1420, 800, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

void AddMenus(HWND hwnd) {
    HMENU hMenubar = CreateMenu();
    AppendMenu(hMenubar, MF_STRING, IDM_SINGLE, TEXT("單人計時賽"));
    AppendMenu(hMenubar, MF_STRING, IDM_DOUBLE, TEXT("雙人鍵盤競速"));
    AppendMenu(hMenubar, MF_STRING, IDM_MULTI, TEXT("多人視覺辨識賽"));
    AppendMenu(hMenubar, MF_STRING, IDM_HELP, TEXT("說明"));
    SetMenu(hwnd, hMenubar);
}

HFONT CreateRotatedFont(int height, int weight, int angle, const TCHAR* fontName) {
    LOGFONT lf;
    ZeroMemory(&lf, sizeof(LOGFONT));
    lf.lfHeight = height;
    lf.lfWeight = weight;
    lf.lfEscapement = angle;
    lf.lfOrientation = angle;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = ANTIALIASED_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
    _tcscpy_s(lf.lfFaceName, fontName);
    return CreateFontIndirect(&lf);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    HDC         hdc;
    PAINTSTRUCT ps;
    HFONT       hFontSmall, hFontCenter, hFontBigJK;
    HFONT       hFontSmallRot, hFontCenterRot;
    HFONT       hOldFont;

    const wchar_t* suits[] = { L"♠", L"♥", L"♦", L"♣" };
    const wchar_t* ranks[] = { L"A", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L"10", L"J", L"Q", L"K" };

    int cardWidth = 88;
    int cardHeight = 135;
    int startX = 15;
    int startY = 20;
    int gapX = 16;
    int gapY = 22;

    switch (message) {
    case WM_CREATE:
        AddMenus(hwnd);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        hFontSmall = CreateRotatedFont(18, FW_BOLD, 0, TEXT("Arial"));
        hFontCenter = CreateRotatedFont(22, FW_NORMAL, 0, TEXT("Arial"));
        hFontBigJK = CreateRotatedFont(58, FW_BOLD, 0, TEXT("Arial"));
        hFontSmallRot = CreateRotatedFont(18, FW_BOLD, 1800, TEXT("Arial"));
        hFontCenterRot = CreateRotatedFont(22, FW_NORMAL, 1800, TEXT("Arial"));

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 13; j++) {
                int x = startX + j * (cardWidth + gapX);
                int y = startY + i * (cardHeight + gapY);

                HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
                Rectangle(hdc, x, y, x + cardWidth, y + cardHeight);
                SelectObject(hdc, hOldPen); SelectObject(hdc, hOldBrush);
                DeleteObject(hPen); DeleteObject(hBrush);

                if (i == 1 || i == 2) SetTextColor(hdc, RGB(220, 20, 60));
                else SetTextColor(hdc, RGB(0, 0, 0));
                SetBkMode(hdc, TRANSPARENT);

                // 【1】左上角
                SetTextAlign(hdc, TA_LEFT | TA_TOP);
                hOldFont = (HFONT)SelectObject(hdc, hFontSmall);
                TextOutW(hdc, x + 6, y + 5, ranks[j], (int)wcslen(ranks[j]));
                TextOutW(hdc, x + 6, y + 23, suits[i], 1);

                // 【2】右下角 (倒立)
                SelectObject(hdc, hFontSmallRot);
                TextOutW(hdc, x + cardWidth - 6, y + cardHeight - 5, ranks[j], (int)wcslen(ranks[j]));
                TextOutW(hdc, x + cardWidth - 6, y + cardHeight - 23, suits[i], 1);

                // 【3】核心中央區域 (1 ~ 10)
                if (j < 10) {
                    int count = suitCounts[j];
                    for (int k = 0; k < count; k++) {
                        int suitX = x + (int)(suitLayouts[j][k].x_pct * cardWidth);
                        int suitY = y + (int)(suitLayouts[j][k].y_pct * cardHeight);

                        if (suitLayouts[j][k].is_inverted) {
                            SelectObject(hdc, hFontCenterRot);
                            SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
                            // 關鍵修正：倒立字型在 180 度旋轉時基準線會跑到上方，故將偏移改為 -10，讓它真正反轉並精確定位
                            TextOutW(hdc, suitX, suitY - 10, suits[i], 1);
                        }
                        else {
                            SelectObject(hdc, hFontCenter);
                            SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
                            // 正立維持原本的向下偏移
                            TextOutW(hdc, suitX, suitY + 6, suits[i], 1);
                        }
                    }
                }
                else {
                    // J, Q, K
                    SelectObject(hdc, hFontBigJK);
                    SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
                    TextOutW(hdc, x + (cardWidth / 2), y + (cardHeight / 2) + 18, ranks[j], 1);
                }

                SelectObject(hdc, hOldFont);
            }
        }

        DeleteObject(hFontSmall); DeleteObject(hFontCenter); DeleteObject(hFontBigJK);
        DeleteObject(hFontSmallRot); DeleteObject(hFontCenterRot);

        EndPaint(hwnd, &ps);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}