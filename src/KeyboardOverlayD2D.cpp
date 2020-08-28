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
#include "KeyboardOverlayD2D.h"
#include "DPIAware.h"
#include "MemDC.h"

#pragma comment(lib, "Winmm.lib")

constexpr int FADE_TIMER = 101;

CKeyboardOverlayWndD2D::~CKeyboardOverlayWndD2D()
{
    DiscardDeviceResources();
}

bool CKeyboardOverlayWndD2D::RegisterAndCreateWindow()
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
    wcx.lpszClassName = L"CKeyboardOverlayWndD2D_{3c0b3390-375e-4553-a1ce-04dc03764e4b}";
    wcx.hIcon         = NULL;
    wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcx.lpszMenuName  = NULL;
    wcx.hIconSm       = NULL;
    if (RegisterWindow(&wcx))
    {
        auto ret = CreateEx(WS_EX_NOREDIRECTIONBITMAP | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOPMOST, WS_POPUP | WS_DISABLED, NULL);
        return ret;
    }
    return false;
}

SIZE CKeyboardOverlayWndD2D::GetRequiredHeight(const std::wstring& text)
{
    RECT rc;
    GetClientRect(*this, &rc);
    ComPtr<IDWriteTextLayout> textLayout;
    if (SUCCEEDED(m_writeFactory->CreateTextLayout(text.c_str(),
                                                   (UINT32)text.length(), m_TextFormat.Get(),
                                                   (float)rc.right, (float)rc.bottom,
                                                   textLayout.GetAddressOf())))
    {
        DWRITE_TEXT_METRICS textMetrics = {};
        if (SUCCEEDED(textLayout->GetMetrics(&textMetrics)))
            return SIZE{(int)textMetrics.width, (int)textMetrics.height};
    }
    return SIZE{};
}

void CKeyboardOverlayWndD2D::Show(const std::wstring& text)
{
    m_text = text;
    InvalidateRect(*this, nullptr, false);
    m_AnimVar       = Animator::Instance().CreateAnimationVariable(255.0);
    auto transKeep  = Animator::Instance().CreateConstantTransition(2.0);
    auto transFade  = Animator::Instance().CreateSmoothStopTransition(0.8, 0.0);
    auto storyBoard = Animator::Instance().CreateStoryBoard();
    storyBoard->AddTransition(m_AnimVar, transKeep);
    storyBoard->AddTransition(m_AnimVar, transFade);
    Animator::Instance().RunStoryBoard(storyBoard, [this]() {
        auto animVar = (BYTE)Animator::GetIntegerValue(m_AnimVar);
        InvalidateRect(*this, nullptr, false);
        if (animVar == 0)
            ShowWindow(*this, SW_HIDE);
    });
}

LRESULT CKeyboardOverlayWndD2D::WinMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
            m_hwnd = hwnd;
            break;
        case WM_LBUTTONUP:
            // user clicked on the popup window
            break;
        case WM_SETFOCUS:
        {
            if (wParam)
                SetFocus((HWND)wParam); // return the focus, we don't want it
        }
        break;
        case WM_NCHITTEST:
            return HTTRANSPARENT;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}
HRESULT CKeyboardOverlayWndD2D::OnRender(ID2D1DeviceContext* dc)
{
    HRESULT hr          = S_OK;
    auto    size        = dc->GetSize();
    auto    roundRadius = (float)CDPIAware::Instance().Scale(*this, 15);
    auto    textOffset  = (float)CDPIAware::Instance().Scale(*this, 3);
    auto    roundedRect = D2D1::RoundedRect(D2D1::RectF(0, 0, size.width, size.height), roundRadius, roundRadius);

    auto animVar = (BYTE)Animator::GetIntegerValue(m_AnimVar);

    ComPtr<ID2D1Bitmap1>    bmp;
    D2D1_SIZE_U             sizeU      = {(UINT32)size.width, (UINT32)size.height};
    D2D1_BITMAP_PROPERTIES1 properties = {};
    properties.pixelFormat.alphaMode   = D2D1_ALPHA_MODE_PREMULTIPLIED;
    properties.pixelFormat.format      = DXGI_FORMAT_B8G8R8A8_UNORM;
    properties.bitmapOptions           = D2D1_BITMAP_OPTIONS_TARGET;
    hr                                 = dc->CreateBitmap(sizeU, nullptr, 0, properties, bmp.GetAddressOf());

    m_dc->SetTarget(bmp.Get());

    auto                         textRect = D2D1::RectF(textOffset, textOffset, size.width - textOffset - textOffset, size.height - textOffset - textOffset);
    ComPtr<ID2D1SolidColorBrush> shadowBrush;
    hr = m_dc->CreateSolidColorBrush(D2D1::ColorF(RGB(10, 10, 10), animVar / 255.0f), shadowBrush.GetAddressOf());
    dc->DrawText(m_text.c_str(), (UINT32)m_text.size(), m_TextFormat.Get(), textRect, shadowBrush.Get());

    m_gaussianBlurEffect->SetInput(0, bmp.Get());
    m_gaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 5.0f);

    m_dc->SetTarget(m_d2dBitmap.Get());

    ComPtr<ID2D1SolidColorBrush> backgroundBrush;
    hr = m_dc->CreateSolidColorBrush(D2D1::ColorF(RGB(128, 128, 128), animVar / 255.0f / 1.4f), backgroundBrush.GetAddressOf());
    dc->FillRoundedRectangle(roundedRect, backgroundBrush.Get());
    //ComPtr<ID2D1SolidColorBrush> backgroundBorderBrush;
    //hr = m_dc->CreateSolidColorBrush(D2D1::ColorF(RGB(120, 10, 10), animVar / 255.0f), backgroundBorderBrush.GetAddressOf());
    //dc->DrawRoundedRectangle(roundedRect, backgroundBorderBrush.Get(), 5.0f);

    D2D1_POINT_2F offset = {7, 7};
    dc->DrawImage(m_gaussianBlurEffect.Get(), &offset);

    ComPtr<ID2D1SolidColorBrush> textBrush;
    hr = m_dc->CreateSolidColorBrush(D2D1::ColorF(RGB(120, 120, 255), animVar / 255.0f / 1.1f), textBrush.GetAddressOf());
    dc->DrawText(m_text.c_str(), (UINT32)m_text.size(), m_TextFormat.Get(), textRect, textBrush.Get());
    return hr;
}

HRESULT CKeyboardOverlayWndD2D::CreateDeviceResources()
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
        hr = m_TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    if (SUCCEEDED(hr))
        hr = m_dc->CreateEffect(CLSID_D2D1GaussianBlur, &m_gaussianBlurEffect);

    return hr;
}
HRESULT CKeyboardOverlayWndD2D::DiscardDeviceResources()
{
    m_TextFormat = nullptr;
    return S_OK;
};
