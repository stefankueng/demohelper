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
#include "RectSelectionWnd.h"
#include "DPIAware.h"
#include "MemDC.h"

#pragma comment(lib, "Winmm.lib")

constexpr int FADE_TIMER = 101;

CRectSelectionWnd::~CRectSelectionWnd()
{
    DiscardDeviceResources();
}

bool CRectSelectionWnd::RegisterAndCreateWindow()
{
    WNDCLASSEX wcx;

    // Fill in the window class structure with default parameters
    wcx.cbSize        = sizeof(WNDCLASSEX);
    wcx.style         = CS_HREDRAW | CS_VREDRAW | CS_CLASSDC;
    wcx.lpfnWndProc   = CWindow::stWinMsgHandler;
    wcx.cbClsExtra    = 0;
    wcx.cbWndExtra    = 0;
    wcx.hInstance     = hResource;
    wcx.hCursor       = LoadCursor(NULL, IDC_HAND);
    wcx.lpszClassName = L"CRectSelectionWnd_{3FD2D1AD-D9BD-4F3B-99F7-1103C8AAE535}";
    wcx.hIcon         = NULL;
    wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcx.lpszMenuName  = NULL;
    wcx.hIconSm       = NULL;
    if (RegisterWindow(&wcx))
    {
        auto ret = CreateEx(WS_EX_NOREDIRECTIONBITMAP | WS_EX_TOPMOST, WS_POPUP, NULL);
        return ret;
    }
    return false;
}

RECT CRectSelectionWnd::Show(HWND hWndParent, RECT wndRect, float aspectRatio)
{
    m_aspectRatio = aspectRatio;
    SetWindowPos(*this, HWND_TOP, wndRect.left, wndRect.top, wndRect.right - wndRect.left, wndRect.bottom - wndRect.top, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);
    // deactivate the parent window
    if (hWndParent)
        ::EnableWindow(hWndParent, FALSE);

    m_startPt = {};
    m_endPt   = {};
    // Main message loop:
    m_bRunning = true;
    MSG  msg   = {0};
    BOOL bRet  = TRUE;
    while (m_bRunning && ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0))
    {
        if (bRet == -1)
        {
            // handle the error and possibly exit
            break;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    // re-enable the parent window
    if (hWndParent)
        ::EnableWindow(hWndParent, TRUE);
    ShowWindow(*this, SW_HIDE);
    if (msg.message == WM_QUIT)
        PostQuitMessage((int)msg.wParam);
    return m_selectedRect;
}

LRESULT CRectSelectionWnd::WinMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
            m_hwnd = hwnd;
            break;
        case WM_LBUTTONDOWN:
            m_startPt.x           = GET_X_LPARAM(lParam);
            m_startPt.y           = GET_Y_LPARAM(lParam);
            m_selectionInProgress = true;
            break;
        case WM_LBUTTONUP:
            if (m_selectionInProgress)
            {
                m_endPt.x = GET_X_LPARAM(lParam);
                m_endPt.y = GET_Y_LPARAM(lParam);
                AdjustEndPoint();
                m_selectedRect.left   = std::min(m_startPt.x, m_endPt.x);
                m_selectedRect.right  = std::max(m_startPt.x, m_endPt.x);
                m_selectedRect.top    = std::min(m_startPt.y, m_endPt.y);
                m_selectedRect.bottom = std::max(m_startPt.y, m_endPt.y);
                m_bRunning            = false;
                m_selectionInProgress = false;
            }
            break;
        case WM_MOUSEMOVE:
            if (m_selectionInProgress)
            {
                m_endPt.x = GET_X_LPARAM(lParam);
                m_endPt.y = GET_Y_LPARAM(lParam);
                AdjustEndPoint();
                InvalidateRect(*this, nullptr, false);
            }
            break;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE)
            {
                if (m_startPt.x == m_endPt.x && m_startPt.y == m_endPt.y)
                {
                    m_selectedRect        = {};
                    m_bRunning            = false;
                    m_selectionInProgress = false;
                }
                else
                {
                    m_startPt             = {};
                    m_endPt               = {};
                    m_selectionInProgress = false;
                }
                InvalidateRect(*this, nullptr, false);
            }
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}
HRESULT CRectSelectionWnd::OnRender(ID2D1DeviceContext* dc)
{
    HRESULT hr      = S_OK;
    auto    size    = dc->GetSize();
    auto    wndRect = D2D1::RectF(0, 0, size.width, size.height);

    ComPtr<ID2D1SolidColorBrush> backgroundBrush;
    hr = m_dc->CreateSolidColorBrush(D2D1::ColorF(RGB(128, 128, 128), 0.5f), backgroundBrush.GetAddressOf());
    ComPtr<ID2D1SolidColorBrush> textBrush;
    hr = m_dc->CreateSolidColorBrush(D2D1::ColorF(RGB(120, 120, 255), 0.9f), textBrush.GetAddressOf());

    std::wstring text = L"select zoom area";
    if (m_TextFormat)
        dc->DrawText(text.c_str(), (UINT32)text.size(), m_TextFormat.Get(), wndRect, textBrush.Get());
    if (m_selectionInProgress)
    {
        auto selRect = D2D1::RectF((float)std::min(m_startPt.x, m_endPt.x),
                                   (float)std::min(m_startPt.y, m_endPt.y),
                                   (float)std::max(m_startPt.x, m_endPt.x),
                                   (float)std::max(m_startPt.y, m_endPt.y));
        // rect above the selRect
        auto topRect = D2D1::RectF(0, 0, wndRect.right, selRect.top);
        // rect below the selRect
        auto bottomRect = D2D1::RectF(0, selRect.bottom, wndRect.right, wndRect.bottom);
        // rect left of the selRect
        auto leftRect = D2D1::RectF(0, selRect.top, selRect.left, selRect.bottom);
        // rect right of the selRect
        auto rightRect = D2D1::RectF(selRect.right, selRect.top, wndRect.right, selRect.bottom);

        dc->FillRectangle(topRect, backgroundBrush.Get());
        dc->FillRectangle(bottomRect, backgroundBrush.Get());
        dc->FillRectangle(leftRect, backgroundBrush.Get());
        dc->FillRectangle(rightRect, backgroundBrush.Get());

        ComPtr<ID2D1SolidColorBrush> selBrush;
        hr = m_dc->CreateSolidColorBrush(D2D1::ColorF(RGB(128, 128, 128), 0.2f), selBrush.GetAddressOf());
        ComPtr<ID2D1SolidColorBrush> lineBrush;
        hr = m_dc->CreateSolidColorBrush(D2D1::ColorF(RGB(255, 0, 0), 0.7f), lineBrush.GetAddressOf());
        dc->FillRectangle(selRect, selBrush.Get());
        dc->DrawRectangle(selRect, lineBrush.Get(), 2.0f);
    }
    else
    {
        dc->FillRectangle(wndRect, backgroundBrush.Get());
    }
    return hr;
}

HRESULT CRectSelectionWnd::CreateDeviceResources()
{
    HRESULT hr = S_OK;
    if (SUCCEEDED(hr))
        hr = m_writeFactory->CreateTextFormat(L"Arial", nullptr,
                                              DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_CONDENSED,
                                              float(CDPIAware::Instance().Scale(*this, 36)),
                                              L"", m_TextFormat.GetAddressOf());
    if (SUCCEEDED(hr))
        hr = m_TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    if (SUCCEEDED(hr))
        hr = m_TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    if (SUCCEEDED(hr))
        hr = m_dc->CreateEffect(CLSID_D2D1GaussianBlur, &m_gaussianBlurEffect);

    return hr;
}
HRESULT CRectSelectionWnd::DiscardDeviceResources()
{
    m_TextFormat = nullptr;
    return S_OK;
}
void CRectSelectionWnd::AdjustEndPoint()
{
    // adjust the end point according to the aspect ratio
    // keep the smaller rect, not the bigger one
    auto width   = std::abs(m_startPt.x - m_endPt.x);
    auto height  = std::abs(m_startPt.y - m_endPt.y);
    auto xFactor = m_startPt.x > m_endPt.x ? -1.0f : 1.0f;
    auto yFactor = m_startPt.y > m_endPt.y ? -1.0f : 1.0f;
    if (width / m_aspectRatio > height)
    {
        m_endPt.x = m_startPt.x + LONG(xFactor * (height * m_aspectRatio));
    }
    else
    {
        m_endPt.y = m_startPt.y + LONG(yFactor * (width / m_aspectRatio));
    }
};
