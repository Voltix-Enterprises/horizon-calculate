#define UNICODE
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

HWND hDisplay;
HWND hAbout = NULL;

// Menu command IDs
#define ID_FILE_EXIT     1001
#define ID_EDIT_COPY     2002
#define ID_EDIT_PASTE    2003
#define ID_EDIT_CLEAR    2004
#define ID_HELP_ABOUT    4001

// RtlGetVersion
typedef LONG (WINAPI *RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

void GetWindowsVersion(RTL_OSVERSIONINFOW *rovi) {
    ZeroMemory(rovi, sizeof(*rovi));
    rovi->dwOSVersionInfoSize = sizeof(*rovi);

    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (!hMod) return;

    RtlGetVersionPtr fx = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
    if (fx) fx(rovi);
}

// ---------------- ABOUT WINDOW PROC ----------------

LRESULT CALLBACK AboutProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        hAbout = NULL;
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ---------------- ABOUT WINDOW POSITIONING ----------------

void PositionAboutWindow(HWND parent) {
    if (hAbout == NULL) return;

    RECT r;
    GetWindowRect(parent, &r);

    int y = r.top;      // perfect flush with title bar
    int x = r.right;    // zero gap

    int width  = 350;
    int height = 300;   // prevents clipping

    SetWindowPos(hAbout, NULL, x, y, width, height, SWP_NOZORDER);
}

// ---------------- ABOUT DIALOG CREATION ----------------

void ShowAboutDialog(HWND parent, HINSTANCE hInstance) {
    if (hAbout != NULL) {
        SetForegroundWindow(hAbout);
        return;
    }

    RTL_OSVERSIONINFOW rovi;
    GetWindowsVersion(&rovi);

    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);

    const wchar_t *arch =
        (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ? L"x64" :
        (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64) ? L"ARM64" :
        (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) ? L"x86" :
        L"Unknown";

    wchar_t text[600];
    swprintf(
        text, 600,
        L"Horizon Calculate\n"
        L"Version 0.6\n"
        L"Build: %hs %hs\n"
        L"© Mr_Dubstep 2026\n\n"
        L"Windows Information:\n"
        L"Version: %lu.%lu.%lu\n"
        L"Architecture: %s\n",
        __DATE__, __TIME__,
        rovi.dwMajorVersion,
        rovi.dwMinorVersion,
        rovi.dwBuildNumber,
        arch
    );

    RECT r;
    GetWindowRect(parent, &r);

    int x = r.right;
    int y = r.top;

    hAbout = CreateWindowW(
        L"HorizonAboutWindow",
        L"About Horizon Calculate",
        WS_CAPTION | WS_SYSMENU | WS_BORDER,
        x, y, 350, 300,
        parent, NULL, hInstance, NULL
    );

    CreateWindowW(L"static", text,
        WS_CHILD | WS_VISIBLE,
        10, 10, 330, 220,
        hAbout, NULL, hInstance, NULL);

    CreateWindowW(L"button", L"OK",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        130, 240, 80, 30,
        hAbout, (HMENU)IDOK, hInstance, NULL);

    ShowWindow(hAbout, SW_SHOW);
}

// ---------------- CALCULATOR LOGIC ----------------

void AppendChar(wchar_t c) {
    wchar_t buf[256];
    GetWindowTextW(hDisplay, buf, 256);
    int len = lstrlenW(buf);
    buf[len] = c;
    buf[len+1] = 0;
    SetWindowTextW(hDisplay, buf);
}

void ClearDisplay() {
    SetWindowTextW(hDisplay, L"");
}

void Calculate() {
    wchar_t buf[256];
    GetWindowTextW(hDisplay, buf, 256);

    float a = 0, b = 0;
    wchar_t op = 0;

    swscanf(buf, L"%f %c %f", &a, &op, &b);

    float result;
    wchar_t out[256];

    switch (op) {
        case L'+': result = a + b; break;
        case L'-': result = a - b; break;
        case L'*': result = a * b; break;
        case L'/':
            if (b == 0) {
                SetWindowTextW(hDisplay, L"Error");
                return;
            }
            result = a / b;
            break;
        default:
            SetWindowTextW(hDisplay, L"Error");
            return;
    }

    swprintf(out, 256, L"%g", result);
    SetWindowTextW(hDisplay, out);
}

// ---------------- MENU CREATION ----------------

void CreateAppMenu(HWND hwnd) {
    HMENU hMenu = CreateMenu();
    HMENU hFile = CreatePopupMenu();
    HMENU hEdit = CreatePopupMenu();
    HMENU hView = CreatePopupMenu();
    HMENU hHelp = CreatePopupMenu();

    AppendMenuW(hFile, MF_STRING, ID_FILE_EXIT, L"Exit");
    AppendMenuW(hEdit, MF_STRING, ID_EDIT_COPY, L"Copy");
    AppendMenuW(hEdit, MF_STRING, ID_EDIT_PASTE, L"Paste");
    AppendMenuW(hEdit, MF_STRING, ID_EDIT_CLEAR, L"Clear");
    AppendMenuW(hHelp, MF_STRING, ID_HELP_ABOUT, L"About Horizon Calculate");

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFile, L"File");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hEdit, L"Edit");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hView, L"View");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hHelp, L"?");

    SetMenu(hwnd, hMenu);
}

// ---------------- MAIN WINDOW PROC ----------------

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE: {
        CreateAppMenu(hwnd);

        hDisplay = CreateWindowW(
            L"edit", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_RIGHT,
            10, 10, 260, 40,
            hwnd, NULL, NULL, NULL
        );

        int x = 10, y = 60;
        int id = 100;

        const wchar_t *labels[5][4] = {
            { L"C", L"/", L"*", L"-" },
            { L"7", L"8", L"9", L"+" },
            { L"4", L"5", L"6", L"=" },
            { L"1", L"2", L"3", L""  },
            { L"0", L"",  L"",  L""  }
        };

        for (int r = 0; r < 5; r++) {
            x = 10;
            for (int c = 0; c < 4; c++) {
                if (labels[r][c][0] != 0) {
                    CreateWindowW(
                        L"button", labels[r][c],
                        WS_VISIBLE | WS_CHILD,
                        x, y, 60, 40,
                        hwnd, (HMENU)(UINT_PTR)id, NULL, NULL
                    );
                }
                id++;
                x += 65;
            }
            y += 45;
        }
        break;
    }

    case WM_MOVE:
        PositionAboutWindow(hwnd);
        break;

    case WM_COMMAND: {
        int id = LOWORD(wParam);

        switch (id) {
        case ID_FILE_EXIT:
            PostQuitMessage(0);
            return 0;

        case ID_EDIT_COPY:
            SendMessageW(hDisplay, WM_COPY, 0, 0);
            return 0;

        case ID_EDIT_PASTE:
            SendMessageW(hDisplay, WM_PASTE, 0, 0);
            return 0;

        case ID_EDIT_CLEAR:
            ClearDisplay();
            return 0;

        case ID_HELP_ABOUT:
            ShowAboutDialog(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
            return 0;
        }

        if (id >= 100 && id < 120) {
            int index = id - 100;

            const wchar_t *labels[20] = {
                L"C", L"/", L"*", L"-",
                L"7", L"8", L"9", L"+",
                L"4", L"5", L"6", L"=",
                L"1", L"2", L"3", L"",
                L"0", L"", L""
            };

            const wchar_t *label = labels[index];

            if (label[0] == L'C') {
                ClearDisplay();
                return 0;
            }

            if (label[0] == L'=') {
                Calculate();
                return 0;
            }

            if (label[0] != 0) {
                if (wcscspn(label, L"+-*/") == 0) {
                    AppendChar(L' ');
                    AppendChar(label[0]);
                    AppendChar(L' ');
                } else {
                    AppendChar(label[0]);
                }
            }
            return 0;
        }
        break;
    }

    case WM_DESTROY:
        if (hAbout != NULL) DestroyWindow(hAbout);
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ---------------- WINMAIN ----------------

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrev, PWSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"HorizonCalculateWindow";

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    wc.hIcon = (HICON)LoadImageW(NULL, L"horizon-calculate.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

    // Register About window class
    WNDCLASSW about = {0};
    about.lpfnWndProc = AboutProc;
    about.hInstance = hInstance;
    about.lpszClassName = L"HorizonAboutWindow";
    about.hIcon = wc.hIcon;
    RegisterClassW(&about);

    RegisterClassW(&wc);

    CreateWindowW(
        CLASS_NAME, L"Horizon Calculate",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
        100, 100, 300, 350,
        NULL, NULL, hInstance, NULL
    );

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
