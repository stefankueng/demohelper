#include "StdAfx.h"
#include "TrayWindow.h"
#include "registry.h"
#include "math.h"

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

#define TRAY_WM_MESSAGE		WM_APP+1

CTrayWindow::~CTrayWindow(void)
{
}

#define PACKVERSION(major,minor) MAKELONG(minor,major)
DWORD CTrayWindow::GetDllVersion(LPCTSTR lpszDllName)
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

bool CTrayWindow::RegisterAndCreateWindow()
{
	WNDCLASSEX wcx; 

	// Fill in the window class structure with default parameters 
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = CWindow::stWinMsgHandler;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hResource;
	wcx.hCursor = NULL;//LoadCursor(NULL, IDC_ARROW);
	wcx.lpszClassName = ResString(hResource, IDS_APP_TITLE);
	wcx.hIcon = LoadIcon(hResource, MAKEINTRESOURCE(IDI_SHOWHELPER));
	wcx.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
	wcx.lpszMenuName = NULL;
	wcx.hIconSm	= LoadIcon(wcx.hInstance, MAKEINTRESOURCE(IDI_SHOWHELPER));
	if (RegisterWindow(&wcx))
	{
		if (CreateEx(NULL, WS_POPUP, NULL))
		{
			// Our 'main' window is in the system tray where we wait to be
			// useful.
			// But first, we have to put ourselves to the system tray

			ZeroMemory(&niData,sizeof(NOTIFYICONDATA));

			ULONGLONG ullVersion = GetDllVersion(_T("Shell32.dll"));
			if (ullVersion >= MAKEDLLVERULL(6,0,0,0))
				niData.cbSize = sizeof(NOTIFYICONDATA);
			else if(ullVersion >= MAKEDLLVERULL(5,0,0,0))
				niData.cbSize = NOTIFYICONDATA_V2_SIZE;
			else niData.cbSize = NOTIFYICONDATA_V1_SIZE;

			niData.uID = IDI_SHOWHELPER;
			niData.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP|NIF_INFO;

			niData.hIcon = (HICON)LoadImage(hResource, MAKEINTRESOURCE(IDI_SHOWHELPER),
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


LRESULT CALLBACK CTrayWindow::WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
			CRegStdWORD regZoom(_T("Software\\ShowHelper\\zoomhotkey"), 0);
			CRegStdWORD regDraw(_T("Software\\ShowHelper\\drawhotkey"), 0);
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
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(*this, &ps);
			{
				CMemDC memdc(hdc);
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
					BitBlt(memdc,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),hDesktopCompatibleDC,0,0,SRCCOPY);
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
								PolyDraw(memdc, (const POINT*)&m_points[i], (const BYTE*)&m_linetypes[i], m_lineindex[i]);
							}
							else if ((m_lineStartPoint[i].x>0) && (m_lineStartPoint[i].y>0) && (m_lineEndPoint[i].x>0) && (m_lineEndPoint[i].y>0))
							{
								DrawArrow(memdc, i);
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
				int xPos = GET_X_LPARAM(lParam); 
				int yPos = GET_Y_LPARAM(lParam); 
				m_lineStartPoint[m_totallines].x = xPos;
				m_lineStartPoint[m_totallines].y = yPos;
				m_lineEndPoint[m_totallines].x = -1;
				m_lineEndPoint[m_totallines].y = -1;
				m_lineStartShiftPoint.x = -1;
				m_lineStartShiftPoint.y = -1;
				m_lineindex[m_totallines] = 0;
				m_points[m_totallines][m_lineindex[m_totallines]] = m_lineStartPoint[m_totallines];
				m_linetypes[m_totallines][m_lineindex[m_totallines]] = PT_MOVETO;
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
					if (m_lineindex[m_totallines] < (LINEARRAYSIZE-2))
					{
						m_lineindex[m_totallines]++;
						POINT pt = {xPos, yPos};
						m_points[m_totallines][m_lineindex[m_totallines]] = pt;
						m_linetypes[m_totallines][m_lineindex[m_totallines]] = PT_LINETO;
					}
					RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
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
					m_lineEndPoint[m_totallines].x = xPos;
					m_lineEndPoint[m_totallines].y = yPos;
					RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
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
					HMENU hMenu = LoadMenu(hResource, MAKEINTRESOURCE(IDC_SHOWHELPER));
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
			RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
		}
		break;
	case WM_KEYDOWN:
		{
			switch (wParam)
			{
			case VK_ESCAPE:
				// first quit zooming mode, then quit presentation mode
				if (m_bZooming)
					m_bZooming = false;
				else
					EndPresentationMode();
				break;
			case VK_BACK:
				m_bDrawing = false;
				m_lineindex[m_totallines] = 0;
				m_lineStartPoint[m_totallines].x = -1;
				m_lineStartPoint[m_totallines].y = -1;
				if (m_totallines > 0)
					m_totallines--;
				RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
				break;
			case VK_UP:
				if (m_bZooming)
				{
					m_zoomfactor += 0.2f;
					if (m_zoomfactor>4.0f)
						m_zoomfactor = 4.0f;
					RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
				}
				else
				// increase pen size
				if (m_totallines>=0)
				{
					if (m_currentpenwidth<32)
					{
						m_currentpenwidth++;
						RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
					}
				}
				UpdateCursor();
				break;
			case VK_DOWN:
				if (m_bZooming)
				{
					m_zoomfactor -= 0.2f;
					if (m_zoomfactor < 1.0f)
						m_zoomfactor = 1.0f;
					RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
				}
				else
				// decrease pen size
				if (m_totallines>=0)
				{
					if (m_currentpenwidth>1)
					{
						m_currentpenwidth--;
						RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
					}
				}
				UpdateCursor();
				break;
			case VK_RIGHT:
				// cycle through colors
				if (m_totallines>=0)
				{
					if (m_colorindex<9)
					{
						m_colorindex++;
						RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
					}
				}
				UpdateCursor();
				break;
			case VK_LEFT:
				// cycle through colors
				if (m_totallines>=0)
				{
					if (m_colorindex>0)
					{
						m_colorindex--;
						RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
					}
				}
				UpdateCursor();
				break;
			case 'E':
				m_bDrawing = false;
				m_lineindex[0] = 0;
				m_lineStartPoint[0].x = -1;
				m_lineStartPoint[0].y = -1;
				m_totallines = -1;
				RedrawWindow(*this, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
				break;
			case VK_RETURN:
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
				}
				break;
			}
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

LRESULT CTrayWindow::DoCommand(int id)
{
	switch (id) 
	{
	case IDM_EXIT:
		Shell_NotifyIcon(NIM_DELETE,&niData);
		::PostQuitMessage(0);
		return 0;
		break;
	case ID_TRAYCONTEXT_OPTIONS:
		{
			// deregister our hotkeys
			UnregisterHotKey(*this, DRAW_HOTKEY);
			UnregisterHotKey(*this, ZOOM_HOTKEY);
			DialogBox(hResource, MAKEINTRESOURCE(IDD_OPTIONS), *this, (DLGPROC)OptionsDlgProc);
			// now register our hotkeys again
			RegisterHotKeys();
		}
		break;
	case ID_TRAYCONTEXT_DRAW:
		StartPresentationMode();
		break;
	case ID_TRAYCONTEXT_ZOOM:
		StartZoomingMode();
		break;
	default:
		break;
	};
	return 1;
}

bool CTrayWindow::StartPresentationMode()
{
	int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	HWND hDesktopWnd = GetDesktopWindow();
	HDC hDesktopDC = GetDC(hDesktopWnd);
	hDesktopCompatibleDC = CreateCompatibleDC(hDesktopDC);
	hDesktopCompatibleBitmap = CreateCompatibleBitmap(hDesktopDC, nScreenWidth, nScreenHeight);
	hOldBmp = (HBITMAP)SelectObject(hDesktopCompatibleDC,hDesktopCompatibleBitmap); 
	BitBlt(hDesktopCompatibleDC,0,0,nScreenWidth,nScreenHeight, hDesktopDC,0,0,SRCCOPY|CAPTUREBLT);
	ReleaseDC(hDesktopWnd,hDesktopDC);

	SetWindowPos(*this, HWND_TOP/*MOST*/, 0, 0, nScreenWidth, nScreenHeight, SWP_SHOWWINDOW);
	SetForegroundWindow(*this);
	if (m_hCursor)
		DestroyCursor(m_hCursor);
	m_hCursor = CreateDrawCursor(m_colors[m_colorindex], m_currentpenwidth);
	m_hPreviousCursor = SetCursor(m_hCursor);
	return true;
}

bool CTrayWindow::EndPresentationMode()
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

bool CTrayWindow::DrawArrow(HDC hdc, int index)
{
	MoveToEx(hdc, m_lineStartPoint[index].x, m_lineStartPoint[index].y, NULL);
	return ArrowTo(hdc, m_lineEndPoint[index].x, m_lineEndPoint[index].y);
}

bool CTrayWindow::ArrowTo(HDC hdc, LONG x, LONG y)
{

	POINT pFrom;
	POINT pBase;
	POINT aptPoly[3];
	float vecLine[2];
	float vecLeft[2];
	float fLength = 1.0f;
	float th;
	float ta;

	// get from point
	MoveToEx(hdc, 0, 0, &pFrom);

	// set to point
	aptPoly[0].x = x;
	aptPoly[0].y = y;

	// build the line vector
	vecLine[0] = (float) aptPoly[0].x - pFrom.x;
	vecLine[1] = (float) aptPoly[0].y - pFrom.y;

	// build the arrow base vector - normal to the line
	vecLeft[0] = -vecLine[1];
	vecLeft[1] = vecLine[0];

	fLength = sqrt(vecLine[0]*vecLine[0] + vecLine[1]*vecLine[1])/fLength;
	th = 10 / (2.0f * fLength);
	ta = 10 / (2.0f * 0.15f * fLength);

	// find the base of the arrow
	pBase.x = (int) (aptPoly[0].x + -ta * vecLine[0]);
	pBase.y = (int) (aptPoly[0].y + -ta * vecLine[1]);

	// build the points on the sides of the arrow
	aptPoly[1].x = (int) (pBase.x + th * vecLeft[0]);
	aptPoly[1].y = (int) (pBase.y + th * vecLeft[1]);
	aptPoly[2].x = (int) (pBase.x + -th * vecLeft[0]);
	aptPoly[2].y = (int) (pBase.y + -th * vecLeft[1]);

	MoveToEx(hdc, pFrom.x, pFrom.y, NULL);

	LineTo(hdc, aptPoly[0].x, aptPoly[0].y);
	Polygon(hdc, aptPoly, 3);
	return true;
}

void CTrayWindow::RegisterHotKeys()
{
	CRegStdWORD regZoom(_T("Software\\ShowHelper\\zoomhotkey"), 0);
	CRegStdWORD regDraw(_T("Software\\ShowHelper\\drawhotkey"), 0);
	WORD zoom = (WORD)(DWORD)regZoom;
	WORD draw = (WORD)(DWORD)regDraw;
	zoom = HotKeyControl2HotKey(zoom);
	draw = HotKeyControl2HotKey(draw);

	RegisterHotKey(*this, DRAW_HOTKEY, HIBYTE(draw), LOBYTE(draw));
	RegisterHotKey(*this, ZOOM_HOTKEY, HIBYTE(zoom), LOBYTE(zoom));
}

bool CTrayWindow::StartZoomingMode()
{
	m_bZooming = true;
	StartPresentationMode();
	return true;
}

bool CTrayWindow::EndZoomingMode()
{
	m_bZooming = false;
	EndPresentationMode();
	return true;
}

bool CTrayWindow::DrawZoom(HDC hdc, POINT pt)
{
	// cursor pos is in screen coordinates - just what we need since our window covers the whole screen
	// to zoom, we need to stretch the part around the cursor to the full screen
	// zoomfactor 1 = whole screen
	// zoomfactor 2 = quarter screen to fullscreen
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);
	int zoomwindowx = int(float(cx) / m_zoomfactor);
	int zoomwindowy = int(float(cy) / m_zoomfactor);
	int x = pt.x - (zoomwindowx / 2);
	int y = pt.y - (zoomwindowy / 2);
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

HCURSOR CTrayWindow::CreateDrawCursor(COLORREF color, int penwidth)
{
	// Get the system display DC
	HDC hDC = ::GetDC(*this);

	// Create helper DC
	HDC hMainDC = ::CreateCompatibleDC(hDC);
	HDC hAndMaskDC = ::CreateCompatibleDC(hDC);
	HDC hXorMaskDC = ::CreateCompatibleDC(hDC);

	// Create the mask bitmaps
	HBITMAP hSourceBitmap = ::CreateCompatibleBitmap(hDC, GetSystemMetrics(SM_CXCURSOR), GetSystemMetrics(SM_CYCURSOR)); // original
	HBITMAP hAndMaskBitmap = ::CreateBitmap(GetSystemMetrics(SM_CXCURSOR), GetSystemMetrics(SM_CYCURSOR), 1, 1, NULL); // monochrome
	HBITMAP hXorMaskBitmap  = ::CreateCompatibleBitmap(hDC, GetSystemMetrics(SM_CXCURSOR), GetSystemMetrics(SM_CYCURSOR)); // color

	// Release the system display DC
	::ReleaseDC(*this, hDC);

	// Select the bitmaps to helper DC
	HBITMAP hOldMainBitmap = (HBITMAP)::SelectObject(hMainDC, hSourceBitmap);
	HBITMAP hOldAndMaskBitmap  = (HBITMAP)::SelectObject(hAndMaskDC, hAndMaskBitmap);
	HBITMAP hOldXorMaskBitmap  = (HBITMAP)::SelectObject(hXorMaskDC, hXorMaskBitmap);

	// fill our bitmap with the 'transparent' color RGB(1,1,1)
	RECT rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = GetSystemMetrics(SM_CXCURSOR);
	rc.bottom = GetSystemMetrics(SM_CYCURSOR);
	SetBkColor(hMainDC, RGB(1,1,1));
	::ExtTextOut(hMainDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	// set up the pen and brush to draw
	HPEN hPen = CreatePen(PS_SOLID, 1, color);
	HPEN hOldPen = NULL;
	HBRUSH hOldBrush = NULL;
	hOldPen = (HPEN)SelectObject(hMainDC, hPen);
	HBRUSH hBrush = CreateSolidBrush(color);
	hOldBrush = (HBRUSH)SelectObject(hMainDC, hBrush);

	// draw the brush circle
	Ellipse(hMainDC, (GetSystemMetrics(SM_CXCURSOR)-penwidth)/2, (GetSystemMetrics(SM_CYCURSOR)-penwidth)/2, (GetSystemMetrics(SM_CXCURSOR)-penwidth)/2+penwidth, (GetSystemMetrics(SM_CYCURSOR)-penwidth)/2+penwidth);

	SelectObject(hMainDC, hOldBrush);
	SelectObject(hMainDC, hOldPen);
	DeleteObject(hBrush);
	DeleteObject(hPen);

	// Assign the monochrome AND mask bitmap pixels so that a pixels of the source bitmap
	//    with 'clrTransparent' will be white pixels of the monochrome bitmap
	::SetBkColor(hMainDC, RGB(1,1,1));
	::BitBlt(hAndMaskDC, 0, 0, GetSystemMetrics(SM_CXCURSOR), GetSystemMetrics(SM_CYCURSOR), hMainDC, 0, 0, SRCCOPY);

	// Assign the color XOR mask bitmap pixels so that a pixels of the source bitmap
	//    with 'clrTransparent' will be black and rest the pixels same as corresponding
	//    pixels of the source bitmap
	::SetBkColor(hXorMaskDC, RGB(0, 0, 0));
	::SetTextColor(hXorMaskDC, RGB(255, 255, 255));
	::BitBlt(hXorMaskDC, 0, 0, GetSystemMetrics(SM_CXCURSOR), GetSystemMetrics(SM_CYCURSOR), hAndMaskDC, 0, 0, SRCCOPY);
	::BitBlt(hXorMaskDC, 0, 0, GetSystemMetrics(SM_CXCURSOR), GetSystemMetrics(SM_CYCURSOR), hMainDC, 0,0, SRCAND);

	// Deselect bitmaps from the helper DC
	::SelectObject(hMainDC, hOldMainBitmap);
	::SelectObject(hAndMaskDC, hOldAndMaskBitmap);
	::SelectObject(hXorMaskDC, hOldXorMaskBitmap);

	// Delete the helper DC
	::DeleteDC(hXorMaskDC);
	::DeleteDC(hAndMaskDC);
	::DeleteDC(hMainDC);

	ICONINFO iconinfo = {0};
	iconinfo.fIcon			= FALSE;
	iconinfo.xHotspot       = GetSystemMetrics(SM_CXCURSOR)/2;
	iconinfo.yHotspot       = GetSystemMetrics(SM_CYCURSOR)/2;
	iconinfo.hbmMask        = hAndMaskBitmap;
	iconinfo.hbmColor       = hXorMaskBitmap;

	return ::CreateIconIndirect(&iconinfo);
}

bool CTrayWindow::UpdateCursor()
{
	DestroyCursor(m_hCursor);
	m_hCursor = CreateDrawCursor(m_colors[m_colorindex], m_currentpenwidth);
	if (m_hCursor)
	{
		SetCursor(m_hCursor);
		return true;
	}
	return false;
}
