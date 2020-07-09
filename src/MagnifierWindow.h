#pragma once
#include "magnification.h"

#pragma comment(lib, "Magnification.lib")

class CMagnifierWindow
{
public:
    CMagnifierWindow()
        : m_hWnd(nullptr)
        , m_magFactor(1.0f)
        , m_sourceRect({})
    {
    }
    ~CMagnifierWindow() {}

    BOOL Create(HINSTANCE hInst, HWND hwndHost, BOOL visible);

    BOOL SetSourceRect(RECT rc);
    BOOL UpdateMagnifier();

    operator HWND() { return m_hWnd; }
    operator HWND() const { return m_hWnd; }

private:
    HWND  m_hWnd;
    float m_magFactor;
    RECT  m_sourceRect;
};
