#include <ctype.h>
#include "C1581_Channel.h"
#include "iec_errors.h"

static C1581 *c1581;

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
	
	c1581->getFileTrackSector((char *)filename, &file_track, &file_sector);
	
	return ERR_OK;
}

uint8_t C1581_Channel::read()
{
	

	return ERR_OK;
}

uint8_t C1581_Channel::write(uint8_t byte)
{
	return ERR_OK;
}

uint8_t C1581_Channel::close()
{
	this->channelopen = false;

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



