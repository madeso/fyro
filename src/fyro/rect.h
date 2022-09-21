#pragma once

#include "fyro/assert.h"



// basic rect
// also heavily inspired by https://halt.software/dead-simple-layouts/

template<typename T>
struct Rect
{
	T left;
	T bottom;
	T right;
	T top;

	constexpr Rect<T>
	(
		  T l
		, T b
		, T r
		, T t
	)
		: left(l)
		, bottom(b)
		, right(r)
		, top(t)
	{
		ASSERT(left <= right && bottom <= top);
	}

	constexpr Rect<T>
	(
		  T width
		, T height
	)
		: left(0)
		, bottom(0)
		, right(width)
		, top(height)
	{
		ASSERT(width >= 0 && height >= 0);
	}

	constexpr Rect<T>
	zero() const
	{
		return {0, 0, right - left, top - bottom};
	}

	constexpr Rect<T>
	scale_uniform(T s) const
	{
		return {left * s, bottom * s, right * s, top * s};
	}

	constexpr bool
	is_inside_inclusive(T x, T y) const
	{
		return x >= left && x <= right
			&& y >= bottom && y <= top;
	}

	constexpr Rect<T>
	translate(T dx, T dy) const
	{
		return {left + dx, bottom + dy, right + dx, top + dy};
	}

	template<glm::qualifier Q>
	constexpr Rect<T>
	translate(const glm::vec<2, T, Q>& v) const
	{
		return translate(v.x, v.y);
	}

	template<glm::qualifier Q>
	constexpr glm::vec<2, T, Q>
	to01(const glm::vec<2, T, Q>& p) const
	{
		return
		{
			(p.x - left) / get_width(),
			(p.y - bottom) / get_height()
		};
	}

	template<glm::qualifier Q>
	constexpr glm::vec<2, T, Q>
	from01(const glm::vec<2, T, Q>& p) const
	{
		return
		{
			left + (p.x * get_width()),
			bottom + (p.y * get_height())
		};
	}

	constexpr T
	get_height() const
	{
		return top - bottom;
	}

	constexpr T
	get_width() const
	{
		return right - left;
	}

	constexpr T
	get_scale_for_height(T new_height) const
	{
		const T old_height = get_height();
		return new_height / old_height;
	}

	constexpr T
	get_scale_for_width(T new_width) const
	{
		const T old_width = get_width();
		return new_width / old_width;
	}

	constexpr Rect<T>
	set_height(T new_height) const
	{
		return scale_uniform(get_scale_for_height(new_height));
	}

	constexpr static Rect<T>
	from_xywh(T x, T y, T w, T h)
	{
		return {x, y, x+w, y+h};
	}

	constexpr Rect<T>
	set_bottom_left(T x, T y)
	{
		return zero().translate(x, y);
	}


	// These will add a rectangle outside of the input rectangle.
	// Useful for tooltips and other overlay elements.
	constexpr Rect<T> add_left(T a) const
	{
		return {left-a, bottom, right, top};
	}

	constexpr Rect<T> add_right(T a) const
	{
		return {left, bottom, right+a, top};
	}

	constexpr Rect<T> add_top(T a) const
	{
		return {left, bottom, right, top+a};
	}

	constexpr Rect<T> add_bottom(T a) const
	{
		return {left, bottom-a, right, top};
	}

	constexpr Rect<T> extend(T a) const
	{
		return add_left(a).add_right(a).add_top(a).add_bottom(a);
	}

	constexpr Rect<T> center_hor(T w) const
	{
		const auto dx = (get_width() - w)/2;
		const auto x = left+dx;
		return {x,bottom, x+w, top};
	}

	constexpr Rect<T> center_vert(T h) const
	{
		const auto dy = (get_height() - h)/2;
		const auto y = bottom+dy;
		return {left,y, right, y+h};
	}

	constexpr Rect<T> cut_to_center(T w, T h) const
	{
		return center_hor(w).center_vert(h);
	}
};


enum class Side
{
	left,
	right,
	top,
	bottom,
};

template<typename T>
struct RectCut
{
	::Rect<T>* rect;
	Side side;
};

template<typename T>
Rect<T>
cut_left(Rect<T>* rect, T a)
{
	T old_left = rect->left;
	rect->left = std::min(rect->right, rect->left + a);
	return { old_left, rect->bottom, rect->left, rect->top };
}

template<typename T>
Rect<T>
cut_right(Rect<T>* rect, T a)
{
	T old_right = rect->right;
	rect->right = std::max(rect->left, rect->right - a);
	return { rect->right, rect->bottom, old_right, rect->top };
}

template<typename T>
Rect<T>
cut_top(Rect<T>* rect, T a)
{
	T old_bottom = rect->bottom;
	rect->bottom = std::min(rect->top, rect->bottom + a);
	return { rect->left, old_bottom, rect->right, rect->bottom };
}

template<typename T>
Rect<T>
cut_bottom(Rect<T>* rect, T a)
{
	T old_top = rect->top;
	rect->top = std::max(rect->bottom, rect->top - a);
	return { rect->left, rect->top, rect->right, old_top };
}

template<typename T>
Rect<T>
rectcut_cut(const RectCut<T>& rectcut, T a)
{
	switch (rectcut.side)
	{
		case Side::left:   return cut_left(rectcut.rect,   a);
		case Side::right:  return cut_right(rectcut.rect,  a);
		case Side::top:	return cut_top(rectcut.rect,	a);
		case Side::bottom: return cut_bottom(rectcut.rect, a);
		default:
			DIE("invalid rectcut side");
			return cut_bottom(rectcut.rect, a);
	}
}


template<typename T>
bool
rect_intersect(const Rect<T>& lhs, const Rect<T>& rhs)
{
	return !(rhs.left   >= lhs.right  ||
             rhs.right  <= lhs.left   ||
             rhs.top    <= lhs.bottom ||
             rhs.bottom >= lhs.top);
}



// Extensions
#if 0

// Same as cut, except they keep the input rect intact.
// Useful for decorations (9-patch-much?).
rect get_left(const trect<T>& rect, T a);
rect get_right(const trect<T>& rect, T a);
rect get_top(const trect<T>& rect, T a);
rect get_bottom(const trect<T>& rect, T a);

#endif


using Rectf = Rect<float>;
using Recti = Rect<int>;

constexpr Rectf
Cint_to_float(const Recti& r)
{
	return
	{
		static_cast<float>(r.left),
		static_cast<float>(r.bottom),
		static_cast<float>(r.right),
		static_cast<float>(r.top)
	};
}
