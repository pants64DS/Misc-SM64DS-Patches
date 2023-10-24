#ifndef UNALIGNED_INCLUDED
#define UNALIGNED_INCLUDED

#include "include/SM64DS_2.h"
#include <tuple>

template<class T> concept UnalignedReadable =
	std::is_object_v<T> &&
	std::is_trivial_v<T> &&
	!std::is_const_v<T> &&
	!std::is_volatile_v<T>;

namespace UnalignedImpl
{
	template<class T>
	inline T Read(const char* ptr)
	{
		static_assert(UnalignedReadable<T>);
		T res;

		for (std::size_t i = 0; i < sizeof(T); ++i)
			reinterpret_cast<char*>(&res)[i] = ptr[i];

		return res;
	}

	template<std::integral T> requires (sizeof(T) == 1) [[gnu::always_inline]]
	inline T Read(const char* ptr) { return *ptr; }

	template<std::integral T> requires (sizeof(T) == 2) [[gnu::always_inline]]
	inline T Read(const char* ptr) { return ReadUnalignedShort(ptr); }

	template<std::integral T> requires (sizeof(T) == 4) [[gnu::always_inline]]
	inline T Read(const char* ptr) { return ReadUnalignedInt(ptr); }

	template<FixedPoint T> [[gnu::always_inline]]
	inline T Read(const char* ptr)
	{
		return {Read<Underlying<T>>(ptr), as_raw};
	}

	template<class... Types>
	constexpr auto offsets = []
	{
		auto offsets = std::to_array<std::size_t>({sizeof(Types)...});

		for (std::size_t nextOffset = 0; std::size_t& offset : offsets)
			nextOffset += std::exchange(offset, nextOffset);

		return offsets;
	}();

	template<class T> concept Vector =
		std::same_as<T, Vector3> ||
		std::same_as<T, Vector3_16> ||
		std::same_as<T, Vector3_16f>;

	template<std::size_t i, class T>
	inline auto& GetVectorElem(T&& v)
	{
		if constexpr (i == 0) return v.x;
		if constexpr (i == 1) return v.y;
		if constexpr (i == 2) return v.z;
	}

	template<std::size_t i, class T>
	using VectorElemType = decltype(auto(GetVectorElem<i>(std::declval<T>())));

	template<Vector T>
	inline void ReadVector(const char* ptr, Vector3& res)
	{
		[&]<std::size_t... i>[[gnu::always_inline]](std::index_sequence<i...>)
		{
			((GetVectorElem<i>(res) = Read<VectorElemType<i, T>>
			(ptr + offsets<VectorElemType<i, T>...>[i])), ...);
		}
		(std::make_index_sequence<3>());
	}

	template<Vector T> [[gnu::always_inline]]
	inline auto Read(const char* ptr)
	{
		return Vector3::Proxy([ptr]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			ReadVector<T>(ptr, res);
		});
	}

	template<UnalignedReadable T>
	using ReadResult = decltype(Read<T>(std::declval<const char*>()));

	template<class... Types> [[gnu::always_inline]]
	inline decltype(auto) Visit(const char* ptr, auto&& f)
	{
		return [&]<std::size_t... i>[[gnu::always_inline]](std::index_sequence<i...>) -> decltype(auto)
		{
			return f(Read<Types>(ptr + offsets<Types...>[i])...);
		}
		(std::make_index_sequence<sizeof...(Types)>());
	}
};

template<UnalignedReadable... Types> [[gnu::always_inline, nodiscard]]
inline auto ReadUnaligned(const char* ptr)
{
	using namespace UnalignedImpl;

	if constexpr (sizeof...(Types) == 1)
		return Read<Types...>(ptr);
	else
		return Visit<Types...>(ptr, std::make_tuple<ReadResult<Types>...>);
}

template<class F> [[gnu::always_inline]]
inline decltype(auto) VisitUnaligned(const char* ptr, F&& f)
{
	return [&]<class R, class... Args>[[gnu::always_inline]](R(F::*)(Args...) const) -> R
	{
		return UnalignedImpl::Visit<Args...>(ptr, f);
	}
	(&F::operator());
}

#endif