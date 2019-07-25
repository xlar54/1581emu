#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "iec_errors.h"
#include "C1581.h"

C1581 c1581; 

void test_outputdirectory(C1581 *c1581)
{
	uint8_t buffer[3000];
	memset(buffer, 0, sizeof(buffer));
	c1581->open(15, (uint8_t *)"$", 15);

	uint8_t byte;
	int ctr = 0;

	while (c1581->read(15, &byte) == ERR_OK)
	{
		buffer[ctr++] = byte;
	}

	c1581->close(15);

	FILE *write_ptr;
	write_ptr = fopen("dir.prg", "wb");  // w for write, b for binary
	fwrite(buffer, ctr, 1, write_ptr); // write 10 bytes from our buffer
	fclose(write_ptr);
}

void test_initialize(C1581 *c1581)
{
	c1581->open(15, (uint8_t *)"I0", 15);

	uint8_t byte;
	int ctr = 0;

	while (c1581->read(15, &byte) == ERR_OK)
	{
		ctr++;
		printf("%c", byte);
	}

	c1581->close(15);
}

void test_format(C1581 *c1581)
{
	c1581->open(15, (uint8_t *)"N0:testdisk,uj", 15);

	uint8_t byte;
	int ctr = 0;

	while (c1581->read(15, &byte) == ERR_OK)
	{
		ctr++;
		printf("%c", byte);
	}

	c1581->close(15);
}

void test_rename(C1581 *c1581)
{
	c1581->open(15, (uint8_t *)"r0:newname=oldname", 15);

	uint8_t byte;
	int ctr = 0;

	while (c1581->read(15, &byte) == ERR_OK)
	{
		ctr++;
		printf("%c", byte);
	}

	c1581->close(15);
}

void test_findfreesector(void)
{
	uint8_t track;
	uint8_t sector;

	int result = c1581.findFreeSector(&track, &sector);
}

void test_getfiletracksector(void)
{
	char *fname = "GEOSPELL";
	uint8_t track;
	uint8_t sector;

	uint8_t err = c1581.getFileTrackSector(fname, &track, &sector);

	if (err == ERR_OK)
	{
		printf("%s - Track %d, Sector %d", fname, track, sector);
	}
	else
	{
		printf("%s - File not found", fname);
	}
	

}

void test_loadfile()
{
	char *fname = "GEOS";
	uint8_t track;
	uint8_t sector;

	uint8_t err = c1581.getFileTrackSector(fname, &track, &sector);

	if (err == ERR_OK)
	{
		uint8_t buffer[2000];
		memset(buffer, 0, sizeof(buffer));

		//err = c1581.openFile(fname);


	}
	else
	{
		printf("%s - File not found", fname);
	}
}

void test_readfile(C1581 *c1581)
{
	c1581->open(1, (uint8_t *)"GEOS", 0);

	uint8_t byte;
	int ctr = 0;

	while (c1581->read(15, &byte) == ERR_OK)
	{
		ctr++;
		printf("%c", byte);
	}

	c1581->close(1);
}

int main( int argc, const char* argv[] )
{
	uint8_t image[819200];

	FILE *f = fopen("GEOS64.d81", "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
	fread(image, 1, fsize, f);
	fclose(f);

	printf( "\n1581 Emulation\n\n" );

    c1581.init(8);
    c1581.powerOn();
    c1581.insertDisk(image);

	//test_readfile(&c1581);
	//test_format(&c1581);
	//test_rename(&c1581);
	test_outputdirectory(&c1581);
	//test_initialize(&c1581);
	//test_findfreesector();
	//test_getfiletracksector();
	
}

