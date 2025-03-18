#include "InputManager.h"

void InputManager::Update()
{
    // Get current mouse position
    GetCursorPos(&currentMousePos);

    if (isFirstFrame)
    {
        this->lastMousePos = currentMousePos;
        isFirstFrame = false;
    }

    // Calculate mouse deltas
    this->mouseDeltaX = static_cast<float>(currentMousePos.x - lastMousePos.x);
    this->mouseDeltaY = static_cast<float>(currentMousePos.y - lastMousePos.y);

    // Update last mouse position
    lastMousePos = currentMousePos;
}

float InputManager::GetMouseDeltaX() const
{
    return this->mouseDeltaX;
}

float InputManager::GetMouseDeltaY() const
{
    return this->mouseDeltaY;
}

void InputManager::HideCursor()
{
    // Hide the cursor
    ShowCursor(false);
}

void InputManager::ShowCursorInClient()
{
    ShowCursor(true);
}

bool InputManager::IsKeyPressed(int virtualKey) const
{
    return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
}

void InputManager::CenterCursor(HWND hwnd)
{
    RECT rect;
    GetClientRect(hwnd, &rect);
    POINT center = { rect.right / 2, rect.bottom / 2 };

    ClientToScreen(hwnd, &center);
    SetCursorPos(center.x, center.y);

    lastMousePos = center; // Reset mouse deltas
}
