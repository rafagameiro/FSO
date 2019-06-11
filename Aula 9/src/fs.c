#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <stdint.h>

#include "fs.h"
#include "disk.h"

#define FS_MAGIC           0x00f00baa
#define POINTERS_PER_INODE 15	// 64/4 - 1 = 15 --> max file size = 15*4K
#define INODES_PER_BLOCK 32	// 4K/128=32
#define MAXFILENAME   63		// 128 bytes per dirent-> 1+63+64

#define FALSE 0
#define TRUE 1

#define VALID 1
#define NON_VALID 0

#define FREE 0
#define NOT_FREE 1

struct fs_superblock {
	uint32_t magic;		// magic number is there when formated
	uint32_t nblocks;	// fs size
	uint32_t ninodeblocks;	// number of blocks for inodes+directory
	uint32_t ninodes;	// max number of inodes/dir entries
};

struct fs_inode {	// inode is also dir entry
	uint8_t isvalid;
	char name[MAXFILENAME];
	uint32_t size;
	uint32_t blk[POINTERS_PER_INODE];
};

union fs_block {
	struct fs_superblock super;
	struct fs_inode inode[INODES_PER_BLOCK];
	char data[DISK_BLOCK_SIZE];
};

struct fs_superblock rootSB; // superblock of the mounted disk

unsigned char * blockBitMap; // Map of used blocks (1char=1block, not a real bitMap)


int fs_format() {
	union fs_block block;
	int i, nblocks, ninodeblocks;

	if (rootSB.magic == FS_MAGIC) {
		printf("Cannot format a mounted disk!\n");
		return 0;
	}
	if (sizeof(block) != DISK_BLOCK_SIZE) {
		printf("Disk block and FS block mismatch\n");
		return 0;
	}
	memset(&block, 0, sizeof(block));

	nblocks = disk_size();
	block.super.magic = FS_MAGIC;
	block.super.nblocks = nblocks;
	block.super.ninodeblocks = (int) ceil((float) nblocks * 0.1);
	block.super.ninodes = block.super.ninodeblocks * INODES_PER_BLOCK;

	printf("superblock:\n");
	printf("    %d blocks\n", block.super.nblocks);
	printf("    %d dir/inode blocks\n", block.super.ninodeblocks);
	printf("    %d inodes/dirents\n", block.super.ninodes);

	/* escrita do superbloco */
	disk_write(0, block.data);
	ninodeblocks = block.super.ninodeblocks;

	/* prepara tabela de inodes/directoria */
	for (i = 0; i < INODES_PER_BLOCK; i++)
		block.inode[i].isvalid = NON_VALID; // just isvalid needs to be inicialized

	/* escrita da tabela de inodes/directoria */
	for (i = 1; i <= ninodeblocks; i++)
		disk_write(i, block.data);

	return 1;
}

void fs_debug() {
	union fs_block block;
	int i, j, k;

	disk_read(0, block.data);

	if (block.super.magic != FS_MAGIC) {
		printf("disk unformatted !\n");
		return;
	}
	printf("superblock:\n");
	printf("    %d blocks\n", block.super.nblocks);
	printf("    %d dir/inode blocks\n", block.super.ninodeblocks);
	printf("    %d inodes/dirents\n", block.super.ninodes);
	printf("**************************************\n");

	printf("inode\tsize\tname\tblocks\n");
	// TODO: /* list all the files */
	for(i = 1; i <= block.super.ninodeblocks; i++) {
		disk_read(i, block.data);
		for(j = 0; j < INODES_PER_BLOCK]; j++)
			if(block.inode[j].isvalid == VALID) {
				printf("%d\t%u\t%s\t", (i-1)*INODES_PER_BLOCK+j, block.inode[j].size, block.inode[j].name);
				for(k = 0; k < POINTERS_PER_INODE; k++) {
					if(k == POINTERS_PER_INODE -1)	
						printf("%u\n", block.inode[j].blk[k])
					printf("%u", block.inode[j].blk[k]);
				}
		}
	}

	printf("**************************************\n");

}

/******************************************************************/
int fs_mount() {
	union fs_block block;
	int i, j, k;

	if (rootSB.magic == FS_MAGIC) {
		printf("disc already mounted!\n");
		return 0;
	}

	disk_read(0, block.data);
	if (block.super.magic != FS_MAGIC) {
		printf("cannot mount an unformatted disc!\n");
		return 0;
	}
	if (block.super.nblocks != disk_size()) {
		printf("file system size and disk size differ!\n");
		return 0;
	}
	/* pode-se tambem testar o numero de inodes e o numero de
	 blocos com inodes ... */

	rootSB = block.super; /* variavel global - superbloco do FS corrente */

	// Construcao do bitmap de blocos em memoria
	if ((blockBitMap = malloc(rootSB.nblocks)) == NULL) {
		printf("malloc failed!\n");
		return 0;
	}

	// Marcar como ocupados o block 0 (superbloco) e os blocos 1 a ninodeblocks
	// (tabela de inodes/diretoria)
	blockBitMap[0] = NOT_FREE;
	for (i = 1; i <= rootSB.ninodeblocks; i++)
		blockBitMap[i] = NOT_FREE;

	// percorrer os inodes e marcar no "bitmap" os blocos neles referidos como ocupados
	for (i = 1; i <= rootSB.ninodeblocks; i++) {
		disk_read(i, block.data);
		for (j = 0; j < INODES_PER_BLOCK; j++) {
			if (block.inode[j].isvalid == VALID)
				for (k = 0; (k < POINTERS_PER_INODE)
							&& (block.inode[j].blk[k] != FREE); k++) //unused should already be set to zero (FREE)
					blockBitMap[block.inode[j].blk[k]] = NOT_FREE;
		}
	}

	return 1;
}

/******************************************************************/
int fs_open(char *name) {
	int inodeBlock = -1;
	union fs_block block;
	int i, j;

	if (rootSB.magic != FS_MAGIC) {
		printf("disc not mounted\n");
		return -1;
	}
	// TODO: obter inode de name
	/* Procura sequencialmente o inode/dirent com name */
	for(i = 1; i <= block.super.ninodeblocks; i++) {
		disk_read(i, bock.data);
		for(j = 0; j < INODES_PER_BLOCK;j++)
			if (block.inode[i].isvalid == VALID)
				if(strcmp(block.inode[i].name, name)) {
					inodeBlock = i;
					fopen(name, O_RDWR);
				}			
	}		
	
	// return -1 if not found
	// (me) inodeBlock is initialized as -1, so I just need to return the inodeBlock, either way
	return inodeBlock;
}

/******************************************************************/
int fs_create(char *name) {
	int freeInode, inodeBlock;
	union fs_block block;
	int i, j;

	if (rootSB.magic != FS_MAGIC) {
		printf("disc not mounted\n");
		return -1;
	}

	if (fs_open(name) != -1) {
		printf("%s already exists\n", name);
		return -1;
	}
	/* Procura sequencialmente um inode/dirent livre */
	freeInode = -1;
	inodeBlock = 0; /* no. do bloco onde se procura: de 1 a ninodeblocks */
	do {
		inodeBlock++;
		disk_read(inodeBlock, block.data);
		/* percurso pelos inode de um bloco */
		i = 0;
		do {
			if (block.inode[i].isvalid == VALID)
				i++;
			else
				freeInode = (inodeBlock - 1) * INODES_PER_BLOCK + i;
		} while ((i < INODES_PER_BLOCK) && (freeInode == -1));
	} while ((inodeBlock <= rootSB.ninodeblocks) && (freeInode == -1));

	if (freeInode == -1) {
		printf("No free inode/dirent\n");
		return -1;
	}
	/* Inicializa o inode */
	block.inode[i].isvalid = VALID;
	block.inode[i].size = 0;
	strncpy(block.inode[i].name, name, MAXFILENAME);
	/* apontadores para blocos sao colocados a 0; */
	for (j = 0; j < POINTERS_PER_INODE; j++)
		block.inode[i].blk[j] = FREE;  //unused should be set to zero (FREE)

	/* Actualiza a tabela de inode no disco */
	disk_write(inodeBlock, block.data);

	return freeInode;
}

void inode_load(int numb, struct fs_inode *ino) {
	int inodeBlock;
	union fs_block block;

	if (numb > rootSB.ninodes) {
		printf("inode number too big \n");
		ino->isvalid = 0;
		return;
	}
	inodeBlock = 1 + (numb / INODES_PER_BLOCK);
	disk_read(inodeBlock, block.data);
	*ino = block.inode[numb % INODES_PER_BLOCK];
}

void inode_save(int numb, struct fs_inode *ino) {
	int inodeBlock;
	union fs_block block;

	if (numb > rootSB.ninodes) {
		printf("inode number too big \n");
		return;
	}
	inodeBlock = 1 + (numb / INODES_PER_BLOCK);
	disk_read(inodeBlock, block.data);  // read all block
	block.inode[numb % INODES_PER_BLOCK] = *ino; // update inode
	disk_write(inodeBlock, block.data); // write all block
}

/******************************************************************/
int fs_delete(char *fsname) {
	int i, inumber;
	struct fs_inode inode;

	if (rootSB.magic != FS_MAGIC) {
		printf("disc not mounted\n");
		return -1;
	}
// TODO:
	//use bitmap later
	inode.isvalid = NON_VALID;
	for(i = 0; i < POINTERS_PER_INODE; i++)
		inode.blk[i] = FREE;
	return 1;
}


int convertOffsetBlock(struct fs_inode inode, int offsetCurrent) {
	int block;

	block = offsetCurrent / DISK_BLOCK_SIZE;

	if (block < POINTERS_PER_INODE) {
		//printf("-> %d\n", inode.direct[block]);
		return inode.blk[block];
	} else {
		printf("offset to big!\n");
		return -2;
	}
}

/**************************************************************/
int fs_read(int inumber, char *data, int length, int offset) {
	int currentBlock, offsetCurrent, offsetInBlock;
	int bytesLeft, nCopy, bytesToRead;
	char *dst;
	union fs_block buff;
	struct fs_inode inode;

	if (rootSB.magic != FS_MAGIC) {
		printf("disc not mounted\n");
		return -1;
	}
	inode_load(inumber, &inode);
	if (inode.isvalid == NON_VALID) {
		printf("inode is not valid\n");
		return -1;
	}
	if (offset > inode.size) {
		printf("offset bigger that file size !\n");
		return -1;
	}

	if (inode.size > (offset + length))
		bytesToRead = length;
	else
		bytesToRead = inode.size - offset; // can't read more than file size
	dst = data;
	offsetCurrent = offset;
	bytesLeft = bytesToRead;
	while (bytesLeft > 0) {	// read all blocks until fulfill bytesToRead bytes
		currentBlock = convertOffsetBlock(inode, offsetCurrent);
		disk_read(currentBlock, buff.data);
		offsetInBlock = offsetCurrent % DISK_BLOCK_SIZE;

		if (bytesLeft < DISK_BLOCK_SIZE - offsetInBlock)
			nCopy = bytesLeft;
		else
			nCopy = DISK_BLOCK_SIZE - offsetInBlock;

		bcopy(buff.data + offsetInBlock, dst, nCopy);
		dst = dst + nCopy;
		bytesLeft = bytesLeft - nCopy;
		offsetCurrent = offsetCurrent + nCopy;
	}

	return bytesToRead;
}

/******************************************************************/
int getFreeBlock() {
	int i, found;

	i = 0;
	found = FALSE;
	do {
		if (blockBitMap[i] == FREE) {
			found = TRUE;
			blockBitMap[i] = NOT_FREE;
		} else
			i++;
	} while ((!found) && (i < rootSB.nblocks));

	//printf("getFreeBlock returns %d\n", i);
	if (i == rootSB.nblocks)
		return -1; /* nao ha blocos livres */
	else
		return i;
}

int fs_write(int inumber, char *data, int length, int offset) {
	int currentBlock, offsetCurrent, offsetInBlock;
	int bytesLeft, nCopy, bytesToWrite, newEntry;
	char *src;
	union fs_block buff;
	struct fs_inode inode;

	if (rootSB.magic != FS_MAGIC) {
		printf("disc not mounted\n");
		return -1;
	}

	inode_load(inumber, &inode);
	if (inode.isvalid == NON_VALID) {
		printf("inode is not valid\n");
		return -1;
	}
	if (offset > inode.size) {
		printf("starting to write after end of file\n"); //can be valid in some FS
		return -1;
	}

	bytesToWrite = length;
	src = data;
	offsetCurrent = offset;
	bytesLeft = bytesToWrite;

	while (bytesLeft > 0) {
		if (offsetCurrent == inode.size) {
			/* necessario mais um bloco */
			currentBlock = getFreeBlock();
			if (currentBlock < 0) {
				/* nao ha blocos livres */
				printf("no more blocks\n");
				inode_save(inumber, &inode);
				return (length - bytesLeft);
			} else {
				/* actualizar blocos no inode */
				newEntry = (inode.size / DISK_BLOCK_SIZE);
				if (newEntry < POINTERS_PER_INODE)
					inode.blk[newEntry] = currentBlock;
				else {
					if (newEntry == POINTERS_PER_INODE) {
						/* nao ha blocos livres */
						printf("file to big for inode\n");
						blockBitMap[currentBlock] = FREE;
						inode_save(inumber, &inode);
						return (length - bytesLeft);
					}
				}
			}
		} else {
			currentBlock = convertOffsetBlock(inode, offsetCurrent);
			disk_read(currentBlock, buff.data);
		}

		offsetInBlock = offsetCurrent % DISK_BLOCK_SIZE;

		if (bytesLeft < (DISK_BLOCK_SIZE - offsetInBlock))
			nCopy = bytesLeft;
		else
			nCopy = DISK_BLOCK_SIZE - offsetInBlock;

		bcopy(src, buff.data + offsetInBlock, nCopy);
		disk_write(currentBlock, buff.data);
		if (offsetCurrent + nCopy > inode.size)
			inode.size = offsetCurrent + nCopy;
		src = src + nCopy;
		bytesLeft = bytesLeft - nCopy;
		offsetCurrent = offsetCurrent + nCopy;
	}

	inode_save(inumber, &inode);

	return length;
}
