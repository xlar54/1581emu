#ifndef C1581_H_
#define C1581_H_

#include "types.h"

#define DISK_SIZE	819200

struct BamAllocationType {
	uint8_t track;
	uint8_t sector;
	bool allocated;
};

typedef BamAllocationType BamAllocation;

class C1581
{
    uint8_t deviceum = 9;
    bool power;
    bool hasDisk;
    uint8_t curdisk[DISK_SIZE];
	uint8_t filesOpen[10] = { 0,0,0,0,0,0,0,0,0,0 };
	bool bamisdirty = false;

    public:
		uint8_t curtrack = 0;
		uint8_t cursector = 0;
		uint8_t nxttrack = 0;
		uint8_t nxtsector = 0;
		uint8_t sectorBuffer[256];
		
		C1581();
        ~C1581();

        void init(uint8_t driveNumber);
        void powerOn(void);
        void powerOff(void);
        void insertDisk(uint8_t *image);
        void ejectDisk(void);
		uint8_t goTrackSector(uint8_t track, uint8_t sector);
		void readSector(void);
		uint8_t nextSector(void);
		uint8_t prevSector(void);
		uint8_t nextTrack(void);
		uint8_t prevTrack(void);
		
		int findFreeSector(uint8_t *track, uint8_t *sector);
		uint8_t getFileTrackSector(char *filename, uint8_t *track, uint8_t *sector);

		uint8_t C1581::open(uint8_t channel, uint8_t *command, uint8_t secondary);
		uint8_t C1581::read(uint8_t channel, uint8_t *byte);
		uint8_t C1581::write(uint8_t channel, uint8_t byte);
		uint8_t C1581::close(uint8_t channel);

};


#endif