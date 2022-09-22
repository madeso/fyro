// #include <functional>
// #include <vector>
// #include <memory>
// #include <cmath>
// #include <set>

#include "fyro/collision2.h"


namespace fyro
{


int Round(float f)
{
	return static_cast<int>(std::lround(f));
}

int Sign(int x)
{
	if(x > 0) { return 1; }
	else { return -1; }
}


void
Level::register_collision(Actor*, Actor*)
{
}

void Level::update(float dt)
{
	for(auto& actor: actors)
	{
		actor->update(dt);
	}

	for(auto& solid: solids)
	{
		solid->update(dt);
	}
}

void Level::render(std::shared_ptr<lox::Object> arg)
{
	for(auto& solid: solids)
	{
		solid->render(arg);
	}

	for(auto& actor: actors)
	{
		actor->render(arg);
	}
}


void ActorList::add(const std::shared_ptr<Actor>& actor)
{
	actors.insert(actor.get());
}

bool ActorList::has_actor(const std::shared_ptr<Actor>& a) const
{
	return actors.find(a.get()) != actors.end();
}

void no_collision_reaction()
{
}



Aabb::Aabb()
	: position(0, 0)
	, size(10, 10)
{
}


Recti Aabb::get_rect(const glm::ivec2& new_position) const
{
	return size.translate(new_position);
}

Recti Aabb::get_rect() const
{
	return get_rect(position);
}

int Aabb::get_left() const { return get_rect().left; }
int Aabb::get_right() const { return get_rect().right; }
int Aabb::get_top() const { return get_rect().top; }
int Aabb::get_bottom() const { return get_rect().bottom; }



bool Actor::collide_at(const glm::ivec2& new_position)
{
	const auto self = get_rect(new_position);

	for(auto& actor: level->actors)
	{
		if(actor.get() == this) { continue; }
		if(rect_intersect(self, actor->get_rect()))
		{
			level->register_collision(this, actor.get());
		}
	}

	for(auto& solid: level->solids)
	{
		if(solid->is_collidable == false) { continue; }

		if(rect_intersect(self, solid->get_rect()))
		{
			return true;
		}
	}

	return false;
}


bool Actor::move_x(float dx, CollisionReaction on_collision) 
{ 
	x_remainder += dx;
	int x_change = Round(x_remainder);
	if (x_change != 0)
	{
		x_remainder -= static_cast<float>(x_change);
		return please_move_x(x_change, std::move(on_collision));
	}
	else
	{
		return false;
	}
}

bool Actor::move_y(float dy, CollisionReaction on_collision) 
{ 
	y_remainder += dy;
	int y_change = Round(y_remainder);
	if (y_change != 0)
	{
		y_remainder -= static_cast<float>(y_change);
		return please_move_y(y_change, std::move(on_collision));
	}
	else
	{
		return false;
	}
}

bool Actor::please_move_x(int dx, CollisionReaction on_collision)
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
			return true;
		}
	}

	return false;
}

bool Actor::please_move_y(int dy, CollisionReaction on_collision)
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
			return true;
		}
	}

	return false;
}




ActorList Solid::get_all_riding_actors()
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

bool Solid::is_overlapping(const std::shared_ptr<Actor>& actor)
{
	return rect_intersect(get_rect(), actor->get_rect());
}

void Solid::Move(float x, float y) 
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

void Solid::please_move_x(int dx, const ActorList& riding)
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


void Solid::please_move_y(int dy, const ActorList& riding)
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


}
