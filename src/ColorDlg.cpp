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
#include "stdafx.h"
#include "ColorDlg.h"
#include "IniSettings.h"
#include "resource.h"
#include <Commdlg.h>

CColorDlg::CColorDlg(HWND hParent)
    : m_hParent(hParent)
{
}

CColorDlg::~CColorDlg()
{
}

LRESULT CColorDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            InitDialog(hwndDlg, IDI_DEMOHELPER);
            m_mvLColor.ConvertToColorButton(hwndDlg, IDC_MOUSEVISUAL_LEFT);
            m_mvMColor.ConvertToColorButton(hwndDlg, IDC_MOUSEVISUAL_MIDDLE);
            m_mvRColor.ConvertToColorButton(hwndDlg, IDC_MOUSEVISUAL_RIGHT);
            auto mvLClr = static_cast<COLORREF>(CIniSettings::Instance().GetInt64(L"Misc", L"mousevisualLcolor", RGB(255, 0, 0)));
            auto mvMClr = static_cast<COLORREF>(CIniSettings::Instance().GetInt64(L"Misc", L"mousevisualMcolor", RGB(0, 0, 255)));
            auto mvRClr = static_cast<COLORREF>(CIniSettings::Instance().GetInt64(L"Misc", L"mousevisualRcolor", RGB(0, 255, 0)));
            m_mvLColor.SetColor(mvLClr);
            m_mvMColor.SetColor(mvMClr);
            m_mvRColor.SetColor(mvRClr);
        }
            return TRUE;
        case WM_COMMAND:
            return DoCommand(LOWORD(wParam));
        default:
            return FALSE;
    }
}

LRESULT CColorDlg::DoCommand(int id)
{
    switch (id)
    {
        case IDOK:
            CIniSettings::Instance().SetInt64(L"Misc", L"mousevisualLcolor", m_mvLColor.GetColor());
            CIniSettings::Instance().SetInt64(L"Misc", L"mousevisualMcolor", m_mvMColor.GetColor());
            CIniSettings::Instance().SetInt64(L"Misc", L"mousevisualRcolor", m_mvRColor.GetColor());
            [[fallthrough]];
        case IDCANCEL:
            EndDialog(*this, id);
            break;
    }
    return 1;
}
