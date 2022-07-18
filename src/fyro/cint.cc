#include "fyro/cint.h"


int
Csizet_to_int(std::size_t t)
{
	return static_cast<int>(t);
}


std::size_t
Cint_to_sizet(int i)
{
	return static_cast<std::size_t>(i);
}


int
Cunsigned_int_to_int(unsigned int ui)
{
	return static_cast<int>(ui);
}


float
Cint_to_float(int i)
{
	return static_cast<float>(i);
}
