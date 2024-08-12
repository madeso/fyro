#include "catch.hpp"
#include "catchy/vectorequals.h"

#include "fyro/render/vertex_layout.h"

using namespace render;

template<>
struct fmt::formatter<VertexType> : formatter<string_view>
{
	template<typename FormatContext>
	auto format(VertexType c, FormatContext& ctx)
	{
		string_view name = "<unknown>";
		switch (c)
		{
		case VertexType::position2: name = "position2"; break;
		case VertexType::position3: name = "position3"; break;
		case VertexType::normal3: name = "normal3"; break;
		case VertexType::color4: name = "color4"; break;
		case VertexType::texture2: name = "texture2"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

namespace
{
bool is_equal(const CompiledVertexElement& lhs, const CompiledVertexElement& rhs)
{
	return lhs.type == rhs.type && lhs.name == rhs.name && lhs.index == rhs.index;
}

bool is_equal(const CompiledVertexElementNoName& lhs, const CompiledVertexElementNoName& rhs)
{
	return lhs.type == rhs.type && lhs.index == rhs.index;
}

catchy::FalseString is_equal(
	const std::vector<CompiledVertexElement> lhs, const std::vector<CompiledVertexElement>& rhs
)
{
	return catchy::VectorEquals(
		lhs,
		rhs,
		[](const CompiledVertexElement& f) -> std::string
		{ return fmt::format("{} {} ({})", f.type, f.name, f.index); },
		[](const CompiledVertexElement& a, const CompiledVertexElement& b) -> catchy::FalseString
		{
			if (is_equal(a, b))
			{
				return catchy::FalseString::True();
			}
			else
			{
				return catchy::FalseString::False(fmt::format(
					"{}!={} {}!={} ({}!={})", a.type, b.type, a.name, b.name, a.index, b.index
				));
			}
		}
	);
}

catchy::FalseString is_equal(
	const std::vector<CompiledVertexElementNoName> lhs,
	const std::vector<CompiledVertexElementNoName>& rhs
)
{
	return catchy::VectorEquals(
		lhs,
		rhs,
		[](const CompiledVertexElementNoName& f) -> std::string
		{ return fmt::format("{} ({})", f.type, f.index); },
		[](const CompiledVertexElementNoName& a,
		   const CompiledVertexElementNoName& b) -> catchy::FalseString
		{
			if (is_equal(a, b))
			{
				return catchy::FalseString::True();
			}
			else
			{
				return catchy::FalseString::False(
					fmt::format("{}!={} ({}!={})", a.type, b.type, a.index, b.index)
				);
			}
		}
	);
}

catchy::FalseString is_equal(const std::vector<VertexType> lhs, const std::vector<VertexType>& rhs)
{
	return catchy::VectorEquals(
		lhs,
		rhs,
		[](const VertexType& f) -> std::string { return fmt::format("{}", f); },
		[](const VertexType& a, const VertexType& b) -> catchy::FalseString
		{
			if (a == b)
			{
				return catchy::FalseString::True();
			}
			else
			{
				return catchy::FalseString::False(fmt::format("{} != {}", a, b));
			}
		}
	);
}

catchy::FalseString is_equal(
	const CompiledShaderVertexAttributes& lhs, const CompiledShaderVertexAttributes& rhs
)
{
	const auto same_elements = is_equal(lhs.elements, rhs.elements);
	const auto same_debug_types = is_equal(lhs.debug_types, rhs.debug_types);

	return catchy::FalseString::Combine(same_elements, same_debug_types);
}

catchy::FalseString is_equal(
	const CompiledGeomVertexAttributes& lhs, const CompiledGeomVertexAttributes& rhs
)
{
	const auto same_elements = is_equal(lhs.elements, rhs.elements);
	const auto same_debug_types = is_equal(lhs.debug_types, rhs.debug_types);

	return catchy::FalseString::Combine(same_elements, same_debug_types);
}
}  //  namespace

TEST_CASE("vertex_layout_test_simple", "[vertex_layout]")
{
	const auto layout_shader_material = ShaderVertexAttributes{
		{VertexType::position3, "aPos"},
		{VertexType::normal3, "aNormal"},
		{VertexType::color4, "aColor"},
		{VertexType::texture2, "aTexCoord"}
	};

	auto layout_compiler = compile_attribute_layouts({layout_shader_material});

	const auto compiled_layout = compile_shader_layout(layout_compiler, layout_shader_material);
	const auto mesh_layout = get_mesh_layout(layout_compiler);

	CHECK(is_equal(
		compiled_layout,
		{{{VertexType::position3, "aPos", 0},
		  {VertexType::normal3, "aNormal", 1},
		  {VertexType::color4, "aColor", 2},
		  {VertexType::texture2, "aTexCoord", 3}},
		 {VertexType::position3, VertexType::normal3, VertexType::color4, VertexType::texture2}}
	));

	CHECK(is_equal(
		mesh_layout,
		{{{VertexType::position3, 0},
		  {VertexType::normal3, 1},
		  {VertexType::color4, 2},
		  {VertexType::texture2, 3}},
		 {VertexType::position3, VertexType::normal3, VertexType::color4, VertexType::texture2}}
	));
}

TEST_CASE("vertex_layout_test_with_custom_layput", "[vertex_layout]")
{
	const auto layout_shader_material = ShaderVertexAttributes{
		{VertexType::position3, "aPos"},
		{VertexType::normal3, "aNormal"},
		{VertexType::color4, "aColor"},
		{VertexType::texture2, "aTexCoord"}
	};

	auto layout_compiler = compile_attribute_layouts(
		{VertexType::color4, VertexType::texture2}, {layout_shader_material}
	);

	const auto compiled_layout = compile_shader_layout(layout_compiler, layout_shader_material);
	const auto mesh_layout = get_mesh_layout(layout_compiler);

	CHECK(is_equal(
		compiled_layout,
		{{{VertexType::position3, "aPos", 2},
		  {VertexType::normal3, "aNormal", 3},
		  {VertexType::color4, "aColor", 0},
		  {VertexType::texture2, "aTexCoord", 1}},
		 {VertexType::position3, VertexType::normal3, VertexType::color4, VertexType::texture2}}
	));

	CHECK(is_equal(
		mesh_layout,
		{{{VertexType::position3, 2},
		  {VertexType::normal3, 3},
		  {VertexType::color4, 0},
		  {VertexType::texture2, 1}},
		 {VertexType::position3, VertexType::normal3, VertexType::color4, VertexType::texture2}}
	));
}

TEST_CASE("vertex_layout_test_material_and_depth", "[vertex_layout]")
{
	const auto layout_shader_material = ShaderVertexAttributes{
		{VertexType::position3, "aPos"},
		{VertexType::normal3, "aNormal"},
		{VertexType::color4, "aColor"},
		{VertexType::texture2, "aTexCoord"}
	};

	const auto layout_shader_depth = ShaderVertexAttributes{{VertexType::position3, "aPos"}};

	auto layout_compiler = compile_attribute_layouts({layout_shader_material, layout_shader_depth});
	const auto compiled_layout_material
		= compile_shader_layout(layout_compiler, layout_shader_material);
	const auto compiled_layout_depth = compile_shader_layout(layout_compiler, layout_shader_depth);
	const auto mesh_layout = get_mesh_layout(layout_compiler);

	CHECK(is_equal(
		compiled_layout_material,
		{{{VertexType::position3, "aPos", 0},
		  {VertexType::normal3, "aNormal", 1},
		  {VertexType::color4, "aColor", 2},
		  {VertexType::texture2, "aTexCoord", 3}},
		 {VertexType::position3, VertexType::normal3, VertexType::color4, VertexType::texture2}}
	));

	CHECK(is_equal(
		compiled_layout_depth,
		{{{VertexType::position3, "aPos", 0}},
		 {VertexType::position3, VertexType::normal3, VertexType::color4, VertexType::texture2}}
	));

	CHECK(is_equal(
		mesh_layout,
		{{{VertexType::position3, 0},
		  {VertexType::normal3, 1},
		  {VertexType::color4, 2},
		  {VertexType::texture2, 3}},
		 {VertexType::position3, VertexType::normal3, VertexType::color4, VertexType::texture2}}
	));
}

TEST_CASE("vertex_layout_test_material_and_different", "[vertex_layout]")
{
	const auto layout_shader_material = ShaderVertexAttributes{
		{VertexType::position3, "aPos"},
		{VertexType::normal3, "aNormal"},
		{VertexType::color4, "aColor"},
		{VertexType::texture2, "aTexCoord"}
	};

	const auto layout_shader_different = ShaderVertexAttributes{
		{VertexType::color4, "aColor"},
		{VertexType::texture2, "aTexCoord"},
		{VertexType::position3, "aPos"},
		{VertexType::normal3, "aNormal"}
	};

	auto layout_compiler
		= compile_attribute_layouts({layout_shader_different, layout_shader_material});
	const auto compiled_layout_material
		= compile_shader_layout(layout_compiler, layout_shader_material);
	const auto compiled_layout_different
		= compile_shader_layout(layout_compiler, layout_shader_different);
	const auto mesh_layout = get_mesh_layout(layout_compiler);

	CHECK(is_equal(
		compiled_layout_material,
		{{{VertexType::position3, "aPos", 0},
		  {VertexType::normal3, "aNormal", 1},
		  {VertexType::color4, "aColor", 2},
		  {VertexType::texture2, "aTexCoord", 3}},
		 {VertexType::position3, VertexType::normal3, VertexType::color4, VertexType::texture2}}
	));

	CHECK(is_equal(
		compiled_layout_different,
		{{{VertexType::color4, "aColor", 2},
		  {VertexType::texture2, "aTexCoord", 3},
		  {VertexType::position3, "aPos", 0},
		  {VertexType::normal3, "aNormal", 1}},
		 {VertexType::position3, VertexType::normal3, VertexType::color4, VertexType::texture2}}
	));

	CHECK(is_equal(
		mesh_layout,
		{{{VertexType::position3, 0},
		  {VertexType::normal3, 1},
		  {VertexType::color4, 2},
		  {VertexType::texture2, 3}},
		 {VertexType::position3, VertexType::normal3, VertexType::color4, VertexType::texture2}}
	));
}

TEST_CASE("vertex_layout_test_crazy", "[vertex_layout]")
{
	const auto layout_shader_a = ShaderVertexAttributes{{VertexType::color4, "rgb"}};

	const auto layout_shader_b = ShaderVertexAttributes{{VertexType::texture2, "uv"}};
	auto layout_compiler = compile_attribute_layouts({layout_shader_a, layout_shader_b});
	const auto compiled_layout_a = compile_shader_layout(layout_compiler, layout_shader_a);
	const auto compiled_layout_b = compile_shader_layout(layout_compiler, layout_shader_b);
	const auto mesh_layout = get_mesh_layout(layout_compiler);

	CHECK(is_equal(
		compiled_layout_a,
		{{{VertexType::color4, "rgb", 0}}, {VertexType::color4, VertexType::texture2}}
	));

	CHECK(is_equal(
		compiled_layout_b,
		{{{VertexType::texture2, "uv", 1}}, {VertexType::color4, VertexType::texture2}}
	));

	CHECK(is_equal(
		mesh_layout,
		{{{VertexType::color4, 0}, {VertexType::texture2, 1}},
		 {VertexType::color4, VertexType::texture2}}
	));
}

TEST_CASE("vertex_layout_test_get_not_requested", "[vertex_layout]")
{
	const auto layout_shader_material = ShaderVertexAttributes{{VertexType::position3, "pos"}};

	const auto not_requested_property = GENERATE(
		VertexType::position2, VertexType::normal3, VertexType::color4, VertexType::texture2
	);

	const auto layout_shader_not_requested
		= ShaderVertexAttributes{{not_requested_property, "not_requested_prop"}};

	auto layout_compiler = compile_attribute_layouts({layout_shader_material});

	const auto mesh_layout = get_mesh_layout(layout_compiler);


	// not requested variables should assert
	REQUIRE_THROWS_WITH(
		compile_shader_layout(layout_compiler, layout_shader_not_requested),
		Catch::Contains("Assertion failed")
			&& Catch::Contains("layout wasn't added to the compilation list")
	);

	const auto compiled_layout = compile_shader_layout(layout_compiler, layout_shader_material);

	CHECK(is_equal(compiled_layout, {{{VertexType::position3, "pos", 0}}, {VertexType::position3}})
	);

	CHECK(is_equal(mesh_layout, {{{VertexType::position3, 0}}, {VertexType::position3}}));
}
