/**
 * Copyright (C) 2026  Zarcx127@github.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

#pragma once

#ifndef INPUT_INPUT_STATE_H
#define INPUT_INPUT_STATE_H

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
