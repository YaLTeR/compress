#pragma once
#include <cstdint>
#include <cstdio>
#include <exception>
#include <iostream>

struct BitFile
{
	BitFile(FILE *f) :
		file(f),
		wbit_buffer(0),
		wbit_buffer_pos(0),
		rbit_buffer(0),
		rbit_buffer_pos(8)
	{
	};

	~BitFile()
	{
		if (wbit_buffer_pos != 0) {
			if (fputc(wbit_buffer, file) == EOF)
				std::cerr << "BitFile::~BitFile(): I/O error on writing.\n";
		}
	}

	void reset()
	{
		if (fseek(file, 0, SEEK_SET) != 0) {
			throw std::runtime_error("BitFile::reset(): I/O error on seeking.");
		}

		wbit_buffer_pos = 0;
		rbit_buffer_pos = 8;
	}

	// Returns the next bit, 0 past the end of file.
	uint8_t read_bit()
	{
		if (rbit_buffer_pos > 7) {
			if (fread(&rbit_buffer, sizeof(rbit_buffer), 1, file) != 1) {
				if (feof(file))
					return 0;
				else
					throw std::runtime_error("BitFile::read_bit(): I/O error on reading.");
			}

			rbit_buffer_pos = 0;
		}

		return (rbit_buffer >> (rbit_buffer_pos++)) & 1;
	}

	void write_bit(uint8_t bit)
	{
		wbit_buffer |= (bit & 1) << (wbit_buffer_pos++);

		if (wbit_buffer_pos > 7) {
			if (fwrite(&wbit_buffer, sizeof(wbit_buffer), 1, file) != 1) {
				throw std::runtime_error("BitFile::write_bit(): I/O error on writing.");
			}

			wbit_buffer = 0;
			wbit_buffer_pos = 0;
		}
	}

	FILE *file;

private:
	uint8_t wbit_buffer;
	uint8_t wbit_buffer_pos;
	uint8_t rbit_buffer;
	uint8_t rbit_buffer_pos;
};
