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
#pragma once
#include "BaseWindowD2D.h"
#include "DPIAware.h"
#include "AnimationManager.h"
#include <string>
#include <vector>

class CKeyboardOverlayWndD2D : public CWindowD2D
{
public:
    CKeyboardOverlayWndD2D(HINSTANCE hInst, const WNDCLASSEX* wcx = NULL)
        : CWindowD2D(hInst, wcx)
    {
        RegisterAndCreateWindow();
    }
    ~CKeyboardOverlayWndD2D(void);

    SIZE GetRequiredHeight(const std::wstring& text);
    void Show(const std::wstring& text);

private:
protected:
    /**
     * Registers the window class and creates the window.
     */
    bool RegisterAndCreateWindow();

private:
    std::wstring            m_text;
    IUIAnimationVariablePtr m_AnimVar;

    // Inherited via CWindowD2D
    virtual LRESULT WinMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    virtual HRESULT OnRender(ID2D1DeviceContext* dc) override;
    virtual HRESULT CreateDeviceResources() override;
    virtual HRESULT DiscardDeviceResources() override;

    ComPtr<IDWriteTextFormat> m_TextFormat;
    ComPtr<ID2D1Effect>       m_gaussianBlurEffect;
};
