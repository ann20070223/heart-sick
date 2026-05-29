#include <windows.h>
#include <string>
#include <vector>
#include <array>
#include <random>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

//=============================================================================
// 基本資料結構
//=============================================================================
struct Suit {
    const wchar_t* symbol;
    COLORREF color;
};

struct CardValue {
    const wchar_t* label;
    int value;
};

struct PipPos {
    int dx;
    int dy;
};

//=============================================================================
// 文字繪製工具類別
//=============================================================================
class TextRenderer {
public:
    static void Draw(
        HDC hdc,
        int x, int y,
        const std::wstring& text,
        int fontSize,
        COLORREF color,
        UINT textAlign = TA_LEFT | TA_TOP,
        int angle = 0,
        const wchar_t* fontName = L"Segoe UI Symbol",
        int fontWeight = FW_NORMAL)
    {
        LOGFONTW lf = { 0 };
        lf.lfHeight = -fontSize;
        lf.lfEscapement = angle * 10;
        lf.lfOrientation = angle * 10;
        lf.lfWeight = fontWeight;
        lf.lfCharSet = DEFAULT_CHARSET;
        lstrcpyW(lf.lfFaceName, fontName);

        HFONT hFont = CreateFontIndirectW(&lf);
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        COLORREF oldColor = SetTextColor(hdc, color);
        int oldBkMode = SetBkMode(hdc, TRANSPARENT);
        UINT oldAlign = SetTextAlign(hdc, textAlign);

        TextOutW(hdc, x, y, text.c_str(), static_cast<int>(text.length()));

        SetTextAlign(hdc, oldAlign);
        SetBkMode(hdc, oldBkMode);
        SetTextColor(hdc, oldColor);
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
    }
};

//=============================================================================
// 撲克牌點數佈局
//=============================================================================
class PipLayout {
public:
    static std::vector<PipPos> GetPositions(int count) {
        int top = -65;
        int bottom = 65;
        int center = 0;
        int midTop = -33;
        int midBottom = 33;
        int v31 = top / 2;
        int v32 = bottom / 2;
        int v41 = -41;
        int v42 = 41;
        int innerTop = (top + v41) / 2;
        int v52 = -v41;
        int v51 = v41;
        int mid = 0;
        int left = -41;
        int right = 41;

        std::vector<std::vector<PipPos>> positions(11);
        PipPos cen = { 0, center },
            top_ = { 0, top }, bot = { 0, bottom },
            tl = { left, top }, tr = { right, top },
            bl = { left, bottom }, br = { right, bottom },
            ml = { left, mid }, mr = { right, mid },
            mtl = { left, midTop }, mtr = { right, midTop },
            mbl = { left, midBottom }, mbr = { right, midBottom },
            ct = { 0, midTop }, cb = { 0, midBottom };

        positions[1] = { cen };
        positions[2] = { top_, bot };
        positions[3] = { top_, cen, bot };
        positions[4] = { tl, tr, bl, br };
        positions[5] = { tl, tr, bl, br, cen };
        positions[6] = { tl, tr, bl, br, ml, mr };
        positions[7] = { tl, tr, bl, br, ml, mr, ct };
        positions[8] = { tl, tr, bl, br, ml, mr, ct, cb };
        positions[9] = { tl, tr, bl, br, mtl, mtr, mbl, mbr, cen };
        positions[10] = { tl, tr, bl, br, mtl, mtr, mbl, mbr, ct, cb };

        if (count < 1 || count > 10) return {};
        return positions[count];
    }
};

//=============================================================================
// 撲克牌類別
//=============================================================================
class Card {
    Suit suit;
    CardValue value;

public:
    static constexpr int WIDTH = 120;
    static constexpr int HEIGHT = 180;
    static constexpr int CORNER_RADIUS = 10;
    static constexpr int PADDING = 10;

    Card(const CardValue& value, const Suit& suit)
        : suit(suit), value(value) {}

    void Draw(HDC hdc, int left, int top) const {
        DrawCardFrame(hdc, left, top);

        int innerLeft = left + PADDING;
        int innerTop = top + PADDING;
        int innerRight = left + WIDTH - PADDING;
        int innerBottom = top + HEIGHT - PADDING;
        int innerCx = left + WIDTH / 2;
        int innerCy = top + HEIGHT / 2;

        DrawTopLeftCorner(hdc, innerLeft, innerTop);
        DrawBottomRightCorner(hdc, innerRight, innerBottom);

        if (value.value >= 1 && value.value <= 10) {
            DrawPips(hdc, value.value, innerCx, innerCy);
        }
        else {
            DrawFaceCard(hdc, innerCx, innerCy);
        }
    }

    static int GetWidth() { return WIDTH; }
    static int GetHeight() { return HEIGHT; }

private:
    void DrawCardFrame(HDC hdc, int left, int top) const {
        RECT rect = { left, top, left + WIDTH, top + HEIGHT };
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        RoundRect(hdc, left, top, left + WIDTH, top + HEIGHT, 10, 10);

        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }

    void DrawTopLeftCorner(HDC hdc, int innerLeft, int innerTop) const {
        TextRenderer::Draw(hdc, innerLeft, innerTop + 16, value.label, 16, suit.color,
            TA_LEFT | TA_BASELINE, 0, L"Segoe UI", FW_NORMAL);
        TextRenderer::Draw(hdc, innerLeft, innerTop + 32, suit.symbol, 16,
            suit.color, TA_LEFT | TA_BASELINE, 0, L"Segoe UI Symbol", FW_NORMAL);
    }

    void DrawBottomRightCorner(HDC hdc, int innerRight, int innerBottom) const {
        TextRenderer::Draw(hdc, innerRight, innerBottom - 16, suit.symbol, 16,
            suit.color, TA_RIGHT | TA_BASELINE, 1800, L"Segoe UI Symbol", FW_NORMAL);
        TextRenderer::Draw(hdc, innerRight, innerBottom, value.label, 16,
            suit.color, TA_RIGHT | TA_BASELINE, 1800, L"Segoe UI", FW_NORMAL);
    }

    void DrawPips(HDC hdc, int count, int cx, int cy) const {
        auto positions = PipLayout::GetPositions(count);
        for (const auto& p : positions) {
            int x = cx + p.dx;
            int y = cy + p.dy;

            if (p.dy > 0) {
                TextRenderer::Draw(hdc, x, y, suit.symbol, 24, suit.color,
                    TA_CENTER | TA_BASELINE, 1800, L"Segoe UI Symbol", FW_NORMAL);
            }
            else {
                TextRenderer::Draw(hdc, x, y, suit.symbol, 24, suit.color,
                    TA_CENTER | TA_BASELINE, 0, L"Segoe UI Symbol", FW_NORMAL);
            }
        }
    }

    void DrawFaceCard(HDC hdc, int cx, int cy) const {
        TextRenderer::Draw(hdc, cx, cy + 14, value.label, 40, suit.color,
            TA_CENTER | TA_BASELINE, 0, L"Segoe UI", FW_BOLD);
    }
};

//=============================================================================
// 視窗類別
//=============================================================================
class PokerWindowApp {
    HINSTANCE hinstance_;
    HWND hwnd_ = nullptr;
    int currentSuitIndex = 0;
    int currentValueIndex = 0;

    // 目前顯示器是在哪一列索引
    int currentShownSeqIndex = 0;

    // 下一個畫面顯示的序列索引
    int nextSeqIndex = 0;

    bool running = false;
    bool anyMatchOccurred = false;

    ULONG64 shownTick = 0;
    bool hasCurrentItem = false;

    // 非連續的 52 張牌，每張是 0~51
    std::vector<int> deckOrder;
    size_t deckPos = 0;

    // 亦即在這 52 張牌中會出現目前這一位，牌面 = 右側文字
    bool matchOccurred = false;

    static constexpr const wchar_t* CLASS_NAME = L"PokerCardWin32MenuReaction";
    static constexpr UINT TIMER_ID = 1001;
    static constexpr UINT TIMER_INT = 1000;
    static constexpr UINT MENU_START = 40001;
    static constexpr UINT MENU_STOP = 40002;
    static constexpr UINT MENU_EXIT = 40003;
    static constexpr UINT ID_SINGLE = 10001;
    static constexpr UINT ID_DOUBLE = 10002;
    static constexpr UINT ID_TRIPLE = 10003;
    static constexpr UINT ID_HELP_ABOUT = 10004;

public:
    explicit PokerWindowApp(HINSTANCE hinstance)
        : hinstance_(hinstance), hmt19937(rd()) {
        std::random_device rd;
        hmt19937.seed(rd());
    }

    bool Create(int nCmdShow) {
        WNDCLASSW wc = { 0 };
        wc.lpfnWndProc = PokerWindowApp::WndProc;
        wc.hInstance = hinstance_;
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

        RegisterClassW(&wc);

        hwnd_ = CreateWindowExW(
            0,
            CLASS_NAME,
            L"Win32 API 撲克反應測試遊戲",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            1000, 700,
            nullptr,
            nullptr,
            hinstance_,
            this
        );

        if (!hwnd_) return false;

        SetMenu(hwnd_, CreateAppMenu());

        ShowWindow(hwnd_, nCmdShow);
        UpdateWindow(hwnd_);

        return true;
    }

    int Run() {
        MSG msg = { 0 };
        while (GetMessageW(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        return static_cast<int>(msg.wParam);
    }

private:
    HMENU CreateAppMenu() {
        HMENU hMenuBar = CreateMenu();
        HMENU hGameMenu = CreatePopupMenu();

        AppendMenuW(hGameMenu, MF_STRING, MENU_START, L"開始 START (&S)");
        AppendMenuW(hGameMenu, MF_STRING, MENU_STOP, L"停止 STOP (&T)");
        AppendMenuW(hGameMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuW(hGameMenu, MF_STRING, MENU_EXIT, L"結束 EXIT (&X)");
        AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hGameMenu, L"遊戲選單 (&G)");

        HMENU hSetMenu = CreatePopupMenu();
        AppendMenuW(hSetMenu, MF_STRING, ID_SINGLE, L"單人計時賽 (&1)");
        AppendMenuW(hSetMenu, MF_STRING, ID_DOUBLE, L"雙人競技模式 (&2)");
        AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hSetMenu, L"模式設定 (&M)");

        AppendMenuW(hMenuBar, MF_STRING, ID_HELP_ABOUT, L"關於 ABOUT (&A)");

        return hMenuBar;
    }

    static std::vector<Suit> GetSuits() {
        static std::array<Suit, 4> suits = {
            Suit{ L"♠", RGB(20, 0, 0) },
            Suit{ L"♥", RGB(220, 0, 0) },
            Suit{ L"♦", RGB(220, 0, 0) },
            Suit{ L"♣", RGB(0, 0, 0) }
        };
        return { suits.begin(), suits.end() };
    }

    static std::vector<CardValue> GetCardValues() {
        static std::array<CardValue, 13> values = {
            CardValue{ L"A", 1 }, CardValue{ L"2", 2 }, CardValue{ L"3", 3 },
            CardValue{ L"4", 4 }, CardValue{ L"5", 5 }, CardValue{ L"6", 6 },
            CardValue{ L"7", 7 }, CardValue{ L"8", 8 }, CardValue{ L"9", 9 },
            CardValue{ L"10", 10 }, CardValue{ L"J", 11 }, CardValue{ L"Q", 12 },
            CardValue{ L"K", 13 }
        };
        return { values.begin(), values.end() };
    }

    static std::vector<std::wstring> GetSequenceLabels() {
        static std::array<const wchar_t*, 13> seq = {
            L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L"10", L"J", L"Q", L"K"
        };
        std::vector<std::wstring> return_seq;
        for (auto s : seq) return_seq.push_back(s);
        return return_seq;
    }

    int RandomIntervalMs() {
        std::uniform_int_distribution<int> dist(100, 1000);
        return dist(hmt19937);
    }

    void BuildShuffledDeck() {
        deckOrder.clear();
        deckOrder.reserve(52);
        for (int i = 0; i < 52; ++i) {
            deckOrder.push_back(i);
        }
        std::shuffle(deckOrder.begin(), deckOrder.end(), hmt19937);
        deckPos = 0;
    }

    bool IsCardAndTextMatched() const {
        const auto& values = GetCardValues();
        const auto& seq = GetSequenceLabels();

        int cardIdx = deckOrder[deckPos] % 13;
        std::wstring cardText = values[cardIdx].label;
        std::wstring shownText = seq[currentShownSeqIndex];

        // 特殊處理 A
        if (cardText == L"A" && shownText == L"1") {
            return true;
        }
        return cardText == shownText;
    }

    void StartSequence() {
        running = true;
        nextSeqIndex = 0;
        currentShownSeqIndex = 0;
        anyMatchOccurred = false;

        BuildShuffledDeck();

        if (!RegisterHotKey(hwnd_, HOTKEY_ID_SPACE, 0, VK_SPACE)) {
            MessageBoxW(hwnd_, L"無法註冊快捷鍵「L」", L"錯誤", MB_OK | MB_ICONERROR);
        }

        ShowNextItem();
        ScheduleNextTimer();
    }

    void StopSequence() {
        KillTimer(hwnd_, TIMER_ID);
        UnregisterHotKey(hwnd_, HOTKEY_ID_SPACE);
        running = false;
    }

    void ScheduleNextTimer() {
        SetTimer(hwnd_, TIMER_ID, RandomIntervalMs(), nullptr);
    }

    bool ShowNextItem() {
        if (deckPos >= deckOrder.size()) {
            return false;
        }

        int cardId = deckOrder[deckPos];
        currentSuitIndex = cardId / 13;
        currentValueIndex = cardId % 13;

        currentShownSeqIndex = nextSeqIndex;
        nextSeqIndex = (nextSeqIndex + 1) % 13;

        shownTick = GetTickCount64();
        hasCurrentItem = true;

        if (IsCardAndTextMatched()) {
            anyMatchOccurred = true;
        }

        InvalidateRect(hwnd_, nullptr, TRUE);
        return true;
    }

    void ShowMatchFailureIfNeeded() {
        if (anyMatchOccurred) {
            // 如果全部顯示完畢卻一次也沒按，或是有相符但沒按到
            MessageBoxW(hwnd_, L"序列顯示完畢，並存在一對以上牌面與數字/JQK 相符，結果：失敗。",
                L"遊戲結束", MB_OK | MB_ICONWARNING);
        }
    }

    void HandleSpacePressed() {
        if (!running || !hasCurrentItem) return;

        ULONG64 now = GetTickCount64();
        ULONG64 elapsed = now - shownTick;

        bool correct = IsCardAndTextMatched();

        StopSequence();

        const auto& seq = GetSequenceLabels();
        const auto& values = GetCardValues();

        std::wstringstream ss;
        ss << L "撲克牌:" << cardText << L" "
           << L "右邊故事:" << ShownText << L" "
            << L"結果:" <<(correct?L"答對": L"失敗")<<L" "
            << L "反應時間:" << elapsed << L"毫秒 "
        MessageBoxW(
            hwnd_,
            ss.str().c_str(),
            correct ? L"恭喜！" : L" 失敗",
            MB_OK | (correct ? MB_ICONINFORMATION : MB_ICONWARNING)
        );
    }

    void OnPaint() {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd_, &ps);

        RECT rc;
        GetClientRect(hwnd_, &rc);

        HBRUSH bg = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdc, &rc, bg);
        DeleteObject(bg);

        if (running && hasCurrentItem) {
            const auto& suits = GetSuits();
            const auto& values = GetCardValues();
            const auto& seq = GetSequenceLabels();

            Card card(values[currentValueIndex], suits[currentSuitIndex]);

            int cardW = Card::GetWidth();
            int cardH = Card::GetHeight();

            int centerX = (rc.left + rc.right) / 2;
            int centerY = (rc.top + rc.bottom) / 2;

            int cardLeft = centerX - cardW / 2;
            int cardTop = centerY - cardH / 2;

            // 繪製中央撲克牌
            card.Draw(hdc, cardLeft, cardTop);

            // 繪製右側數字/JQK
            int labelX = cardLeft + cardW + 100;
            int labelY = centerY;

            TextRenderer::Draw(hdc, labelX, labelY, seq[currentShownSeqIndex],
                64, RGB(80, 80, 80), TA_CENTER | TA_BASELINE,
                0, L"Microsoft JhengHei UI", FW_NORMAL);

            std::wstringstream ss;
            ss << L"剩餘牌數：" << (52 - deckPos) << L" 張";
            TextRenderer::Draw(hdc,
                rc.bottom - 30, 20,
                ss.str(), 20,
                RGB(80, 80, 80), TA_LEFT | TA_BASELINE,
                0, L"Microsoft JhengHei UI",
                FW_NORMAL);
        }

        EndPaint(hwnd_, &ps);
    }

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
            case MENU_START:
                StartSequence();
                return 0;
            case MENU_STOP:
                StopSequence();
                return 0;
            case MENU_EXIT:
                PostQuitMessage(0);
                return 0;
            case ID_HELP_ABOUT:
                MessageBoxW(hwnd_,
                    L"【遊戲規則】\r\n1. 點擊開始\r\n2. 52 張牌會隨機洗牌後再依序顯示\r\n3. 若牌面與右側數字 / JQK 一致，按空白鍵。\r\n4. 52 張全部看完均無符合則顯示失敗。",
                    L"關於關於", MB_OK | MB_ICONINFORMATION);
                break;
            }
        }
                       return 0;

        case WM_TIMER:
            if (wParam == TIMER_ID && running) {
                deckPos++;
                if (deckPos < deckOrder.size()) {
                    ShowNextItem();
                    ScheduleNextTimer();
                }
                else {
                    StopSequence();
                    ShowMatchFailureIfNeeded();
                }
            }
            return 0;

        case WM_HOTKEY:
            if (wParam == HOTKEY_ID_SPACE) {
                HandleSpacePressed();
            }
            break;

        case WM_PAINT:
            OnPaint();
            return 0;

        case WM_DESTROY:
            KillTimer(hwnd_, TIMER_ID);
            UnregisterHotKey(hwnd_, HOTKEY_ID_SPACE);
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hwnd_, msg, wParam, lParam);
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        PokerWindowApp* pThis = nullptr;
        if (msg == WM_NCCREATE) {
            CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
            pThis = static_cast<PokerWindowApp*>(pCreate->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            pThis->hwnd_ = hwnd;
        }
        else {
            pThis = reinterpret_cast<PokerWindowApp*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (pThis) {
            return pThis->HandleMessage(msg, wParam, lParam);
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    std::mt19937 hmt19937;
    std::random_device rd;
    static const int HOTKEY_ID_SPACE = 1;
};

//=============================================================================
// WinMain
//=============================================================================
int WINAPI wWinMain(HINSTANCE hlnstance, HINSTANCE, PWSTR, int nCmdShow) {
    PokerWindowApp app(hlnstance);
    if (!app.Create(nCmdShow)) {
        return 0;
    }
    return app.Run();
}
//=============================================================================