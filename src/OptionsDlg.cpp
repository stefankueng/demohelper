#include "stdafx.h"
#include "TrayWindow.h"
#include "registry.h"

BOOL CALLBACK CTrayWindow::OptionsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
	switch (message) 
	{
	case WM_INITDIALOG:
		{
			CRegStdWORD regZoom(_T("Software\\ShowHelper\\zoomhotkey"), 0);
			CRegStdWORD regDraw(_T("Software\\ShowHelper\\drawhotkey"), 0);
			CRegStdWORD regCursor(_T("Software\\ShowHelper\\capturecursor"), TRUE);
			SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_ZOOMMODE), HKM_SETHOTKEY, (WPARAM)(DWORD)regZoom, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_DRAWMODE), HKM_SETHOTKEY, (WPARAM)(DWORD)regDraw, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_CURSORCHECK), BM_SETCHECK, DWORD(regCursor) ? BST_CHECKED : BST_UNCHECKED, 0);

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

			SetWindowPos(hwndDlg, HWND_TOP, rcOwner.left + (rc.right / 2), rcOwner.top + (rc.bottom / 2), 0, 0,	SWP_NOSIZE); 
		}
		break;
	case WM_COMMAND: 
		switch (LOWORD(wParam)) 
		{
		case IDOK: 
			{
				CRegStdWORD regZoom(_T("Software\\ShowHelper\\zoomhotkey"), 0);
				CRegStdWORD regDraw(_T("Software\\ShowHelper\\drawhotkey"), 0);
				CRegStdWORD regCursor(_T("Software\\ShowHelper\\capturecursor"), TRUE);
				LRESULT res = SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_DRAWMODE), HKM_GETHOTKEY, 0, 0);
				regDraw = res;
				res = SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_ZOOMMODE), HKM_GETHOTKEY, 0, 0);
				regZoom = res;
				res = SendMessage(GetDlgItem(hwndDlg, IDC_CURSORCHECK), BM_GETCHECK, 0, 0);
				regCursor = (res == BST_CHECKED ? TRUE : FALSE);
			}
			// Fall through. 
		case IDCANCEL: 
			EndDialog(hwndDlg, wParam); 
			return TRUE; 
		} 
	} 
	return FALSE; 
}

WORD CTrayWindow::HotKeyControl2HotKey(WORD hk)
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

WORD CTrayWindow::HotKey2HotKeyControl(WORD hk)
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