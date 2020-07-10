#include "stdafx.h"
#include "MagnifierWindow.h"

BOOL CMagnifierWindow::Create(HINSTANCE hInst, HWND hwndHost, BOOL visible)
{
    DWORD dwStyle = WS_CHILD | WS_EX_COMPOSITED | MS_SHOWMAGNIFIEDCURSOR | MS_CLIPAROUNDCURSOR;
    if (visible)
        dwStyle |= WS_VISIBLE;

    m_hWnd = CreateWindow(WC_MAGNIFIER, // Magnifier window class name defined in magnification.h
                          L"MagnifierWindow2",
                          dwStyle,
                          0, 0, 100, 100,
                          hwndHost, nullptr, hInst, nullptr);

    if (m_hWnd == nullptr)
        return FALSE;

    return TRUE;
}

BOOL CMagnifierWindow::SetMagnification(POINT mousePt, float magnification)
{
    // first adjust the mousePt to the current zoom rect
    RECT client = {0};
    GetWindowRect(*this, &client);
    auto clientWidth  = client.right - client.left;
    auto clientHeight = client.bottom - client.top;
    auto sourceWidth  = m_sourceRect.right - m_sourceRect.left;
    auto sourceHeight = m_sourceRect.bottom - m_sourceRect.top;

    mousePt.x = mousePt.x - client.left;
    mousePt.y = mousePt.y - client.top;

    mousePt.x = mousePt.x * sourceWidth / clientWidth + m_sourceRect.left;
    mousePt.y = mousePt.y * sourceHeight / clientHeight + m_sourceRect.top;

    if (magnification == 0.0f)
        magnification = m_magFactor;

    m_sourceRect.left   = LONG(mousePt.x - ((clientWidth / magnification) / 2.0f));
    m_sourceRect.right  = LONG(mousePt.x + ((clientWidth / magnification) / 2.0f));
    m_sourceRect.top    = LONG(mousePt.y - ((clientHeight / magnification) / 2.0f));
    m_sourceRect.bottom = LONG(mousePt.y + ((clientHeight / magnification) / 2.0f));

    if (m_sourceRect.left < client.left)
    {
        m_sourceRect.left  = client.left;
        m_sourceRect.right = LONG(m_sourceRect.left + (clientWidth / magnification));
    }
    if (m_sourceRect.top < client.top)
    {
        m_sourceRect.top    = client.top;
        m_sourceRect.bottom = LONG(m_sourceRect.top + (clientHeight / magnification));
    }

    if (m_sourceRect.right > client.right)
    {
        m_sourceRect.right = client.right;
        m_sourceRect.left  = LONG(client.right - (clientWidth / magnification));
    }
    if (m_sourceRect.bottom > client.bottom)
    {
        m_sourceRect.bottom = client.bottom;
        m_sourceRect.top    = LONG(client.bottom - (clientHeight / magnification));
    }

    MAGTRANSFORM matrix;
    memset(&matrix, 0, sizeof(matrix));
    matrix.v[0][0] = magnification;
    matrix.v[1][1] = magnification;
    matrix.v[2][2] = 1.0f;

    m_magFactor = magnification;

    MagShowSystemCursor(false);
    MagSetWindowTransform(m_hWnd, &matrix);
    return MagSetWindowSource(m_hWnd, m_sourceRect);
}

BOOL CMagnifierWindow::SetSourceRect(RECT rc)
{
    m_sourceRect = rc;

    RECT client = {0};
    GetClientRect(*this, &client);
    auto         magFactor = float(client.right - client.left) / float(m_sourceRect.right - m_sourceRect.left);
    MAGTRANSFORM matrix;
    memset(&matrix, 0, sizeof(matrix));
    matrix.v[0][0] = magFactor;
    matrix.v[1][1] = magFactor;
    matrix.v[2][2] = 1.0f;

    m_magFactor = magFactor;

    MagShowSystemCursor(false);
    MagSetWindowTransform(m_hWnd, &matrix);
    return MagSetWindowSource(m_hWnd, m_sourceRect);
}

BOOL CMagnifierWindow::UpdateMagnifier()
{
    return MagSetWindowSource(m_hWnd, m_sourceRect);
}

BOOL CMagnifierWindow::Reset()
{
    MAGTRANSFORM matrix;
    memset(&matrix, 0, sizeof(matrix));
    matrix.v[0][0] = 1.0f;
    matrix.v[1][1] = 1.0f;
    matrix.v[2][2] = 1.0f;

    m_magFactor = 1.0f;

    MagShowSystemCursor(true);
    return MagSetWindowTransform(m_hWnd, &matrix);
}
