#include "fyro/fyro.h"

#include <array>
#include <vector>

#include "sprites/onebit.h"
#include "sprites/onebit_font.h"

#include "fyro/random.h"


// todo(Gustav): add markers.. bomb and question
// todo(Gustav): add bombs counter
// todo(Gustav): wiggle things a bit
// todo(Gustav): animate floodfill
// todo(Gustav): add some colors
// todo(Gustav): ui for selecting level size and number of bombs


constexpr auto black = glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};
constexpr auto font = ::onebit::font{12.0f};

enum class GameState { playing, game_over, game_completed};

struct Minesweeper
{
    int width;
    int height;
    int bombs;
    std::vector<bool> revealed;
    // todo(Gustav): never used font for 1+ rendering... so switch to a enum class
    std::vector<int> states; // -1 == bomb, 0=empty, 1-9=number of neighbour bombs
    bool initialized = false;
    GameState current_game_state = GameState::playing;

    Rectf play_area = {100.0f, 100.0f};

    Minesweeper(int w, int h, int b)
        : width(w), height(h), bombs(b)
        , revealed(Cint_to_sizet(w*h), false)
        , states(Cint_to_sizet(w*h), 0)
    {
    }

    void init()
    {
        if(initialized) { return; }
        initialized = true;

        using free_type = std::vector<glm::ivec2>;
        free_type free;
        for(int y=0; y<height; y+=1)
        {
            for(int x=0; x<width; x+=1)
            {
                if(false == revealed[get_index(x, y)])
                {
                    free.emplace_back(glm::ivec2{x, y});
                }
            }
        }
        Random r;
        for(int i=0; i<bombs; i+=1)
        {
            const auto index = r.get_excluding(free.size());
            const auto p = free[index];
            free.erase(free.begin() + static_cast<free_type::difference_type>(index));
            states[get_index(p.x, p.y)] = -1;
        }

        for(int y=0; y<height; y+=1)
        {
            for(int x=0; x<width; x+=1)
            {
                if(states[get_index(x, y)]==0)
                {
                    states[get_index(x, y)] = count_neighbours(x, y);
                }
            }
        }
    }

    void click_on(int x, int y)
    {
        revealed[get_index(x, y)] = true;

        init();

        if(states[get_index(x, y)] == -1)
        {
            game_over();
        }

        // auto-click on empties
        if(states[get_index(x, y)] == 0)
        {
            for(int dx=-1; dx<=1; dx+=1)
            {
                for(int dy=-1; dy<=1; dy+=1)
                {
                    const auto xx = x + dx;
                    const auto yy = y + dy;
                    if(xx <0 || yy < 0 || xx >= width || yy >= height) { continue; }
                    if(false == revealed[get_index(xx, yy)])
                    {
                        click_on(xx, yy);
                    }
                }
            }
        }
    }

    void game_over()
    {
        for(int y=0; y<height; y+=1)
        {
            for(int x=0; x<width; x+=1)
            {
                revealed[get_index(x, y)] = true;
            }
        }
        current_game_state = GameState::game_over;
    }

    void on_mouse(const glm::vec2& m)
    {
        for(int y=0; y<height; y+=1)
        {
            for(int x=0; x<width; x+=1)
            {
                if(get_rect(x, y).is_inside_inclusive(m.x, m.y))
                {
                    click_on(x, y);
                }
            }
        }
    }

    std::size_t get_index(int x, int y) const
    {
        ASSERT(x >= 0 && x<width);
        ASSERT(y >= 0 && y<height);
        return Cint_to_sizet(x + y * width);
    }

    static constexpr float window_size = 200.0f;
    static constexpr float window_spacing = 10.0f;

    Rectf sprite() const
    {
        const auto w = play_area.get_width()/static_cast<float>(width);
        const auto h = play_area.get_height()/static_cast<float>(height);
        const auto s = std::min(w, h);
        return {s, s};
    }

    Rectf get_rect(int x, int y) const
    {
        const auto center = [](float sprite_size, float avaiable, int count) -> float
        {
            return (avaiable - sprite_size * static_cast<float>(count)) / 2.0f;
        };
        const auto s = sprite();
        const auto base_x = play_area.left + center(s.get_width(), play_area.get_width(), width);
        const auto base_y = play_area.bottom + center(s.get_height(), play_area.get_height(), height);
        return s.translate
        (
            base_x + static_cast<float>(x) * s.get_width(),
            base_y + static_cast<float>(y) * s.get_height()
        );
    }

    int count_neighbours(int x, int y) const
    {
        int count = 0;
        for(int dx=-1; dx<=1; dx+=1)
        {
            for(int dy=-1; dy<=1; dy+=1)
            {
                if(dx == 0 && dy == 0) { continue; }
                const auto xx = x + dx;
                const auto yy = y + dy;
                if(xx <0 || yy < 0 || xx >= width || yy >= height) { continue; }
                if(states[get_index(xx, yy)] == -1)
                {
                    count += 1;
                }
            }
        }

        return count;
    }

    void render(render::SpriteBatch* batch, render::Texture* onebit)
    {
        for(int y=0; y<height; y+=1)
        {
            for(int x=0; x<width; x+=1)
            {
                auto draw = [batch, onebit](const Rectf& r, const Recti& sprite, const glm::vec4& color)
                {
                    batch->quad
                    (
                        onebit,
                        r,
                        sprite, color
                    );
                };

                const auto index = get_index(x, y);
                const auto is_revealed = revealed[index];
                const auto state = states[index];
                if(is_revealed && state == 0) { continue; }

                constexpr auto color = black;

                const auto r = get_rect(x, y);

                if(is_revealed == false)
                {
                    draw(r, ::onebit::box, color);
                }
                else
                {
                    switch(state)
                    {
                    case -1: draw(r, ::onebit::bomb, color); break;
                    case 1:  draw(r, ::onebit::n1, color); break;
                    case 2:  draw(r, ::onebit::n2, color); break;
                    case 3:  draw(r, ::onebit::n3, color); break;
                    case 4:  draw(r, ::onebit::n4, color); break;
                    case 5:  draw(r, ::onebit::n5, color); break;
                    case 6:  draw(r, ::onebit::n6, color); break;
                    case 7:  draw(r, ::onebit::n7, color); break;
                    case 8:  draw(r, ::onebit::n8, color); break;
                    case 9:  draw(r, ::onebit::n9, color); break;
                    }
                }
            }
        }
    }
};

struct MinesweeperGame : public Game
{
    render::Texture onebit;
    glm::vec2 mouse;

    Minesweeper ms;

    MinesweeperGame()
        : onebit
        (
            ::onebit::load_texture()
        )
        , mouse(0, 0)
        , ms(10, 10, 10)
    {
        new_game();
    }

    void new_game()
    {
        ms = Minesweeper{20, 10, 10};
    }

    render::LayoutData get_main_layout() const
    {
        // return {viewport_style::black_bars, 200.0f, 200.0f};
        return {render::ViewportStyle::extended, 200.0f, 200.0f};
    }

    float title_anim = 0.0f;
    bool on_update(float dt) override
    {
        title_anim += dt * 1.0f;
        while(title_anim > 1.0f) { title_anim -= 1.0f;}
        return true;
    }

    void
    on_render(const render::RenderCommand& rc) override
    {
        const std::string game_title = "minesweeper 42";

        constexpr float spacing = 0.2f;
        constexpr float button_size = 15.0f;
        constexpr float wavy_range = 1.5f;
        auto r = with_layer2(rc, get_main_layout());

        auto game_world = Rectf{200.0f, 200.0f};

        const auto title_rect = cut_bottom(&game_world, font.size + wavy_range + spacing * 2)
            .center_hor(font.get_width_of_string(game_title))
            .center_vert(font.size);
        const auto restart_rect = cut_bottom(&game_world, button_size + spacing * 2);
        /* const auto score_rect = */ cut_bottom(&game_world, font.size + spacing * 2);
        ms.play_area = game_world.extend(-1.0f);
        game_button = restart_rect.cut_to_center(button_size, button_size);

        r.batch->quad({}, r.viewport_aabb_in_worldspace, {}, {0.8, 0.8, 0.8, 1.0f});

        ms.render(r.batch, &onebit);

        auto draw_game_button = [&](const Recti& sprite)
        {
            r.batch->quad(&onebit, game_button, sprite, black);
        };

        switch(ms.current_game_state)
        {
            case GameState::playing: draw_game_button(::onebit::smiley_happy); break;
            case GameState::game_completed: draw_game_button(::onebit::smiley_joy); break;
            case GameState::game_over: draw_game_button(::onebit::smiley_skull); break;
        }

        font.simple_text(r.batch, &onebit, black, title_rect.left, title_rect.bottom, game_title, ::onebit::siny_animation{wavy_range, 0.2f, title_anim});
    }

    void on_mouse_position(const render::InputCommand&, const glm::ivec2& p) override
    {
        // todo(gustav): transform to world...
        mouse = {p.x, p.y};
    }

    Rectf game_button = Rectf{15.0f, 15.0f}.translate(100, 160);

    void on_mouse_button(const render::InputCommand& c, input::MouseButton button, bool down) override
    {
        if(down != false) { return; }
        if(button != input::MouseButton::left) { return; }

        auto r = with_layer(c, get_main_layout());
        const auto world = r.mouse_to_world(mouse);
        ms.on_mouse(world);

        if(game_button.is_inside_inclusive(world.x, world.y))
        {
            new_game();
        }
    }
};

int
main(int, char**)
{
    return run_game
    (
        "minesweeper", glm::ivec2{800, 600}, false, []()
        {
            return std::make_shared<MinesweeperGame>();
        }
    );
}
