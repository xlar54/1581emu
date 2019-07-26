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
	strcpy((char *)this->file_name, (char *)filename);
	
	this->channelopen = true;
	
	return c1581->getFileTrackSector((char *)this->file_name, &this->file_track, &this->file_sector);
		
	/*
	// load entire file into localbuffer
	do
	{
		uint8_t rawsector[256];
		moredata = c1581->getNextFileSector(filename);

		//skip the next track/sector bytes
		for (int t = 2; t < 256; t++)
			localbuffer[localbufferidx++] = c1581->sectorBuffer[t];

	} while (moredata == true);

	return ERR_OK;*/
}

bool C1581_Channel::readblock()
{
	bool moredata = false;

	static bool firstcall = true;
	int ptr = 0;

	if (firstcall)
	{
		c1581->goTrackSector(this->file_track, this->file_sector);
		firstcall = false;
	}
	else
	{
		c1581->goTrackSector(c1581->nxttrack, c1581->nxtsector);
	}

	c1581->readSector();
	memcpy(this->databuffer, c1581->sectorBuffer, 256);
	
	c1581->nxttrack = c1581->sectorBuffer[0x00];
	c1581->nxtsector = c1581->sectorBuffer[0x01];

	if (c1581->nxttrack == 0x00)
		return false;

	return true;
}

uint8_t C1581_Channel::read(uint8_t *byte)
{
	static bool moredata = false;

	// is it time to read a new block of data?
	if (databufferidx == 0)
	{
		moredata = readblock();
		databufferidx += 2;
	}
		
	// return a byte
	*byte = databuffer[databufferidx];

	if (databufferidx == 255 && moredata == false)
	{
		databufferidx=0;
		return ERR_READ_ERROR;
	}
	else
	{
		databufferidx++;
		return ERR_OK;
	}
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



