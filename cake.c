/*-----------------------------------------------------
   CAKE.C programmed by Charles Petzold, November 1985
  -----------------------------------------------------*/
#include "stdafx.h"
#include <windows.h>
#include "cake.h"

static char szAppName[] = "Cake";
static char strCake[80] = "<text>";
static int cCandles = 1;
static BOOL bFlash = FALSE;
static HINSTANCE hInst;

LRESULT CALLBACK WndProc(HWND, unsigned, WPARAM, LPARAM);
INT_PTR CALLBACK SettingsProc(HWND, unsigned, WPARAM, LPARAM);
void Candle(HDC, DWORD, int, int);
HBITMAP CreateFlameBitmap(HDC, int);
void Firework(HDC);
void DisplayText(HDC, POINT, POINT, int);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpszCmdLine*/, int /*nCmdShow*/)
{
    HWND     hwnd;
    MSG      msg;
    WNDCLASS wndclass;

    hInst = hInstance;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.lpszMenuName = szAppName;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass))
        return FALSE;

    hwnd = CreateWindow(szAppName, "Birthday Cake",
        WS_TILEDWINDOW, 0, 0, 0, 0,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, SHOW_FULLSCREEN);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    static DWORD clrIcing = RGB(0xFF, 0xC0, 0xC0);
    static POINT pt1 = { 40, 125 };
    static POINT pt2 = { 600, 275 };
    static int ht = 50;
    static DWORD clrCandle[] = { RGB(0xFF, 0, 0),
                                 RGB(0, 0xFF, 0),
                                 RGB(0, 0, 0xFF) };
    static HBITMAP hbmFlame[3];
    static int iTextColor;

    HPEN        hpen;
    HDC hdc;
    HDC hdcMem;
    int         i;
    PAINTSTRUCT ps;

    switch (iMessage)
    {
    case WM_CREATE:
        hdc = GetDC(hwnd);
        for (i = 0; i < 3; i++)
            hbmFlame[i] = CreateFlameBitmap(hdc, i);
        ReleaseDC(hwnd, hdc);

        SetFocus(hwnd);
        break;

    case WM_COMMAND:
        switch (wParam)
        {
        case IDM_SETTINGS:
            if (DialogBox(hInst, "SettingsBox", /* MAKEINTRESOURCE(IDD_SETTINGS), */
                hwnd, SettingsProc))
            {
                if (bFlash)
                {
                    SetTimer(hwnd, 2, 250, nullptr);
                    SetTimer(hwnd, 3, 1, nullptr);
                }
                else
                {
                    KillTimer(hwnd, 2);
                    KillTimer(hwnd, 3);
                }

                InvalidateRect(hwnd, nullptr, TRUE);
            }
            break;

        case IDM_BURNING:
            SetTimer(hwnd, 1, 1, nullptr);
            break;
        }
        break;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        SelectObject(hdc, (HANDLE)CreateSolidBrush(clrIcing));

        Ellipse(hdc, pt1.x, pt2.y - ht / 2,
            pt2.x, pt2.y + ht / 2);

        hpen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));

        Rectangle(hdc, pt1.x, pt1.y,
            pt2.x, pt2.y);

        SelectObject(hdc, (HANDLE)hpen);

        Ellipse(hdc, pt1.x, pt1.y - ht / 2,
            pt2.x, pt1.y + ht / 2);

        DeleteObject(SelectObject(hdc, GetStockObject(BLACK_BRUSH)));

        SelectObject(hdc, GetStockObject(BLACK_PEN));

        MoveToEx(hdc, pt1.x, pt1.y, nullptr);
        LineTo(hdc, pt1.x, pt2.y);
        MoveToEx(hdc, pt2.x, pt1.y, nullptr);
        LineTo(hdc, pt2.x, pt2.y);

        DisplayText(hdc, pt1, pt2, ht);

        for (i = 0; i < cCandles; i++)
        {
            int x = ((cCandles - i) * pt1.x + (i + 1) * pt2.x) / (cCandles + 1);
            int y = pt1.y;

            if (i % 2 == 1)
                y -= 5;
            else
                y += 5;

            Candle(hdc, clrCandle[i % 3], x, y);
        }

        EndPaint(hwnd, &ps);
        break;

    case WM_TIMER:
        hdc = GetDC(hwnd);

        switch (wParam)
        {
        case 1:
            hdcMem = CreateCompatibleDC(hdc);

            for (i = 0; i < cCandles; i++)
            {
                int x = ((cCandles - i) * pt1.x + (i + 1) * pt2.x) / (cCandles + 1);
                int y = pt1.y;

                if (i % 2 == 1)
                    y -= 5;
                else
                    y += 5;

                SelectObject(hdcMem, (HANDLE)hbmFlame[rand() % 3]);
                BitBlt(hdc, x - 7, y - 115, 15, 25, hdcMem, 0, 0, SRCCOPY);
            }
            DeleteDC(hdcMem);

            break;

        case 2:
            SetTextColor(hdc, iTextColor % 4 ? RGB(0, 0, 0) : RGB(255, 0, 0));
            DisplayText(hdc, pt1, pt2, ht);
            iTextColor++;
            break;

        case 3:
            Firework(hdc);
            break;
        }
        ReleaseDC(hwnd, hdc);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, iMessage, wParam, lParam);
    }
    return 0L;
}

INT_PTR CALLBACK SettingsProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hdlg, ID_CAKETEXT, strCake);
        SetDlgItemInt(hdlg, ID_NUMCANDLES, cCandles, FALSE);
        SendDlgItemMessage(hdlg, ID_FLASH, BM_SETCHECK, (WPARAM)bFlash, 0L);
        return TRUE;

    case WM_COMMAND:
        switch (wParam)
        {
        case IDOK:
            GetDlgItemText(hdlg, ID_CAKETEXT, strCake, sizeof(strCake) - 1);
            cCandles = GetDlgItemInt(hdlg, ID_NUMCANDLES, nullptr, FALSE);
            bFlash = (BOOL)SendDlgItemMessage(hdlg, ID_FLASH, BM_GETCHECK, 0, 0L);
            EndDialog(hdlg, TRUE);
            return TRUE;

        case IDCANCEL:
            EndDialog(hdlg, FALSE);
            return TRUE;
        }
        break;

    }
    return FALSE;
}

void Candle(HDC hdc, DWORD clr, int x, int y)
{
    int cx = 10;
    int cy = 75;
    int h = 4;
    int wick = 15;
    HBRUSH hbr = CreateSolidBrush(clr);

    SelectObject(hdc, (HANDLE)hbr);
    Rectangle(hdc, x - cx / 2, y - cy, x + cx / 2, y);

    SelectObject(hdc, GetStockObject(WHITE_PEN));
    SelectObject(hdc, GetStockObject(WHITE_BRUSH));
    Rectangle(hdc, x - 2, y - cy, x + 2, y - cy - 16);

    SelectObject(hdc, GetStockObject(BLACK_PEN));
    DeleteObject((HANDLE)hbr);
}

HBITMAP CreateFlameBitmap(HDC hdc, int i)
{
    RECT rect;
    static const int pos[] = { 0, 7, 15 };
    static POINT apt[] = { 0, 0, 0, 15, 15, 15 };

    static const int pos2[] = { 3, 7, 11 };
    static POINT aptInner[] = { 7, 10,  3, 17,   7, 20,   11, 17 };
    HBITMAP hbm = CreateCompatibleBitmap(hdc, 15, 25);
    HDC hdcMem = CreateCompatibleDC(hdc);
    SelectObject(hdcMem, (HANDLE)hbm);

    SetRect(&rect, 0, 0, 15, 25);
    FillRect(hdcMem, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

    SelectObject(hdcMem, (HANDLE)CreateSolidBrush(RGB(0xFF, 0xFF, 0)));
    SelectObject(hdcMem, (HANDLE)GetStockObject(NULL_PEN));
    Ellipse(hdcMem, 0, 10, 15, 25);

    apt[0].x = pos[i];
    aptInner[0].x = pos2[i];

    Polygon(hdcMem, apt, 3);
    DeleteObject(SelectObject(hdcMem, (HANDLE)CreateSolidBrush(RGB(0xFF, 0, 0))));
    Polygon(hdcMem, aptInner, 4);
    DeleteObject(SelectObject(hdcMem, (HANDLE)GetStockObject(WHITE_BRUSH)));

    DeleteDC(hdcMem);
    return hbm;
}

void Firework(HDC hdc)
{
    static int xOrig;
    static int yOrig;
    static int iStep = 0;
    static int x[8];
    static int y[8];
    static const int xInc[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    static const int yInc[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
    static DWORD clr[8];
    static DWORD Black = RGB(0, 0, 0);
    static DWORD White = RGB(255, 255, 255);
    int i;

    if (iStep % 6 == 0)
    {
        xOrig = (rand() % 640);
        yOrig = (rand() % 125);
        iStep = 0;

        for (i = 0; i < 8; i++)
        {
            x[i] = xOrig + xInc[i];
            y[i] = yOrig + yInc[i];
        }
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            SetPixel(hdc, x[i], y[i], clr[i]);
            x[i] += xInc[i] * iStep;
            y[i] += yInc[i] * iStep;
        }
    }

    if (iStep % 6 != 5)
        for (i = 0; i < 8; i++)
        {
            clr[i] = GetPixel(hdc, x[i], y[i]);
            SetPixel(hdc, x[i], y[i], White);
        }
    iStep++;
}

void DisplayText(HDC hdc, POINT pt1, POINT pt2, int ht)
{
    RECT rect;
    HFONT hFont = CreateFont(67, 0, 0, 0, 700, 0, 0, 0, OEM_CHARSET,
        0, 0, 0, 0, "Script");

    SelectObject(hdc, (HANDLE)hFont);
    SetRect(&rect, pt1.x + 50, pt1.y + ht / 2, pt2.x - 50, pt2.y);

    SetBkMode(hdc, TRANSPARENT);
    DrawText(hdc, strCake, lstrlen(strCake), &rect,
        DT_CENTER | DT_VCENTER | DT_WORDBREAK);

    OffsetRect(&rect, 1, 1);

    DrawText(hdc, strCake, lstrlen(strCake), &rect,
        DT_CENTER | DT_VCENTER | DT_WORDBREAK);

    SetBkMode(hdc, OPAQUE);
    DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
}


