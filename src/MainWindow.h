// demoHelper - screen drawing and presentation tool

// Copyright (C) 2007-2008, 2012, 2020 - Stefan Kueng

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

#include "BaseWindow.h"
#include "resource.h"
#include "hyperlink.h"
#include "MemDC.h"
#include "AnimationManager.h"
#include "KeyboardOverlay.h"
#include "KeyboardOverlayD2D.h"
#include "MouseOverlay.h"
#include "MagnifierWindow.h"
#include "ColorButton.h"
#include <shellapi.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <vector>
#include <deque>

#define DRAW_HOTKEY 100
#define ZOOM_HOTKEY 101
#define LENS_HOTKEY 102

#define TIMER_ID_DRAW 101
#define TIMER_ID_ZOOM 102
#define TIMER_ID_FADE 103
#define TIMER_ID_LENS 104

#define LINE_ALPHA 100

enum class LineType
{
    hand,
    straight,
    arrow,
    rectangle,
    ellipse
};

class DrawLine
{
public:
    DrawLine()
    {
    }

    LineType                    lineType  = LineType::hand;
    int                         lineIndex = 0;
    std::vector<Gdiplus::Point> points;
    BYTE                        alpha = 100;

    Gdiplus::Point lineStartPoint = {-1, -1};
    Gdiplus::Point lineEndPoint   = {-1, -1};

    int penWidth   = 1;
    int colorIndex = 0;
};

class CMainWindow : public CWindow
{
public:
    CMainWindow(HINSTANCE hInst, const WNDCLASSEX* wcx = NULL)
        : CWindow(hInst, wcx)
        , m_bDrawing(false)
        , m_bZooming(false)
        , m_zoomfactor(1.2f)
        , hDesktopCompatibleDC(NULL)
        , hDesktopCompatibleBitmap(NULL)
        , hOldBmp(NULL)
        , m_colorindex(1)
        , m_currentpenwidth(6)
        , m_hCursor(NULL)
        , m_currentalpha(LINE_ALPHA)
        , m_bMarker(false)
        , m_bInlineZoom(false)
        , m_fadeseconds(0)
        , m_hPreviousCursor(nullptr)
        , m_lineStartShiftPoint({})
        , m_ptInlineZoomStartPoint({})
        , m_ptInlineZoomEndPoint({})
        , m_oldalpha(0)
        , m_oldcolorindex(0)
        , m_oldpenwidth(6)
        , niData({0})
        , m_rcScreen({0})
    {
        SetWindowTitle((LPCTSTR)ResString(hResource, IDS_APP_TITLE));
        m_colors[0]   = RGB(255, 255, 0);
        m_colors[1]   = RGB(255, 0, 0);
        m_colors[2]   = RGB(150, 0, 0);
        m_colors[3]   = RGB(0, 255, 0);
        m_colors[4]   = RGB(0, 150, 0);
        m_colors[5]   = RGB(0, 0, 255);
        m_colors[6]   = RGB(0, 0, 150);
        m_colors[7]   = RGB(0, 0, 0);
        m_colors[8]   = RGB(150, 150, 150);
        m_colors[9]   = RGB(0, 255, 255);
        m_AnimVarZoom = Animator::Instance().CreateAnimationVariable(1.2);
    };
    ~CMainWindow(void){};

    bool RegisterAndCreateWindow();

protected:
    /// the message handler for this window
    LRESULT CALLBACK WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// Handles all the WM_COMMAND window messages (e.g. menu commands)
    LRESULT DoCommand(int id);

    bool    StartPresentationMode();
    bool    EndPresentationMode();
    bool    StartInlineZoom();
    bool    StartZoomingMode();
    bool    EndZoomingMode();
    bool    DrawZoom(HDC hdc, POINT pt);
    HCURSOR CreateDrawCursor(COLORREF color, int penwidth);

    static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK ColorDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static WORD          HotKeyControl2HotKey(WORD hk);
    static WORD          HotKey2HotKeyControl(WORD hk);
    static BOOL CALLBACK HelpDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT       LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT       LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

    void        RegisterHotKeys();
    bool        UpdateCursor();
    static void ClearOutdatedPopupWindows();

protected:
    NOTIFYICONDATA niData;
    HDC            hDesktopCompatibleDC;
    HBITMAP        hDesktopCompatibleBitmap;
    HBITMAP        hOldBmp;

    bool  m_bDrawing;
    float m_zoomfactor;
    bool  m_bZooming;

    int  m_colorindex;
    int  m_currentpenwidth;
    BYTE m_currentalpha;
    int  m_fadeseconds;

    POINT m_lineStartShiftPoint;

    COLORREF m_colors[10];

    HCURSOR m_hCursor;
    HCURSOR m_hPreviousCursor;

    bool m_bMarker;
    int  m_oldpenwidth;
    int  m_oldcolorindex;
    BYTE m_oldalpha;

    bool  m_bInlineZoom;
    POINT m_ptInlineZoomStartPoint;
    POINT m_ptInlineZoomEndPoint;

    RECT                    m_rcScreen;
    IUIAnimationVariablePtr m_AnimVarZoom;
    std::deque<DrawLine>    m_drawLines;

    static HHOOK  m_hKeyboardHook;
    static HHOOK  m_hMouseHook;
    static DWORD  m_lastHookTime;
    static POINT  m_lastHookPoint;
    static WPARAM m_lastHookMsg;
    //static CKeyboardOverlayWnd       m_infoOverlay;
    static std::unique_ptr<CKeyboardOverlayWndD2D>             m_infoOverlay;
    static CMouseOverlayWnd                                    m_mouseOverlay;
    static CMagnifierWindow                                    m_magnifierWindow;
    static bool                                                m_bLensMode;
    static bool                                                m_bMouseVisuals;
    static bool                                                m_bMouseClicks;
    static COLORREF                                            m_mvLColor;
    static COLORREF                                            m_mvMColor;
    static COLORREF                                            m_mvRColor;
    static std::vector<std::wstring>                           m_keySequence;
    static std::deque<std::unique_ptr<CKeyboardOverlayWndD2D>> m_overlayWnds;
};
