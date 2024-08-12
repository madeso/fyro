#include "fyro/sprite.h"

Sprite::Sprite()
	: screen(1.0f, 1.0f)
	, uv(1.0f, 1.0f)
{
}

void SpriteAnimation::setup(float a_speed, int a_total_sprites)
{
	speed = a_speed;
	total_sprites = a_total_sprites;

	// todo(Gustav): randomize!
	// accum = make_random(...);
	// current_index = make_random(...);
}

void SpriteAnimation::update(float dt)
{
	assert(total_sprites >= 0);
	if (total_sprites == 0)
	{
		return;
	}

	accum += dt;
	while (accum > speed)
	{
		accum -= speed;
		current_index += 1;
		current_index = current_index % total_sprites;
	}
}

ScriptSprite::ScriptSprite()
	: animation(std::make_shared<SpriteAnimation>())
{
}
