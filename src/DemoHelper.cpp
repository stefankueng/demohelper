// demoHelper - screen drawing and presentation tool

// Copyright (C) 2007-2008, 2015, 2020 - Stefan Kueng

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
#include "PathUtils.h"
#include "SmartHandle.h"

// Global Variables:
HINSTANCE g_hInstance; // current instance
HINSTANCE g_hResource; // the resource dll

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    INITCOMMONCONTROLSEX used = {
        sizeof(INITCOMMONCONTROLSEX),
        ICC_STANDARD_CLASSES | ICC_BAR_CLASSES};
    InitCommonControlsEx(&used);
    SetDllDirectory(L"");
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr))
        return -1;
    OnOutOfScope(CoUninitialize());

    g_hResource = hInstance;
    g_hInstance = hInstance;

    auto iniFilePath = CPathUtils::GetModuleDir() + L"\\DemoHelper.ini";
    {
        CAutoFile hFile = CreateFile(iniFilePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, nullptr);
        if (!hFile)
        {
            auto appDataPath = CPathUtils::GetAppDataPath() + L"\\DemoHelper";
            CreateDirectory(appDataPath.c_str(), nullptr);
            MessageBox(nullptr, appDataPath.c_str(), L"DemoHelper", MB_ICONINFORMATION);
            iniFilePath = appDataPath + L"\\DemoHelper.ini";
        }
    }
    CIniSettings::Instance().SetIniPath(iniFilePath);
    OnOutOfScope(CIniSettings::Instance().Save());
    ULONG_PTR                    gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    OnOutOfScope(Gdiplus::GdiplusShutdown(gdiplusToken));
    MSG         msg;
    CMainWindow trayWindow(g_hResource);
    if (trayWindow.RegisterAndCreateWindow())
    {
        HACCEL hAccelTable = LoadAccelerators(g_hResource, MAKEINTRESOURCE(IDR_DEMOHELPER));
        // Main message loop:
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        return (int)msg.wParam;
    }
    return 1;
}
