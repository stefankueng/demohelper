// demoHelper - screen drawing and presentation tool

// Copyright (C) 2007-2008, 2012-2013, 2015, 2020 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#include "stdafx.h"
#include "MainWindow.h"
#include "Registry.h"
#include "DPIAware.h"

#include <algorithm>

#define TRAY_WM_MESSAGE WM_APP + 1

bool CMainWindow::RegisterAndCreateWindow()
{
    WNDCLASSEX wcx;

    // Fill in the window class structure with default parameters
    wcx.cbSize      = sizeof(WNDCLASSEX);
    wcx.style       = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = CWindow::stWinMsgHandler;
    wcx.cbClsExtra  = 0;
    wcx.cbWndExtra  = 0;
    wcx.hInstance   = hResource;
    wcx.hCursor     = NULL;
    ResString clsname(hResource, IDS_APP_TITLE);
    wcx.lpszClassName = clsname;
    wcx.hIcon         = LoadIcon(hResource, MAKEINTRESOURCE(IDI_DEMOHELPER));
    wcx.hbrBackground = NULL;
    wcx.lpszMenuName  = NULL;
    wcx.hIconSm       = LoadIcon(wcx.hInstance, MAKEINTRESOURCE(IDI_DEMOHELPER));
    if (RegisterWindow(&wcx))
    {
        if (CreateEx(NULL, WS_POPUP, NULL))
        {
            // since our main window is hidden most of the time
            // we have to add an auxiliary window to the system tray

            const auto iconSizeX = CDPIAware::Instance().Scale(*this, GetSystemMetrics(SM_CXSMICON));
            const auto iconSizeY = CDPIAware::Instance().Scale(*this, GetSystemMetrics(SM_CYSMICON));
            SecureZeroMemory(&niData, sizeof(NOTIFYICONDATA));

            niData.uID    = IDI_DEMOHELPER;
            niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;

            niData.hIcon            = (HICON)LoadImage(hResource, MAKEINTRESOURCE(IDI_DEMOHELPER),
                                            IMAGE_ICON, iconSizeX, iconSizeY, LR_DEFAULTCOLOR);
            niData.hWnd             = *this;
            niData.uCallbackMessage = TRAY_WM_MESSAGE;

            Shell_NotifyIcon(NIM_ADD, &niData);
            DestroyIcon(niData.hIcon);

            return true;
        }
    }
    return false;
}

LRESULT CALLBACK CMainWindow::WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            m_hwnd = hwnd;
            RegisterHotKeys();
        }
        break;
        case WM_COMMAND:
            return DoCommand(LOWORD(wParam));
            break;
        case WM_HOTKEY:
        {
            WORD key = MAKEWORD(HIWORD(lParam), LOWORD(lParam));
            key      = HotKey2HotKeyControl(key);
            CRegStdDWORD regZoom(_T("Software\\DemoHelper\\zoomhotkey"), 0x331);
            CRegStdDWORD regDraw(_T("Software\\DemoHelper\\drawhotkey"), 0x332);
            if (key == (WORD)(DWORD)regZoom)
            {
                m_bZooming = true;
                StartZoomingMode();
            }
            else if (key == (WORD)(DWORD)regDraw)
            {
                StartPresentationMode();
            }
        }
        break;
        case WM_ERASEBKGND:
            return 1; // don't erase the background!
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC         hdc = BeginPaint(*this, &ps);
            {
                CMemDC memdc(hdc, ps.rcPaint);
                if (m_bZooming)
                {
                    // we're zooming,
                    // just stretch the part of the original window around the mouse pointer
                    POINT pt;
                    GetCursorPos(&pt);
                    DrawZoom(memdc, pt);
                }
                else
                {
                    BitBlt(memdc,
                           ps.rcPaint.left,
                           ps.rcPaint.top,
                           ps.rcPaint.right - ps.rcPaint.left,
                           ps.rcPaint.bottom - ps.rcPaint.top,
                           hDesktopCompatibleDC,
                           ps.rcPaint.left,
                           ps.rcPaint.top,
                           SRCCOPY);
                    Gdiplus::Graphics graphics(memdc);
                    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

                    for (const auto& line : m_drawLines)
                    {
                        Gdiplus::Color color;
                        color.SetValue(Gdiplus::Color::MakeARGB(line.alpha, GetRValue(m_colors[line.colorIndex]), GetGValue(m_colors[line.colorIndex]), GetBValue(m_colors[line.colorIndex])));
                        Gdiplus::Pen pen(color, (Gdiplus::REAL)line.penWidth);

                        if (line.lineType == LineType::hand)
                        {
                            graphics.DrawLines(&pen, line.points.data(), (int)line.points.size());
                        }
                        else
                        {
                            if (line.lineType == LineType::arrow)
                                pen.SetEndCap(Gdiplus::LineCap::LineCapArrowAnchor);
                            if ((line.lineStartPoint.X >= 0) && (line.lineStartPoint.Y >= 0) && (line.lineEndPoint.X >= 0) && (line.lineEndPoint.Y >= 0))
                            {
                                graphics.DrawLine(&pen, line.lineStartPoint, line.lineEndPoint);
                            }
                        }
                    }
                }
            }
            EndPaint(*this, &ps);
        }
        break;
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
        {
            if (m_bInlineZoom)
            {
                GetCursorPos(&m_ptInlineZoomStartPoint);
                GetCursorPos(&m_ptInlineZoomEndPoint);
            }
            if (m_bZooming)
            {
                m_bZooming = false;
                // now make the zoomed window the 'default'
                HDC   hdc = GetDC(*this);
                POINT pt;
                GetCursorPos(&pt);
                DrawZoom(hdc, pt);
                DeleteDC(hdc);
                RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
            }
            m_bDrawing = true;
            DrawLine drawLine;
            drawLine.lineStartPoint.X = GET_X_LPARAM(lParam);
            drawLine.lineStartPoint.Y = GET_Y_LPARAM(lParam);
            drawLine.alpha            = m_currentalpha;
            drawLine.points.push_back(drawLine.lineStartPoint);
            drawLine.colorIndex = m_colorindex;
            drawLine.penWidth   = m_currentpenwidth;
            m_drawLines.push_back(std::move(drawLine));
        }
        break;
        case WM_RBUTTONUP:
        case WM_LBUTTONUP:
            if (m_bInlineZoom)
            {
                m_bInlineZoom = false;
                StretchBlt(hDesktopCompatibleDC,
                           0, 0,
                           GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                           hDesktopCompatibleDC,
                           m_ptInlineZoomStartPoint.x, m_ptInlineZoomStartPoint.y,
                           abs(m_ptInlineZoomStartPoint.x - m_ptInlineZoomEndPoint.x), abs(m_ptInlineZoomStartPoint.y - m_ptInlineZoomEndPoint.y),
                           SRCCOPY);
                UpdateCursor();
                RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
            }
            else
            {
                m_bDrawing              = false;
                m_lineStartShiftPoint.x = -1;
                m_lineStartShiftPoint.y = -1;
            }
            break;
        case WM_MOUSEMOVE:
        {
            if (m_bInlineZoom)
            {
                if ((m_ptInlineZoomStartPoint.x >= 0) && (m_ptInlineZoomStartPoint.y >= 0) &&
                    (m_ptInlineZoomEndPoint.x >= 0) && (m_ptInlineZoomEndPoint.y >= 0))
                {
                    HDC hDC = GetDC(*this);
                    SetROP2(hDC, R2_NOT);
                    SelectObject(hDC, GetStockObject(NULL_BRUSH));
                    Rectangle(hDC, m_ptInlineZoomStartPoint.x, m_ptInlineZoomStartPoint.y, m_ptInlineZoomEndPoint.x, m_ptInlineZoomEndPoint.y);
                    GetCursorPos(&m_ptInlineZoomEndPoint);
                    Rectangle(hDC, m_ptInlineZoomStartPoint.x, m_ptInlineZoomStartPoint.y, m_ptInlineZoomEndPoint.x, m_ptInlineZoomEndPoint.y);
                    ReleaseDC(*this, hDC);
                }
            }
            else if (m_bZooming)
            {
                RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
            }
            else if (m_bDrawing)
            {
                int xPos = GET_X_LPARAM(lParam);
                int yPos = GET_Y_LPARAM(lParam);
                if (wParam & MK_LBUTTON)
                {
                    if (wParam & MK_SHIFT)
                    {
                        if (m_lineStartShiftPoint.x > 0 && m_lineStartShiftPoint.y > 0)
                        {
                            // if shift is pressed, the user want's to draw straight lines
                            if (abs(xPos - m_lineStartShiftPoint.x) > abs(yPos - m_lineStartShiftPoint.y))
                            {
                                // straight line horizontally
                                yPos = m_lineStartShiftPoint.y;
                            }
                            else
                            {
                                // straight line vertically
                                xPos = m_lineStartShiftPoint.x;
                            }
                        }
                        else
                        {
                            m_lineStartShiftPoint.x = xPos;
                            m_lineStartShiftPoint.y = yPos;
                        }
                    }
                    else
                    {
                        m_lineStartShiftPoint.x = -1;
                        m_lineStartShiftPoint.y = -1;
                    }
                    if (wParam & MK_CONTROL)
                    {
                        auto& line    = m_drawLines.back();
                        line.lineType = LineType::straight;

                        RECT invalidRect;
                        invalidRect.left   = std::min(line.lineStartPoint.X, xPos);
                        invalidRect.top    = std::min(line.lineStartPoint.Y, yPos);
                        invalidRect.right  = std::max(line.lineStartPoint.X, xPos);
                        invalidRect.bottom = std::max(line.lineStartPoint.Y, yPos);

                        invalidRect.left   = std::min(line.lineStartPoint.X, line.lineEndPoint.X);
                        invalidRect.top    = std::min(line.lineStartPoint.Y, line.lineEndPoint.Y);
                        invalidRect.right  = std::max(line.lineStartPoint.X, line.lineEndPoint.X);
                        invalidRect.bottom = std::max(line.lineStartPoint.Y, line.lineEndPoint.Y);

                        InflateRect(&invalidRect, 10 * m_currentpenwidth, 10 * m_currentpenwidth);
                        invalidRect.left = std::max(0L, invalidRect.left);
                        invalidRect.top  = std::max(0L, invalidRect.top);
                        InvalidateRect(*this, &invalidRect, FALSE);
                        line.lineEndPoint.X = xPos;
                        line.lineEndPoint.Y = yPos;
                    }
                    else
                    {
                        auto& line = m_drawLines.back();

                        RECT           invalidRect = {0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
                        Gdiplus::Point pt          = {xPos, yPos};
                        line.points.push_back(pt);
                        line.lineType = LineType::hand;
                        for (const auto& linept : line.points)
                        {
                            invalidRect.left   = std::min(linept.X, (int)invalidRect.left);
                            invalidRect.top    = std::min(linept.Y, (int)invalidRect.top);
                            invalidRect.right  = std::max(linept.X, (int)invalidRect.right);
                            invalidRect.bottom = std::max(linept.Y, (int)invalidRect.bottom);
                        }
                        InflateRect(&invalidRect, 2 * m_currentpenwidth, 2 * m_currentpenwidth);
                        invalidRect.left = std::max(0L, invalidRect.left);
                        invalidRect.top  = std::max(0L, invalidRect.top);
                        InvalidateRect(*this, &invalidRect, FALSE);
                    }
                }
                else if (wParam & MK_RBUTTON)
                {
                    auto& line = m_drawLines.back();
                    if (wParam & MK_SHIFT)
                    {
                        // if shift is pressed, the user want's to draw straight lines
                        if (abs(xPos - line.lineStartPoint.X) > abs(yPos - line.lineStartPoint.Y))
                        {
                            // straight line horizontally
                            yPos = line.lineStartPoint.Y;
                        }
                        else
                        {
                            // straight line vertically
                            xPos = line.lineStartPoint.X;
                        }
                    }
                    if (wParam & MK_CONTROL)
                    {
                        // control pressed means normal lines, not arrows
                        line.lineType = LineType::straight;
                    }
                    else
                    {
                        line.lineType = LineType::arrow;
                    }
                    RECT invalidRect;
                    invalidRect.left   = std::min(line.lineStartPoint.X, xPos);
                    invalidRect.top    = std::min(line.lineStartPoint.Y, yPos);
                    invalidRect.right  = std::max(line.lineStartPoint.X, xPos);
                    invalidRect.bottom = std::max(line.lineStartPoint.Y, yPos);

                    invalidRect.left   = std::min(line.lineStartPoint.X, line.lineEndPoint.X);
                    invalidRect.top    = std::min(line.lineStartPoint.Y, line.lineEndPoint.Y);
                    invalidRect.right  = std::max(line.lineStartPoint.X, line.lineEndPoint.X);
                    invalidRect.bottom = std::max(line.lineStartPoint.Y, line.lineEndPoint.Y);

                    InflateRect(&invalidRect, 10 * m_currentpenwidth, 10 * m_currentpenwidth);
                    invalidRect.left = std::max(0L, invalidRect.left);
                    invalidRect.top  = std::max(0L, invalidRect.top);
                    InvalidateRect(*this, &invalidRect, FALSE);
                    line.lineEndPoint.X = xPos;
                    line.lineEndPoint.Y = yPos;
                }
            }
        }
        break;
        case TRAY_WM_MESSAGE:
        {
            switch (lParam)
            {
                case WM_LBUTTONDBLCLK:
                    StartPresentationMode();
                    break;
                case WM_RBUTTONUP:
                case WM_CONTEXTMENU:
                {
                    POINT pt;
                    GetCursorPos(&pt);
                    HMENU hMenu    = LoadMenu(hResource, MAKEINTRESOURCE(IDC_DEMOHELPER));
                    HMENU hPopMenu = GetSubMenu(hMenu, 0);
                    SetForegroundWindow(*this);
                    TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, *this, NULL);
                    DestroyMenu(hMenu);
                }
                break;
            }
        }
        break;
        case WM_MOUSEWHEEL:
        {
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (m_bZooming)
            {
                if (zDelta < 0)
                {
                    m_zoomfactor += 0.2f;
                    if (m_zoomfactor > 4.0f)
                        m_zoomfactor = 4.0f;
                }
                else
                {
                    m_zoomfactor -= 0.2f;
                    if (m_zoomfactor < 1.0f)
                        m_zoomfactor = 1.0f;
                }
            }
            else
            {
                if (wParam & MK_CONTROL)
                {
                    if (zDelta < 0)
                        DoCommand(ID_CMD_DECREASE);
                    else
                        DoCommand(ID_CMD_INCREASE);
                }
                else
                {
                    if (zDelta < 0)
                        DoCommand(ID_CMD_PREVCOLOR);
                    else
                        DoCommand(ID_CMD_NEXTCOLOR);
                }
            }
            RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
        }
        break;
        case WM_TIMER:
            if (wParam == TIMER_ID_DRAW)
            {
                KillTimer(*this, TIMER_ID_DRAW);
                StartPresentationMode();
            }
            else if (wParam == TIMER_ID_ZOOM)
            {
                KillTimer(*this, TIMER_ID_ZOOM);
                StartZoomingMode();
            }
            else if (wParam == TIMER_ID_FADE)
            {
                // go through all lines and reduce the fade-count value
                auto dec = std::max(BYTE(LINE_ALPHA / (m_fadeseconds * 10)), (BYTE)1);
                for (auto& line : m_drawLines)
                {
                    if (line.alpha > dec)
                        line.alpha -= dec;
                    else
                        line.alpha = 0;
                }
                // go through all lines again, and remove all lines with a pen width
                // of zero
                for (auto it = m_drawLines.begin(); it != m_drawLines.end();)
                {
                    if (it->alpha == 0)
                    {
                        it = m_drawLines.erase(it);
                    }
                    else
                        ++it;
                }
                RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
            }
            break;
        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &niData);
            bWindowClosed = TRUE;
            PostQuitMessage(0);
            break;
        case WM_CLOSE:
            ::DestroyWindow(m_hwnd);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
};

bool CMainWindow::StartPresentationMode()
{
    int  nScreenWidth        = GetSystemMetrics(SM_CXSCREEN);
    int  nScreenHeight       = GetSystemMetrics(SM_CYSCREEN);
    HWND hDesktopWnd         = GetDesktopWindow();
    HDC  hDesktopDC          = GetDC(hDesktopWnd);
    hDesktopCompatibleDC     = CreateCompatibleDC(hDesktopDC);
    hDesktopCompatibleBitmap = CreateCompatibleBitmap(hDesktopDC, nScreenWidth, nScreenHeight);
    hOldBmp                  = (HBITMAP)SelectObject(hDesktopCompatibleDC, hDesktopCompatibleBitmap);
    BitBlt(hDesktopCompatibleDC, 0, 0, nScreenWidth, nScreenHeight, hDesktopDC, 0, 0, SRCCOPY | CAPTUREBLT);
    CRegStdDWORD regShowCursor(_T("Software\\DemoHelper\\capturecursor"), TRUE);
    if (DWORD(regShowCursor))
    {
        // capture the cursor
        CURSORINFO ci;
        ci.cbSize = sizeof(CURSORINFO);
        GetCursorInfo(&ci);
        if (ci.flags & CURSOR_SHOWING)
        {
            HICON    hIcon = CopyIcon(ci.hCursor);
            ICONINFO ii;
            GetIconInfo(hIcon, &ii);
            DrawIcon(hDesktopCompatibleDC, ci.ptScreenPos.x - ii.xHotspot, ci.ptScreenPos.y - ii.yHotspot, hIcon);
            DestroyIcon(hIcon);
        }
    }

    ReleaseDC(hDesktopWnd, hDesktopDC);

    SetWindowPos(*this, HWND_TOP /*MOST*/, 0, 0, nScreenWidth, nScreenHeight, SWP_SHOWWINDOW | SWP_DRAWFRAME);
    if (!m_bZooming)
    {
        if (m_hCursor)
            DestroyCursor(m_hCursor);
        m_hCursor         = CreateDrawCursor(m_colors[m_colorindex], std::max(2, m_currentpenwidth));
        m_hPreviousCursor = SetCursor(m_hCursor);
    }
    else
    {
        SetCursor(NULL);
    }
    m_bInlineZoom = false;

    CRegStdDWORD regFadeSeconds(_T("Software\\DemoHelper\\fadeseconds"), 0);
    m_fadeseconds = int(DWORD(regFadeSeconds));
    if (m_fadeseconds > 0)
    {
        ::SetTimer(*this, TIMER_ID_FADE, 100, NULL);
    }
    return true;
}

bool CMainWindow::EndPresentationMode()
{
    SetWindowPos(*this, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOSIZE);
    SelectObject(hDesktopCompatibleDC, hOldBmp);
    DeleteObject(hDesktopCompatibleBitmap);
    DeleteDC(hDesktopCompatibleDC);
    m_drawLines.clear();
    m_bDrawing = false;
    SetCursor(m_hPreviousCursor);
    if (m_hCursor)
    {
        DestroyCursor(m_hCursor);
        m_hCursor = NULL;
    }
    m_bInlineZoom = false;
    return true;
}

void CMainWindow::RegisterHotKeys()
{
    CRegStdDWORD regZoom(_T("Software\\DemoHelper\\zoomhotkey"), 0x331);
    CRegStdDWORD regDraw(_T("Software\\DemoHelper\\drawhotkey"), 0x332);
    WORD         zoom = (WORD)(DWORD)regZoom;
    WORD         draw = (WORD)(DWORD)regDraw;
    zoom              = HotKeyControl2HotKey(zoom);
    draw              = HotKeyControl2HotKey(draw);

    RegisterHotKey(*this, DRAW_HOTKEY, HIBYTE(draw), LOBYTE(draw));
    RegisterHotKey(*this, ZOOM_HOTKEY, HIBYTE(zoom), LOBYTE(zoom));
}

bool CMainWindow::StartZoomingMode()
{
    m_bZooming = true;
    StartPresentationMode();
    return true;
}

bool CMainWindow::EndZoomingMode()
{
    m_bZooming = false;
    EndPresentationMode();
    return true;
}

bool CMainWindow::DrawZoom(HDC hdc, POINT pt)
{
    // cursor pos is in screen coordinates - just what we need since our window covers the whole screen.
    // to zoom, we need to stretch the part around the cursor to the full screen
    // zoomfactor 1 = whole screen
    // zoomfactor 2 = quarter screen to fullscreen
    int cx          = GetSystemMetrics(SM_CXSCREEN);
    int cy          = GetSystemMetrics(SM_CYSCREEN);
    int zoomwindowx = int(float(cx) / m_zoomfactor);
    int zoomwindowy = int(float(cy) / m_zoomfactor);

    Gdiplus::Rect desktopRC = {0, 0, cx, cy};
    // adjust the cursor position to the zoom factor
    int x = pt.x * (cx - zoomwindowx) / cx;
    int y = pt.y * (cy - zoomwindowy) / cy;

    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (x + zoomwindowx > cx)
        x = cx - zoomwindowx;
    if (y + zoomwindowy > cy)
        y = cy - zoomwindowy;
    return !!StretchBlt(hdc, 0, 0, cx, cy, hDesktopCompatibleDC, x, y, zoomwindowx, zoomwindowy, SRCCOPY);
}

bool CMainWindow::UpdateCursor()
{
    if (m_bZooming)
    {
        SetCursor(NULL);
    }
    DestroyCursor(m_hCursor);
    m_hCursor = CreateDrawCursor(m_colors[m_colorindex], std::max(2, m_currentpenwidth));
    if (m_hCursor)
    {
        SetCursor(m_hCursor);
        return true;
    }
    return false;
}

bool CMainWindow::StartInlineZoom()
{
    m_bInlineZoom = true;
    HCURSOR hCur  = LoadCursor(NULL, IDC_CROSS);
    SetCursor(hCur);
    m_ptInlineZoomStartPoint.x = -1;
    m_ptInlineZoomStartPoint.y = -1;
    m_ptInlineZoomEndPoint.x   = -1;
    m_ptInlineZoomEndPoint.y   = -1;
    return true;
}