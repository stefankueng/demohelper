// DemoHelper.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DemoHelper.h"
#include "MainWindow.h"

#pragma comment(lib, "comctl32")

#ifndef WIN64
#	pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#	pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"") 
#endif

#define MAX_LOADSTRING 100

// Global Variables:
// Global Variables:
HINSTANCE hInst;								// current instance
HINSTANCE hResource;							// the resource dll


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

	hResource = hInstance;
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	CMainWindow trayWindow(hResource);

	if (trayWindow.RegisterAndCreateWindow())
	{
		hAccelTable = LoadAccelerators(hResource, MAKEINTRESOURCE(IDR_DEMOHELPER));
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



