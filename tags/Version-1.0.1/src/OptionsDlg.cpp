#include "stdafx.h"
#include "DemoHelper.h"
#include "MainWindow.h"
#include "registry.h"

CHyperLink	CMainWindow::m_link;

BOOL CALLBACK CMainWindow::OptionsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
	switch (message) 
	{
	case WM_INITDIALOG:
		{
			CRegStdWORD regZoom(_T("Software\\DemoHelper\\zoomhotkey"), 0x331);
			CRegStdWORD regDraw(_T("Software\\DemoHelper\\drawhotkey"), 0x332);
			CRegStdWORD regCursor(_T("Software\\DemoHelper\\capturecursor"), TRUE);
			SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_ZOOMMODE), HKM_SETHOTKEY, (WPARAM)(DWORD)regZoom, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_DRAWMODE), HKM_SETHOTKEY, (WPARAM)(DWORD)regDraw, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_CURSORCHECK), BM_SETCHECK, DWORD(regCursor) ? BST_CHECKED : BST_UNCHECKED, 0);

			TCHAR buffer[128] = {0};
			LoadString(hInst, IDS_WEBLINK, buffer, 128);
			m_link.ConvertStaticToHyperlink(hwndDlg, IDC_WEBLINK, buffer);

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
				CRegStdWORD regZoom(_T("Software\\DemoHelper\\zoomhotkey"), 0x331);
				CRegStdWORD regDraw(_T("Software\\DemoHelper\\drawhotkey"), 0x332);
				CRegStdWORD regCursor(_T("Software\\DemoHelper\\capturecursor"), TRUE);
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