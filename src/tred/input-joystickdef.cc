#include "tred/input-joystickdef.h"

#include <cassert>
#include <string>
#include "fmt/format.h"

#include "tred/input-config.h"
#include "tred/input-dummyactiveunit.h"
#include "tred/input-taxisbind.h"
#include "tred/input-trangebind.h"
#include "tred/input-actionmap.h"
#include "tred/input-bindmap.h"
#include "tred/input-commondef.h"
#include "tred/input-joystickactiveunit.h"


namespace input
{


JoystickDef::JoystickDef(const config::JoystickDef& data, const InputActionMap&)
{
    for (const auto& d: data.axis)
    {
        auto common = d.common;
        const int axis = d.axis;
        if (axis < 0)
        {
            const std::string error = fmt::format("Invalid joystick axis for {} action", common.bindname);
            throw error;
        }
        axes.push_back(BindDef<int>(common.bindname, axis, d));
    }
    for (const auto& d: data.button)
    {
        auto common = d.common;
        const int key = d.button;

        if (key < 0)
        {
            const std::string error = fmt::format("Invalid joystick button for the {} action", common.bindname);
            throw error;
        }
        buttons.push_back(BindDef<int>(common.bindname, key, d));
    }
    for (const auto& d: data.hat)
    {
        auto common = d.common;
        const int hat = d.hat;
        if (hat < 0)
        {
            const std::string error = fmt::format("Invalid joystick hat for the {} action", common.bindname);
            throw error;
        }

        const auto axis = d.axis;
        hats.push_back(BindDef<HatAxis>(common.bindname, HatAxis(hat, axis), d));
    }
}


std::shared_ptr<ActiveUnit> JoystickDef::Create(InputDirector* director, BindMap* map)
{
    assert(director);
    assert(map);

    /// @todo fix the joystick number
    int js = 0;

    std::vector<std::shared_ptr<TAxisBind<int>>> axisbinds = CreateBinds<TAxisBind<int>, int>(axes, map);
    std::vector<std::shared_ptr<TRangeBind<int>>> buttonbinds = CreateBinds<TRangeBind<int>, int>(buttons, map);
    std::vector<std::shared_ptr<TAxisBind<HatAxis>>> hatbinds = CreateBinds<TAxisBind<HatAxis>, HatAxis>(hats, map);

    std::shared_ptr<ActiveUnit> unit(new JoystickActiveUnit(js, director, axisbinds, buttonbinds, hatbinds));
    return unit;
}


}  // namespace input
