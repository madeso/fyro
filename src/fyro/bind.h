#pragma once


#define LOX_ASSERT(check) \
	do \
	{ \
		if (! (check)) \
		{ \
			assert(false && #check); \
			lox::raise_error("fyro internal error: " #check); \
			return nullptr; \
		} \
	} while (false)
#define LOX_ERROR(check, message) \
	do \
	{ \
		if (! (check)) \
		{ \
			lox::raise_error(message); \
			return nullptr; \
		} \
	} while (false)
