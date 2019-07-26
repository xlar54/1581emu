
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "C1581.h"
#include "C1581_CmdChannel.h"
#include "iec_errors.h"

C1581* C1581_CmdChannel::c1581;


C1581_CmdChannel::C1581_CmdChannel()
{
	
}

void C1581_CmdChannel::init(C1581 *parent)
{
	c1581 = parent;
}
C1581_CmdChannel::~C1581_CmdChannel()
{
	c1581 = NULL;
}

uint8_t C1581_CmdChannel::open(uint8_t *command, uint8_t secondary)
{
	uint8_t track;
	uint8_t sector;
	localbuffer = new uint8_t[65536];
	localbufferidx = 0;
	
	channelopen = true;

	command_t parsed_command;
	char commandtxt[52];
	strcpy(commandtxt, (char *)command);

	parse_command(commandtxt, &parsed_command);

	if (strcmp(parsed_command.cmd, "$") == 0)
	{
		strcpy((char *)lastcommand, (char *)command);
		localbufferidx = c1581->get_directory(localbuffer);
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



