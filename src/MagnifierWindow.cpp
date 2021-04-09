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
#include "stdafx.h"
#include "MagnifierWindow.h"

BOOL CMagnifierWindow::Create(HINSTANCE hInst, HWND hwndHost, BOOL visible)
{
    DWORD dwStyle = WS_CHILD | WS_EX_COMPOSITED | MS_SHOWMAGNIFIEDCURSOR | MS_CLIPAROUNDCURSOR;
    if (visible)
        dwStyle |= WS_VISIBLE;

    m_hWnd = CreateWindow(WC_MAGNIFIER, // Magnifier window class name defined in magnification.h
                          L"MagnifierWindow2",
                          dwStyle,
                          0, 0, 100, 100,
                          hwndHost, nullptr, hInst, nullptr);

    if (m_hWnd == nullptr)
        return FALSE;

    return TRUE;
}

BOOL CMagnifierWindow::SetMagnification(POINT mousePt, float magnification)
{
    // first adjust the mousePt to the current zoom rect
    RECT client = {0};
    GetWindowRect(*this, &client);
    auto clientWidth  = client.right - client.left;
    auto clientHeight = client.bottom - client.top;
    auto sourceWidth  = m_sourceRect.right - m_sourceRect.left;
    auto sourceHeight = m_sourceRect.bottom - m_sourceRect.top;

    mousePt.x = mousePt.x - client.left;
    mousePt.y = mousePt.y - client.top;

    mousePt.x = mousePt.x * sourceWidth / clientWidth + m_sourceRect.left;
    mousePt.y = mousePt.y * sourceHeight / clientHeight + m_sourceRect.top;

    if (magnification == 0.0f)
        magnification = m_magFactor;

    m_sourceRect.left   = static_cast<LONG>(mousePt.x - ((clientWidth / magnification) / 2.0f));
    m_sourceRect.right  = static_cast<LONG>(mousePt.x + ((clientWidth / magnification) / 2.0f));
    m_sourceRect.top    = static_cast<LONG>(mousePt.y - ((clientHeight / magnification) / 2.0f));
    m_sourceRect.bottom = static_cast<LONG>(mousePt.y + ((clientHeight / magnification) / 2.0f));

    if (m_sourceRect.left < client.left)
    {
        m_sourceRect.left  = client.left;
        m_sourceRect.right = static_cast<LONG>(m_sourceRect.left + (clientWidth / magnification));
    }
    if (m_sourceRect.top < client.top)
    {
        m_sourceRect.top    = client.top;
        m_sourceRect.bottom = static_cast<LONG>(m_sourceRect.top + (clientHeight / magnification));
    }

    if (m_sourceRect.right > client.right)
    {
        m_sourceRect.right = client.right;
        m_sourceRect.left  = static_cast<LONG>(client.right - (clientWidth / magnification));
    }
    if (m_sourceRect.bottom > client.bottom)
    {
        m_sourceRect.bottom = client.bottom;
        m_sourceRect.top    = static_cast<LONG>(client.bottom - (clientHeight / magnification));
    }

    MAGTRANSFORM matrix;
    memset(&matrix, 0, sizeof(matrix));
    matrix.v[0][0] = magnification;
    matrix.v[1][1] = magnification;
    matrix.v[2][2] = 1.0f;

    m_magFactor = magnification;

    MagShowSystemCursor(false);
    MagSetWindowTransform(m_hWnd, &matrix);
    return MagSetWindowSource(m_hWnd, m_sourceRect);
}

BOOL CMagnifierWindow::SetSourceRect(RECT rc)
{
    m_sourceRect = rc;

    RECT client = {0};
    GetClientRect(*this, &client);
    auto         magFactor = static_cast<float>(client.right - client.left) / static_cast<float>(m_sourceRect.right - m_sourceRect.left);
    MAGTRANSFORM matrix;
    memset(&matrix, 0, sizeof(matrix));
    matrix.v[0][0] = magFactor;
    matrix.v[1][1] = magFactor;
    matrix.v[2][2] = 1.0f;

    m_magFactor = magFactor;

    MagShowSystemCursor(false);
    MagSetWindowTransform(m_hWnd, &matrix);
    return MagSetWindowSource(m_hWnd, m_sourceRect);
}

BOOL CMagnifierWindow::UpdateMagnifier() const
{
    return MagSetWindowSource(m_hWnd, m_sourceRect);
}

BOOL CMagnifierWindow::Reset()
{
    MAGTRANSFORM matrix;
    memset(&matrix, 0, sizeof(matrix));
    matrix.v[0][0] = 1.0f;
    matrix.v[1][1] = 1.0f;
    matrix.v[2][2] = 1.0f;

    m_magFactor = 1.0f;

    MagShowSystemCursor(true);
    return MagSetWindowTransform(m_hWnd, &matrix);
}
