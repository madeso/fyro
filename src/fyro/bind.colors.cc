#include "fyro/bind.colors.h"

#include "lox/lox.h"
#include "fyro/rgb.h"

namespace bind
{

void bind_named_colors(lox::Lox* lox)
{
	auto rgb = lox->in_package("fyro.rgb");
	rgb->add_native_getter("white", [lox]() { return lox->make_native(Rgb{255, 255, 255}); });
	rgb->add_native_getter("light_gray", [lox]() { return lox->make_native(Rgb{160, 160, 160}); });
	rgb->add_native_getter("gray", [lox]() { return lox->make_native(Rgb{127, 127, 127}); });
	rgb->add_native_getter("dark_gray", [lox]() { return lox->make_native(Rgb{87, 87, 87}); });
	rgb->add_native_getter("black", [lox]() { return lox->make_native(Rgb{0, 0, 0}); });
	rgb->add_native_getter("red", [lox]() { return lox->make_native(Rgb{173, 35, 35}); });
	rgb->add_native_getter("pure_red", [lox]() { return lox->make_native(Rgb{255, 0, 0}); });
	rgb->add_native_getter("blue", [lox]() { return lox->make_native(Rgb{42, 75, 215}); });
	rgb->add_native_getter("pure_blue", [lox]() { return lox->make_native(Rgb{0, 0, 255}); });
	rgb->add_native_getter("light_blue", [lox]() { return lox->make_native(Rgb{157, 175, 255}); });
	rgb->add_native_getter("normal_blue", [lox]() { return lox->make_native(Rgb{127, 127, 255}); });
	rgb->add_native_getter(
		"cornflower_blue", [lox]() { return lox->make_native(Rgb{100, 149, 237}); }
	);
	rgb->add_native_getter("green", [lox]() { return lox->make_native(Rgb{29, 105, 20}); });
	rgb->add_native_getter("pure_green", [lox]() { return lox->make_native(Rgb{0, 255, 0}); });
	rgb->add_native_getter("light_green", [lox]() { return lox->make_native(Rgb{129, 197, 122}); });
	rgb->add_native_getter("yellow", [lox]() { return lox->make_native(Rgb{255, 238, 51}); });
	rgb->add_native_getter("pure_yellow", [lox]() { return lox->make_native(Rgb{255, 255, 0}); });
	rgb->add_native_getter("orange", [lox]() { return lox->make_native(Rgb{255, 146, 51}); });
	rgb->add_native_getter("pure_orange", [lox]() { return lox->make_native(Rgb{255, 127, 0}); });
	rgb->add_native_getter("brown", [lox]() { return lox->make_native(Rgb{129, 74, 25}); });
	rgb->add_native_getter("pure_brown", [lox]() { return lox->make_native(Rgb{250, 75, 0}); });
	rgb->add_native_getter("purple", [lox]() { return lox->make_native(Rgb{129, 38, 192}); });
	rgb->add_native_getter("pure_purple", [lox]() { return lox->make_native(Rgb{128, 0, 128}); });
	rgb->add_native_getter("pink", [lox]() { return lox->make_native(Rgb{255, 205, 243}); });
	rgb->add_native_getter("pure_pink", [lox]() { return lox->make_native(Rgb{255, 192, 203}); });
	rgb->add_native_getter("pure_beige", [lox]() { return lox->make_native(Rgb{245, 245, 220}); });
	rgb->add_native_getter("tan", [lox]() { return lox->make_native(Rgb{233, 222, 187}); });
	rgb->add_native_getter("pure_tan", [lox]() { return lox->make_native(Rgb{210, 180, 140}); });
	rgb->add_native_getter("cyan", [lox]() { return lox->make_native(Rgb{41, 208, 208}); });
	rgb->add_native_getter("pure_cyan", [lox]() { return lox->make_native(Rgb{0, 255, 255}); });
}

void bind_rgb(lox::Lox* lox)
{
	auto fyro = lox->in_package("fyro");
	fyro->define_native_class<Rgb>(
		"Rgb",
		[](lox::ArgumentHelper& ah) -> Rgb
		{
			const auto r = static_cast<float>(ah.require_float("red"));
			const auto g = static_cast<float>(ah.require_float("green"));
			const auto b = static_cast<float>(ah.require_float("blue"));
			if(ah.complete()) return Rgb{0, 0, 0};
			return Rgb{r, g, b};
		}
	);
}

}  //  namespace bind
