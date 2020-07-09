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
