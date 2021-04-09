﻿// demoHelper - screen drawing and presentation tool

// Copyright (C) 2020-2021 - Stefan Kueng

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
#include "MouseOverlay.h"
#include "DPIAware.h"
#include "MemDC.h"
#include <wrl.h>
using namespace Microsoft::WRL;
#pragma warning(push)
#pragma warning(disable : 4458) // declaration of 'xxx' hides class member
#include <gdiplus.h>
#pragma warning(pop)
#pragma comment(lib, "gdiplus.lib")

#pragma comment(lib, "Winmm.lib")

#define WIN_HALF_WIDTH  long(CDPIAware::Instance().Scale(*this, 30))
#define WIN_HALF_HEIGHT long(CDPIAware::Instance().Scale(*this, 30))

CMouseOverlayWnd::~CMouseOverlayWnd()
{
}

bool CMouseOverlayWnd::RegisterAndCreateWindow()
{
    WNDCLASSEX wcx;

    // Fill in the window class structure with default parameters
    wcx.cbSize        = sizeof(WNDCLASSEX);
    wcx.style         = CS_HREDRAW | CS_VREDRAW | CS_CLASSDC;
    wcx.lpfnWndProc   = CWindow::stWinMsgHandler;
    wcx.cbClsExtra    = 0;
    wcx.cbWndExtra    = 0;
    wcx.hInstance     = hResource;
    wcx.hCursor       = LoadCursor(nullptr, IDC_HAND);
    wcx.lpszClassName = L"CMouseOverlayWnd_{eee30d59-743f-42e5-8414-6ef9f311c49a}";
    wcx.hIcon         = nullptr;
    wcx.hbrBackground = reinterpret_cast<HBRUSH>((COLOR_WINDOW + 1));
    wcx.lpszMenuName  = nullptr;
    wcx.hIconSm       = nullptr;
    if (RegisterWindow(&wcx))
    {
        if (CreateEx(WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOPMOST, WS_POPUP | WS_DISABLED, nullptr))
        {
            // Make the window fully transparent.
            return SetLayeredWindowAttributes(*this, 0, 255, LWA_COLORKEY | LWA_ALPHA);
        }
    }
    return false;
}

void CMouseOverlayWnd::Show(POINT screenPos, COLORREF color, double fadeTo)
{
    m_color = color;
    SetWindowPos(*this, HWND_TOPMOST, screenPos.x - WIN_HALF_WIDTH, screenPos.y - WIN_HALF_HEIGHT, 2 * WIN_HALF_WIDTH, 2 * WIN_HALF_HEIGHT, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(*this, nullptr, false);
    m_animVar       = Animator::Instance().CreateAnimationVariable(255.0, fadeTo);
    auto transFade  = Animator::Instance().CreateSmoothStopTransition(m_animVar, 1.0 * ((255.0 - fadeTo) / 255.0), fadeTo);
    auto storyBoard = Animator::Instance().CreateStoryBoard();
    storyBoard->AddTransition(m_animVar.m_animVar, transFade);
    Animator::Instance().RunStoryBoard(storyBoard, [this]() {
        auto animVar = static_cast<BYTE>(Animator::GetIntegerValue(m_animVar));
        SetLayeredWindowAttributes(*this, 0, animVar / 2, LWA_COLORKEY | LWA_ALPHA);
        InvalidateRect(*this, nullptr, false);
        if (animVar == 0)
            ShowWindow(*this, SW_HIDE);
    });

    SetLayeredWindowAttributes(*this, 0, static_cast<BYTE>(128), LWA_COLORKEY | LWA_ALPHA);
}

void CMouseOverlayWnd::UpdatePos(POINT screenPos)
{
    if (m_animVar.m_animVar)
    {
        auto animVar = static_cast<BYTE>(Animator::GetIntegerValue(m_animVar));
        if (animVar)
            SetWindowPos(*this, HWND_TOPMOST, screenPos.x - WIN_HALF_WIDTH, screenPos.y - WIN_HALF_HEIGHT, 2 * WIN_HALF_WIDTH, 2 * WIN_HALF_HEIGHT, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
}

void CMouseOverlayWnd::Fade()
{
    if (m_animVar.m_animVar)
    {
        auto animVar = static_cast<BYTE>(Animator::GetIntegerValue(m_animVar));
        if (animVar)
        {
            auto transFade  = Animator::Instance().CreateSmoothStopTransition(m_animVar, 1.0, 0.0);
            auto storyBoard = Animator::Instance().CreateStoryBoard();
            storyBoard->AddTransition(m_animVar.m_animVar, transFade);
            Animator::Instance().RunStoryBoard(storyBoard, [this]() {
                auto animVar = static_cast<BYTE>(Animator::GetIntegerValue(m_animVar));
                SetLayeredWindowAttributes(*this, 0, animVar / 2, LWA_COLORKEY | LWA_ALPHA);
                InvalidateRect(*this, nullptr, false);
                if (animVar == 0)
                    ShowWindow(*this, SW_HIDE);
            });
        }
    }
}

LRESULT CALLBACK CMouseOverlayWnd::WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                {
                    CMemDC memDC(hdc);
                    OnPaint(memDC, &rect);
                }
                EndPaint(hwnd, &ps);
            }
        }
        break;
        case WM_SETFOCUS:
        {
            if (wParam)
                SetFocus(reinterpret_cast<HWND>(wParam)); // return the focus, we don't want it
        }
        break;
        case WM_DESTROY:
        {
            if (m_animVar.m_animVar && Animator::IsInstanceActive())
            {
                ComPtr<IUIAnimationStoryboard> storyBoard;
                if (SUCCEEDED(m_animVar.m_animVar->GetCurrentStoryboard(storyBoard.GetAddressOf())))
                {
                    if (storyBoard)
                    {
                        storyBoard->Abandon();
                    }
                }
            }
            m_animVar.m_animVar = nullptr;
        }
        break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
};

void CMouseOverlayWnd::OnPaint(HDC hDC, LPRECT pRect)
{
    auto              animVar = static_cast<BYTE>(Animator::GetIntegerValue(m_animVar));
    Gdiplus::Rect     rect    = {pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top};
    Gdiplus::Graphics graphics(hDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    Gdiplus::SolidBrush transparentBrush(Gdiplus::Color::MakeARGB(255, 0, 0, 0));
    graphics.FillRectangle(&transparentBrush, rect);

    auto          radius = animVar * (pRect->right - pRect->left) / 255 / 2;
    Gdiplus::Rect circle = {pRect->left + (pRect->right - pRect->left) / 2 - radius,
                            pRect->top + (pRect->bottom - pRect->top) / 2 - radius,
                            2 * radius,
                            2 * radius};

    Gdiplus::Color surroundColors[]   = {Gdiplus::Color::MakeARGB(static_cast<BYTE>(animVar), GetRValue(m_color), GetGValue(m_color), GetBValue(m_color))};
    INT            surroundColorCount = _countof(surroundColors);

    Gdiplus::GraphicsPath gp;
    gp.AddEllipse(circle);
    Gdiplus::PathGradientBrush pgb(&gp);
    pgb.SetCenterPoint(Gdiplus::Point(circle.Width / 2, circle.Height / 2));
    pgb.SetCenterColor(Gdiplus::Color::MakeARGB(static_cast<BYTE>(animVar), 255, 255, 255));
    pgb.SetSurroundColors(surroundColors, &surroundColorCount);

    graphics.FillPath(&pgb, &gp);
}
