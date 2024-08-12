#pragma once

template<typename TSource, typename TData>
struct Cache
{
	using Loader = std::function<std::shared_ptr<TData>(const TSource&)>;

	Loader load;
	std::unordered_map<TSource, std::weak_ptr<TData>> loaded;

	explicit Cache(Loader&& l)
		: load(l)
	{
	}

	std::shared_ptr<TData> get(const TSource& source)
	{
		if (auto found = loaded.find(source); found != loaded.end())
		{
			auto ret = found->second.lock();
			if (ret != nullptr)
			{
				return ret;
			}
		}

		auto ret = load(source);
		assert(ret != nullptr);
		loaded[source] = ret;
		return ret;
	}
};
