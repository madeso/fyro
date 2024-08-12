#pragma once

#include "fyro/rect.h"
#include "fyro/render/texture.h"

struct Sprite
{
	Sprite();

	std::shared_ptr<render::Texture> texture;
	Rectf screen;
	Rectf uv;
};

struct SpriteAnimation
{
	float speed = 0.0f;
	float accum = 0.0f;
	int current_index = 0;
	int total_sprites = 0;

	void setup(float a_speed, int a_total_sprites);
	void update(float dt);
};

struct ScriptSprite
{
	std::vector<Sprite> sprites;
	std::shared_ptr<SpriteAnimation> animation;	 // may be shared between sprites

	ScriptSprite();
};
