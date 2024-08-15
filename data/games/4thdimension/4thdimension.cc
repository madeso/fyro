#include "fyro/fyro.h"
#include "sprites/onebit.h"
#include "sprites/onebit_font.h"
#include "fyro/cint.h"
#include "fyro/random.h"

#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <set>

constexpr float sprite_square = 40.0f;
constexpr Rectf sprite_size = {sprite_square, sprite_square};
constexpr auto font = onebit::font{30.0f};
constexpr int width = 800;
constexpr int height = 600;
constexpr float fade_time_intro = 2;
constexpr float fade_time_outro = 1;

struct render_data
{
	Rectf viewport_aabb_in_worldspace;
	render::Texture* onebit;
	render::SpriteBatch* batch;
};

using namespace std;


void Click();

struct Menu;

struct game_settings
{
	bool play_against_computer = true;
	bool use_bad_ai = true;
	bool use_hard_mode = false;
};

struct Label
{
	Label(const std::string& iText, float iX, float iY)
		: text(iText)
		, x(iX)
		, y(iY)
	{
	}

	void render(const render_data& data)
	{
		float gray = 0.6f;
		font.simple_text(data.batch, data.onebit, {gray, gray, gray, 1}, x, y, text, onebit::no_text_animation{});
	}

	std::string text;
	float x;
	float y;
};

struct Button
{
	Button(const std::string& iText, float iX, float iY)
		: text(iText)
		, x(iX)
		, y(iY)
		, over(false)
	{
	}

	void render(const render_data& data) const
	{
		constexpr glm::vec4 over_color = {0, 0, 1, 1};
		constexpr glm::vec4 default_color = {1, 1, 1, 1};
		const auto color = over ? over_color : default_color;
		font.simple_text(data.batch, data.onebit, color, x, y, text, onebit::no_text_animation{});
	}

	bool test(const glm::vec2& mouse)
	{
		const float w = font.get_width_of_string(text);
		const float h = font.size;
		const auto rect = ::Rectf{x, y, x + w, y + h};
		over = rect.is_inside_inclusive(mouse.x, mouse.y);
		return over;
	}

	std::string text;
	float x;
	float y;
	bool over;
};

struct Menu
{
	Menu()
		: state(0)
		, mouse_was_down(false)
		, hardAi("human versus hard ai", 250, 200)
		, easyAi("human versus easy ai", 250, 250)
		, noAi("human versus human", 250, 300)
		, aiLabel("select gameplay:", 200, 100)
		, allRules("use all rules", 150, 350)
		, allRulesDesc(" - only for 4d masters", 200, 400)
		, mostRules("use most rules", 150, 200)
		, mostRulesDesc(" ignores most trans-square rules\n - recommended for beginners", 200, 250)
		, rulesLabel("select how many rules you want to use", 100, 100)
	{
	}

	std::function<void()> on_start_game;

	void update(game_settings* gd, const glm::vec2& mouse, bool mouse_is_down, float)
	{
		const auto clicked = !mouse_is_down && mouse_was_down;
		mouse_was_down = mouse_is_down;

		if (state == 0)
		{
			if (hardAi.test(mouse) && clicked)
			{
				++state;
				gd->play_against_computer = true;
				gd->use_bad_ai = false;
				Click();
			}
			else if (easyAi.test(mouse) && clicked)
			{
				++state;
				gd->play_against_computer = true;
				gd->use_bad_ai = true;
				Click();
			}
			else if (noAi.test(mouse) && clicked)
			{
				++state;
				gd->play_against_computer = false;
				gd->use_bad_ai = false;
				Click();
			}
		}
		else if (state == 1)
		{
			if (allRules.test(mouse) && clicked)
			{
				state++;
				Click();
				gd->use_hard_mode = true;
			}
			else if (mostRules.test(mouse) && clicked)
			{
				state++;
				Click();
				gd->use_hard_mode = false;
			}
		}
		else
		{
			on_start_game();
			state = 0;
		}
	}

	void render(const render_data& rd)
	{
		if (state == 0)
		{
			aiLabel.render(rd);
			hardAi.render(rd);
			easyAi.render(rd);
			noAi.render(rd);
		}
		else
		{
			allRules.render(rd);
			allRulesDesc.render(rd);
			mostRules.render(rd);
			mostRulesDesc.render(rd);
			rulesLabel.render(rd);
		}
	}

	int state;
	bool mouse_was_down;

	Button hardAi;
	Button easyAi;
	Button noAi;
	Label aiLabel;

	Button allRules;
	Label allRulesDesc;
	Button mostRules;
	Label mostRulesDesc;
	Label rulesLabel;
};

struct cursor;

void ExecuteComputerMove(cursor* cur, const game_settings& gd, Random* rand);

struct Object
{
	Object()
		: should_kill(false)
	{
	}

	virtual ~Object() = default;

	virtual void render(const render_data&)
	{
	}

	// const foo_global_data& gd, float delta, bool interact, const glm::vec2& mouse, bool down
	virtual void update(const game_settings&, float, bool, const glm::vec2&, bool)
	{
	}

	void kill()
	{
		should_kill = true;
	}

	bool should_kill;
};

struct Background : Object
{
	Background(const std::string&)
	{
	}

	void render(const render_data& rd) override
	{
		// sprite->RenderStretch(0, 0, WIDTH, HEIGHT);
		rd.batch->quad({}, rd.viewport_aabb_in_worldspace, {}, {0.8, 0.8, 0.8, 1.0f});
	}
};

struct FullscreenColorSprite : Object
{
	FullscreenColorSprite(const glm::vec4& iColor) : color(iColor)
	{
	}

	void render(const render_data& data) override
	{
		data.batch->quad({}, {width, height}, {}, color);
	}

	glm::vec4 color;
};

struct FadeFromBlack : FullscreenColorSprite
{
	FadeFromBlack(float iTime)
		: FullscreenColorSprite({0, 0, 0, 1})
		, time(iTime)
	{
	}

	void update(const game_settings&, float delta, bool, const glm::vec2&, bool) override
	{
		color.a -= delta / time;
		if (color.a <= 0.0f)
		{
			kill();
		}
		color.a = std::max(color.a, 0.0f);
	}

	const float time;
};

struct FadeToBlackAndExit : FullscreenColorSprite
{
	const float time;
	std::function<void()> callback;

	FadeToBlackAndExit(float iTime, std::function<void ()>&& f)
		: FullscreenColorSprite({0, 0, 0, 0})
		, time(iTime)
		, callback(f)
	{
	}

	void update(const game_settings&, float delta, bool, const glm::vec2&, bool) override
	{
		color.a += delta / time;
		if (color.a > 1.0f)
		{
			callback();
		}
		color.a = std::max(color.a, 0.0f);
	}

};


struct SuggestedLocation
{
	SuggestedLocation()
	{
		clear();
	}

	void set(float x, float y)
	{
		valid = true;
		mx = x;
		my = y;
	}

	void clear()
	{
		valid = false;
	}

	bool get(float* out_x, float* out_y)
	{
		if (valid)
		{
			float dx = 0;
			float dy = 0;
			*out_x = mx + dx;
			*out_y = my + dy;
			return true;
		}
		return false;
	}

private:
	float mx;
	float my;
	bool valid;
};


enum class cross_or_circle
{
	neither,
	cross,
	circle,
	ai
};

struct cursor
{
	cross_or_circle cursor_state = cross_or_circle::cross;

	void set_complete_cursor()
	{
		cursor_state = cross_or_circle::neither;
	}

	void set_start_cursor()
	{
		cursor_state = cross_or_circle::cross;
	}

	void flip_state(const game_settings& settings)
	{
		if (cursor_state == cross_or_circle::cross)
		{
			if (settings.play_against_computer)
			{
				cursor_state = cross_or_circle::ai;
			}
			else
			{
				cursor_state = cross_or_circle::circle;
			}
		}
		else
		{
			cursor_state = cross_or_circle::cross;
		}
	}
};


struct IconPlacer : Object
{
	IconPlacer(cursor* c, SuggestedLocation* l)
		: cur(c)
		, gSuggestedLocation(l)
	{
	}

	cursor* cur;
	SuggestedLocation* gSuggestedLocation;

	void render(const render_data& data) override
	{
		std::optional<Recti> cursor = onebit::cross;

		if (cur->cursor_state == cross_or_circle::circle)
		{
			cursor = onebit::circle;
		}

		if (cur->cursor_state == cross_or_circle::neither)
		{
			cursor = {};
		}

		if (cur->cursor_state == cross_or_circle::ai)
		{
			cursor = {}; // hourglass ?
		}

		float mx = 0;
		float my = 0;
		if (cursor && gSuggestedLocation->get(&mx, &my))
		{
			// if a location isn't suggested, the data isn't modified
			data.batch->quad(data.onebit, sprite_size.translate(mx, my), *cursor);
		}
	}
};

struct Index
{
	Index()
		: cube(-1)
		, column(-1)
		, row(-1)
	{
	}

	Index(int cu, int co, int ro)
		: cube(cu)
		, column(co)
		, row(ro)
	{
	}

	bool operator==(const Index& i) const
	{
		return cube == i.cube &&
			column == i.column &&
			row == i.row;
	}

	void operator+=(const Index& i)
	{
		cube += i.cube;
		column += i.column;
		row += i.row;
	}

	void clear()
	{
		cube = -1;
		column = -1;
		row = -1;
	}

	int cube;
	int column;
	int row;
};

struct World;

struct WinningCombination
{
	Index combination[4];
	cross_or_circle test(const World& iWorld) const;
};

typedef std::function<WinningCombination ()> TestWinningConditionFunction;

struct World
{
	bool is_modified;
	cross_or_circle state[4][4][4];
	std::vector<TestWinningConditionFunction> winning_conditions;

	World()
		: is_modified(false)
	{
		clear();
	}

	cross_or_circle get_state(const Index& index) const
	{
		return state[index.cube][index.column][index.row];
	}

	void set_state(int cube, int col, int row, cross_or_circle new_state)
	{
		state[cube][col][row] = new_state;
		is_modified = true;
	}

	void clear()
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				for (int k = 0; k < 4; ++k)
				{
					state[i][j][k] = cross_or_circle::neither;
				}
			}
		}
		is_modified = true;
	}

	bool can_place_marker() const
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				for (int k = 0; k < 4; ++k)
				{
					if (state[i][j][k] == cross_or_circle::neither)
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	bool hasBeenModifiedSinceLastCall()
	{
		bool modified = is_modified;
		is_modified = false;
		return modified;
	}

	cross_or_circle testWinningConditions(WinningCombination* oCombo) const
	{
		const std::size_t count = winning_conditions.size();
		for (std::size_t i = 0; i < count; ++i)
		{
			const WinningCombination combo = winning_conditions[i]();
			const cross_or_circle result = combo.test(*this);
			if (result != cross_or_circle::neither)
			{
				*oCombo = combo;
				return result;
			}
		}

		return cross_or_circle::neither;
	}

	void add_winning_condition(TestWinningConditionFunction&& condition)
	{
		winning_conditions.emplace_back(condition);
	}

	vector<WinningCombination> get_winning_conditions() const
	{
		vector<WinningCombination> conditions;
		for (const auto& cond: winning_conditions)
		{
			conditions.emplace_back(cond());
		}
		return conditions;
	}

	bool is_place_free(Index index) const
	{
		return state[index.cube][index.column][index.row] == cross_or_circle::neither;
	}
};


Index gCurrent;

cross_or_circle WinningCombination::test(const World& iWorld) const
{
	cross_or_circle first = iWorld.get_state(combination[0]);
	for (int i = 1; i < 4; ++i)
	{
		if (first != iWorld.get_state(combination[i]))
		{
			return cross_or_circle::neither;
		}
	}

	return first;
}



void place_marker(cursor* cur, const game_settings& gd, World* world, int cube, int col, int row);

struct Part
{
	int cube;
	int row;
	int col;

	float* ox;
	float* oy;
	World* world;

	float extraScale = 0;
	float direction = 1;
	bool animateIntro = true;
	int* pulsating_index;

	Part(World* iWorld, float* iox, float* ioy, int icube, int r, int c, int* pc)
		: cube(icube)
		, row(r)
		, col(c)
		, ox(iox)
		, oy(ioy)
		, world(iWorld)
		, pulsating_index(pc)
	{
	}

	void start()
	{
		animateIntro = true;
	}

	void render(const render_data& rd) const
	{
		const auto state = world->get_state({cube, col, row});
		const auto sprite = state == cross_or_circle::neither
			? onebit::box
			: (
				state == cross_or_circle::cross
					? onebit::cross
					: onebit::circle
			);

		const auto extra_size = sprite_size.get_width() * (1 + extraScale) - sprite_size.get_width();

		rd.batch->quad
		(
			rd.onebit, sprite_size.translate
			(
				*ox + Cint_to_float(col) * sprite_size.get_width(),
				*oy + Cint_to_float(row) * sprite_size.get_height()
			).extend(extra_size), sprite
		);
	}

	[[nodiscard]] const Recti* getSprite() const
	{
		const auto state = world->get_state({cube, col, row});
		if (state != cross_or_circle::neither)
		{
			if (state == cross_or_circle::cross)
			{
				return &onebit::cross;
			}
			return &onebit::circle;
		}
		return nullptr;
	}

	void update(WinningCombination* combo, float delta)
	{
		constexpr auto speed = 5.0f;
		constexpr auto min = 0.0f;
		constexpr auto max = 2.0f;

		if (combo)
		{
			const auto scale = combo && combo->combination[*pulsating_index] == Index{ cube, col, row };

			if (!scale)
			{
				return;
			}


			extraScale += delta * max * speed * direction * 0.7f;
			if (extraScale > max)
			{
				extraScale = max;
				direction = -1;
			}
			if (extraScale < min)
			{
				extraScale = min;
				direction = 1;
				*pulsating_index += 1;
				if (*pulsating_index == 4)
				{
					*pulsating_index = 0;
				}
			}
		}
		else
		{
			const auto* sprite = getSprite();
			if (sprite && animateIntro)
			{
				extraScale = 4;
				animateIntro = false;
			}

			if (extraScale > 0)
			{
				extraScale -= delta * 4 * speed;
				if (extraScale < 0)
				{
					extraScale = 0;
				}
			}
		}
	}

	void test_placements(cursor* cur, const game_settings& settings, SuggestedLocation* suggested_location, float mx, float my, bool mouse) const
	{
		const cross_or_circle state = world->get_state({cube, col, row});
		if (state == cross_or_circle::neither)
		{
			const auto w = sprite_size.get_width();
			const auto h = sprite_size.get_height();
			const auto rx = *ox + Cint_to_float(col) * w;
			const auto ry = *oy + Cint_to_float(row) * h;
			const auto rect = ::Rectf{rx, ry, rx + w, ry + h};
			const auto over = rect.is_inside_inclusive(mx, my);
			const Index me(cube, col, row);

			if (mouse && over)
			{
				gCurrent = me;
			}

			if (!mouse && over && me == gCurrent)
			{
				place_marker(cur, settings, world, cube, col, row);
			}

			if (over)
			{
				suggested_location->set(rx, ry);
			}
		}
	}
};

void place_marker(cursor* cur, const game_settings& gd, World* world, int cube, int col, int row)
{
	if (cur->cursor_state == cross_or_circle::circle || cur->cursor_state == cross_or_circle::cross)
	{
		world->set_state(cube, col, row, cur->cursor_state);
		cur->flip_state(gd);
		gCurrent.clear();
		Click();
	}
}

std::array<Part, 16> make_parts(World* world, float* x, float* y, const int cube, int* pc)
{
	const auto w = [=](int row, int col) -> Part
	{
		return {world, x, y, cube, row-1, col-1, pc};
	};
	return
	{
		w(1, 1), w(1, 2), w(1, 3), w(1, 4),
		w(2, 1), w(2, 2), w(2, 3), w(2, 4),
		w(3, 1), w(3, 2), w(3, 3), w(3, 4),
		w(4, 1), w(4, 2), w(4, 3), w(4, 4)
	};
}

struct Cube : Object
{
	float x;
	float y;
	World& world;
	const int cube;
	WinningCombination** combo;
	std::array<Part, 16> parts;

	Cube(float ix, float iy, World& iWorld, int iCube, WinningCombination** icombo, int* pc)
		: x(ix)
		, y(iy)
		, world(iWorld)
		, cube(iCube)
		, combo(icombo)
		, parts(make_parts(&world, &x, &y, cube, pc))
	{
	}

	static size_t index(int col, int row)
	{
		return Cint_to_sizet(row * 4 + col);
	}

	void render(const render_data& rd) override
	{
		constexpr float g = 0.5f;
		constexpr auto s = sprite_size.get_width() * 4.0f;
		constexpr auto back = Rectf{s, s};
		rd.batch->quad({}, back.translate(x, y), {}, {g, g, g, 1.0f});
		for (int col = 0; col < 4; ++col)
		{
			for (int row = 0; row < 4; ++row)
			{
				parts[index(col, row)].render(rd);
			}
		}
	}

	void start()
	{
		for (int col = 0; col < 4; ++col)
		{
			for (int row = 0; row < 4; ++row)
			{
				parts[index(col, row)].start();
			}
		}
	}

	void update(const game_settings&, float delta, bool, const glm::vec2&, bool) override
	{
		for (int col = 0; col < 4; ++col)
		{
			for (int row = 0; row < 4; ++row)
			{
				parts[index(col, row)].update(*combo, delta);
			}
		}
	}

	void test_placements(cursor* cur, const game_settings& gd, SuggestedLocation* location, float mx, float my, bool mouse) const
	{
		for (int col = 0; col < 4; ++col)
		{
			for (int row = 0; row < 4; ++row)
			{
				parts[index(col, row)].test_placements(cur, gd, location, mx, my, mouse);
			}
		}
	}
};

struct StartStep
{
	int start;
	int step;

	StartStep(int iStart, int iStep)
		: start(iStart)
		, step(iStep)
	{
	}

	StartStep(int iStep)
		: start(0)
		, step(iStep)
	{
		if (iStep < 0)
		{
			start = 3;
		}
	}
};

struct WinningConditionModular
{
	WinningConditionModular(const StartStep& cube, const StartStep& col, const StartStep& row)
		: start(cube.start, col.start, row.start)
		, step(cube.step, col.step, row.step)
	{
	}

	WinningCombination operator()()
	{
		WinningCombination combo;
		Index index = start;
		for (int i = 0; i < 4; ++i)
		{
			combo.combination[i] = index;
			index += step;
		}

		return combo;
	}

	Index start;
	Index step;
};

struct PressKeyToContinue : Object
{
	float time = 0;
	bool interact = false;

	void update(const game_settings&, float delta, bool iInteract, const glm::vec2&, bool) override
	{
		interact = !iInteract;
		if (interact)
		{
			time += delta;
			if (time > 1.0f)
			{
				time -= 1.0f;
			}
		}
	}

	void render(const render_data& rd) override
	{
		if (interact)
		{
			const std::string text = "click to play again";
			constexpr auto factor = 0.5f;

			const float PI = 3.14159265f;
			const float max = 2 * PI;

			const auto sin01 = (sin(time * max)+1) / 2.0f;
			const auto alpha = factor + (1-factor) * sin01;
			// fmt::print("factor is {} {}\n", sin01, alpha);
			font.simple_text
			(
				rd.batch, rd.onebit, {0, 0, 0, alpha}, (width - font.get_width_of_string(text)) / 2.0f, 35.0f, text,
				onebit::siny_animation{5.0f, 0.1f, time}
			);
		}
	}
};

struct OldGame;
void AddStartNewGameFader(OldGame& iGame);

struct AiPlacerObject : Object
{
	float thinkTime;
	Random* rand;
	cursor* cur;

	AiPlacerObject(Random* r, cursor* c)
		: thinkTime(0.35f)
		, rand(r)
		, cur(c)
	{
	}

	void update(const game_settings& gd, float delta, bool interact, const glm::vec2&, bool) override
	{
		if (interact)
		{
			thinkTime -= delta;
			if (thinkTime < 0)
			{
				kill();
				ExecuteComputerMove(cur, gd, rand);
			}
		}
	}
};

struct OldGame
{
	int pulsating_index = 0;
	cursor current_cursor;

	SuggestedLocation suggested_location;
	OldGame()
		: quit(false)
		, is_quiting(false)
		, has_result(false)
		, combo(nullptr)
		, is_interactive(true)
		, ai_has_moved(false)
	{
		add(std::make_shared<FullscreenColorSprite>(glm::vec4{0.8f, 0.8f, 0.8f, 1.0f}));
		for (int i = 0; i < 4; ++i)
		{
			add
			(
				cube[i] = std::make_shared<Cube>
				(
					Cint_to_float(25 + i * 193),
					Cint_to_float(48 + i * 126),
					world, i, &combo, &pulsating_index
				)
			);
		}
		add(std::make_shared<PressKeyToContinue>());
		add(std::make_shared<IconPlacer>(&current_cursor, &suggested_location));
		add(std::make_shared<FadeFromBlack>(fade_time_intro));
	}

	void buildRules(const game_settings& gd)
	{
		for (int cube_index = 0; cube_index < 4; ++cube_index)
		{
			world.add_winning_condition(WinningConditionModular(StartStep(cube_index, 0), StartStep(0, 1), StartStep(0, 1)));
			world.add_winning_condition(WinningConditionModular(StartStep(cube_index, 0), StartStep(0, 1), StartStep(3, -1)));
			for (int i = 0; i < 4; ++i)
			{
				world.add_winning_condition(WinningConditionModular(StartStep(cube_index, 0), StartStep(i, 0), StartStep(0, 1)));
				world.add_winning_condition(WinningConditionModular(StartStep(cube_index, 0), StartStep(0, 1), StartStep(i, 0)));
			}
		}
		if (gd.use_hard_mode)
		{
			world.add_winning_condition(WinningConditionModular(StartStep(0, 1), StartStep(0, 1), StartStep(0, 1)));
			world.add_winning_condition(WinningConditionModular(StartStep(0, 1), StartStep(0, 1), StartStep(3, -1)));

			world.add_winning_condition(WinningConditionModular(StartStep(3, -1), StartStep(0, 1), StartStep(0, 1)));
			world.add_winning_condition(WinningConditionModular(StartStep(3, -1), StartStep(0, 1), StartStep(3, -1)));
			for (int i = 0; i < 4; ++i)
			{
				world.add_winning_condition(WinningConditionModular(StartStep(0, 1), StartStep(i, 0), StartStep(0, 1)));
				world.add_winning_condition(WinningConditionModular(StartStep(0, 1), StartStep(0, 1), StartStep(i, 0)));

				world.add_winning_condition(WinningConditionModular(StartStep(0, 1), StartStep(i, 0), StartStep(3, -1)));
				world.add_winning_condition(WinningConditionModular(StartStep(0, 1), StartStep(3, -1), StartStep(i, 0)));
			}
		}

		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				world.add_winning_condition(WinningConditionModular(StartStep(0, 1), StartStep(col, 0), StartStep(row, 0)));
			}
		}
	}

	void render(const render_data& rd)
	{
		const std::size_t length = mObjects.size();
		for (std::size_t i = 0; i < length; ++i)
		{
			mObjects[i]->render(rd);
		}
	}

	void clearBoard()
	{
		world.clear();
		has_result = false;
		combo = nullptr;
		for (int c = 0; c < 4; ++c)
		{
			cube[c]->start();
		}
	}

	void newGame()
	{
		current_cursor.set_start_cursor();
		is_interactive = true;
	}

	void click()
	{
		fmt::print("sfx: click\n");
	}

	void cheer()
	{
		fmt::print("sfx: cheer\n");
	}

	void on_escape_key()
	{
		if (!is_quiting)
		{
			add(std::make_shared<FadeToBlackAndExit>(fade_time_outro, [this]() {quit = false; }));
			is_quiting = true;
		}
	}

	void update(const game_settings& gd, Random* rand, bool mouse, bool oldMouse, float delta, const glm::vec2& mouse_position, bool enter_state)
	{
		suggested_location.clear();

		if (is_interactive && has_result && mouse == false && mouse != oldMouse)
		{
			AddStartNewGameFader(*this);
			is_interactive = false;
		}
		const bool interact = !has_result && is_interactive;
		const std::size_t length = mObjects.size();

		for (std::size_t i = 0; i < length; ++i)
		{
			mObjects[i]->update(gd, delta, interact, mouse_position, mouse);
		}
		mObjects.erase
		(
			std::remove_if
			(
				mObjects.begin(), mObjects.end(),
				[](const std::shared_ptr<Object>&object)
				{
					return object->should_kill;
				}
			),
			mObjects.end()
		);

		if (interact)
		{
			for(int i=0; i<4; i+=1)
			{
				cube[i]->test_placements(&current_cursor, gd, &suggested_location, mouse_position.x, mouse_position.y, mouse);
			}
		}

		if (is_interactive && enter_state)
		{
			display_no_winner();
		}

		if (!mouse)
		{
			gCurrent.clear();
		}

		if (world.hasBeenModifiedSinceLastCall())
		{
			WinningCombination winning_combo;
			const auto result = world.testWinningConditions(&winning_combo);
			if (result != cross_or_circle::neither)
			{
				display_winner(winning_combo);
			}
			else if (!world.can_place_marker())
			{
				display_no_winner();
			}
		}

		if (is_interactive && gd.play_against_computer)
		{
			if (current_cursor.cursor_state == cross_or_circle::ai && !ai_has_moved)
			{
				add(std::make_shared<AiPlacerObject>(rand, &current_cursor));
				ai_has_moved = true;
			}
			else if (current_cursor.cursor_state != cross_or_circle::ai)
			{
				ai_has_moved = false;
			}
		}
	}

	void display_no_winner()
	{
		has_result = true;
		winning_combination = WinningCombination{};
		current_cursor.set_complete_cursor();
		pulsating_index = 0;
		combo = nullptr;
	}

	void display_winner(const WinningCombination& new_combo)
	{
		has_result = true;
		winning_combination = new_combo;
		current_cursor.set_complete_cursor();
		pulsating_index = 0;
		combo = &winning_combination;
		cheer();
	}

	void add(std::shared_ptr<Object> object)
	{
		mObjects.push_back(object);
	}

	vector<std::shared_ptr<Object>> mObjects;
	bool quit;
	bool is_quiting;
	World world;
	std::shared_ptr<Cube> cube[4];
	bool has_result;
	WinningCombination winning_combination;
	WinningCombination* combo;
	bool is_interactive;
	bool ai_has_moved;
};

OldGame* gGame = nullptr;

void ComputerPlaceMarker(cursor* cur, const game_settings& agd, int cube, int col, int row)
{
	auto gd = agd;
	gd.play_against_computer = false;
	cur->cursor_state = cross_or_circle::circle;
	[[maybe_unused]] const bool free = gGame->world.is_place_free({cube, col, row});
	ASSERT(free);
	place_marker(cur, gd, &gGame->world, cube, col, row);
}

float GetWinFactorPlacement(const WinningCombination& combo)
{
	float value = 0;
	for (int i = 0; i < 4; ++i)
	{
		switch (gGame->world.get_state(combo.combination[i]))
		{
		case cross_or_circle::circle:
			value += 1;
			break;
		case cross_or_circle::neither:
			break;
		case cross_or_circle::cross:
			return 0;
		default:
			return -1;
		}
	}

	return value;
}

float GetStopFactorPlacement(const WinningCombination& combo)
{
	float value = 1;
	for (int i = 0; i < 4; ++i)
	{
		switch (gGame->world.get_state(combo.combination[i]))
		{
		case cross_or_circle::circle:
			return 0;
			break;
		case cross_or_circle::cross:
			value += 1;
			break;
		case cross_or_circle::neither:
			break;
		default:
			return -1;
		}
	}

	if (value > 1) return value;
	return 0;
}

bool ExecutePlaceWin(cursor* cur, const game_settings& gd, Random* rand)
{
	vector<WinningCombination> combinations = gGame->world.get_winning_conditions();
	WinningCombination bestCombo;
	float bestFactor = -1;
	for (const auto& combo: combinations)
	{
		auto is_place_free = [&](int j) -> bool
		{
			return gGame->world.is_place_free(combo.combination[j]);
		};

		if (is_place_free(0) || is_place_free(1) || is_place_free(2) || is_place_free(3))
		{
			const auto factor = GetWinFactorPlacement(combo) + GetStopFactorPlacement(combo);
			if (bestFactor < 0 || (factor > bestFactor && rand->get_excluding(100) < 73))
			{
				bestFactor = factor;
				bestCombo = combo;
			}
		}
	}

	if (bestFactor > 0)
	{
		int index = 0;
		do
		{
			index = rand->get_excluding(4);
		}
		while (!gGame->world.is_place_free(bestCombo.combination[index]));
		const auto place = bestCombo.combination[index];
		ComputerPlaceMarker(cur, gd, place.cube, place.column, place.row);
		return true;
	}

	return false;
}

int GenerateRandomPlace(Random* r)
{
	return r->get_excluding(4);
}

void ExecuteRandomPlacement(cursor* cur, const game_settings& gd, Random* r)
{
	int cube = -1;
	int col = -1;
	int row = -1;
	do
	{
		cube = GenerateRandomPlace(r);
		col = GenerateRandomPlace(r);
		row = GenerateRandomPlace(r);
	}
	while (!gGame->world.is_place_free({cube, col, row}));
	ComputerPlaceMarker(cur, gd, cube, col, row);
}

typedef int placing_factor;

placing_factor GetPlacingFactor(const Index& i, const vector<WinningCombination>& rules)
{
	placing_factor factor = 0;
	for (std::size_t ruleIndex = 0; ruleIndex < rules.size(); ++ruleIndex)
	{
		bool hasMe = false;
		bool hasFree = false;
		for (int placementIndex = 0; placementIndex < 4; ++placementIndex)
		{
			if (gGame->world.is_place_free(rules[ruleIndex].combination[placementIndex]))
			{
				hasFree = true;
			}
			if (rules[ruleIndex].combination[placementIndex] == i)
			{
				hasMe = true;
			}
		}
		if (hasMe && hasFree)
		{
			// this place is worth taking a deeper look into
			int crosses = 0;
			int circles = 0;
			for (int placementIndex = 0; placementIndex < 4; ++placementIndex)
			{
				const Index at = rules[ruleIndex].combination[placementIndex];
				switch (gGame->world.get_state(at))
				{
				case cross_or_circle::circle:
					circles += 1;
					break;
				case cross_or_circle::cross:
					crosses += 1;
					break;
				default:
					break;
				}
			}

			placing_factor aloneCircles = 0;
			placing_factor aloneCrosses = 0;
			if (circles == 0)
			{
				aloneCrosses = crosses;
			}
			if (crosses == 0)
			{
				aloneCircles = circles;
			}
			if (aloneCrosses == 3)
			{
				aloneCrosses *= 500;
			}
			if (aloneCrosses >= 2)
			{
				aloneCrosses *= aloneCrosses;
			}
			factor += aloneCircles * aloneCircles + aloneCrosses;
		}
	}

	return factor;
}

struct Placement
{
	Placement(const vector<WinningCombination>& rules, const Index& i)
		: index(i)
		, factor(0)
	{
		if (gGame->world.is_place_free(index))
		{
			factor = GetPlacingFactor(index, rules);
		}
		else
		{
			factor = -1;
		}
	}

	Index index;
	placing_factor factor;

	bool operator<(const Placement& i) const
	{
		return factor < i.factor;
	}
};

void ExecuteComputerMoveTest(cursor* cur, const game_settings& gd, Random* rand)
{
	multiset<Placement> placements;
	const vector<WinningCombination> rules = gGame->world.get_winning_conditions();

	for (int cube = 0; cube < 4; ++cube)
	{
		for (int col = 0; col < 4; ++col)
		{
			for (int row = 0; row < 4; ++row)
			{
				auto placement = Placement{rules, Index{cube, col, row}};
				if (placement.factor >= 0)
				{
					placements.insert(placement);
				}
			}
		}
	}

	// this would indicate that there is nowhere to place and that is handled elsewhere
	ASSERT(!placements.empty());

	vector<Index> bestPlacements;
	const int bestValue = placements.rbegin()->factor;
	for (auto placement : placements)
	{
		if (placement.factor == bestValue)
		{
			bestPlacements.push_back(placement.index);
		}
	}

	const int index = rand->get_excluding(Csizet_to_int(bestPlacements.size()));
	const auto place_here = bestPlacements[Cint_to_sizet(index)];
	ComputerPlaceMarker(cur, gd, place_here.cube, place_here.column, place_here.row);
}

void ExecuteComputerMove(cursor* cur, const game_settings& gd, Random* rand)
{
	if (gd.use_bad_ai)
	{
		if (ExecutePlaceWin(cur, gd, rand)) return;
		ExecuteRandomPlacement(cur, gd, rand);
	}
	else
	{
		ExecuteComputerMoveTest(cur, gd, rand);
	}
}

void Click()
{
	gGame->click();
}

struct StartNewGameFader : FullscreenColorSprite
{
	StartNewGameFader()
		: FullscreenColorSprite({0, 0, 0, 0})
		, direction(1)
	{
	}

	void update(const game_settings&, float delta, bool, const glm::vec2&, bool) override
	{
		color.a += delta * direction;

		if (color.a > 1)
		{
			gGame->clearBoard();
			direction = -1;
		}
		if (color.a < 0)
		{
			gGame->newGame();
			kill();
		}
	}

	float direction;
};

void AddStartNewGameFader(OldGame& iGame)
{
	iGame.add(std::make_shared<StartNewGameFader>());
}

enum struct GameState { game, menu };

struct GameWrapper : State
{
	render::Texture onebit;
	glm::vec2 mouse;

	bool mouse_button = false;
	bool old_mouse_button = false;
	bool enter_state = false;
	Random random;

	game_settings gd;

	OldGame game;
	Menu menu;

	void on_mouse_button(const render::InputCommand&, input::MouseButton button, bool down) override
	{
		if (button != input::MouseButton::left) { return; }

		mouse_button = down;
	}

	GameState state = GameState::menu;

	void start_game()
	{
		state = GameState::game;
		game.buildRules(gd);
	}

	Game* root_game;

	GameWrapper(Game* gg)
		: onebit
		(
			onebit::load_texture()
		)
		, mouse(0, 0)
		, root_game(gg)
	{
		gGame = &game;
		menu.on_start_game = [this]()
		{
			start_game();
		};
	}

	void
	on_update(float dt) override
	{
		switch (state)
		{
		case GameState::game:
			game.update(gd, &random, mouse_button, old_mouse_button, dt, mouse, enter_state);
			root_game->run = !game.quit;
			break;
		case GameState::menu:
			menu.update(&gd, mouse, mouse_button, dt);
			break;
		default:
			DIE("invalid gamestate");
			break;
		}

		old_mouse_button = mouse_button;
	}

	constexpr static auto layout = render::LayoutData{render::ViewportStyle::black_bars, width, height};

	void
	on_render(const render::RenderCommand& rc) override
	{
		auto r = render::with_layer2(rc, layout);

		const auto rd = render_data{r.viewport_aabb_in_worldspace, &onebit, r.batch};

		switch (state)
		{
		case GameState::game:
			game.render(rd);
			break;
		case GameState::menu:
			menu.render(rd);
			break;
		default:
			DIE("invalid render state");
			break;
		}
	}

	void on_mouse_position(const render::InputCommand& c, const glm::ivec2& p) override
	{
		mouse = with_layer(c, layout).mouse_to_world({p.x, p.y});
	}
};

int
main(int, char**)
{
	return run_game
	(
		"4th Dimension", glm::ivec2{800, 600}, false, []()
		{
			auto game = std::make_shared<Game>();
			game->state = std::make_shared<GameWrapper>(game.get());
			return game;
		}
	);
}
