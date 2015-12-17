#pragma once
#include <cstdint>
#include <iostream>

namespace Boundaries {
	static const uint32_t MAX_B = (1u << 30);
	static const uint32_t AGGR_NORMAL = 510000;
	static const uint32_t AGGR_HUGE = MAX_B / 7;

	static uint32_t bt_1[258];
	static uint32_t bt_2[258];

	void prefill()
	{
		for (int i = 0; i < 258; ++i) {
			bt_1[i] = i;
			bt_2[i] = i;
		}
	}

	void print(const uint32_t b[258])
	{
		for (uint16_t i = 0; i < 258; ++i) {
			std::cerr << "b[" << i << "] = " << b[i] << "\n";
		}
	}

	uint32_t max_dif(const uint32_t b[258])
	{
		uint32_t r = b[1];
		for (uint16_t i = 2; i < 258; ++i) {
			uint32_t d = b[i] - b[i - 1];
			if (d > r)
				r = d;
		}

		return r;
	}

	void normalize(uint32_t b[258])
	{
		for (uint16_t i = 1; i < 258; ++i) {
			b[i] /= 2;
			if (b[i] <= b[i - 1])
				b[i] = b[i - 1] + 1;
		}
	}

	void update_table(uint32_t b[258], uint8_t c, uint32_t aggr)
	{
		// std::cerr << "PRE update_table(" << static_cast<uint32_t>(c) << ");\n";
		// print(b);

		while (MAX_B - aggr <= b[257])
			normalize(b);

		// std::cerr << "POST normalize();\n";
		// print(b);

		for (uint16_t i = c + 1; i < 258; ++i) {
			b[i] += aggr;
		}

		// std::cerr << "POST update_table(" << static_cast<uint32_t>(c) << ");\n";
		// print(b);

		// for (uint16_t i = 1; i < 258; ++i) {
		// 	if (b[i] <= b[i - 1])
		// 		std::cerr << "ERROR! WRONG BOUNDARY\n";
		// }
	}

	void update(uint8_t c)
	{
		update_table(bt_1, c, AGGR_NORMAL);
		update_table(bt_2, c, AGGR_HUGE);
	}

	const uint32_t *get()
	{
		//if ((max_dif(bt_2) * static_cast<uint64_t>(MAX_B)) / bt_2[257] >= (AGGR_HUGE * 5)) {
		//	// std::cerr << "Using bt_2.\n";
		//	return bt_2;
		//}

		// std::cerr << "Using bt_1.\n";
		return bt_1;
	}
}
