#define _CRT_SECURE_NO_WARNINGS
#define _FILE_OFFSET_BITS 64
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>
#include <vector>

#include "bitfile.h"
#include "boundaries.h"
#include "ppm.h"
#include "mtf.h"
#include "bwt.h"
#include "bwtfile.h"

void usage()
{
	std::cerr << "Usage:\n"
		"\tcompress c <input file> <output file> ari\n"
		"\tcompress c <input file> <output file> ppm\n"
		"\tcompress c <input file> <output file> bwt\n"
		"\tcompress d <input file> <output file>\n";
}

void bits_plus_follow(BitFile& f, uint64_t count, uint8_t bit)
{
	f.write_bit(bit);
	for (; count > 0; --count)
		f.write_bit(!bit);
}

void compress(FILE *in, FILE *out, bool ppm, bool bwt)
{
	if (fseek(in, 0, SEEK_END) != 0)
		throw std::runtime_error("compress(): error seeking the input file.");

#ifdef _WIN32
	__int64 size = _ftelli64(in);
#else
	off_t size = ftello(in);
#endif
	if (size == -1)
		throw std::runtime_error("compress(): error getting the input file size.");
	//if (size >= (1ull << 32))
	//	throw std::runtime_error("compress(): input file too big (bigger than 4 GiB).");

	if (fseek(in, 0, SEEK_SET) != 0)
		throw std::runtime_error("compress(): error seeking the input file.");

	BitFile bf_out(out);
	bf_out.write_bit(ppm);
	bf_out.write_bit(bwt);

	if (ppm)
		PPM::init();
	else
		Boundaries::prefill();

	if (bwt)
		MTF::init();

	BwtFile bwti(in);

	uint64_t l_prev = 0,
		h_prev = (1ull << 32) - 1,
		first_qtr = (h_prev + 1) / 4,
		half = first_qtr * 2,
		third_qtr = first_qtr * 3,
		bit_count = 0;

	//int percentage = 0;
	while (true) {
		int c = bwt ? bwti.getc() : fgetc(in);
		if (c == EOF)
			c = 256;

//#ifdef _WIN32
//		int64_t p = _ftelli64(in) * 100 / size;
//#else
//		int64_t p = ftello(in) * 100 / size;
//#endif
//		if (p != percentage) {
//			percentage = p;
//			std::cout << "Progress: " << p << "%\n";
//			// std::cerr << "Progress: " << p << "%\n";
//		}

		if (bwt)
			c = MTF::process(c);

		const uint32_t *b;
		if (ppm)
			b = PPM::get();
		else
			b = Boundaries::get();

		uint32_t denom = b[257];
		uint64_t l = l_prev + (b[c] * (h_prev - l_prev + 1)) / denom;
		uint64_t h = l_prev + (b[c + 1] * (h_prev - l_prev + 1)) / denom - 1;

		// std::cerr << "denom: " << denom << "\n";
		// std::cerr << "l_prev: " << l_prev << "; h_prev: " << h_prev << "\n";
		// std::cerr << "b[c]: " << b[c] << "; b[c+1]: " << b[c+1] << "\n";
		// std::cerr << "b[c] * (h_prev - l_prev + 1): " << b[c] * (h_prev - l_prev + 1) << "\n";
		// std::cerr << "b[c+1] * (h_prev - l_prev + 1): " << b[c+1] * (h_prev - l_prev + 1) << "\n";
		// std::cerr << "(b[c] * (h_prev - l_prev + 1)) / denom: " << (b[c] * (h_prev - l_prev + 1)) / denom << "\n";
		// std::cerr << "(b[c+1] * (h_prev - l_prev + 1)) / denom: " << (b[c+1] * (h_prev - l_prev + 1)) / denom << "\n";
		// std::cerr << "l: " << l << "; h: " << h << "\n";

		for (;;) {
			if (h < half) {
				bits_plus_follow(bf_out, bit_count, 0);
				bit_count = 0;
			} else if (l >= half) {
				bits_plus_follow(bf_out, bit_count, 1);
				bit_count = 0;
				l -= half;
				h -= half;
			} else if ((l >= first_qtr) && (h < third_qtr)) {
				bit_count++;
				l -= first_qtr;
				h -= first_qtr;
			} else {
				break;
			}

			l += l;
			h += h + 1;
		}

		if (c == 256)
			break;

		l_prev = l;
		h_prev = h;

		if (ppm)
			PPM::update(static_cast<uint8_t>(c));
		else
			Boundaries::update(static_cast<uint8_t>(c));
	}

	bf_out.write_bit(1);

	if (ferror(in))
		throw std::runtime_error("compress(): I/O error on reading.");
}

void decompress(FILE *in, FILE *out)
{
	BitFile bf_in(in);
	auto ppm = bf_in.read_bit();
	auto bwt = bf_in.read_bit();

	if (ppm)
		PPM::init();
	else
		Boundaries::prefill();

	if (bwt)
		MTF::init();

	BwtFile bwto(out);

	uint64_t l_prev = 0,
		h_prev = (1ull << 32) - 1,
		first_qtr = (h_prev + 1) / 4,
		half = first_qtr * 2,
		third_qtr = first_qtr * 3;

	uint32_t value = 0;
	for (int i = 0; i < 32; ++i) {
		value += value + bf_in.read_bit();
	}

	bool loop = true;
	while (loop) {
		const uint32_t *b;
		if (ppm)
			b = PPM::get();
		else
			b = Boundaries::get();

		uint32_t denom = b[257];
		for (uint16_t c = 0; c < 257; ++c) {
			uint64_t l = l_prev + (b[c] * (h_prev - l_prev + 1)) / denom;
			uint64_t h = l_prev + (b[c + 1] * (h_prev - l_prev + 1)) / denom - 1;

			if (value >= l && value <= h) {
				auto original_symbol = c;
				if (bwt)
					c = MTF::process_reverse(c);

				if (c == 256) {
					loop = false;
					break;
				}

				for (;;) {
					if (h < half) {
						;
					} else if (l >= half) {
						value -= static_cast<uint32_t>(half);
						l -= half;
						h -= half;
					} else if ((l >= first_qtr) && (h < third_qtr)) {
						value -= static_cast<uint32_t>(first_qtr);
						l -= first_qtr;
						h -= first_qtr;
					} else {
						break;
					}

					l += l;
					h += h + 1;

					value += value + bf_in.read_bit();
				}

				l_prev = l;
				h_prev = h;

				uint8_t symbol = static_cast<uint8_t>(c);
				if (bwt) {
					bwto.putc(symbol);
				} else {
					if (fwrite(&symbol, sizeof(symbol), 1, out) != 1)
						throw std::runtime_error("decompress(): I/O error on writing.");
				}

				if (ppm)
					PPM::update(static_cast<uint8_t>(original_symbol));
				else
					Boundaries::update(static_cast<uint8_t>(original_symbol));

				break;
			}
		}
	}
}

#ifdef _WIN32
int wmain(int argc, wchar_t *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	if (argc < 4 || argc > 5) {
		usage();
		return 1;
	}

	if ((argv[1][0] != 'c' && argv[1][0] != 'd') || argv[1][1] != '\0') {
		usage();
		return 1;
	}

	if (argc == 5 && (argv[1][0] != 'c' ||
#ifdef _WIN32
		(wcscmp(argv[4], L"ari") && wcscmp(argv[4], L"ppm") && wcscmp(argv[4], L"bwt"))
#else
		(strcmp(argv[4], "ari") && strcmp(argv[4], "ppm") && strcmp(argv[4], "bwt"))
#endif
		)) {
		usage();
		return 1;
	}

#ifdef _WIN32
	FILE *in = _wfopen(argv[2], L"rb");
#else
	FILE *in = fopen(argv[2], "rb");
#endif
	if (!in) {
		perror("Error opening the input file");
		return 1;
	}

#ifdef _WIN32
	FILE *out = _wfopen(argv[3], L"wb");
#else
	FILE *out = fopen(argv[3], "wb");
#endif
	if (!out) {
		perror("Error opening the output file");
		return 1;
	}

	try {
		if (argv[1][0] == 'c') {
#ifdef _WIN32
			auto ppm = (argc == 5) && (argv[4][0] == L'p');
			auto bwt = (argc == 5) && (argv[4][0] == L'b');
#else
			auto ppm = (argc == 5) && (argv[4][0] == 'p');
			auto bwt = (argc == 5) && (argv[4][0] == 'b');
#endif
			compress(in, out, ppm, bwt);
		} else {
			decompress(in, out);
		}
	} catch (const std::exception& ex) {
		std::cerr << "Error: " << ex.what() << "\n";
	}

#ifdef _WIN32
	_fcloseall();
#else
	fcloseall();
#endif
	return 0;
}
