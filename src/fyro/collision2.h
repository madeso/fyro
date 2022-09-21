#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <cmath>
#include <set>

#include "fyro/rect.h"
#include "lox/object.h"

namespace fyro
{



/*
2d "pixel-perfect" collison system
based on https://maddythorson.medium.com/celeste-and-towerfall-physics-d24bd2ae0fc5
  * All colliders are axis-aligned bounding boxes (AABBs)
  * All collider positions, widths, and heights are integer numbers
  * Except for special circumstances, Actors and Solids will never overlap
  * Solids do not interact with other Solids
*/

// todo(Gustav): test
// todo(Gustav): don't iterate all "object", compute aabb, get a subsection and iterate that


struct Actor;
struct Solid;


struct Level
{
	std::vector<std::shared_ptr<Actor>> actors;
	std::vector<std::shared_ptr<Solid>> solids;

	void register_collision(Actor* lhs, Actor* rhs);

	void update(float dt);
	void render(std::shared_ptr<lox::Object> arg);
};


struct ActorList
{
	std::set<Actor*> actors;
	
	void add(const std::shared_ptr<Actor>& actor);
	bool has_actor(const std::shared_ptr<Actor>& a) const;
};


using CollisionReaction = std::function<void ()>;
void no_collision_reaction();



struct Aabb
{
	Aabb();

	glm::ivec2 position;
	Recti size;

	Recti get_rect(const glm::ivec2& new_position) const;
	Recti get_rect() const;

	int get_right() const;
	int get_left() const;
	int get_top() const;
	int get_bottom() const;
};


struct Actor : Aabb
{
	Level* level = nullptr;

	float x_remainder = 0.0f;
	float y_remainder = 0.0f;

	virtual ~Actor() = default;


	// true = solid, false = "empty"
	bool collide_at(const glm::ivec2& new_position);

	virtual void update(float dt) = 0;
	virtual void render(std::shared_ptr<lox::Object> arg) = 0;

	/*
	Typically, an Actor is riding a Solid if that Actor is immediately above the Solid.
	Some Actors might want to override this function to change how it behaves — for example:
	 * riding Solids that are ledge grabbed
	 * flying monsters never ride Solids.
	*/
	virtual bool is_riding_solid(Solid* solid) = 0;

	// define the behavior when an Actor is squeezed between two Solids
	virtual void get_squished() = 0;

	bool move_x(float dx, CollisionReaction on_collision);
	bool move_y(float dy, CollisionReaction on_collision);
	bool please_move_x(int dx, CollisionReaction on_collision);
	bool please_move_y(int dy, CollisionReaction on_collision);
};


struct Solid : Aabb
{
	Level* level = nullptr;

	Solid() = default;
	virtual ~Solid() = default;

	float x_remainder = 0.0f;
	float y_remainder = 0.0f;
	bool is_collidable = true;

	virtual void update(float dt) = 0;
	virtual void render(std::shared_ptr<lox::Object> arg) = 0;

	ActorList get_all_riding_actors();

	bool is_overlapping(const std::shared_ptr<Actor>& actor);

	void Move(float x, float y);

	void please_move_x(int dx, const ActorList& riding);
	void please_move_y(int dy, const ActorList& riding);
};



}
