#pragma once

#include <memory>


namespace input
{


struct Table;
struct ConnectedUnits;


/** Represents a player.
The idea behind decoupling the active units and the player is that the unit
could be disconnected and swapped but the player should remain.
*/
struct Player
{
    Player();
    ~Player();

    void UpdateTable(Table* table, float dt);
    bool IsConnected();
    bool IsAnyConnectionConsideredJoystick();

    void UpdateConnectionStatus();

    std::unique_ptr<ConnectedUnits> connected_units;
};

}  // namespace input
