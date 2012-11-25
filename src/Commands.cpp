// demoHelper - screen drawing and presentation tool

// Copyright (C) 2007-2008 - Stefan Kueng

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


LRESULT CMainWindow::DoCommand(int id)
{
    switch (id)
    {
    case ID_CMD_TOGGLEROP:
        if (m_currentrop == R2_MASKPEN)
            m_currentrop = R2_COPYPEN;
        else
            m_currentrop = R2_MASKPEN;
        break;
    case ID_CMD_QUITMODE:
        m_bZooming = false;
        EndPresentationMode();
        UpdateCursor();
        break;
    case ID_CMD_UNDOLINE:
        m_bDrawing = false;
        m_lineindex[m_totallines] = 0;
        m_lineStartPoint[m_totallines].x = -1;
        m_lineStartPoint[m_totallines].y = -1;
        if (m_totallines > 0)
            m_totallines--;
        RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
        break;
    case ID_CMD_REMOVEFIRST:
        {
            m_bDrawing = false;
            for (int j=0; j<m_totallines; ++j)
            {
                m_lineindex[j] = m_lineindex[j+1];
                m_rop[j] = m_rop[j+1];
                m_fadecount[j] = m_fadecount[j+1];
                m_penwidth[j] = m_penwidth[j+1];
                m_linecolorindex[j] = m_linecolorindex[j+1];
                m_lineStartPoint[j] = m_lineStartPoint[j+1];
                m_lineEndPoint[j] = m_lineEndPoint[j+1];
                m_lineType[j] = m_lineType[j+1];
            }
            memmove_s(&m_points[0], (MAX_NUMBEROFLINES)*LINEARRAYSIZE*sizeof(POINT),
                &m_points[LINEARRAYSIZE], (MAX_NUMBEROFLINES-1)*LINEARRAYSIZE*sizeof(POINT));
            memmove_s(&m_linetypes[0], (MAX_NUMBEROFLINES)*LINEARRAYSIZE*sizeof(BYTE),
                &m_linetypes[LINEARRAYSIZE], (MAX_NUMBEROFLINES-1)*LINEARRAYSIZE*sizeof(BYTE));
            if (m_totallines > 0)
                m_totallines--;
            RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
        }
        break;
    case ID_CMD_INCREASE:
        if (m_bZooming)
        {
            m_zoomfactor += 0.2f;
            if (m_zoomfactor>4.0f)
                m_zoomfactor = 4.0f;
            RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
        }
        else if (m_totallines>=0)
        {
            // increase pen size
            if (m_currentpenwidth<32)
            {
                m_currentpenwidth++;
                RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
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
            RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
        }
        else if (m_totallines>=0)
        {
            // decrease pen size
            if (m_currentpenwidth>1)
            {
                m_currentpenwidth--;
                RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
            }
        }
        UpdateCursor();
        break;
    case ID_CMD_NEXTCOLOR:
        // cycle through colors
        if (m_totallines>=0)
        {
            if (m_colorindex<9)
                m_colorindex++;
            else
                m_colorindex = 0;
            RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
        }
        UpdateCursor();
        break;
    case ID_CMD_PREVCOLOR:
        // cycle through colors
        if (m_totallines>=0)
        {
            if (m_colorindex>0)
                m_colorindex--;
            else
                m_colorindex = 9;
            RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
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
        m_lineindex[0] = 0;
        m_lineStartPoint[0].x = -1;
        m_lineStartPoint[0].y = -1;
        m_totallines = -1;
        RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
        break;
    case ID_CMD_QUICKTOMARKER:
        // marker mode - quick way to select the biggest brush size and color yellow
        if (m_bMarker)
        {
            m_currentpenwidth = m_oldpenwidth;
            m_colorindex = m_oldcolorindex;
            m_currentrop = m_oldrop;
            m_bMarker = false;
        }
        else
        {
            m_oldpenwidth = m_currentpenwidth;
            m_oldcolorindex = m_colorindex;
            m_oldrop = m_currentrop;

            m_currentpenwidth = GetSystemMetrics(SM_CXCURSOR);
            m_colorindex = 0;
            m_currentrop = R2_MASKPEN;

            m_bMarker = true;
        }
        UpdateCursor();
        break;
    case ID_CMD_ACCEPT:
        if (m_bZooming)
        {
            m_bZooming = false;
            // now make the zoomed window the 'default'
            HDC hdc = GetDC(*this);
            POINT pt;
            GetCursorPos(&pt);
            DrawZoom(hdc, pt);
            BitBlt(hDesktopCompatibleDC,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),hdc,0,0,SRCCOPY);
            DeleteDC(hdc);
            RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
            UpdateCursor();
        }
        break;
    case IDM_EXIT:
        Shell_NotifyIcon(NIM_DELETE,&niData);
        ::PostQuitMessage(0);
        return 0;
        break;
    case ID_TRAYCONTEXT_OPTIONS:
        {
            // deregister our hotkeys
            UnregisterHotKey(*this, DRAW_HOTKEY);
            UnregisterHotKey(*this, ZOOM_HOTKEY);
            DialogBox(hResource, MAKEINTRESOURCE(IDD_OPTIONS), *this, (DLGPROC)OptionsDlgProc);
            // now register our hotkeys again
            RegisterHotKeys();
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
            rect.left = 0;
            rect.top = 0;
            rect.right = GetSystemMetrics(SM_CXSCREEN);
            rect.bottom = GetSystemMetrics(SM_CYSCREEN);
            SetBkColor(hDesktopCompatibleDC, ::GetSysColor(COLOR_WINDOW));
            ::ExtTextOut(hDesktopCompatibleDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
            // also clear all lines already drawn
            m_bDrawing = false;
            m_lineindex[0] = 0;
            m_lineStartPoint[0].x = -1;
            m_lineStartPoint[0].y = -1;
            m_totallines = -1;
            RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
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
