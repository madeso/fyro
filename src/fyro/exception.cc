#include "fyro/exception.h"

Exception Exception::append(const std::string& str)
{
	errors.emplace_back(str);
	return *this;
}

Exception Exception::append(const std::vector<std::string>& li)
{
	for (const auto& str: li)
	{
		errors.emplace_back(str);
	}
	return *this;
}

Exception collect_exception()
{
	try
	{
		throw;
	}
	catch (const Exception& e)
	{
		return e;
	}
	catch (const std::exception& e)
	{
		return {{e.what()}};
	}
	catch (...)
	{
		return {{"unknown errror"}};
	}
}
