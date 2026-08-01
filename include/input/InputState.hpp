#pragma once

#ifndef INPUT_STATE_H
#define INPUT_STATE_H

struct InputState
{
    bool keyW { false };
    bool keyA { false };
    bool keyS { false };
    bool keyD { false };
    bool keyE { false };
    bool keyQ { false };

    bool rightMouseDown { false };
    
    double mouseX { 0.0 };
    double mouseY { 0.0 };

    double mouseDeltaX { 0.0 };
    double mouseDeltaY { 0.0 };
};

#endif
