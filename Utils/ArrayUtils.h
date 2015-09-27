#pragma once

namespace ArrayUtils
{

template <typename T, size_t N>
constexpr size_t ArraySize(T (&arr)[N])
{
	return N;
}

}
