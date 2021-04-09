// demoHelper - screen drawing and presentation tool

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
#pragma once
#include "BaseWindowD2D.h"

class CRectSelectionWnd : public CWindowD2D
{
public:
    explicit CRectSelectionWnd(HINSTANCE hInst, const WNDCLASSEX* wcx = nullptr)
        : CWindowD2D(hInst, wcx)
        , m_bRunning(false)
        , m_selectedRect{}
        , m_startPt{}
        , m_endPt{}
        , m_selectionInProgress(false)
        , m_aspectRatio(1.0f)
    {
        RegisterAndCreateWindow();
    }
    ~CRectSelectionWnd();

    RECT Show(HWND hWndParent, RECT wndRect, float aspectRatio);

private:
protected:
    /**
     * Registers the window class and creates the window.
     */
    bool RegisterAndCreateWindow();

private:
    // Inherited via CWindowD2D
    LRESULT WinMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    HRESULT OnRender(ID2D1DeviceContext* dc) override;
    HRESULT CreateDeviceResources() override;
    HRESULT DiscardDeviceResources() override;
    void    AdjustEndPoint();

    ComPtr<IDWriteTextFormat> m_textFormat;
    ComPtr<ID2D1Effect>       m_gaussianBlurEffect;
    bool                      m_bRunning;
    RECT                      m_selectedRect;
    POINT                     m_startPt;
    POINT                     m_endPt;
    bool                      m_selectionInProgress;
    float                     m_aspectRatio;
};
