#pragma once
#include <cstdint>
#include <cstdio>
#include <exception>
#include <iostream>
#include <vector>

#include "bwt.h"

struct BwtFile
{
	const uint16_t BUFFER_SIZE = 32768;

	BwtFile(FILE *f) :
		file(f),
		pos(0),
		wpos(-2)
	{
	};

	~BwtFile()
	{
		if (wpos > 0) {
			for (auto c : BWT::bwt_inverse(buffer, bwt_pos)) {
				if (fputc(c, file) == EOF) {
					std::cerr << "BwtFile::~BwtFile(): I/O error on writing.\n";
				}
			}
		}
	}

	int getc()
	{
		if (pos == -1) {
			pos++;
			return *(reinterpret_cast<const uint8_t*>(&bwt_pos) + 1);
		}

		if (pos < buffer.size()) {
			return buffer[pos++];
		}

		if (feof(file))
			return EOF;

		buffer.clear();
		int c;
		uint16_t i = 0;
		while (i++ < BUFFER_SIZE && (c = fgetc(file)) != EOF) {
			buffer.push_back(static_cast<uint8_t>(c));
		}
		pos = -1;

		bwt_pos = static_cast<uint16_t>(BWT::bwt(buffer));
		return *reinterpret_cast<const uint8_t*>(&bwt_pos);
	}

	void putc(uint8_t c)
	{
		if (wpos == -2) {
			wpos++;
			*reinterpret_cast<uint8_t*>(&bwt_pos) = c;
			return;
		}

		if (wpos == -1) {
			wpos++;
			*(reinterpret_cast<uint8_t*>(&bwt_pos) + 1) = c;
			return;
		}

		if (wpos < BUFFER_SIZE) {
			wpos++;
			buffer.push_back(c);
			return;
		}

		for (auto c : BWT::bwt_inverse(buffer, bwt_pos)) {
			if (fputc(c, file) == EOF) {
				std::cerr << "BwtFile::putc(): I/O error on writing.\n";
			}
		}

		buffer.clear();
		*reinterpret_cast<uint8_t*>(&bwt_pos) = c;
		wpos = -1;
	}

	FILE *file;

private:
	std::vector<uint8_t> buffer;
	uint16_t bwt_pos;
	int pos;
	int wpos;
};
