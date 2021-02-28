#include "unitdef.gamecontroller.h"

#include <cassert>
#include <string>
#include "fmt/format.h"

#include "tred/log.h"
#include "tred/input/config.h"
#include "tred/input/bind.h"
#include "tred/input/key.h"
#include "tred/input/axis.h"
#include "tred/input/actionmap.h"
#include "tred/input/activeunit.gamecontroller.h"
#include "tred/input/unitsetup.h"
#include "tred/input/platform.h"
#include "tred/input/director.h"
#include "tred/input/index.h"


namespace input
{


GamecontrollerDef::GamecontrollerDef(int id, const config::GamecontrollerDef&)
    : index(id)
{
}


std::optional<std::string> GamecontrollerDef::ValidateKey(int key)
{
    if(key <= ToIndex(GamecontrollerButton::INVALID)
    || key > ToIndex(GamecontrollerButton::TRIGGER_RIGHT))
    {
        return fmt::format("Invalid bind to invalid gamecontroller button: {}", key);
    }

    return std::nullopt;
}


std::optional<std::string> GamecontrollerDef::ValidateAxis(AxisType type, int target, int axis)
{
    if(type != AxisType::GeneralAxis) { return fmt::format("Only general axcis for game controllers: {}", type); }
    if(target != 0) { return fmt::format("Target needs to be 0 for controller: {}", target); }

    switch(FromIndex<GamecontrollerAxis>(axis))
    {
        case GamecontrollerAxis::LEFTX:
        case GamecontrollerAxis::LEFTY:
        case GamecontrollerAxis::RIGHTX:
        case GamecontrollerAxis::RIGHTY:
            return std::nullopt;
        default:
            return fmt::format("Invalid axis: {}", axis);
    }
}


bool GamecontrollerDef::IsConsideredJoystick()
{
    return true;
}


bool GamecontrollerDef::CanDetect(InputDirector* director, UnitDiscovery, UnitSetup* setup, Platform* platform)
{
    const auto potential_joysticks = platform->ActiveAndFreeJoysticks();
    for(auto joy: potential_joysticks)
    {
        const auto selected = setup->HasJoystick(joy) == false && director->WasGameControllerStartJustPressed(joy);
        if(selected)
        {
            platform->StartUsing(joy, JoystickType::GameController);
            setup->AddJoystick(index, joy);
            return true;
        }
    }

    return false;
}


std::unique_ptr<ActiveUnit> GamecontrollerDef::Create(InputDirector* director, const UnitSetup& setup)
{
    assert(director);
    return std::make_unique<GamecontrollerActiveUnit>(setup.GetJoystick(index), director);
}


}  // namespace input