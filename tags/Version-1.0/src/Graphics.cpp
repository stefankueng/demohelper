#include "stdafx.h"
#include "MainWindow.h"
#include "math.h"

HCURSOR CMainWindow::CreateDrawCursor(COLORREF color, int penwidth)
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

bool CMainWindow::ArrowTo(HDC hdc, LONG x, LONG y, int width)
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
	th = width / (2.0f * fLength);
	ta = width / (2.0f * 0.15f * fLength);

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
