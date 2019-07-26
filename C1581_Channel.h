#ifndef C1581_Channel_H_
#define C1581_Channel_H_

#include "C1581.h"
#include "types.h"

class C1581_Channel
{
	// the first two bytes are track/sector, 
	// so the block size is actually only 254 bytes
	uint8_t databuffer[254];
	uint8_t databufferidx = 0;
	uint8_t *localbuffer = 0;
	int localbufferidx = 0;
	int localblocknum = 0;
	int blockidx = 0;
	uint8_t file_track = 0;
	uint8_t file_sector = 0;
	static C1581 *c1581;

public:
	C1581_Channel();
	~C1581_Channel();
	
	bool channelopen;
	bool nomoredata = true;
	
	uint8_t number;
	void init(C1581 *parent);
	uint8_t open(uint8_t *filename, uint8_t secondary);
	bool readblock();
	uint8_t read(uint8_t *byte);
	uint8_t write(uint8_t byte);
	uint8_t close();

	uint8_t loadFile(uint8_t *filename);
	uint8_t saveFile(uint8_t *filename);
};

#endif