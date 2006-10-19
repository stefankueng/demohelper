#pragma once
#include "basewindow.h"
#include "Resource.h"
#include "shellapi.h"
#include "shlwapi.h"
#include "hyperlink.h"
#include <commctrl.h>
#include <vector>

#define LINEARRAYSIZE 4000
#define MAX_NUMBEROFLINES 50
#define DRAW_HOTKEY 100
#define ZOOM_HOTKEY 101

#define TIMER_ID_DRAW	101
#define TIMER_ID_ZOOM	102


typedef enum LineTypes
{
	normal,
	arrow,
} LineTypes;

class CMainWindow :
	public CWindow
{
public:
	CMainWindow(HINSTANCE hInst, const WNDCLASSEX* wcx = NULL) : CWindow(hInst, wcx)
		, m_bDrawing(false)
		, m_bZooming(false)
		, m_zoomfactor(2.0f)
		, hDesktopCompatibleDC(NULL)
		, hDesktopCompatibleBitmap(NULL)
		, hOldBmp(NULL)
		, m_totallines(-1)
		, m_colorindex(1)
		, m_currentpenwidth(6)
		, m_hCursor(NULL)
		, m_currentrop(R2_MASKPEN)
		, m_bMarker(false)
		, m_bInlineZoom(false)
	{
		SetWindowTitle((LPCTSTR)ResString(hResource, IDS_APP_TITLE));
		m_colors[0] = RGB(255, 255, 0);
		m_colors[1] = RGB(255, 0, 0);
		m_colors[2] = RGB(150, 0, 0);
		m_colors[3] = RGB(0, 255, 0);
		m_colors[4] = RGB(0, 150, 0);
		m_colors[5] = RGB(0, 0, 255);
		m_colors[6] = RGB(0, 0, 150);
		m_colors[7] = RGB(0, 0, 0);
		m_colors[8] = RGB(150, 150, 150);
		m_colors[9] = RGB(0, 255, 255);
		m_points = new POINT[MAX_NUMBEROFLINES*LINEARRAYSIZE];
		m_linetypes = new BYTE[MAX_NUMBEROFLINES*LINEARRAYSIZE];
	};
	~CMainWindow(void)
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
	bool				StartInlineZoom();
	bool				StartZoomingMode();
	bool				EndZoomingMode();
	bool				DrawArrow(HDC hdc, int index);
	bool				ArrowTo(HDC hdc, LONG x, LONG y, int width);
	bool				DrawZoom(HDC hdc, POINT pt);
	HCURSOR				CreateDrawCursor(COLORREF color, int penwidth);

	static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static WORD			HotKeyControl2HotKey(WORD hk);
	static WORD			HotKey2HotKeyControl(WORD hk);
	static BOOL CALLBACK HelpDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

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
	int					m_rop[MAX_NUMBEROFLINES];

	POINT				m_lineStartPoint[MAX_NUMBEROFLINES];
	POINT				m_lineEndPoint[MAX_NUMBEROFLINES];
	LineTypes			m_lineType[MAX_NUMBEROFLINES];

	int					m_penwidth[MAX_NUMBEROFLINES];
	int					m_linecolorindex[MAX_NUMBEROFLINES];
	int					m_colorindex;
	int					m_currentpenwidth;
	int					m_currentrop;

	POINT				m_lineStartShiftPoint;

	COLORREF			m_colors[10];

	HCURSOR				m_hCursor;
	HCURSOR				m_hPreviousCursor;

	bool				m_bMarker;
	int					m_oldpenwidth;
	int					m_oldcolorindex;
	int					m_oldrop;

	bool				m_bInlineZoom;
	POINT				m_ptInlineZoomStartPoint;
	POINT				m_ptInlineZoomEndPoint;

	static CHyperLink	m_link;
};
