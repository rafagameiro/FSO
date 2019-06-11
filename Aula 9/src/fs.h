#ifndef FS_H
#define FS_H

void fs_debug();
int  fs_format();
int  fs_mount();

int  fs_create( char *fsname );
int  fs_open( char *fsname );
int  fs_delete( char *fsname );
//int  fs_getsize();

int  fs_read( int inumber, char *data, int length, int offset );
int  fs_write( int inumber, char *data, int length, int offset );

#endif
