// demoHelper - screen drawing and presentation tool

// Copyright (C) 2021 - Stefan Kueng

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
#include "magnification.h"

#pragma comment(lib, "Magnification.lib")

class CMagnifierWindow
{
public:
    CMagnifierWindow()
        : m_hWnd(nullptr)
        , m_magFactor(1.0f)
        , m_sourceRect({})
    {
    }
    ~CMagnifierWindow() {}

    BOOL Create(HINSTANCE hInst, HWND hwndHost, BOOL visible);

    BOOL  SetMagnification(POINT mousePt, float magnification);
    BOOL  SetSourceRect(RECT rc);
    BOOL  UpdateMagnifier() const;
    float GetMagnification() const { return m_magFactor; }
    BOOL  Reset();

    operator HWND() { return m_hWnd; }
    operator HWND() const { return m_hWnd; }

private:
    HWND  m_hWnd;
    float m_magFactor;
    RECT  m_sourceRect;
};
