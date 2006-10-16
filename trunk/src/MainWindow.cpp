#include "StdAfx.h"
#include "MainWindow.h"
#include "registry.h"

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

#define TRAY_WM_MESSAGE		WM_APP+1

#pragma comment(lib, "Msimg32")


#define PACKVERSION(major,minor) MAKELONG(minor,major)
DWORD CMainWindow::GetDllVersion(LPCTSTR lpszDllName)
{
	HINSTANCE hinstDll;
	DWORD dwVersion = 0;

	hinstDll = LoadLibrary(lpszDllName);

	if (hinstDll)
	{
		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, 
			"DllGetVersion");

		if (pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			HRESULT hr;

			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);

			hr = (*pDllGetVersion)(&dvi);

			if (SUCCEEDED(hr))
			{
				dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
			}
		}

		FreeLibrary(hinstDll);
	}
	return dwVersion;
}

bool CMainWindow::RegisterAndCreateWindow()
{
	WNDCLASSEX wcx; 

	// Fill in the window class structure with default parameters 
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = CWindow::stWinMsgHandler;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hResource;
	wcx.hCursor = NULL;
	wcx.lpszClassName = ResString(hResource, IDS_APP_TITLE);
	wcx.hIcon = LoadIcon(hResource, MAKEINTRESOURCE(IDI_DEMOHELPER));
	wcx.hbrBackground = NULL;
	wcx.lpszMenuName = NULL;
	wcx.hIconSm	= LoadIcon(wcx.hInstance, MAKEINTRESOURCE(IDI_DEMOHELPER));
	if (RegisterWindow(&wcx))
	{
		if (CreateEx(NULL, WS_POPUP, NULL))
		{
			// since our main window is hidden most of the time
			// we have to add an auxiliary window to the system tray

			ZeroMemory(&niData,sizeof(NOTIFYICONDATA));

			ULONGLONG ullVersion = GetDllVersion(_T("Shell32.dll"));
			if (ullVersion >= MAKEDLLVERULL(6,0,0,0))
				niData.cbSize = sizeof(NOTIFYICONDATA);
			else if(ullVersion >= MAKEDLLVERULL(5,0,0,0))
				niData.cbSize = NOTIFYICONDATA_V2_SIZE;
			else niData.cbSize = NOTIFYICONDATA_V1_SIZE;

			niData.uID = IDI_DEMOHELPER;
			niData.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP|NIF_INFO;

			niData.hIcon = (HICON)LoadImage(hResource, MAKEINTRESOURCE(IDI_DEMOHELPER),
				IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
			niData.hWnd = *this;
			niData.uCallbackMessage = TRAY_WM_MESSAGE;

			Shell_NotifyIcon(NIM_ADD,&niData);
			DestroyIcon(niData.hIcon);

			return true;
		}
	}
	return false;
}


LRESULT CALLBACK CMainWindow::WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		{
			m_hwnd = hwnd;
			RegisterHotKeys();
		}
		break;
	case WM_COMMAND:
		return DoCommand(LOWORD(wParam));
		break;
	case WM_HOTKEY:
		{
			WORD key = MAKEWORD(HIWORD(lParam), LOWORD(lParam));
			key = HotKey2HotKeyControl(key);
			CRegStdWORD regZoom(_T("Software\\DemoHelper\\zoomhotkey"), 0x331);
			CRegStdWORD regDraw(_T("Software\\DemoHelper\\drawhotkey"), 0x332);
			if (key == (WORD)(DWORD)regZoom)
			{
				m_bZooming = true;
				StartZoomingMode();
			}
			else if (key == (WORD)(DWORD)regDraw)
			{
				StartPresentationMode();
			}
		}
		break;
	case WM_ERASEBKGND:
		return 1;		// don't erase the background!
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(*this, &ps);
			{
				CMemDC memdc(hdc, ps.rcPaint);
				SetROP2(memdc, R2_MASKPEN);
				if (m_bZooming)
				{
					// we're zooming,
					// just stretch the part of the original window around the mouse pointer
					POINT pt;
					GetCursorPos(&pt);
					DrawZoom(memdc, pt);
				}
				else
				{
					BitBlt(memdc,
						ps.rcPaint.left,
						ps.rcPaint.top,
						ps.rcPaint.right-ps.rcPaint.left,
						ps.rcPaint.bottom-ps.rcPaint.top,
						hDesktopCompatibleDC,
						ps.rcPaint.left,
						ps.rcPaint.top,
						SRCCOPY);
					if (m_totallines >= 0)
					{
						for (int i=0; i<=m_totallines; ++i)
						{
							HPEN hPen = CreatePen(PS_SOLID, m_penwidth[i], m_colors[m_linecolorindex[i]]);
							HPEN hOldPen = NULL;
							HBRUSH hOldBrush = NULL;
							if (hPen)
							{
								hOldPen = (HPEN)SelectObject(memdc, hPen);
							}
							HBRUSH hBrush = CreateSolidBrush(m_colors[m_linecolorindex[i]]);
							if (hBrush)
							{
								hOldBrush = (HBRUSH)SelectObject(memdc, hBrush);
							}

							if (m_lineindex[i])
							{
								SetROP2(memdc, m_rop[i]);
								PolyDraw(memdc, (const POINT*)&m_points[i*LINEARRAYSIZE], (const BYTE*)&m_linetypes[i*LINEARRAYSIZE], m_lineindex[i]);
							}
							else if ((m_lineStartPoint[i].x>=0) && (m_lineStartPoint[i].y>=0) && (m_lineEndPoint[i].x>=0) && (m_lineEndPoint[i].y>=0))
							{
								SetROP2(memdc, m_rop[i]);
								if (m_lineType[i] == arrow)
									DrawArrow(memdc, i);
								else
								{
									MoveToEx(memdc, m_lineStartPoint[i].x, m_lineStartPoint[i].y, NULL);
									LineTo(memdc, m_lineEndPoint[i].x, m_lineEndPoint[i].y);
								}
							}

							if (hOldPen)
								SelectObject(memdc, hOldPen);
							DeleteObject(hPen);
							if (hOldBrush)
								SelectObject(memdc, hOldBrush);
							DeleteObject(hBrush);
						}
					}
				}
			}
			EndPaint(*this, &ps);
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		{
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
			}
			if (m_totallines < (MAX_NUMBEROFLINES-2))
			{
				m_bDrawing = true;
				m_totallines++;
				m_rop[m_totallines] = m_currentrop;
				int xPos = GET_X_LPARAM(lParam); 
				int yPos = GET_Y_LPARAM(lParam); 
				m_lineStartPoint[m_totallines].x = xPos;
				m_lineStartPoint[m_totallines].y = yPos;
				m_lineEndPoint[m_totallines].x = -1;
				m_lineEndPoint[m_totallines].y = -1;
				m_lineStartShiftPoint.x = -1;
				m_lineStartShiftPoint.y = -1;
				m_lineindex[m_totallines] = 0;
				m_points[m_totallines*LINEARRAYSIZE + m_lineindex[m_totallines]] = m_lineStartPoint[m_totallines];
				m_linetypes[m_totallines*LINEARRAYSIZE + m_lineindex[m_totallines]] = PT_MOVETO;
				m_linecolorindex[m_totallines] = m_colorindex;
				m_penwidth[m_totallines] = m_currentpenwidth;
			}
		}
		break;
	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
		m_bDrawing = false;
		m_lineStartShiftPoint.x = -1;
		m_lineStartShiftPoint.y = -1;
		break;
	case WM_MOUSEMOVE:
		{
			if (m_bZooming)
			{
				RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
			}
			else if (m_bDrawing)
			{
				int xPos = GET_X_LPARAM(lParam); 
				int yPos = GET_Y_LPARAM(lParam);
				if (wParam & MK_LBUTTON)
				{
					if (wParam & MK_SHIFT)
					{
						if (m_lineStartShiftPoint.x > 0 && m_lineStartShiftPoint.y > 0)
						{
							// if shift is pressed, the user want's to draw straight lines
							if (abs(xPos-m_lineStartShiftPoint.x) > abs(yPos-m_lineStartShiftPoint.y))
							{
								// straight line horizontally
								yPos = m_lineStartShiftPoint.y;
							}
							else
							{
								// straight line vertically
								xPos = m_lineStartShiftPoint.x;
							}
						}
						else
						{
							m_lineStartShiftPoint.x = xPos;
							m_lineStartShiftPoint.y = yPos;
						}
					}
					else
					{
						m_lineStartShiftPoint.x = -1;
						m_lineStartShiftPoint.y = -1;
					}
					if (wParam & MK_CONTROL)
					{
						m_lineType[m_totallines] = normal;
						RECT invalidRect;
						invalidRect.left = min(m_lineStartPoint[m_totallines].x, xPos);
						invalidRect.top = min(m_lineStartPoint[m_totallines].y, yPos);
						invalidRect.right = max(m_lineStartPoint[m_totallines].x, xPos);
						invalidRect.bottom = max(m_lineStartPoint[m_totallines].y, yPos);

						invalidRect.left = min(m_lineStartPoint[m_totallines].x, m_lineEndPoint[m_totallines].x);
						invalidRect.top = min(m_lineStartPoint[m_totallines].y, m_lineEndPoint[m_totallines].y);
						invalidRect.right = max(m_lineStartPoint[m_totallines].x, m_lineEndPoint[m_totallines].x);
						invalidRect.bottom = max(m_lineStartPoint[m_totallines].y, m_lineEndPoint[m_totallines].y);

						InflateRect(&invalidRect, 10*m_currentpenwidth, 10*m_currentpenwidth);
						invalidRect.left = max(0, invalidRect.left);
						invalidRect.top = max(0, invalidRect.top);
						InvalidateRect(*this, &invalidRect, FALSE);
						m_lineEndPoint[m_totallines].x = xPos;
						m_lineEndPoint[m_totallines].y = yPos;
					}
					else
					{
						if (m_lineindex[m_totallines] < (LINEARRAYSIZE-2))
						{
							RECT invalidRect = {0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN)};
							m_lineindex[m_totallines]++;
							POINT pt = {xPos, yPos};
							m_points[m_totallines*LINEARRAYSIZE + m_lineindex[m_totallines]] = pt;
							m_linetypes[m_totallines*LINEARRAYSIZE + m_lineindex[m_totallines]] = PT_LINETO;
							for (int i=0; i<m_lineindex[m_totallines]; ++i)
							{
								invalidRect.left = min(m_points[m_totallines*LINEARRAYSIZE + i].x, invalidRect.left);
								invalidRect.top = min(m_points[m_totallines*LINEARRAYSIZE + i].y, invalidRect.top);
								invalidRect.right = max(m_points[m_totallines*LINEARRAYSIZE + i].x, invalidRect.right);
								invalidRect.bottom = max(m_points[m_totallines*LINEARRAYSIZE + i].y, invalidRect.bottom);
							}
							InflateRect(&invalidRect, 2*m_currentpenwidth, 2*m_currentpenwidth);
							invalidRect.left = max(0, invalidRect.left);
							invalidRect.top = max(0, invalidRect.top);
							InvalidateRect(*this, &invalidRect, FALSE);
						}
					}
				}
				else if (wParam & MK_RBUTTON)
				{
					if (wParam & MK_SHIFT)
					{
						// if shift is pressed, the user want's to draw straight lines
						if (abs(xPos-m_lineStartPoint[m_totallines].x) > abs(yPos-m_lineStartPoint[m_totallines].y))
						{
							// straight line horizontally
							yPos = m_lineStartPoint[m_totallines].y;
						}
						else
						{
							// straight line vertically
							xPos = m_lineStartPoint[m_totallines].x;
						}
					}
					if (wParam & MK_CONTROL)
					{
						// control pressed means normal lines, not arrows
						m_lineType[m_totallines] = normal;
					}
					else
					{
						m_lineType[m_totallines] = arrow;
					}
					RECT invalidRect;
					invalidRect.left = min(m_lineStartPoint[m_totallines].x, xPos);
					invalidRect.top = min(m_lineStartPoint[m_totallines].y, yPos);
					invalidRect.right = max(m_lineStartPoint[m_totallines].x, xPos);
					invalidRect.bottom = max(m_lineStartPoint[m_totallines].y, yPos);

					invalidRect.left = min(m_lineStartPoint[m_totallines].x, m_lineEndPoint[m_totallines].x);
					invalidRect.top = min(m_lineStartPoint[m_totallines].y, m_lineEndPoint[m_totallines].y);
					invalidRect.right = max(m_lineStartPoint[m_totallines].x, m_lineEndPoint[m_totallines].x);
					invalidRect.bottom = max(m_lineStartPoint[m_totallines].y, m_lineEndPoint[m_totallines].y);

					InflateRect(&invalidRect, 10*m_currentpenwidth, 10*m_currentpenwidth);
					invalidRect.left = max(0, invalidRect.left);
					invalidRect.top = max(0, invalidRect.top);
					InvalidateRect(*this, &invalidRect, FALSE);
					m_lineEndPoint[m_totallines].x = xPos;
					m_lineEndPoint[m_totallines].y = yPos;
				}
			}
		}
		break;
	case TRAY_WM_MESSAGE:
		{
			switch(lParam)
			{
			case WM_LBUTTONDBLCLK:
				StartPresentationMode();
				break;
			case WM_RBUTTONUP:
			case WM_CONTEXTMENU:
				{
					POINT pt;
					GetCursorPos(&pt);
					HMENU hMenu = LoadMenu(hResource, MAKEINTRESOURCE(IDC_DEMOHELPER));
					HMENU hPopMenu = GetSubMenu(hMenu, 0);
					SetForegroundWindow(*this);
					TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, *this, NULL); 
					DestroyMenu(hMenu);
				}
				break;
			}
		}
		break;
	case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (m_bZooming)
			{
				if (zDelta < 0)
				{
					m_zoomfactor += 0.2f;
					if (m_zoomfactor>4.0f)
						m_zoomfactor = 4.0f;
				}
				else
				{
					m_zoomfactor -= 0.2f;
					if (m_zoomfactor < 1.0f)
						m_zoomfactor = 1.0f;
				}
			}
			else
			{
				if (wParam & MK_CONTROL)
				{
					if (zDelta < 0)
						DoCommand(ID_CMD_DECREASE);
					else
						DoCommand(ID_CMD_INCREASE);
				}
				else
				{
					if (zDelta < 0)
						DoCommand(ID_CMD_PREVCOLOR);
					else
						DoCommand(ID_CMD_NEXTCOLOR);
				}
			}
			RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
		}
		break;
	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE,&niData);
		bWindowClosed = TRUE;
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		::DestroyWindow(m_hwnd);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
};

bool CMainWindow::StartPresentationMode()
{
	int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	HWND hDesktopWnd = GetDesktopWindow();
	HDC hDesktopDC = GetDC(hDesktopWnd);
	hDesktopCompatibleDC = CreateCompatibleDC(hDesktopDC);
	hDesktopCompatibleBitmap = CreateCompatibleBitmap(hDesktopDC, nScreenWidth, nScreenHeight);
	hOldBmp = (HBITMAP)SelectObject(hDesktopCompatibleDC,hDesktopCompatibleBitmap); 
	BitBlt(hDesktopCompatibleDC,0,0,nScreenWidth,nScreenHeight, hDesktopDC,0,0,SRCCOPY|CAPTUREBLT);
	CRegStdWORD regShowCursor(_T("Software\\DemoHelper\\capturecursor"), TRUE);
	if (DWORD(regShowCursor))
	{
		// capture the cursor
		CURSORINFO ci;
		ci.cbSize = sizeof(CURSORINFO);
		GetCursorInfo(&ci);
		if (ci.flags & CURSOR_SHOWING)
		{
			HICON hIcon = CopyIcon(ci.hCursor);
			ICONINFO ii;
			GetIconInfo(hIcon, &ii);
			DrawIcon(hDesktopCompatibleDC, ci.ptScreenPos.x-ii.xHotspot, ci.ptScreenPos.y-ii.yHotspot, hIcon);
			DestroyIcon(hIcon);
		}
	}

	ReleaseDC(hDesktopWnd,hDesktopDC);

	SetWindowPos(*this, HWND_TOP/*MOST*/, 0, 0, nScreenWidth, nScreenHeight, SWP_SHOWWINDOW);
	if (!m_bZooming)
	{
		if (m_hCursor)
			DestroyCursor(m_hCursor);
		m_hCursor = CreateDrawCursor(m_colors[m_colorindex], m_currentpenwidth);
		m_hPreviousCursor = SetCursor(m_hCursor);
	}
	else
	{
		SetCursor(NULL);
	}
	return true;
}

bool CMainWindow::EndPresentationMode()
{
	SetWindowPos(*this, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOREPOSITION|SWP_NOSIZE);
	SelectObject(hDesktopCompatibleDC, hOldBmp);
	DeleteObject(hDesktopCompatibleBitmap);
	DeleteDC(hDesktopCompatibleDC);
	m_lineindex[0] = 0;
	m_lineStartPoint[0].x = -1;
	m_lineStartPoint[0].y = -1;
	m_totallines = -1;
	m_bDrawing = false;
	SetCursor(m_hPreviousCursor);
	if (m_hCursor)
	{
		DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	}
	return true;
}

bool CMainWindow::DrawArrow(HDC hdc, int index)
{
	MoveToEx(hdc, m_lineStartPoint[index].x, m_lineStartPoint[index].y, NULL);
	return ArrowTo(hdc, m_lineEndPoint[index].x, m_lineEndPoint[index].y, m_penwidth[index]*3);
}

void CMainWindow::RegisterHotKeys()
{
	CRegStdWORD regZoom(_T("Software\\DemoHelper\\zoomhotkey"), 0x331);
	CRegStdWORD regDraw(_T("Software\\DemoHelper\\drawhotkey"), 0x332);
	WORD zoom = (WORD)(DWORD)regZoom;
	WORD draw = (WORD)(DWORD)regDraw;
	zoom = HotKeyControl2HotKey(zoom);
	draw = HotKeyControl2HotKey(draw);

	RegisterHotKey(*this, DRAW_HOTKEY, HIBYTE(draw), LOBYTE(draw));
	RegisterHotKey(*this, ZOOM_HOTKEY, HIBYTE(zoom), LOBYTE(zoom));
}

bool CMainWindow::StartZoomingMode()
{
	m_bZooming = true;
	StartPresentationMode();
	return true;
}

bool CMainWindow::EndZoomingMode()
{
	m_bZooming = false;
	EndPresentationMode();
	return true;
}

bool CMainWindow::DrawZoom(HDC hdc, POINT pt)
{
	// cursor pos is in screen coordinates - just what we need since our window covers the whole screen.
	// to zoom, we need to stretch the part around the cursor to the full screen
	// zoomfactor 1 = whole screen
	// zoomfactor 2 = quarter screen to fullscreen
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);
	int zoomwindowx = int(float(cx) / m_zoomfactor);
	int zoomwindowy = int(float(cy) / m_zoomfactor);

	// adjust the cursor position to the zoom factor
	int x = pt.x*(cx-zoomwindowx)/cx;
	int y = pt.y*(cy-zoomwindowy)/cy;
	
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (x + zoomwindowx > cx)
		x = cx-zoomwindowx;
	if (y + zoomwindowy > cy)
		y = cy-zoomwindowy;
	return !!StretchBlt(hdc,0,0,cx,cy,hDesktopCompatibleDC,x,y,zoomwindowx,zoomwindowy,SRCCOPY);
}

bool CMainWindow::UpdateCursor()
{
	if (m_bZooming)
	{
		SetCursor(NULL);
	}
	DestroyCursor(m_hCursor);
	m_hCursor = CreateDrawCursor(m_colors[m_colorindex], m_currentpenwidth);
	if (m_hCursor)
	{
		SetCursor(m_hCursor);
		return true;
	}
	return false;
}
