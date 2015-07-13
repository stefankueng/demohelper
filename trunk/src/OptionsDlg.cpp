// demoHelper - screen drawing and presentation tool

// Copyright (C) 2007-2008, 2012, 2015 - Stefan Kueng

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
#include "DemoHelper.h"
#include "MainWindow.h"
#include "Registry.h"

CHyperLink  CMainWindow::m_link;

BOOL CALLBACK CMainWindow::OptionsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            CRegStdDWORD regZoom(_T("Software\\DemoHelper\\zoomhotkey"), 0x331);
            CRegStdDWORD regDraw(_T("Software\\DemoHelper\\drawhotkey"), 0x332);
            CRegStdDWORD regCursor(_T("Software\\DemoHelper\\capturecursor"), TRUE);
            CRegStdDWORD regFadeSeconds(_T("Software\\DemoHelper\\fadeseconds"), 0);
            SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_ZOOMMODE), HKM_SETHOTKEY, (WPARAM)(DWORD)regZoom, 0);
            SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_DRAWMODE), HKM_SETHOTKEY, (WPARAM)(DWORD)regDraw, 0);
            SendMessage(GetDlgItem(hwndDlg, IDC_CURSORCHECK), BM_SETCHECK, DWORD(regCursor) ? BST_CHECKED : BST_UNCHECKED, 0);

            TCHAR buffer[128] = {0};
            LoadString(g_hInstance, IDS_WEBLINK, buffer, _countof(buffer));
            m_link.ConvertStaticToHyperlink(hwndDlg, IDC_WEBLINK, buffer);
            _stprintf_s(buffer, _countof(buffer), _T("%ld"), (DWORD)regFadeSeconds);
            SetWindowText(GetDlgItem(hwndDlg, IDC_FADESECONDS), buffer);

            // position the dialog box on the screen
            HWND hwndOwner;
            RECT rc, rcDlg, rcOwner;

            hwndOwner = GetDesktopWindow();

            GetWindowRect(hwndOwner, &rcOwner);
            GetWindowRect(hwndDlg, &rcDlg);
            CopyRect(&rc, &rcOwner);

            OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
            OffsetRect(&rc, -rc.left, -rc.top);
            OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

            SetWindowPos(hwndDlg, HWND_TOP, rcOwner.left + (rc.right / 2), rcOwner.top + (rc.bottom / 2), 0, 0, SWP_NOSIZE);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            {
                CRegStdDWORD regZoom(_T("Software\\DemoHelper\\zoomhotkey"), 0x331);
                CRegStdDWORD regDraw(_T("Software\\DemoHelper\\drawhotkey"), 0x332);
                CRegStdDWORD regCursor(_T("Software\\DemoHelper\\capturecursor"), TRUE);
                CRegStdDWORD regFadeSeconds(_T("Software\\DemoHelper\\fadeseconds"), 0);
                LRESULT res = SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_DRAWMODE), HKM_GETHOTKEY, 0, 0);
                regDraw = (DWORD)res;
                res = SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_ZOOMMODE), HKM_GETHOTKEY, 0, 0);
                regZoom = (DWORD)res;
                res = SendMessage(GetDlgItem(hwndDlg, IDC_CURSORCHECK), BM_GETCHECK, 0, 0);
                regCursor = (res == BST_CHECKED ? TRUE : FALSE);
                TCHAR buffer[128];
                GetWindowText(GetDlgItem(hwndDlg, IDC_FADESECONDS), buffer, _countof(buffer));
                regFadeSeconds = _ttol(buffer);
            }
            // Fall through.
        case IDCANCEL:
            EndDialog(hwndDlg, wParam);
            return TRUE;
        }
    }
    return FALSE;
}

WORD CMainWindow::HotKeyControl2HotKey(WORD hk)
{
    UINT flags = 0;
    if (HIBYTE(hk) & HOTKEYF_ALT)
        flags |= MOD_ALT;
    if (HIBYTE(hk) & HOTKEYF_SHIFT)
        flags |= MOD_SHIFT;
    if (HIBYTE(hk) & HOTKEYF_EXT)
        flags |= MOD_WIN;
    if (HIBYTE(hk) & HOTKEYF_CONTROL)
        flags |= MOD_CONTROL;
    return MAKEWORD(LOBYTE(hk), flags);
}

WORD CMainWindow::HotKey2HotKeyControl(WORD hk)
{
    UINT flags = 0;
    if (HIBYTE(hk) & MOD_ALT)
        flags |= HOTKEYF_ALT;
    if (HIBYTE(hk) & MOD_SHIFT)
        flags |= HOTKEYF_SHIFT;
    if (HIBYTE(hk) & MOD_WIN)
        flags |= HOTKEYF_EXT;
    if (HIBYTE(hk) & MOD_CONTROL)
        flags |= HOTKEYF_CONTROL;
    return MAKEWORD(LOBYTE(hk), flags);
}
