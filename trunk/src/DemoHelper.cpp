// demoHelper - screen drawing and presentation tool

// Copyright (C) 2007-2008, 2015 - Stefan Kueng

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


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInstance;      // current instance
HINSTANCE g_hResource;      // the resource dll


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
        ICC_STANDARD_CLASSES | ICC_BAR_CLASSES
    };
    InitCommonControlsEx(&used);

    g_hResource = hInstance;
    g_hInstance = hInstance;
    // TODO: Place code here.
    MSG msg;

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
        return (int) msg.wParam;
    }
    return 1;
}
