#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "C1581.h"
#include "C1581_Channel.h"
#include "iec_errors.h"

C1581* C1581_Channel::c1581;

C1581_Channel::C1581_Channel()
{
	
}
C1581_Channel::~C1581_Channel()
{
	c1581 = NULL;
}

void C1581_Channel::init(C1581 *parent)
{
	c1581 = parent;
}

uint8_t C1581_Channel::open(uint8_t *filename, uint8_t secondary)
{
	uint8_t track;
	uint8_t sector;
	localbuffer = new uint8_t[65535];
	localbufferidx = 0;

	channelopen = true;
	bool moredata = false;
	
	// load entire file into localbuffer
	do
	{
		uint8_t rawsector[256];
		moredata = c1581->getNextFileSector(filename);

		//skip the next track/sector bytes
		for (int t = 2; t < 256; t++)
			localbuffer[localbufferidx++] = c1581->sectorBuffer[t];

	} while (moredata == true);

	return ERR_OK;
}

bool C1581_Channel::readblock()
{
	// localbuffer contains the full command response (all the data)
	// we load it, 256 bytes at a time, into the databuffer
	bool lastblock = false;

	if (localbuffer == 0)
		return ERR_FILE_NOT_OPEN;

	// the first two bytes are track/sector, 
	// so the block size is actually only 254 bytes
	int blockdatasize = 254;

	// clear the databuffer
	memset(databuffer, 0, sizeof(databuffer));

	// this should capture end of file
	if (localbufferidx < (localblocknum + 1) * 254)
	{
		blockdatasize = ((localblocknum * 254) % localbufferidx);
		localblocknum = 0;
		lastblock = true;
	}

	// fill the channel buffer
	for (int t = 0; t < blockdatasize; t++)
	{
		uint8_t b = localbuffer[localblocknum * 254 + t];
		databuffer[t] = b;
	}

	localblocknum++;
	return lastblock;
}

uint8_t C1581_Channel::read(uint8_t *byte)
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

uint8_t C1581_Channel::write(uint8_t byte)
{
	return ERR_OK;
}

uint8_t C1581_Channel::close()
{
	this->channelopen = false;
	delete[] localbuffer;
	return ERR_OK;
}

uint8_t C1581_Channel::loadFile(uint8_t *filename)
{
	return ERR_OK;
}

uint8_t C1581_Channel::saveFile(uint8_t *filename)
{
	return ERR_OK;
}



