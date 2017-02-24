/**
 * freethewads.c
 * Wii FREE THE WADS
 * by Superken7
 *
 * I take no responsability for bad use of this tool,
 * even if it bricks your wii. you have been warned
 * Dont use it for piracy, this is for educational purposes only.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <limits.h>

#define	OFFSET_REGION		(0x19d)
#define	OFFSET_TMD		(0xd00)
#define	OFFSET_BF		(0x1c1)

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

char *region_str[] = { "JAP/NTSC-J", "NTSC-U", "PAL", "*FREE*", "UNKNOWN", NULL};

void hexdump(u8 *data, u32 len)
{
        int i;

        for(i=0; i<len; i++) {
                printf("%02x ",(u8)data[i]);
                if (((i+1)%8) == 0) printf(" ");
                if (((i+1)%16) == 0) printf("\n");
        }
        printf("\n");

        return;
}

int trucha_sign_ticket(u8 *ticket, u32 ticket_len)
{
	u8 sha1[20];
	u16 i;

	for(i=0; i<65535; i++) {
		memcpy(ticket+OFFSET_BF, &i, sizeof(u16));
		SHA1(ticket+0x140, ticket_len-0x140, sha1);
		if (sha1[0] == 0x00) break;
	}

	if (i == 65535)
		return 0;
	else	return 1;
}


int patch_timelimit(FILE *f, u32 tmd_len, u8 p)
{
	u8 *tmd;
	u8 region;
	char input[10];

	fseek(f, OFFSET_TMD, SEEK_SET);
	tmd = (u8*)malloc(tmd_len);
	if (tmd == NULL) {
		fprintf(stderr, "mem error\n");
		return 0;
	}

	fread((u8*)tmd, tmd_len, 1, f);

	region = tmd[OFFSET_REGION];
	if (p) {
		int i;
		do {
			printf("Region is set to %s\n",region_str[region]);
			printf("New region: \n");
			for(i=0; i<4; i++)
				printf("%d- %s\n",i, region_str[i]);
			printf("Enter your new choice: (oh man you gotta save him!): ");fflush(stdout);
			fgets(input, 9, stdin);
			region = atoi(input);
			if (region > 4) 
				region = 0x04;
				
			printf("New region is set to %s, do you agree? (y/n) ",region_str[region]); fflush(stdout);
			fgets(input, 9, stdin);
		} while(input[0] != 'y');
		
	} else {
		region = 0x03;
	}

	tmd[OFFSET_REGION] = region;
	
	printf("\tSigning...\n");
	if (trucha_sign_ticket(tmd, tmd_len) == 0) {
		fprintf(stderr, "Error signing TMD\n");
		return 0;
	}
	printf("\tdone.\n\n");

        fseek(f, OFFSET_TMD, SEEK_SET);
	fwrite((u8*)tmd, tmd_len, 1, f);

	free(tmd);
	return 1;
}

int main(int argc, char **argv)
{
	FILE *f;
	u8 p;
	u32 tmd_len;

	printf("Free the WADs *testing version* by Superken7\n\n");

	if (argc < 2) {
		printf("usage:\t%s <WAD file>\n",argv[0]);
		return 0;
	}
	if (argc >= 3)
		p = 1;
	else	p = 0;

	f = fopen(argv[1], "r+");
	if (f == NULL) {
		printf("cannot open file %s\n",argv[0]);
		return 0;
	}

	fseek(f, 0x14, SEEK_SET);
	fread((u8*)&tmd_len, 4, 1, f);
	tmd_len = ntohl(tmd_len);
	printf("tmd_len %04x\n",tmd_len);

	printf("Patching... \n");

	patch_timelimit(f, tmd_len, p);

	printf("done.\n\nFOR FREEEDOOOOM!!\n");

	fclose(f);

	return 0;
}
