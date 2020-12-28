#include "tred/input-activelist.h"
#include <cassert>

#include "tred/input-activeaxis.h"
#include "tred/input-activerange.h"
#include "tred/input-activerangetoaxis.h"
#include "tred/input-activeaxistorange.h"
#include "tred/input-activemasteraxis.h"
#include "tred/input-activemasterrange.h"

#include "tred/input-action.h"
#include "tred/input-table.h"


namespace input
{
void ActiveList::Add(std::shared_ptr<ActiveRange> range)
{
    range_binds.push_back(range);
}

void ActiveList::Add(std::shared_ptr<ActiveAxis> axis)
{
    axis_binds.push_back(axis);
}

void ActiveList::Add(std::shared_ptr<ActiveAxisToRange> axis)
{
    axis_to_range_binds.push_back(axis);
}

void ActiveList::Add(std::shared_ptr<ActiveRangeToAxis> axis)
{
    range_to_axis_binds.push_back(axis);
}

void ActiveList::Add(std::shared_ptr<ActiveMasterAxis> axis)
{
    master_axis_binds.push_back(axis);
}

void ActiveList::Add(std::shared_ptr<ActiveMasterRange> axis)
{
    master_range_binds.push_back(axis);
}

void ActiveList::UpdateTable(Table* table)
{
    assert(table);

    for (auto b: master_range_binds)
    {
        table->Set(b->action->scriptvarname, b->GetNormalizedState());
    }

    for (auto b: master_axis_binds)
    {
        table->Set(b->action->scriptvarname, b->GetNormalizedState());
    }
}

void ActiveList::Update(float dt)
{
    for (auto range: range_binds)
    {
        range->Update(dt);
    }
    for (auto axis: axis_binds)
    {
        axis->Update(dt);
    }
    for (auto axis: axis_to_range_binds)
    {
        axis->Update(dt);
    }
    for (auto axis: range_to_axis_binds)
    {
        axis->Update(dt);
    }
    for (auto axis: master_axis_binds)
    {
        axis->Update(dt);
    }
    for (auto axis: master_range_binds)
    {
        axis->Update(dt);
    }
}

}  // namespace input