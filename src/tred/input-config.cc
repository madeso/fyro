#include "tred/input-config.h"

namespace input::config
{

Action::Action(const std::string& n, const std::string& v, ::Range r, bool g)
    : name(n)
    , var(v)
    , range(r)
    , global(g)
{
}


KeyboardButton::KeyboardButton(const std::string& bind, ::Key k, bool i, float s)
    : bindname(bind)
    , key(k)
    , invert(i)
    , scale(s)
{
}

MouseAxis::MouseAxis(const std::string& bind, ::Axis a, bool i, float s)
    : bindname(bind)
    , axis(a)
    , invert(i)
    , scale(s)
{
}


MouseButton::MouseButton(const std::string& bind, ::MouseButton k, bool i, float s)
    : bindname(bind)
    , key(k)
    , invert(i)
    , scale(s)
{
}


JoystickAxis::JoystickAxis(const std::string& bind, int a, bool i, float s)
    : bindname(bind)
    , axis(a)
    , invert(i)
    , scale(s)
{
}


JoystickButton::JoystickButton(const std::string& bind, int b, bool i, float s)
    : bindname(bind)
    , button(b)
    , invert(i)
    , scale(s)
{
}


JoystickHat::JoystickHat(const std::string& bind, int h, ::Axis a, bool i, float s)
    : bindname(bind)
    , hat(h)
    , axis(a)
    , invert(i)
    , scale(s)
{
}


KeyboardDef::KeyboardDef()
{
}


KeyboardDef::KeyboardDef(const std::vector<KeyboardButton>& b)
    : binds(b)
{
}


MouseDef::MouseDef()
{
}


MouseDef::MouseDef(const std::vector<MouseAxis>& a, const std::vector<MouseButton>& b)
    : axis(a)
    , button(b)
{
}


JoystickDef::JoystickDef()
{
}


JoystickDef::JoystickDef(const std::vector<JoystickAxis>& a, const std::vector<JoystickButton>& b, const std::vector<JoystickHat>& h)
    : axis(a)
    , button(b)
    , hat(h)
{
}


UnitInterface::UnitInterface(const KeyboardDef& k)
    : keyboard(k)
{
}


UnitInterface::UnitInterface(const MouseDef& m)
    : mouse(m)
{
}


UnitInterface::UnitInterface(const JoystickDef& j)
    : joystick(j)
{
}


Config::Config(const std::string& n, const std::vector<UnitInterface>& u)
    : name(n)
    , units(u)
{
}


InputSystem::InputSystem(const ActionMap& a, const KeyConfigs& k, const std::vector<std::string>& p)
    : actions(a)
    , keys(k)
    , players(p)
{
}


}
