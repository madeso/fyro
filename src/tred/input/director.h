#pragma once

#include <vector>
#include <map>

#include "tred/input/key.h"
#include "tred/input/axis.h"
#include "tred/input/platform.h"

namespace input
{


struct KeyboardActiveUnit;
struct MouseActiveUnit;
struct JoystickActiveUnit;


struct InputDirector
{
    void Add(KeyboardActiveUnit* kb);
    void Remove(KeyboardActiveUnit* kb);

    void Add(MouseActiveUnit* au);
    void Remove(MouseActiveUnit* au);

    void Add(JoystickActiveUnit* au);
    void Remove(JoystickActiveUnit* au);

    void OnKeyboardKey(Key key, bool down);

    void OnMouseAxis(Axis axis, float value);
    void OnMouseWheel(Axis axis, float value);
    void OnMouseButton(MouseButton button, bool down);

    void OnJoystickBall(JoystickId joystick, Axis type, int ball, float value);
    void OnJoystickHat(JoystickId joystick, Axis type, int hat, float value);
    void OnJoystickButton(JoystickId joystick, int button, bool down);
    void OnJoystickAxis(JoystickId joystick, int axis, float value);

    std::vector<KeyboardActiveUnit*> keyboards;
    std::vector<MouseActiveUnit*> mouses;
    std::map<JoystickId, JoystickActiveUnit*> joysticks;
};

}  // namespace input
