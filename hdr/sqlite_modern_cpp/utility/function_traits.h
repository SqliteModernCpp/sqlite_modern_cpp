#pragma once

#include <tuple>

namespace sqlite {
namespace utility {

template<typename> struct function_traits;

template <typename Function>
struct function_traits : public function_traits<
	decltype(&Function::operator())
> { };

template <
	typename    ClassType,
	typename    ReturnType,
	typename... Arguments
>
struct function_traits<
	ReturnType(ClassType::*)(Arguments...) const
> {
	typedef ReturnType result_type;

	template <std::size_t Index>
	using argument = typename std::tuple_element<
		Index,
		std::tuple<Arguments...>
	>::type;

	static const std::size_t arity = sizeof...(Arguments);
};

}
}
