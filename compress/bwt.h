#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace BWT {
	static size_t len;

	static int cmp(const void *a, const void *b)
	{
		auto x = reinterpret_cast<const uint8_t*>(a);
		auto y = reinterpret_cast<const uint8_t*>(b);

		for (size_t i = 0; i < len; ++i) {
			if (x[i] < y[i])
				return -1;
			else if (x[i] > y[i])
				return 1;
		}

		return 0;
	}

	static void shift(size_t n, const uint8_t *in, uint8_t *out)
	{
		std::memcpy(out, in + 1, (n - 1) * sizeof(uint8_t));
		out[n - 1] = in[0];
	}

	size_t bwt(std::vector<uint8_t>& buf)
	{
		//std::cout << "bwt() input: ";
		//for (auto x : buf)
		//	std::cout << static_cast<uint8_t>(x);
		//std::cout << "\n";

		auto src = buf;
		auto n = buf.size();
		buf.resize(n * n);

		for (size_t i = 0; i < n - 1; ++i)
			shift(n, buf.data() + i * n, buf.data() + (i + 1) * n);

		//std::cout << "bwt() shifts:\n";
		//for (size_t i = 0; i < n; ++i) {
		//	for (size_t j = 0; j < n; ++j)
		//		std::cout << static_cast<uint8_t>(buf[i * n + j]);
		//	std::cout << "\n";
		//}

		len = n;
		std::qsort(buf.data(), n, n * sizeof(uint8_t), cmp);

		//std::cout << "bwt() sort:\n";
		//for (size_t i = 0; i < n; ++i) {
		//	for (size_t j = 0; j < n; ++j)
		//		std::cout << static_cast<uint8_t>(buf[i * n + j]);
		//	std::cout << "\n";
		//}

		size_t ret = 0;
		for (; ret < n; ++ret)
			if (!cmp(buf.data() + ret * n, src.data()))
				break;

		for (size_t i = 0; i < n; ++i)
			buf[i] = buf[n * (i + 1) - 1];

		buf.resize(n);
		return ret;
	}

	std::vector<uint8_t> bwt_inverse(const std::vector<uint8_t>& buf, size_t pos)
	{
		// Алгоритм быстрого обратного BWT на основе
		// приведённого в "Операция BWT, или новые методы сжатия" Юкина В.А.
		std::vector<uint16_t> count(257, 0);
		for (auto x : buf)
			count[x]++;

		uint16_t sum = 0;
		for (int i = 0; i < 257; ++i) {
			sum += count[i];
			count[i] = sum - count[i];
		}

		std::vector<uint16_t> T(buf.size());
		for (int i = 0; i < buf.size(); ++i)
			T[count[buf[i]]++] = i;

		std::vector<uint8_t> out(buf.size());
		for (size_t i = 0, j = pos; i < buf.size(); ++i) {
			j = T[j];
			out[i] = buf[j];
		}

		return out;
	}
}
