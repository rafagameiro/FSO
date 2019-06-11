/*
 ============================================================================
 Name        : fso-L09.c
 Author      : vad
 Version     :
 Copyright   : FSO 17/18 - DI-FCT/UNL
 Description : Lab 09 - sistema de ficheiros
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "fs.h"
#include "disk.h"

static int do_copyin(const char *filename, char *fsname);
static int do_copyout(char *fsname, const char *filename);

int main(int argc, char*argv[]) {

	char line[1024];
	char cmd[1024];
	char arg1[1024];
	char arg2[1024];
	int inumber, result, args, nblocks;

	if (argc != 3 && argc != 2) {
		printf("use: %s <diskfile> [nblocks]\n", argv[0]);
		return 1;
	}
	if (argc == 3)
		nblocks = atoi(argv[2]);
	else
		nblocks = -1;

	if (!disk_init(argv[1], nblocks)) {
		printf("couldn't initialize %s: %s\n", argv[1], strerror(errno));
		return 1;
	}

	printf("opened emulated disk image %s with %d blocks\n", argv[1],
			disk_size());

	while (1) {
		printf(">> ");
		fflush(stdout);

		if (!fgets(line, sizeof(line), stdin))
			break;

		if (line[0] == '\n')
			continue;
		line[strlen(line) - 1] = 0;

		args = sscanf(line, "%s %s %s", cmd, arg1, arg2);
		if (args == 0)
			continue;

		if (!strcmp(cmd, "format")) {
			if (args == 1)
				if (fs_format())
					printf("disk formatted.\n");
				else
					printf("format failed!\n");
			else
				printf("use: format\n");

		} else if (!strcmp(cmd, "mount")) {
			if (args == 1) {
				if (fs_mount()) {
					printf("disk mounted.\n");
				} else {
					printf("mount failed!\n");
				}
			} else {
				printf("use: mount\n");
			}

		} else if (!strcmp(cmd, "debug")) {
			if (args == 1) {
				fs_debug();
			} else {
				printf("use: debug\n");
			}

		} else if (!strcmp(cmd, "create")) {
			if (args == 2) {
				inumber = fs_create(arg1);
				if (inumber >= 0) {
					printf("created inode %d\n", inumber);
				} else {
					printf("create failed!\n");
				}
			} else {
				printf("use: create\n");
			}

		} else if (!strcmp(cmd, "delete")) {
			if (args == 2) {
				if (fs_delete(arg1)) {
					printf("file %s deleted.\n", arg1);
				} else {
					printf("delete failed!\n");
				}
			} else {
				printf("use: delete <inumber>\n");
			}
		} else if (!strcmp(cmd, "cat")) {
			if (args == 2) {
				if (!do_copyout(arg1, "/dev/stdout")) {
					printf("cat failed!\n");
				}
			} else {
				printf("use: cat <fsname>\n");
			}

		} else if (!strcmp(cmd, "copyin")) {
			if (args == 3) {
				if (do_copyin(arg1, arg2)) {
					printf("copied file %s to fs %s\n", arg1, arg2);
				} else {
					printf("copy failed!\n");
				}
			} else {
				printf("use: copyin <filename> <fsname>\n");
			}

		} else if (!strcmp(cmd, "copyout")) {
			if (args == 3) {
				if (do_copyout(arg1, arg2)) {
					printf("copied fs %s to file %s\n", arg1, arg2);
				} else {
					printf("copy failed!\n");
				}
			} else {
				printf("use: copyout <fsname> <filename>\n");
			}

		} else if (!strcmp(cmd, "help")) {
			printf("Commands are:\n");
			printf("    format\n");
			printf("    mount\n");
			printf("    debug\n");
			printf("    open <name>\n");
			printf("    create <name>\n");
			printf("    delete  <name>\n");
			printf("    cat     <name>\n");
			printf("    copyin  <file> <file>\n");
			printf("    copyout <file> <file>\n");
			printf("    help\n");
			printf("    quit\n");
			printf("    exit\n");
		} else if (!strcmp(cmd, "quit")) {
			break;
		} else if (!strcmp(cmd, "exit")) {
			break;
		} else {
			printf("unknown command: %s\n", cmd);
			printf("type 'help' for a list of commands.\n");
			result = 1;
		}
	}

	printf("closing emulated disk.\n");
	disk_close();

	return EXIT_SUCCESS;
}

static int do_copyin(const char *filename, char *fsname) {
	FILE *file;
	int offset = 0, result, actual, inumber;
	char buffer[16384];

	file = fopen(filename, "r");
	if (!file) {
		printf("couldn't open %s: %s\n", filename, strerror(errno));
		return 0;
	}
	inumber = fs_create(fsname);
	while (inumber >= 0) {
		result = fread(buffer, 1, sizeof(buffer), file);
		if (result <= 0)
			break;
		if (result > 0) {
			actual = fs_write(inumber, buffer, result, offset);
			if (actual < 0) {
				printf("ERROR: fs_write return invalid result %d\n", actual);
				break;
			}
			offset += actual;
			if (actual != result) {
				printf("WARNING: fs_write only wrote %d bytes, not %d bytes\n",
						actual, result);
				break;
			}
		}
	}

	printf("%d bytes copied\n", offset);

	fclose(file);
	return 1;
}

static int do_copyout(char *fsname, const char *filename) {
	FILE *file;
	int offset = 0, result, inumber;
	char buffer[16384];

	file = fopen(filename, "w");
	if (!file) {
		printf("couldn't open %s: %s\n", filename, strerror(errno));
		return 0;
	}
	inumber = fs_open(fsname);
	while (1) {
		result = fs_read(inumber, buffer, sizeof(buffer), offset);
		if (result <= 0)
			break;
		fwrite(buffer, 1, result, file);
		offset += result;
	}

	printf("%d bytes copied\n", offset);

	fclose(file);

	return 1;
}
