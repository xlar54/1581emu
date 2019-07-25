
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "C1581_CmdChannel.h"
#include "iec_errors.h"


C1581_CmdChannel::C1581_CmdChannel()
{

}

void C1581_CmdChannel::init(C1581 *parent)
{
	c1581 = parent;
}
C1581_CmdChannel::~C1581_CmdChannel()
{
}

uint8_t C1581_CmdChannel::open(uint8_t *command, uint8_t secondary)
{
	uint8_t track;
	uint8_t sector;
	localbuffer = new uint8_t[65535];
	localbufferidx = 0;
	
	channelopen = true;

	command_t parsed_command;
	char commandtxt[52];
	strcpy(commandtxt, (char *)command);

	parse_command(commandtxt, &parsed_command);

	if (strcmp(parsed_command.cmd, "$") == 0)
	{
		strcpy((char *)lastcommand, (char *)command);
		localbufferidx = get_directory(localbuffer);
		return ERR_OK;
	}

	if (strcmp(parsed_command.cmd, "I0") == 0)
	{
		localbufferidx = initialize();
	}

	if (strcmp(parsed_command.cmd, "N") == 0)
	{
		localbufferidx = initialize();
	}

	return ERR_OK;
}

bool C1581_CmdChannel::readblock()
{
	// localbuffer contains the full command response (all the data)
	// we load it, 256 bytes at a time, into the databuffer
	bool lastblock = false;

	if (localbuffer == 0)
		return ERR_FILE_NOT_OPEN;

	int blockdatasize = 256;

	// clear the databuffer
	memset(databuffer, 0, sizeof(databuffer));

	// this should capture end of file
	if (localbufferidx < localblocknum * 256)
	{
		blockdatasize = ((localblocknum * 256) % localbufferidx);
		localblocknum = 0;
		lastblock = true;
	}
	
	// fill the channel buffer
	for (int t = 0; t < blockdatasize; t++)
	{
		uint8_t b = localbuffer[localblocknum * 256 + t];
		databuffer[t] = b;
	}
	
	localblocknum++;
	return lastblock;
}

uint8_t C1581_CmdChannel::read(uint8_t *byte)
{
	static bool lastblock = false;

	// is it time to read a new block of data?
	if (databufferidx == 0)
		lastblock = readblock();
	
	// return a byte
	*byte = databuffer[databufferidx++];

	if (databufferidx == 0 && lastblock)
		return ERR_READ_ERROR;
	else
		return ERR_OK;
}

uint8_t C1581_CmdChannel::write(uint8_t byte)
{
	return ERR_OK;
}

uint8_t C1581_CmdChannel::close()
{
	databufferidx = 0;
	delete[] localbuffer;
	this->channelopen = false;

	return ERR_OK;
}

void C1581_CmdChannel::parse_command(char *buffer, command_t *command)
{
	// initialize
	for (int i = 0; i < 5; i++) {
		command->names[i].name = 0;
		command->names[i].drive = -1;
		command->names[i].separator = ' ';
	}
	command->digits = -1;
	command->cmd = buffer;

	// First strip off any carriage returns, in any case
	int len = strlen(buffer);
	while ((len > 0) && (buffer[len - 1] == 0x0d)) {
		len--;
		buffer[len] = 0;
	}

	// look for , and split
	int idx = 0;
	command->names[0].name = buffer;
	for (int i = 0; i < len; i++) {
		if ((buffer[i] == ',') || (buffer[i] == '=')) {
			idx++;
			if (idx == 5) {
				break;
			}
			command->names[idx].name = buffer + i + 1;
			command->names[idx].separator = buffer[i];
			buffer[i] = 0;
		}
	}

	// look for : and split
	for (int j = 0; j < 5; j++) {
		char *s = command->names[j].name;
		if (!s) {
			break;
		}
		len = strlen(s);
		for (int i = 0; i < len; i++) {
			if (s[i] == ':') {
				s[i] = 0;
				command->names[j].name = s + i + 1;
				// now snoop off the digits and place them into drive number
				int mult = 1;
				int dr = 0;
				while (isdigit(s[--i])) {
					dr += (s[i] - '0') * mult;
					mult *= 10;
					s[i] = 0;
				}
				if (mult > 1) { // digits found
					command->names[j].drive = dr;
				}
				break;
			}
		}
	}
	if (command->names[0].name == command->cmd) {
		command->names[0].name = 0;
		int mult = 1;
		int dr = 0;
		char *s = command->cmd;
		int i = strlen(s);
		while (isdigit(s[--i])) {
			dr += (s[i] - '0') * mult;
			mult *= 10;
			//s[i] = 0; // we should not snoop them off; they may be part of a filename
		}
		if (mult > 1) { // digits found
			command->digits = dr;
		}
	}
}

int  C1581_CmdChannel::initialize(void)
{
	uint8_t resp[] = { 0x30, 0x0d, 'O', 'K', 0x0d, 0x30, 0x0d, 0x30, 0x0d };

	for (int x = 0; x < 9; x++)
		localbuffer[x] = resp[x];
	
	return sizeof(resp);
}
uint8_t C1581_CmdChannel::format(uint8_t* diskname, uint8_t* id)
{
	return ERR_OK;
}
uint8_t C1581_CmdChannel::scratchFile(uint8_t *filename)
{
	return ERR_OK;
}
uint8_t C1581_CmdChannel::renameFile(uint8_t *filename, uint8_t *newfilename)
{
	return ERR_OK;
}
uint8_t C1581_CmdChannel::copyFile(uint8_t *srcfilename, uint8_t *destfilename)
{
	return ERR_OK;
}
uint8_t C1581_CmdChannel::validate(void)
{
	return ERR_OK;
}

int C1581_CmdChannel::get_directory(uint8_t *buffer)
{
	DirectoryEntry dirEntry;
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

	c1581->goTrackSector(40, 0);
	c1581->readSector();

	buffer[ptr++] = 0x01;
	buffer[ptr++] = 0x08;

	nextLinePtr = ptr;
	ptr += 2;

	// disk header
	sprintf(((char *)buffer + ptr), "%c%c%c%c", 0x00, 0x00, 0x12, 0x22);
	ptr += 4;

	for (int t = 0x04; t < 0x14; t++)
	{
		sprintf((char *)buffer + ptr, "%c", c1581->sectorBuffer[t]);
		ptr++;
	}

	sprintf(((char *)buffer + ptr), "%c %c%c %c%c\0", 0x22, c1581->sectorBuffer[0x16],
		c1581->sectorBuffer[0x17], c1581->sectorBuffer[0x19], c1581->sectorBuffer[0x1A]);
	ptr += 8;

	// testing with normal C64 line links
	//buffer[nextLinePtr] = (0x0801 + ptr) & 0xff;
	//buffer[nextLinePtr + 1] = ((0x0801 + ptr) >> 8) & 0xff;

	// actual data has no line links (so that it can be linked by other cbm machines)
	buffer[nextLinePtr] = 0x0101 & 0xff;
	buffer[nextLinePtr + 1] = (0x0101 >> 8) & 0xff;

	int x = getNextDirectoryEntry(&dirEntry);
	char line[29];
	int linePtr = 0;

	while (x == 0)
	{
		linePtr = 0;

		nextLinePtr = ptr;
		ptr += 2;

		line[linePtr++] = dirEntry.size_lo;
		line[linePtr++] = dirEntry.size_hi;

		int blocks = dirEntry.size_hi * 256 + dirEntry.size_lo;
		blocksfree -= blocks;
		int digits = 0;
		while (blocks != 0)
		{
			blocks /= 10;
			++digits;
		}
		int spaces = 4 - digits;
		for (; spaces > 0; spaces--)
			line[linePtr++] = ' ';

		line[linePtr++] = 0x22;

		int filenamlen = 0;
		for (; filenamlen < 16; filenamlen++)
		{
			if (dirEntry.filename[filenamlen] == 0xa0)
				break;

			line[linePtr++] = dirEntry.filename[filenamlen];
		}

		line[linePtr++] = 0x22;
		
		for (; filenamlen < 16 ; filenamlen++)
			line[linePtr++] = ' ';

		uint8_t tmptype = dirEntry.file_type;
		tmptype = tmptype & ~128;
		tmptype = tmptype & ~64;
		tmptype = tmptype & ~32;
		tmptype = tmptype & ~16;

		if ((dirEntry.file_type & 0x80) != 0x80)
			line[linePtr++] = '*';
		else
			line[linePtr++] = ' ';

		line[linePtr++] = filetypes[tmptype][0];
		line[linePtr++] = filetypes[tmptype][1];
		line[linePtr++] = filetypes[tmptype][2];

		if ((dirEntry.file_type & 64) == 64)
			line[linePtr++] = '<';
		else
			line[linePtr++] = ' ';

		line[linePtr++] = 0;

		if (linePtr > 29)
		{
			abort();
		}

		for (int x = 0; x < linePtr; x++)
			buffer[ptr++] = line[x];

		buffer[nextLinePtr] = (0x0801 + ptr) & 0xff;
		buffer[nextLinePtr + 1] = ((0x0801 + ptr) >> 8) & 0xff;

		x = getNextDirectoryEntry(&dirEntry);
	}

	nextLinePtr = ptr;
	ptr += 2;
	sprintf(((char *)buffer + ptr), "%c%cBLOCKS FREE.              ", blocksfree % 256, blocksfree / 256);
	ptr += 28;

	buffer[nextLinePtr] = (0x0801 + ptr) & 0xff;
	buffer[nextLinePtr + 1] = ((0x0801 + ptr) >> 8) & 0xff;

	// end of program
	buffer[ptr] = 0;
	buffer[ptr + 1] = 0;
	buffer[ptr + 2] = 0;
	ptr += 3;

	return ptr;
}

int C1581_CmdChannel::getFirstDirectoryEntry(DirectoryEntry *dirEntry)
{
	c1581->goTrackSector(40, 3);
	c1581->readSector();

	c1581->nxttrack = c1581->sectorBuffer[0x00];
	c1581->nxtsector = c1581->sectorBuffer[0x01];

	if (c1581->nxttrack == 0x00 && c1581->sectorBuffer[0x02] == 0)
		return -1;

	dirEntry->file_type = c1581->sectorBuffer[0x02];
	dirEntry->first_data_track = c1581->sectorBuffer[0x03];
	dirEntry->first_data_sector = c1581->sectorBuffer[0x04];

	for (int v = 0; v < 16; v++)
		dirEntry->filename[v] = c1581->sectorBuffer[5 + v];

	dirEntry->first_track_ssb = c1581->sectorBuffer[0x15];
	dirEntry->first_sector_ssb = c1581->sectorBuffer[0x16];
	dirEntry->rel_file_length = c1581->sectorBuffer[0x17];

	for (int v = 0; v < 6; v++)
		dirEntry->unused[v] = c1581->sectorBuffer[0x18 + v];

	dirEntry->size_lo = c1581->sectorBuffer[0x1e];
	dirEntry->size_hi = c1581->sectorBuffer[0x1f];

	return 0;
}

int C1581_CmdChannel::getNextDirectoryEntry(DirectoryEntry *dirEntry)
{
	static bool firstcall = true;
	static int dirctr = 0;

	if (firstcall)
	{
		firstcall = false;
		c1581->goTrackSector(40, 3);
		c1581->readSector();
	}
	else
	{
		if (dirctr % 9 == 0)
		{
			c1581->goTrackSector(c1581->nxttrack, c1581->nxtsector);
			c1581->readSector();
			dirctr = 0;
		}
	}

	int offset = dirctr * 32;

	c1581->nxttrack = c1581->sectorBuffer[0x00 + offset];
	c1581->nxtsector = c1581->sectorBuffer[0x01 + offset];
	

	if (c1581->nxttrack == 0x00 && c1581->sectorBuffer[0x02 + offset] == 0)
		return -1;

	dirEntry->file_type = c1581->sectorBuffer[0x02 + offset];
	dirEntry->first_data_track = c1581->sectorBuffer[0x03 + offset];
	dirEntry->first_data_sector = c1581->sectorBuffer[0x04 + offset];

	for (int v = 0; v < 16; v++)
		dirEntry->filename[v] = c1581->sectorBuffer[(5 + v) + offset];

	dirEntry->first_track_ssb = c1581->sectorBuffer[0x15 + offset];
	dirEntry->first_sector_ssb = c1581->sectorBuffer[0x16 + offset];
	dirEntry->rel_file_length = c1581->sectorBuffer[0x17 + offset];

	for (int v = 0; v < 6; v++)
		dirEntry->unused[v * dirctr] = c1581->sectorBuffer[0x18 + v + offset];

	dirEntry->size_lo = c1581->sectorBuffer[0x1e + (dirctr * 32)];
	dirEntry->size_hi = c1581->sectorBuffer[0x1f + (dirctr * 32)];

	dirctr++;
	return 0;

}

