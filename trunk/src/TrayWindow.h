#pragma once
#include "basewindow.h"
#include "Resource.h"
#include "shellapi.h"
#include "shlwapi.h"
#include <commctrl.h>
#include <vector>

#define LINEARRAYSIZE 4000
#define MAX_NUMBEROFLINES 50
#define DRAW_HOTKEY 100
#define ZOOM_HOTKEY 101

class CTrayWindow :
	public CWindow
{
public:
	CTrayWindow(HINSTANCE hInst, const WNDCLASSEX* wcx = NULL) : CWindow(hInst, wcx)
		, m_bDrawing(false)
		, m_bZooming(false)
		, m_zoomfactor(2.0f)
		, hDesktopCompatibleDC(NULL)
		, hDesktopCompatibleBitmap(NULL)
		, hOldBmp(NULL)
		, m_totallines(-1)
		, m_colorindex(0)
		, m_currentpenwidth(3)
		, m_hCursor(NULL)
	{
		SetWindowTitle((LPCTSTR)ResString(hResource, IDS_APP_TITLE));
		m_colors[0] = RGB(255, 0, 0);
		m_colors[1] = RGB(150, 0, 0);
		m_colors[2] = RGB(0, 255, 0);
		m_colors[3] = RGB(0, 150, 0);
		m_colors[4] = RGB(0, 0, 255);
		m_colors[5] = RGB(0, 0, 150);
		m_colors[6] = RGB(0, 0, 0);
		m_colors[7] = RGB(150, 150, 150);
		m_colors[8] = RGB(255, 255, 255);
		m_colors[9] = RGB(0, 255, 255);
		m_points = new POINT[MAX_NUMBEROFLINES*LINEARRAYSIZE];
		m_linetypes = new BYTE[MAX_NUMBEROFLINES*LINEARRAYSIZE];
	};
	~CTrayWindow(void)
	{
		delete [] m_points;
		delete [] m_linetypes;
	};

	bool				RegisterAndCreateWindow();

protected:
	/// the message handler for this window
	LRESULT CALLBACK	WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/// Handles all the WM_COMMAND window messages (e.g. menu commands)
	LRESULT				DoCommand(int id);

	bool				StartPresentationMode();
	bool				EndPresentationMode();
	bool				StartZoomingMode();
	bool				EndZoomingMode();
	bool				DrawArrow(HDC hdc, int index);
	bool				ArrowTo(HDC hdc, LONG x, LONG y);
	bool				DrawZoom(HDC hdc, POINT pt);
	HCURSOR				CreateDrawCursor(COLORREF color, int penwidth);

	static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static WORD			HotKeyControl2HotKey(WORD hk);
	static WORD			HotKey2HotKeyControl(WORD hk);

	void				RegisterHotKeys();
	bool				UpdateCursor();
	DWORD				GetDllVersion(LPCTSTR lpszDllName);
protected:
	NOTIFYICONDATA		niData; 
	HDC					hDesktopCompatibleDC;
	HBITMAP				hDesktopCompatibleBitmap;
	HBITMAP				hOldBmp;

	bool				m_bDrawing;
	float				m_zoomfactor;
	bool				m_bZooming;

	int					m_totallines;
	int					m_lineindex[MAX_NUMBEROFLINES];
	POINT *				m_points;
	BYTE *				m_linetypes;
	POINT				m_lineStartPoint[MAX_NUMBEROFLINES];
	POINT				m_lineEndPoint[MAX_NUMBEROFLINES];
	int					m_penwidth[MAX_NUMBEROFLINES];
	int					m_linecolorindex[MAX_NUMBEROFLINES];
	int					m_colorindex;
	int					m_currentpenwidth;

	POINT				m_lineStartShiftPoint;

	COLORREF			m_colors[10];

	HCURSOR				m_hCursor;
	HCURSOR				m_hPreviousCursor;
};
