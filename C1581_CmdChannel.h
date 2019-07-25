#ifndef C1581_CmdChannel_H_
#define C1581_CmdChannel_H_

#include "C1581.h"
#include "types.h"

typedef struct {
	int drive;
	char *name;
	char separator;
} name_t;

typedef struct {
	char *cmd;
	int digits;
	name_t names[5];
} command_t;

struct DirectoryEntryType {
	uint8_t file_type;
	uint8_t first_data_track;
	uint8_t first_data_sector;
	uint8_t filename[16];
	uint8_t first_track_ssb;	// side sector block (REL file only)
	uint8_t first_sector_ssb;
	uint8_t rel_file_length;
	uint8_t unused[6];
	uint8_t size_lo;
	uint8_t size_hi;
};

typedef DirectoryEntryType DirectoryEntry;

class C1581_CmdChannel
{
	uint8_t databuffer[256];
	uint8_t databufferidx = 0;
	uint8_t *localbuffer;
	int localbufferidx = 0;
	int localblocknum = 0;
	int blockidx = 0;
	static C1581 *c1581;
	
	
public:
	C1581_CmdChannel();
	~C1581_CmdChannel();
	
	bool channelopen;
	uint8_t lastcommand[80];

	void init(C1581 *parent);

	uint8_t open(uint8_t *command, uint8_t secondary);
	bool readblock();
	uint8_t read(uint8_t* byte);
	uint8_t write(uint8_t byte);
	uint8_t close(void);

	void parse_command(char *buffer, command_t *command);
	int initialize(void);
	uint8_t format(uint8_t* diskname, uint8_t* id);
	uint8_t scratchFile(uint8_t *filename);
	uint8_t renameFile(uint8_t *filename, uint8_t *newfilename);
	uint8_t copyFile(uint8_t *srcfilename, uint8_t *destfilename);
	uint8_t validate(void);
	
	int get_directory(uint8_t *localbuffer);

	int getNextDirectoryEntry(DirectoryEntry **direntry);
	
};

#endif