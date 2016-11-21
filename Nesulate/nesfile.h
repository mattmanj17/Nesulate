#pragma once
#include <cstdio>
#include "Types.h"
#include <cassert>

// https://wiki.nesdev.com/w/index.php/INES
// https://wiki.nesdev.com/w/index.php/NES_2.0
// load NES 2.0 files

class NesFile
{
public:
	void Load(FILE* pFile)
	{
		// read header

		byte header[16];
		fread(&header, sizeof(byte), 16, pFile);

		// validate file format
		// begins with "NES" followed by MS-DOS end-of-file
		assert(WordAt(header) == 0x4E45531A); 

		// make sure we are a nes 2.0 rom
		// if header byte 7 AND $0C = $08
		// then assume we are nes 2.0
		assert((header[7] & 0x0C) == 0x08);

		// program rom size
		// header 4 and lower 4 bits from header 9
		half nPrgRom = header[4];
		nPrgRom |= (header[9] & 0xF0) << 8;

		// character rom size
		// header 4 and upper 4 bits from header 9
		// (0 indicates CHR RAM)
		half nChrRom = header[5];
		nChrRom |= (header[9] & 0x0F) << 4;

		// mapper number (12 bits)
		// top 4 bits of byte 6 are lower 4 bits of mapper number
		half nMapper = (header[6] & 0xF0) >> 4;
		
		// top 4 bits of byte 7 are next 4 bits of mapper number
		nMapper |= (header[7] & 0xF0);

		// lower 4 bits of byte 8 are highest 4 bits of mapper number
		nMapper |= (header[8] & 0x0F) << 8;

		// sub mapper number
		// upper 4 bits of byte 8
		byte nSubMapper = (header[8] & 0xF0) >> 4;

	}
private:
	word flags;		// flags 6, 7, 9, 10
};
