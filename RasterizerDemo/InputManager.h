#pragma once
#include <Windows.h>
class InputManager
{
private:
    POINT lastMousePos = { 0, 0 };
    POINT currentMousePos = { 0, 0 };
    bool isFirstFrame = true;

    float mouseDeltaX = 0.0f;
    float mouseDeltaY = 0.0f;

public:
    InputManager() = default;

    // Call this every frame to update input states
    void Update();
    
    // delata 
    float GetMouseDeltaX() const;
    float GetMouseDeltaY() const;
    
    // Hide or show cursor function
    void HideCursor(); 
        
    

    void ShowCursorInClient();

    // Optional: Capture keyboard input or other functionality
    bool IsKeyPressed(int virtualKey) const;
  

    void CenterCursor(HWND hwnd);
};