#pragma once

#include <type_traits>

namespace sqlite {
namespace utility {

template <bool Condition>
using enable_if = typename std::enable_if<Condition, std::size_t>::type;

template <bool Condition>
using disable_if = enable_if<!Condition>;

}
}
