#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "C1581.h"
#include "iec_errors.h"
#include "C1581_Channel.h"
#include "C1581_CmdChannel.h"

#define MAX_TRACK       80
#define MAX_SECRTOR     39
#define MAX_DIR_ENTRIES	296

uint32_t trackOffset[80] = {
		0x00000,0x02800,0x05000,0x07800,0x0A000,0x0C800,0x0F000,0x11800,0x14000,0x16800,0x19000,0x1B800,0x1E000,0x20800,0x23000,0x25800,
		0x28000,0x2A800,0x2D000,0x2F800,0x32000,0x34800,0x37000,0x39800,0x3C000,0x3E800,0x41000,0x43800,0x46000,0x48800,0x4B000,0x4D800,
		0x50000,0x52800,0x55000,0x57800,0x5A000,0x5C800,0x5F000,0x61800,0x64000,0x66800,0x69000,0x6B800,0x6E000,0x70800,0x73000,0x75800,
		0x78000,0x7A800,0x7D000,0x7F800,0x82000,0x84800,0x87000,0x89800,0x8C000,0x8E800,0x91000,0x93800,0x96000,0x98800,0x9B000,0x9D800,
		0xA0000,0xA2B00,0xA5000,0xA7800,0xAA000,0xAC800,0xAF000,0xB1800,0xB4000,0xB6800,0xB9000,0xBB800,0xBE000,0xC0800,0xC3000,0xC5800
};

BamAllocation alloc;
C1581_CmdChannel cmd_channel;
C1581_Channel channels[14];

C1581 ::C1581()
{
	
}
C1581 ::~C1581()
{
}

void C1581 :: init(uint8_t deviceNumber)
{
	for (int x = 0; x < 14; x++)
	{
		channels[x].init(this);
		channels[x].number = x + 1;
	}

	cmd_channel.init(this);

    deviceum = deviceNumber;
}
void C1581 :: powerOn(void)
{
    power = true;
}
void C1581 :: powerOff(void)
{
    power = false;
}

void C1581 :: insertDisk(uint8_t *image)
{
	memcpy(curdisk, image, DISK_SIZE);
}

void C1581 :: ejectDisk(void)
{
    hasDisk = false;
}

uint8_t C1581::goTrackSector(uint8_t track, uint8_t sector)
{
	if (track < 1 || track > MAX_TRACK || sector < 0 || sector > MAX_SECRTOR)
	{
		return -1;
	}
	curtrack = track;
	cursector = sector;
	nxttrack = 0;
	nxtsector = 0;
	return 0;
}

void C1581::readSector(void)
{
	uint32_t offset = trackOffset[this->curtrack-1];
	offset = offset + this->cursector * 256;
	
	for (int x = 0; x < 256; x++)
		sectorBuffer[x] = curdisk[offset + x];

	return;
}

uint8_t C1581::nextSector(void)
{
	if (this->cursector == 39)
		return -1;
	
	this->cursector++;
	return 0;
}
uint8_t C1581::prevSector(void)
{
	if (this->cursector == 0)
		return -1;

	this->cursector--;
	return 0;
}
uint8_t C1581::nextTrack(void)
{
	if (this->curtrack == 80)
		return -1;

	this->curtrack++;
	return 0;
}
uint8_t C1581::prevTrack(void)
{
	if (this->curtrack == 1)
		return -1;

	this->curtrack--;
	return 0;
}


int C1581::findFreeSector(uint8_t *track, uint8_t *sector)
{
	goTrackSector(40, 1);
	readSector();

	bool notFound = true;
	int offset = 0;
	uint8_t side = 1;

	alloc.track = 1;
	alloc.sector = 0;
	uint8_t *bamdata = sectorBuffer + 0x10;

	while (1)
	{
		if (bamdata[offset] > 0)
		{
			// next 5 bytes are sector allocation for the track in reverse
			for (int z = 0; z < 5; z++)
			{			
				uint8_t bit = 128;	
				uint8_t b = bamdata[++offset];

				// reverse the bits
				b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
				b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
				b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
					
				do {
					if ((b & bit) == bit)
					{
						*track = alloc.track;
						*sector = alloc.sector;

						// test output
						//printf("\n * Track %d, Sector %d", alloc.track, alloc.sector);
						return ERR_OK;
					}
					else
					{
						// test output
						//printf("\n x Track %d, Sector %d", alloc.track, alloc.sector);
					}

					bit = bit / 2;	
					alloc.sector++;

				} while (bit >= 1);
			
			}

			alloc.track++;
			alloc.sector = 0;
			offset++;
		}
		else
		{
			//printf("\n x Track %d", alloc.track);
			alloc.track++;
			alloc.sector = 0;
			offset += 6;
		}

		// allocated, so move to next track data
		if (alloc.track == 41)
		{
			side++;
			goTrackSector(40, 2);
			readSector();
			bamdata = sectorBuffer + 0x10;
			offset = 0;
		}

		// if all of side 2 is allocated
		if (alloc.track == 81)
		{
			// disk is full
			return ERR_DISK_FULL;
		}

	}
}

uint8_t C1581::getFileTrackSector(char *filename, uint8_t *track, uint8_t *sector)
{
	int ptr = 0;
	uint8_t linePtr = 0;
	DirectoryEntry *dirEntry = new DirectoryEntry;
	int localbufferidx = 0;

	int x = 0;

	localbufferidx = getNextDirectoryEntry(dirEntry);

	while (localbufferidx != -1)
	{
		char line[17];
		uint8_t z = 0;
		for(; z < 16; z++)
		{
			if (dirEntry->filename[z] == 0xa0)
				break;

			line[z] = dirEntry->filename[z];
		}
		
		line[z] = 0;

		if (strcmp(line, filename) == 0)
		{
			*track = dirEntry->first_data_track;
			*sector = dirEntry->first_data_sector;
			return ERR_OK;
		}

		localbufferidx = getNextDirectoryEntry(dirEntry);
	}

	return ERR_FILE_NOT_FOUND;
}

uint8_t C1581::open(uint8_t channel, uint8_t *command, uint8_t secondary)
{
	if (channel < 15)
		return channels[channel - 1].open(command, secondary);
	else
		return cmd_channel.open(command, secondary);
}

uint8_t C1581::read(uint8_t channel, uint8_t *byte)
{
	if (channel < 15)
	{
		return channels[channel-1].read(byte);
	}
	else
	{
		return cmd_channel.read(byte);
	}

	return ERR_OK;
}

uint8_t C1581::write(uint8_t channel, uint8_t byte)
{
	return ERR_OK;
}

uint8_t C1581::close(uint8_t channel)
{
	if (channel < 15)
		return channels[channel - 1].close();
	else
		return cmd_channel.close();
}

int C1581::get_directory(uint8_t *buffer)
{
	DirectoryEntry *dirEntry = new DirectoryEntry;
	int ptr = 0;
	uint16_t blocksfree = 3160;
	uint16_t nextLinePtr = 0;

	uint8_t filetypes[6][3] = {
			{ 'D', 'E', 'L' },
			{ 'S', 'E', 'Q' },
			{ 'P', 'R', 'G' },
			{ 'U', 'S', 'R' },
			{ 'R', 'E', 'L' },
			{ 'C', 'B', 'M' }
	};

	goTrackSector(40, 0);
	readSector();

	buffer[ptr++] = 0x01;
	buffer[ptr++] = 0x08;

	nextLinePtr = ptr;
	ptr += 2;

	// disk header
	sprintf(((char *)buffer + ptr), "%c%c%c%c", 0x00, 0x00, 0x12, 0x22);
	ptr += 4;

	for (int t = 0x04; t < 0x14; t++)
	{
		sprintf((char *)buffer + ptr, "%c", sectorBuffer[t]);
		ptr++;
	}

	sprintf(((char *)buffer + ptr), "%c %c%c %c%c\0", 0x22, sectorBuffer[0x16],
		sectorBuffer[0x17], sectorBuffer[0x19], sectorBuffer[0x1A]);
	ptr += 8;

	// testing with normal C64 line links
	buffer[nextLinePtr] = (0x0801 + ptr) & 0xff;
	buffer[nextLinePtr + 1] = ((0x0801 + ptr) >> 8) & 0xff;

	// actual data has no line links (so that it can be linked by other cbm machines)
	//buffer[nextLinePtr] = 0x0101 & 0xff;
	//buffer[nextLinePtr + 1] = (0x0101 >> 8) & 0xff;

	int x = getNextDirectoryEntry(dirEntry);
	char line[30];
	int linePtr = 0;
	int ctr = 0;
	while (x == 0)
	{
		if (dirEntry->file_type != 0)
		{
			ctr++;
			linePtr = 0;

			nextLinePtr = ptr;
			ptr += 2;

			line[linePtr++] = dirEntry->size_lo;
			line[linePtr++] = dirEntry->size_hi;

			int blocks = dirEntry->size_hi * 256 + dirEntry->size_lo;
			blocksfree -= blocks;
			int digits = 1;
			while (blocks >= 0)
			{
				blocks /= 10;
				++digits;

				if (blocks == 0)
					break;
			}
			int spaces = 5 - digits;
			for (; spaces > 0; spaces--)
				line[linePtr++] = ' ';

			line[linePtr++] = 0x22;

			int filenamlen = 0;
			for (; filenamlen < 16; filenamlen++)
			{
				if (dirEntry->filename[filenamlen] == 0xa0)
					break;

				line[linePtr++] = dirEntry->filename[filenamlen];
			}

			line[linePtr++] = 0x22;

			for (; filenamlen < 16; filenamlen++)
				line[linePtr++] = ' ';

			uint8_t tmptype = dirEntry->file_type;
			tmptype = tmptype & ~128;
			tmptype = tmptype & ~64;
			tmptype = tmptype & ~32;
			tmptype = tmptype & ~16;

			if ((dirEntry->file_type & 0x80) != 0x80)
				line[linePtr++] = '*';
			else
				line[linePtr++] = ' ';

			line[linePtr++] = filetypes[tmptype][0];
			line[linePtr++] = filetypes[tmptype][1];
			line[linePtr++] = filetypes[tmptype][2];

			if ((dirEntry->file_type & 64) == 64)
				line[linePtr++] = '<';
			else
				line[linePtr++] = ' ';

			line[linePtr++] = 0;

			if (linePtr > 30)
			{
				abort();
			}

			for (int x = 0; x < linePtr; x++)
				buffer[ptr++] = line[x];

			buffer[nextLinePtr] = (0x0801 + ptr) & 0xff;
			buffer[nextLinePtr + 1] = ((0x0801 + ptr) >> 8) & 0xff;
		}

		x = getNextDirectoryEntry(dirEntry);
	}

	nextLinePtr = ptr;
	ptr += 2;
	sprintf(((char *)buffer + ptr), "%c%cBLOCKS FREE.              ", blocksfree % 256, blocksfree / 256);
	ptr += 29;

	buffer[nextLinePtr] = (0x0801 + ptr) & 0xff;
	buffer[nextLinePtr + 1] = ((0x0801 + ptr) >> 8) & 0xff;

	// end of program
	buffer[ptr] = 0;
	buffer[ptr + 1] = 0;
	buffer[ptr + 2] = 0;
	ptr += 3;


	//delete dirEntry;
	return ptr;
}

int C1581::getNextDirectoryEntry(DirectoryEntry *dirEntry)
{
	static bool firstcall = true;
	static int dirctr = 0;
	static bool lastdirsector = false;
	bool readnextsector = false;

	int offset = 0;

	if (firstcall)
	{
		firstcall = false;
		goTrackSector(40, 3);
		readnextsector = true;
	}
	else
	{
		if (dirctr % 8 == 0)
		{
			if (lastdirsector)
				return -1;

			goTrackSector(nxttrack, nxtsector);
			dirctr = 0;
			readnextsector = true;
		}
	}

	offset = dirctr * 32;

	if (readnextsector)
	{
		readnextsector = false;
		readSector();
		nxttrack = sectorBuffer[0x00 + offset];
		nxtsector = sectorBuffer[0x01 + offset];
	}

	if (nxttrack == 0x00 && nxtsector == 0xff)
		lastdirsector = true;
	
	dirEntry->file_type = sectorBuffer[0x02 + offset];
	dirEntry->first_data_track = sectorBuffer[0x03 + offset];
	dirEntry->first_data_sector = sectorBuffer[0x04 + offset];

	for (int v = 0; v < 16; v++)
		dirEntry->filename[v] = sectorBuffer[(5 + v) + offset];

	dirEntry->first_track_ssb = sectorBuffer[0x15 + offset];
	dirEntry->first_sector_ssb = sectorBuffer[0x16 + offset];
	dirEntry->rel_file_length = sectorBuffer[0x17 + offset];

	for (int v = 0; v < 6; v++)
		dirEntry->unused[v + dirctr] = sectorBuffer[0x18 + v + offset];

	dirEntry->size_lo = sectorBuffer[0x1e + (dirctr * 32)];
	dirEntry->size_hi = sectorBuffer[0x1f + (dirctr * 32)];

	dirctr++;

	return 0;

}

bool C1581::getNextFileSector(uint8_t* filename)
{
	static bool firstcall = true;
	uint8_t file_track;
	uint8_t file_sector;
	int ptr = 0;

	if (firstcall)
	{
		getFileTrackSector((char *)filename, &file_track, &file_sector);
		goTrackSector(file_track, file_sector);
	}
	else
	{
		goTrackSector(nxttrack, nxtsector);
	}
	
	readSector();
	nxttrack = sectorBuffer[0x00];
	nxtsector = sectorBuffer[0x01];
	
	if (nxttrack == 0x00)
		return false;

	return true;
}