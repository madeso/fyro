#include "fyro/fyro.h"

#include <array>

#include "fyro/random.h"

constexpr std::array<int, 64> endesga64 =
{
	0xff0040, 0x131313, 0x1b1b1b, 0x272727, 0x3d3d3d, 0x5d5d5d, 0x858585, 0xb4b4b4,
	0xffffff, 0xc7cfdd, 0x92a1b9, 0x657392, 0x424c6e, 0x2a2f4e, 0x1a1932, 0x0e071b,
	0x1c121c, 0x391f21, 0x5d2c28, 0x8a4836, 0xbf6f4a, 0xe69c69, 0xf6ca9f, 0xf9e6cf,
	0xedab50, 0xe07438, 0xc64524, 0x8e251d, 0xff5000, 0xed7614, 0xffa214, 0xffc825,
	0xffeb57, 0xd3fc7e, 0x99e65f, 0x5ac54f, 0x33984b, 0x1e6f50, 0x134c4c, 0x0c2e44,
	0x00396d, 0x0069aa, 0x0098dc, 0x00cdf9, 0x0cf1ff, 0x94fdff, 0xfdd2ed, 0xf389f5,
	0xdb3ffd, 0x7a09fa, 0x3003d9, 0x0c0293, 0x03193f, 0x3b1443, 0x622461, 0x93388f,
	0xca52c9, 0xc85086, 0xf68187, 0xf5555d, 0xea323c, 0xc42430, 0x891e2b, 0x571c27
};

struct ColorFlips
{
	static constexpr int width = 100;
	static constexpr int height = 100;
	static constexpr float update_time = 15.0f;
	static constexpr float max_time = 15.0;
	static constexpr float size = 1.7f;
	static constexpr float spacing = 2.0f;
	static constexpr float startx = 0.0f;
	static constexpr float starty = 0.0f;

	float timer[width * height] = { 0.0f, };
	std::size_t index[width * height] = { 0, };

	ColorFlips()
	{
		auto r = Random{};

		for(std::size_t i=0; i<width*height; i+=1) { timer[i] = r.get_including(max_time); }
		for(std::size_t i=0; i<width*height; i+=1) { index[i] = r.get_excluding(endesga64.size()); }
	}

	void update(float dt)
	{
		for(std::size_t i=0; i<width*height; i+=1)
		{
			timer[i] -= dt;
			if(timer[i] < 0.0f)
			{
				timer[i] += update_time;
				index[i] = (index[i] + 1) % endesga64.size();
			}
		}
	}

	void render(render::SpriteBatch* batch)
	{
		const auto color = [](int c, int s) -> float
		{
			return static_cast<float>((c >> s) & 0xff) / 255.0f;
		};

		for(std::size_t x=0; x<width; x+=1)
		for(std::size_t y=0; y<height; y+=1)
		{
			const auto color_index = index[x * width + y];
			const auto c = endesga64[color_index];
			batch->quad
			(
				{},
				Rectf{size, size}
					.translate(startx, starty)
					.translate(static_cast<float>(x)*spacing, static_cast<float>(y)*spacing),
				{}, {color(c, 2), color(c, 1), color(c, 0), 1.0f});
		}
	}
};

#if 0
struct ExampleGame : public State
{
	render::Texture cards;
	render::Texture letter_g;
	ColorFlips flips;
	glm::vec2 mouse;

	ExampleGame()
		: cards
		(
			::cards::load_texture()
		)
		, letter_g
		(
			render::load_image_from_embedded
			(
				LETTER_G_PNG,
				render::TextureEdge::clamp,
				render::TextureRenderStyle::smooth,
				render::Transparency::include
			)
		)
		, mouse(0, 0)
	{
	}

	void
	on_update(float dt) override
	{
		flips.update(dt);
	}

	void
	on_render(const render::RenderCommand& rc) override
	{
		auto r = render::with_layer2(rc, render::LayoutData{render::ViewportStyle::extended, 200.0f, 200.0f});

		r.batch->quad({}, r.viewport_aabb_in_worldspace, {}, {0.8, 0.8, 0.8, 1.0f});

		r.batch->submit();

		flips.render(r.batch);

		constexpr auto card_sprite = Cint_to_float(::cards::back).zero().set_height(30);

		r.batch->quad(&cards, card_sprite.translate(100, 100), ::cards::hearts[2]);
		r.batch->quad(&cards, card_sprite.translate(r.mouse_to_world(mouse)), ::cards::back);

		r.batch->quad(&letter_g, Rectf{40, 40}.translate(40, 40), {});
	}

	void on_mouse_position(const render::InputCommand&, const glm::ivec2& p) override
	{
		// todo(Gustav): store world mouse?
		mouse = {p.x, p.y};
	}
};
#else
struct ExampleGame : public State
{
	ColorFlips flips;
	glm::vec2 mouse;

	ExampleGame()
		: mouse(0, 0)
	{
	}

	void
	on_update(float dt) override
	{
		flips.update(dt);
	}

	void
	on_render(const render::RenderCommand& rc) override
	{
		auto r = render::with_layer2(rc, render::LayoutData{render::ViewportStyle::extended, 200.0f, 200.0f});

		r.batch->quad({}, r.viewport_aabb_in_worldspace, {}, {0.8, 0.8, 0.8, 1.0f});

		r.batch->submit();

		flips.render(r.batch);
	}

	void on_mouse_position(const render::InputCommand&, const glm::ivec2& p) override
	{
		// todo(Gustav): store world mouse?
		mouse = {p.x, p.y};
	}
};
#endif

int
main(int, char**)
{
	return run_game
	(
		"Example", glm::ivec2{800, 600}, false, []()
		{
			auto game = std::make_shared<Game>();
			game->state = std::make_shared<ExampleGame>();
			return game;
		}
	);
}
