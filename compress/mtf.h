#pragma once
#include <cstdint>

namespace MTF {
	static uint16_t stack[257];

	void init()
	{
		for (int i = 0; i < 257; ++i)
			stack[i] = i;
	}

	uint16_t process(uint16_t c)
	{
		uint16_t i = 0;
		uint16_t tmp1 = stack[i];
		uint16_t tmp2;
		while (tmp1 != c) {
			tmp2 = tmp1;
			i++;
			tmp1 = stack[i];
			stack[i] = tmp2;
		}

		stack[0] = c;
		return i;
	}

	uint16_t process_reverse(uint16_t c)
	{
		uint16_t ret = stack[c];
		for (uint16_t i = c; i > 0; --i)
			stack[i] = stack[i - 1];

		stack[0] = ret;
		return ret;
	}
}
