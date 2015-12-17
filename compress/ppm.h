#pragma once
#include <cstdint>
#include <iostream>

namespace PPM {
	static const uint32_t MAX_B = (1u << 30);
	static const uint32_t AGGR = 100;
	static const uint8_t PPM_DEPTH = 5; // WE NEED TO GO DEEPER.

	uint32_t equprob[258];
	uint8_t symbol_sequence[PPM_DEPTH];
	uint8_t current_depth = 0;
	uint32_t(******probability)[258] = nullptr; // PPM_DEPTH + 1 stars.

	void prefill(uint32_t b[258])
	{
		for (int i = 0; i < 258; ++i) {
			b[i] = i;
		}
	}

	void init()
	{
		prefill(equprob);
		probability = reinterpret_cast<uint32_t(******)[258]>(calloc(258, sizeof(*probability)));
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
		if (current_depth < PPM_DEPTH) {
			std::memcpy(&symbol_sequence[current_depth++], &c, sizeof(c));
			return;
		}

		// PPM_DEPTH ifs.
		if (!probability[symbol_sequence[0]]) {
			probability[symbol_sequence[0]] = reinterpret_cast<uint32_t(*****)[258]>(calloc(258, sizeof(**probability)));
		}
		if (!probability[symbol_sequence[0]][symbol_sequence[1]]) {
			probability[symbol_sequence[0]][symbol_sequence[1]] = reinterpret_cast<uint32_t(****)[258]>(calloc(258, sizeof(***probability)));
		}
		if (!probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]]) {
			probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]] = reinterpret_cast<uint32_t(***)[258]>(calloc(258, sizeof(****probability)));
		}
		if (!probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]][symbol_sequence[3]]) {
			probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]][symbol_sequence[3]] = reinterpret_cast<uint32_t(**)[258]>(calloc(258, sizeof(*****probability)));
		}
		if (!probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]][symbol_sequence[3]][symbol_sequence[4]]) {
			probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]][symbol_sequence[3]][symbol_sequence[4]] = reinterpret_cast<uint32_t(*)[258]>(malloc(sizeof(******probability)));
			prefill(*(probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]][symbol_sequence[3]][symbol_sequence[4]]));
		}

		update_table(*(probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]][symbol_sequence[3]][symbol_sequence[4]]), c, AGGR);

		for (int i = PPM_DEPTH - 1; i > 0; --i)
			symbol_sequence[i - 1] = symbol_sequence[i];
		symbol_sequence[PPM_DEPTH - 1] = c;
	}

	const uint32_t *get()
	{
		if (current_depth < PPM_DEPTH) {
			return equprob;
		}

		// PPM_DEPTH ifs.
		if (!probability[symbol_sequence[0]]) {
			probability[symbol_sequence[0]] = reinterpret_cast<uint32_t(*****)[258]>(calloc(258, sizeof(**probability)));
		}
		if (!probability[symbol_sequence[0]][symbol_sequence[1]]) {
			probability[symbol_sequence[0]][symbol_sequence[1]] = reinterpret_cast<uint32_t(****)[258]>(calloc(258, sizeof(***probability)));
		}
		if (!probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]]) {
			probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]] = reinterpret_cast<uint32_t(***)[258]>(calloc(258, sizeof(****probability)));
		}
		if (!probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]][symbol_sequence[3]]) {
			probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]][symbol_sequence[3]] = reinterpret_cast<uint32_t(**)[258]>(calloc(258, sizeof(*****probability)));
		}
		if (!probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]][symbol_sequence[3]][symbol_sequence[4]]) {
			probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]][symbol_sequence[3]][symbol_sequence[4]] = reinterpret_cast<uint32_t(*)[258]>(malloc(sizeof(******probability)));
			prefill(*(probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]][symbol_sequence[3]][symbol_sequence[4]]));
		}

		return *(probability[symbol_sequence[0]][symbol_sequence[1]][symbol_sequence[2]][symbol_sequence[3]][symbol_sequence[4]]);
	}
}
