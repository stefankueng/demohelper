// demoHelper - screen drawing and presentation tool

// Copyright (C) 2007-2008, 2020 - Stefan Kueng

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
#include "IniSettings.h"

extern HINSTANCE g_hInstance; // current instance
extern HINSTANCE g_hResource; // the resource dll

LRESULT CMainWindow::DoCommand(int id)
{
    switch (id)
    {
        case ID_CMD_TOGGLEROP:
            if (m_currentalpha == 255)
                m_currentalpha = LINE_ALPHA;
            else
                m_currentalpha = 255;
            break;
        case ID_CMD_QUITMODE:
            if (!m_bLensMode)
            {
                m_bZooming = false;
                EndPresentationMode();
                UpdateCursor();
            }
            break;
        case ID_CMD_UNDOLINE:
            m_bDrawing = false;
            if (!m_drawLines.empty())
                m_drawLines.pop_back();
            RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
            break;
        case ID_CMD_REMOVEFIRST:
        {
            m_bDrawing = false;
            if (!m_drawLines.empty())
                m_drawLines.pop_front();
            RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
        }
        break;
        case ID_CMD_INCREASE:
            if (m_bZooming)
            {
                m_zoomfactor += 0.2f;
                if (m_zoomfactor > 4.0f)
                    m_zoomfactor = 4.0f;
                RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
            }
            else if (!m_drawLines.empty())
            {
                // increase pen size
                if (m_currentpenwidth < 32)
                {
                    m_currentpenwidth++;
                    RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
                }
            }
            UpdateCursor();
            break;
        case ID_CMD_DECREASE:
            if (m_bZooming)
            {
                m_zoomfactor -= 0.2f;
                if (m_zoomfactor < 1.0f)
                    m_zoomfactor = 1.0f;
                RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
            }
            else if (!m_drawLines.empty())
            {
                // decrease pen size
                if (m_currentpenwidth > 1)
                {
                    m_currentpenwidth--;
                    RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
                }
            }
            UpdateCursor();
            break;
        case ID_CMD_NEXTCOLOR:
            // cycle through colors
            if (!m_drawLines.empty())
            {
                if (m_colorindex < 9)
                    m_colorindex++;
                else
                    m_colorindex = 0;
                RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
            }
            UpdateCursor();
            break;
        case ID_CMD_PREVCOLOR:
            // cycle through colors
            if (!m_drawLines.empty())
            {
                if (m_colorindex > 0)
                    m_colorindex--;
                else
                    m_colorindex = 9;
                RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
            }
            UpdateCursor();
            break;
        case ID_CMD_COLOR0:
            m_colorindex = 0;
            UpdateCursor();
            break;
        case ID_CMD_COLOR1:
            m_colorindex = 1;
            UpdateCursor();
            break;
        case ID_CMD_COLOR2:
            m_colorindex = 2;
            UpdateCursor();
            break;
        case ID_CMD_COLOR3:
            m_colorindex = 3;
            UpdateCursor();
            break;
        case ID_CMD_COLOR4:
            m_colorindex = 4;
            UpdateCursor();
            break;
        case ID_CMD_COLOR5:
            m_colorindex = 5;
            UpdateCursor();
            break;
        case ID_CMD_COLOR6:
            m_colorindex = 6;
            UpdateCursor();
            break;
        case ID_CMD_COLOR7:
            m_colorindex = 7;
            UpdateCursor();
            break;
        case ID_CMD_COLOR8:
            m_colorindex = 8;
            UpdateCursor();
            break;
        case ID_CMD_COLOR9:
            m_colorindex = 9;
            UpdateCursor();
            break;
        case ID_CMD_CLEARLINES:
            m_bDrawing = false;
            m_drawLines.clear();
            RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
            break;
        case ID_CMD_QUICKTOMARKER:
            // marker mode - quick way to select the biggest brush size and color yellow
            if (m_bMarker)
            {
                m_currentpenwidth = m_oldpenwidth;
                m_colorindex      = m_oldcolorindex;
                m_currentalpha    = m_oldalpha;
                m_bMarker         = false;
            }
            else
            {
                m_oldpenwidth   = m_currentpenwidth;
                m_oldcolorindex = m_colorindex;
                m_oldalpha      = m_currentalpha;

                m_currentpenwidth = GetSystemMetrics(SM_CXCURSOR);
                m_colorindex      = 0;
                m_currentalpha    = LINE_ALPHA;

                m_bMarker = true;
            }
            UpdateCursor();
            break;
        case ID_CMD_ACCEPT:
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
                UpdateCursor();
            }
            break;
        case IDM_EXIT:
            Shell_NotifyIcon(NIM_DELETE, &niData);
            if (m_hKeyboardHook)
                UnhookWindowsHookEx(m_hKeyboardHook);
            if (m_hMouseHook)
                UnhookWindowsHookEx(m_hMouseHook);
            m_hKeyboardHook = nullptr;
            m_hMouseHook = nullptr;
            ::PostQuitMessage(0);
            return 0;
            break;
        case ID_TRAYCONTEXT_OPTIONS:
        {
            // deregister our hotkeys
            UnregisterHotKey(*this, DRAW_HOTKEY);
            UnregisterHotKey(*this, ZOOM_HOTKEY);
            UnregisterHotKey(*this, LENS_HOTKEY);
            // remove hooks
            if (m_hKeyboardHook)
                UnhookWindowsHookEx(m_hKeyboardHook);
            if (m_hMouseHook)
                UnhookWindowsHookEx(m_hMouseHook);
            m_hKeyboardHook = nullptr;
            m_hMouseHook    = nullptr;
            DialogBox(hResource, MAKEINTRESOURCE(IDD_OPTIONS), *this, (DLGPROC)OptionsDlgProc);
            // now register our hotkeys again
            RegisterHotKeys();
            // and install the hooks if requested
            if (CIniSettings::Instance().GetInt64(L"Hooks", L"mouse", 1))
                m_hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, g_hInstance, 0);
            if (CIniSettings::Instance().GetInt64(L"Hooks", L"keyboard", 1))
                m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, g_hInstance, 0);
            m_bMouseVisuals = CIniSettings::Instance().GetInt64(L"Misc", L"mousevisual", 1) != 0;
            m_mvLColor = (COLORREF)CIniSettings::Instance().GetInt64(L"Misc", L"mousevisualLcolor", RGB(255, 0, 0));
            m_mvMColor = (COLORREF)CIniSettings::Instance().GetInt64(L"Misc", L"mousevisualMcolor", RGB(0, 0, 255));
            m_mvRColor = (COLORREF)CIniSettings::Instance().GetInt64(L"Misc", L"mousevisualRcolor", RGB(0, 255, 0));
        }
        break;
        case ID_TRAYCONTEXT_DRAW:
            SetTimer(*this, TIMER_ID_DRAW, 300, NULL);
            break;
        case ID_TRAYCONTEXT_ZOOM:
            SetTimer(*this, TIMER_ID_ZOOM, 300, NULL);
            break;
        case ID_CMD_INLINEZOOM:
            StartInlineZoom();
            break;
        case ID_CMD_CLEARSCREEN:
        {
            // clear the whole screen for drawing on it
            RECT rect;
            rect.left   = GetSystemMetrics(SM_XVIRTUALSCREEN);
            rect.top    = GetSystemMetrics(SM_YVIRTUALSCREEN);
            rect.right  = rect.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
            rect.bottom = rect.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
            SetBkColor(hDesktopCompatibleDC, ::GetSysColor(COLOR_WINDOW));
            ::ExtTextOut(hDesktopCompatibleDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
            // also clear all lines already drawn
            m_bDrawing = false;
            m_drawLines.clear();
            RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE);
        }
        break;
        case IDHELP:
            DialogBox(hResource, MAKEINTRESOURCE(IDD_HELPDIALOG), *this, (DLGPROC)HelpDlgProc);
            break;
        default:
            break;
    };
    return 1;
}
