#pragma once

#include <vector>
#include <map>

#include "tred/input/axis.h"
#include "tred/input/key.h"
#include "tred/input/activeunit.h"
#include "tred/input/bind.h"


namespace input
{


struct Table;
struct InputDirector;


struct MouseActiveUnit : public ActiveUnit
{
    MouseActiveUnit
    (
        InputDirector* director, Converter* converter,
        const std::vector<BindDef<Axis>>& axes,
        const std::vector<BindDef<MouseButton>>& buttons
    );
    ~MouseActiveUnit();

    void Recieve(ValueReciever* reciever) override;

    void OnAxis(const Axis& axis, float state);
    void OnButton(MouseButton button, float state);

    InputDirector* director;
    BindMap<Axis> axes;
    BindMap<MouseButton> buttons;
};


}  // namespace input
