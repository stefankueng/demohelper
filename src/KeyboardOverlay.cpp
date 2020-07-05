// demoHelper - screen drawing and presentation tool

// Copyright (C) 2020 - Stefan Kueng

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
#include "KeyboardOverlay.h"
#include "DPIAware.h"
#include "MemDC.h"
#pragma warning(push)
#pragma warning(disable : 4458) // declaration of 'xxx' hides class member
#include <gdiplus.h>
#pragma warning(pop)
#pragma comment(lib, "gdiplus.lib")

#pragma comment(lib, "Winmm.lib")

constexpr int FADE_TIMER = 101;

CKeyboardOverlayWnd::~CKeyboardOverlayWnd()
{
}

bool CKeyboardOverlayWnd::RegisterAndCreateWindow()
{
    WNDCLASSEX wcx;

    // Fill in the window class structure with default parameters
    wcx.cbSize        = sizeof(WNDCLASSEX);
    wcx.style         = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW | CS_CLASSDC;
    wcx.lpfnWndProc   = CWindow::stWinMsgHandler;
    wcx.cbClsExtra    = 0;
    wcx.cbWndExtra    = 0;
    wcx.hInstance     = hResource;
    wcx.hCursor       = LoadCursor(NULL, IDC_HAND);
    wcx.lpszClassName = _T("StatusBarMsgWnd_{BAB03407-CF65-4942-A1D5-063FA1CA8530}");
    wcx.hIcon         = NULL;
    wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcx.lpszMenuName  = NULL;
    wcx.hIconSm       = NULL;
    if (RegisterWindow(&wcx))
    {
        if (CreateEx(WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, WS_POPUP | WS_DISABLED, NULL))
        {
            // Make the window fully transparent.
            return SetLayeredWindowAttributes(*this, 0, 255, LWA_COLORKEY);
        }
    }
    return false;
}

void CKeyboardOverlayWnd::Show(const std::wstring& text)
{
    m_text = text;
    InvalidateRect(*this, nullptr, false);
    SetTimer(*this, FADE_TIMER, 100, nullptr);
    m_fadingCounter = 255;
    SetLayeredWindowAttributes(*this, 0, 255, LWA_COLORKEY);
}

LRESULT CALLBACK CKeyboardOverlayWnd::WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
            m_hwnd = hwnd;
            break;
        case WM_LBUTTONUP:
            // user clicked on the popup window
            break;
        case WM_ERASEBKGND:
            return TRUE;
        case WM_PAINT:
        {
            RECT rect;
            if (GetUpdateRect(*this, &rect, false))
            {
                ::GetClientRect(*this, &rect);
                PAINTSTRUCT ps;
                HDC         hdc = BeginPaint(hwnd, &ps);
                CMemDC      memdc(hdc);
                OnPaint(memdc, &rect);
                EndPaint(hwnd, &ps);
            }
        }
        break;
        case WM_TIMER:
        {
            m_fadingCounter -= 5;
            if (m_fadingCounter < 0)
                m_fadingCounter = 0;
            if (m_fadingCounter)
                InvalidateRect(*this, nullptr, false);
            else
                ShowWindow(*this, SW_HIDE);
        }
        break;
        case WM_SETFOCUS:
        {
            if (wParam)
                SetFocus((HWND)wParam); // return the focus, we don't want it
        }
        break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
};

void CKeyboardOverlayWnd::OnPaint(HDC hDC, LPRECT pRect)
{
    Gdiplus::Rect     rect  = {pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top};
    Gdiplus::RectF    rectF = {(Gdiplus::REAL)pRect->left,
                            (Gdiplus::REAL)pRect->top,
                            Gdiplus::REAL(pRect->right - pRect->left),
                            Gdiplus::REAL(pRect->bottom - pRect->top)};
    Gdiplus::Graphics graphics(hDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    Gdiplus::SolidBrush transparentBrush(Gdiplus::Color::MakeARGB(255, 0, 0, 0));
    Gdiplus::SolidBrush textBrush(Gdiplus::Color::MakeARGB((BYTE)m_fadingCounter, 255, 255, 255));
    Gdiplus::SolidBrush shadowBrush(Gdiplus::Color::MakeARGB((BYTE)m_fadingCounter, 1, 1, 1));
    graphics.FillRectangle(&transparentBrush, rect);

    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);

    Gdiplus::FontFamily   fontFamily(L"Arial");
    Gdiplus::GraphicsPath path;
    path.AddString(m_text.c_str(), (int)m_text.size(),
                   &fontFamily, Gdiplus::FontStyleRegular,
                   (Gdiplus::REAL)CDPIAware::Instance().PointsToPixels(*this, 20), rectF, &format);

    for (int i = 1; i < 8; ++i)
    {
        Gdiplus::Pen pen(Gdiplus::Color((BYTE)m_fadingCounter / 8, 192, 20, 20), (Gdiplus::REAL)i);
        pen.SetLineJoin(Gdiplus::LineJoinRound);
        graphics.DrawPath(&pen, &path);
    }

    graphics.FillPath(&textBrush, &path);
}
