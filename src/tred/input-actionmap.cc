#include "tred/input-actionmap.h"
#include <cassert>
#include <stdexcept>
#include <fstream>
#include "fmt/format.h"

#include "tred/input-config.h"
#include "tred/input-range.h"
#include "tred/input-action.h"


namespace input
{

    
void InputActionMap::Add(const std::string& name, std::shared_ptr<InputAction> action)
{
    assert(action);
    actions.insert(std::make_pair(name, action));
}


std::shared_ptr<InputAction> InputActionMap::Get(const std::string& name) const
{
    auto res = actions.find(name);
    if (res == actions.end())
    {
        throw fmt::format("Unable to find action: {}", name);
    }

    assert(res->second);
    return res->second;
}


std::vector<std::shared_ptr<InputAction>> InputActionMap::GetActionList() const
{
    std::vector<std::shared_ptr<InputAction>> ret;
    for (auto a: actions)
    {
        ret.push_back(a.second);
    }
    return ret;
}


void Load(InputActionMap* map, const input::config::ActionMap& root)
{
    assert(map);

    for (const auto& d: root)
    {
        std::shared_ptr<InputAction> action(new InputAction(d.name, d.var, d.range));
        map->Add(d.name, action);
    }
}

}  // namespace input