// demoHelper - screen drawing and presentation tool

// Copyright (C) 2007-2008, 2020-2021 - Stefan Kueng

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
#include "DPIAware.h"

HCURSOR CMainWindow::CreateDrawCursor(COLORREF color, int penwidth)
{
    // Get the system display DC
    HDC hDC = ::GetDC(*this);

    // Create helper DC
    HDC hMainDC    = ::CreateCompatibleDC(hDC);
    HDC hAndMaskDC = ::CreateCompatibleDC(hDC);
    HDC hXorMaskDC = ::CreateCompatibleDC(hDC);

    auto cursorSizeX = CDPIAware::Instance().Scale(*this, GetSystemMetrics(SM_CXCURSOR));
    auto cursorSizeY = CDPIAware::Instance().Scale(*this, GetSystemMetrics(SM_CYCURSOR));

    // Create the mask bitmaps
    auto hSourceBitmap  = ::CreateCompatibleBitmap(hDC, cursorSizeX, cursorSizeY); // original
    auto hAndMaskBitmap = ::CreateBitmap(cursorSizeX, cursorSizeY, 1, 1, nullptr); // monochrome
    auto hXorMaskBitmap = ::CreateCompatibleBitmap(hDC, cursorSizeX, cursorSizeY); // color

    // Release the system display DC
    ::ReleaseDC(*this, hDC);

    // Select the bitmaps to helper DC
    auto hOldMainBitmap    = static_cast<HBITMAP>(::SelectObject(hMainDC, hSourceBitmap));
    auto hOldAndMaskBitmap = static_cast<HBITMAP>(::SelectObject(hAndMaskDC, hAndMaskBitmap));
    auto hOldXorMaskBitmap = static_cast<HBITMAP>(::SelectObject(hXorMaskDC, hXorMaskBitmap));

    // fill our bitmap with the 'transparent' color RGB(1,1,1)
    RECT rc;
    rc.left   = 0;
    rc.top    = 0;
    rc.right  = cursorSizeX;
    rc.bottom = cursorSizeY;
    SetBkColor(hMainDC, RGB(1, 1, 1));
    ::ExtTextOut(hMainDC, 0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr);
    // set up the pen and brush to draw
    auto hPen      = CreatePen(PS_SOLID, 1, color);
    auto hOldPen   = static_cast<HPEN>(SelectObject(hMainDC, hPen));
    auto hBrush    = CreateSolidBrush(color);
    auto hOldBrush = static_cast<HBRUSH>(SelectObject(hMainDC, hBrush));

    // draw the real cursor
    HICON    hIcon = CopyIcon(LoadCursor(nullptr, IDC_ARROW));
    ICONINFO ii;
    GetIconInfo(hIcon, &ii);
    DrawIcon(hMainDC, cursorSizeX / 2, cursorSizeY / 2, hIcon);
    DestroyIcon(hIcon);

    // draw the brush circle
    Ellipse(hMainDC, (cursorSizeX - penwidth) / 2, (cursorSizeY - penwidth) / 2, (cursorSizeX - penwidth) / 2 + penwidth, (cursorSizeY - penwidth) / 2 + penwidth);

    SelectObject(hMainDC, hOldBrush);
    SelectObject(hMainDC, hOldPen);
    DeleteObject(hBrush);
    DeleteObject(hPen);

    // Assign the monochrome AND mask bitmap pixels so that a pixels of the source bitmap
    //    with 'clrTransparent' will be white pixels of the monochrome bitmap
    ::SetBkColor(hMainDC, RGB(1, 1, 1));
    ::BitBlt(hAndMaskDC, 0, 0, cursorSizeX, cursorSizeY, hMainDC, 0, 0, SRCCOPY);

    // Assign the color XOR mask bitmap pixels so that a pixels of the source bitmap
    //    with 'clrTransparent' will be black and rest the pixels same as corresponding
    //    pixels of the source bitmap
    ::SetBkColor(hXorMaskDC, RGB(0, 0, 0));
    ::SetTextColor(hXorMaskDC, RGB(255, 255, 255));
    ::BitBlt(hXorMaskDC, 0, 0, cursorSizeX, cursorSizeY, hAndMaskDC, 0, 0, SRCCOPY);
    ::BitBlt(hXorMaskDC, 0, 0, cursorSizeX, cursorSizeY, hMainDC, 0, 0, SRCAND);

    // Deselect bitmaps from the helper DC
    ::SelectObject(hMainDC, hOldMainBitmap);
    ::SelectObject(hAndMaskDC, hOldAndMaskBitmap);
    ::SelectObject(hXorMaskDC, hOldXorMaskBitmap);

    // Delete the helper DC
    ::DeleteDC(hXorMaskDC);
    ::DeleteDC(hAndMaskDC);
    ::DeleteDC(hMainDC);

    ICONINFO iconInfo = {0};
    iconInfo.fIcon    = FALSE;
    iconInfo.xHotspot = cursorSizeX / 2;
    iconInfo.yHotspot = cursorSizeY / 2;
    iconInfo.hbmMask  = hAndMaskBitmap;
    iconInfo.hbmColor = hXorMaskBitmap;

    return ::CreateIconIndirect(&iconInfo);
}
