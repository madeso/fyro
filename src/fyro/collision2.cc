#include <functional>
#include <vector>
#include <memory>
#include <cmath>
#include <set>

#include "fyro/rect.h"


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



int Round(float f)
{
	return static_cast<int>(std::lround(f));
}

int Sign(int x)
{
	if(x > 0) { return 1; }
	else { return 0; }
}

struct Actor;
struct Solid;

Recti get_rect(const Actor& a, const glm::ivec2& position);
Recti get_rect(const Solid& s, const glm::ivec2& position);
Recti get_rect(const Actor& a);
Recti get_rect(const Solid& s);


struct Level
{
	std::vector<std::shared_ptr<Actor>> actors;
	std::vector<std::shared_ptr<Solid>> solids;

	void register_collision(Actor* lhs, Actor* rhs);
};


struct ActorList
{
	std::set<Actor*> actors;
	void add(const std::shared_ptr<Actor>& actor)
	{
		actors.insert(actor.get());
	}
	bool has_actor(const std::shared_ptr<Actor>& a) const
	{
		return actors.find(a.get()) != actors.end();
	}
};


using CollisionReaction = std::function<void ()>;
void no_collision_reaction() {};

bool rect_intersect(const Recti& lhs, const Recti& rhs);

struct Aabb
{
	Aabb()
		: position(0, 0)
		, size(10, 10)
	{
	}

	glm::ivec2 position;
	Recti size;

	Recti get_rect(const glm::ivec2& new_position) const
	{
		return size.translate(new_position);
	}

	Recti get_rect() const
	{
		return get_rect(position);
	}

	int get_right() const { return get_rect().right; }
	int get_left() const { return get_rect().left; }
	int get_top() const { return get_rect().top; }
	int get_bottom() const { return get_rect().bottom; }
};


struct Actor : Aabb
{
	Level* level;

	float x_remainder = 0.0f;
	float y_remainder = 0.0f;

	virtual ~Actor() = default;


	// true = solid, false = "empty"
	bool collide_at(const glm::ivec2& new_position)
	{
		const auto self = fyro::get_rect(*this, new_position);

		for(auto& actor: level->actors)
		{
			if(rect_intersect(self, fyro::get_rect(*actor)))
			{
				level->register_collision(this, actor.get());
			}
		}

		for(auto& solid: level->solids)
		{
			if(rect_intersect(self, fyro::get_rect(*solid)))
			{
				return true;
			}
		}

		return false;
	}

	/*
	Typically, an Actor is riding a Solid if that Actor is immediately above the Solid.
	Some Actors might want to override this function to change how it behaves — for example:
	 * riding Solids that are ledge grabbed
	 * flying monsters never ride Solids.
	*/
	virtual bool is_riding_solid(Solid* solid) = 0;

	// define the behavior when an Actor is squeezed between two Solids
	virtual void get_squished() = 0;

	void move_x(float dx, CollisionReaction on_collision) 
	{ 
		x_remainder += dx;
		int x_change = Round(x_remainder);
		if (x_change != 0)
		{
			x_remainder -= static_cast<float>(x_change);
			please_move_x(x_change, std::move(on_collision));
		}
	}

	void move_y(float dy, CollisionReaction on_collision) 
	{ 
		y_remainder += dy;
		int y_change = Round(y_remainder);
		if (y_change != 0)
		{
			y_remainder -= static_cast<float>(y_change);
			please_move_y(y_change, std::move(on_collision));
		}
	}

	void please_move_x(int dx, CollisionReaction on_collision)
	{
		const int sign = Sign(dx);
		int steps_to_move = dx;
		while (steps_to_move != 0)
		{
			if (false == collide_at(position + glm::ivec2(sign, 0)))
			{
				position.x += sign;
				steps_to_move -= sign;
			}
			else
			{
				on_collision();
				break;
			}
		}
	}

	void please_move_y(int dy, CollisionReaction on_collision)
	{
		const int sign = Sign(dy);
		int steps_to_move = dy;
		while (steps_to_move != 0)
		{
			if (false == collide_at(position + glm::ivec2(0, sign)))
			{
				position.y += sign;
				steps_to_move -= sign;
			}
			else
			{
				on_collision();
				break;
			}
		}
	}
};


struct Solid : Aabb
{
	Level* level;

	float x_remainder = 0;
	float y_remainder = 0;
	bool is_collidable = true;

	ActorList get_all_riding_actors()
	{
		ActorList r;
		for(auto& actor: level->actors)
		{
			if(actor->is_riding_solid(this))
			{
				r.add(actor);
			}
		}
		return r;
	}

	bool is_overlapping(const std::shared_ptr<Actor>& a)
	{
		return rect_intersect(fyro::get_rect(*this), fyro::get_rect(*a));
	}

	void Move(float x, float y) 
	{
		x_remainder += x;
		y_remainder += y;
		const int dx = Round(x_remainder);
		const int dy = Round(y_remainder);

		if (dx != 0 || dy != 0)
		{
			// Loop through every Actor in the Level, add it to a list if actor.is_riding_solid(this) is true
			// It’s important we do this before we actually move, because the movement could put us out of range for the is_riding_solid checks.
			const auto riding = get_all_riding_actors();
			
			// Make this Solid non-collidable for Actors, so that Actors moved by it do not get stuck on it
			is_collidable = false;

			// todo(Gustav): while loop here as well?

			if (dx != 0)
			{
				x_remainder -= static_cast<float>(dx);
				please_move_x(dx, riding);
			}
			
			if (dy != 0)
			{
				y_remainder -= static_cast<float>(dy);
				please_move_y(dy, riding);
			}
			
			//Re-enable collisions for this Solid
			is_collidable = true;
		}
	}

	void please_move_x(int dx, const ActorList& riding)
	{
		position.x += dx;
		if (dx > 0)
		{
			for(auto& actor: level->actors)
			{
				if (is_overlapping(actor))
				{
					// push
					actor->please_move_x(this->get_right() - actor->get_left(), [&]() { actor->get_squished(); });
				}
				else if (riding.has_actor(actor))
				{
					// carry
					actor->please_move_x(dx, no_collision_reaction);
				}
			}
		}
		else
		{
			for(auto& actor: level->actors)
			{
				if (is_overlapping(actor))
				{
					// push
					actor->please_move_x(this->get_left() - actor->get_right(), [&]() { actor->get_squished(); });
				}
				else if (riding.has_actor(actor))
				{
					// carry
					actor->please_move_x(dx, no_collision_reaction);
				}
			}
		}
	}


	void please_move_y(int dy, const ActorList& riding)
	{
		position.y += dy;
		if (dy > 0)
		{
			for(auto& actor: level->actors)
			{
				if (is_overlapping(actor))
				{
					// push
					actor->please_move_y(this->get_top() - actor->get_bottom(), [&]() { actor->get_squished(); });
				}
				else if (riding.has_actor(actor))
				{
					// carry
					actor->please_move_y(dy, no_collision_reaction);
				}
			}
		}
		else
		{
			for(auto& actor: level->actors)
			{
				if (is_overlapping(actor))
				{
					// push
					actor->please_move_y(this->get_bottom() - actor->get_top(), [&]() { actor->get_squished(); });
				}
				else if (riding.has_actor(actor))
				{
					// carry
					actor->please_move_y(dy, no_collision_reaction);
				}
			}
		}
	}
};


Recti get_rect(const Actor& x) { return x.get_rect(); }
Recti get_rect(const Solid& x) { return x.get_rect(); }

Recti get_rect(const Actor& x, const glm::ivec2& position) { return x.get_rect(position); }
Recti get_rect(const Solid& x, const glm::ivec2& position) { return x.get_rect(position); }


}
