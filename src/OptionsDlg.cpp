// demoHelper - screen drawing and presentation tool

// Copyright (C) 2007-2008, 2012, 2015, 2020-2021 - Stefan Kueng

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
#include "IniSettings.h"
#include "ColorDlg.h"

BOOL CALLBACK CMainWindow::OptionsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            auto zoom            = static_cast<WORD>(CIniSettings::Instance().GetInt64(L"HotKeys", L"zoom", 0x231));
            auto draw            = static_cast<WORD>(CIniSettings::Instance().GetInt64(L"HotKeys", L"draw", 0x232));
            auto lens            = static_cast<WORD>(CIniSettings::Instance().GetInt64(L"HotKeys", L"lens", 0x233));
            auto allmonitors     = CIniSettings::Instance().GetInt64(L"Misc", L"allmonitors", 0);
            auto fadeseconds     = CIniSettings::Instance().GetInt64(L"Draw", L"fadeseconds", 0);
            auto keyhook         = CIniSettings::Instance().GetInt64(L"Hooks", L"keyboard", 1);
            auto mousehook       = CIniSettings::Instance().GetInt64(L"Hooks", L"mouse", 1);
            auto mousevisual     = CIniSettings::Instance().GetInt64(L"Misc", L"mousevisual", 1);
            auto overlayPosition = static_cast<OverlayPosition>(CIniSettings::Instance().GetInt64(L"Misc", L"OvlPosition", static_cast<int64_t>(OverlayPosition::BottomRight)));
            SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_ZOOMMODE), HKM_SETHOTKEY, static_cast<WPARAM>(zoom), 0);
            SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_DRAWMODE), HKM_SETHOTKEY, static_cast<WPARAM>(draw), 0);
            SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_LENSMODE), HKM_SETHOTKEY, static_cast<WPARAM>(lens), 0);
            CheckRadioButton(hwndDlg, IDC_CURRENTMONITOR, IDC_ALLMONITORS, allmonitors ? IDC_ALLMONITORS : IDC_CURRENTMONITOR);
            CheckDlgButton(hwndDlg, IDC_KEYHOOK, keyhook ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_MOUSEHOOK, mousehook ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_MOUSEVISUALS, mousevisual ? BST_CHECKED : BST_UNCHECKED);
            CheckRadioButton(hwndDlg, IDC_POS_TOPLEFT, IDC_POS_BOTTOMRIGHT, static_cast<int>(overlayPosition) + IDC_POS_TOPLEFT);
            TCHAR buffer[128] = {0};
            LoadString(g_hInstance, IDS_WEBLINK, buffer, _countof(buffer));
            _stprintf_s(buffer, _countof(buffer), _T("%ld"), static_cast<DWORD>(fadeseconds));
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
                    LRESULT res = SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_DRAWMODE), HKM_GETHOTKEY, 0, 0);
                    CIniSettings::Instance().SetInt64(L"HotKeys", L"draw", res);
                    res = SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_ZOOMMODE), HKM_GETHOTKEY, 0, 0);
                    CIniSettings::Instance().SetInt64(L"HotKeys", L"zoom", res);
                    res = SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_LENSMODE), HKM_GETHOTKEY, 0, 0);
                    CIniSettings::Instance().SetInt64(L"HotKeys", L"lens", res);
                    TCHAR buffer[128];
                    GetWindowText(GetDlgItem(hwndDlg, IDC_FADESECONDS), buffer, _countof(buffer));
                    CIniSettings::Instance().SetString(L"Draw", L"fadeseconds", buffer);
                    CIniSettings::Instance().SetInt64(L"Misc", L"allmonitors", IsDlgButtonChecked(hwndDlg, IDC_ALLMONITORS) ? 1 : 0);
                    CIniSettings::Instance().SetInt64(L"Hooks", L"keyboard", IsDlgButtonChecked(hwndDlg, IDC_KEYHOOK) ? 1 : 0);
                    CIniSettings::Instance().SetInt64(L"Hooks", L"mouse", IsDlgButtonChecked(hwndDlg, IDC_MOUSEHOOK) ? 1 : 0);
                    CIniSettings::Instance().SetInt64(L"Misc", L"mousevisual", IsDlgButtonChecked(hwndDlg, IDC_MOUSEVISUALS) ? 1 : 0);
                    if (IsDlgButtonChecked(hwndDlg, IDC_POS_TOPLEFT))
                        CIniSettings::Instance().SetInt64(L"Misc", L"OvlPosition", static_cast<int64_t>(OverlayPosition::TopLeft));
                    if (IsDlgButtonChecked(hwndDlg, IDC_POS_TOPRIGHT))
                        CIniSettings::Instance().SetInt64(L"Misc", L"OvlPosition", static_cast<int64_t>(OverlayPosition::TopRight));
                    if (IsDlgButtonChecked(hwndDlg, IDC_POS_BOTTOMLEFT))
                        CIniSettings::Instance().SetInt64(L"Misc", L"OvlPosition", static_cast<int64_t>(OverlayPosition::BottomLeft));
                    if (IsDlgButtonChecked(hwndDlg, IDC_POS_BOTTOMRIGHT))
                        CIniSettings::Instance().SetInt64(L"Misc", L"OvlPosition", static_cast<int64_t>(OverlayPosition::BottomRight));
                }
                    [[fallthrough]];
                case IDCANCEL:
                    EndDialog(hwndDlg, wParam);
                    return TRUE;
                case IDC_CONF_COLORS:
                {
                    CColorDlg dlg(hwndDlg);
                    dlg.DoModal(g_hResource, IDD_COLORS, hwndDlg);
                }
                break;
            }
            break;
        case WM_NOTIFY:

            switch (reinterpret_cast<LPNMHDR>(lParam)->code)
            {
                case NM_CLICK: // Fall through to the next case.
                case NM_RETURN:
                {
                    PNMLINK pNMLink = reinterpret_cast<PNMLINK>(lParam);
                    LITEM   item    = pNMLink->item;

                    if ((reinterpret_cast<LPNMHDR>(lParam)->hwndFrom == GetDlgItem(hwndDlg, IDC_SYSLINK1)) && (item.iLink == 0))
                    {
                        ShellExecute(nullptr, L"open", item.szUrl, nullptr, nullptr, SW_SHOW);
                    }

                    break;
                }
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
