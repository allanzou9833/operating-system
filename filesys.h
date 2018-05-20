 	#ifndef FILESYS_H
#define FILESYS_H

#include "types.h"

#define BLOCK_SIZE	4096

#define FTYPE_RTC 0
#define FTYPE_DIR 1
#define FTYPE_FILE 2
typedef struct dentry{
	uint8_t fname[32];	//32B file name
	uint32_t ftype;
	uint32_t n_inode;
	uint8_t reserved[24];
} dentry_t;

typedef struct inode{
	uint32_t length;	//length in bytes
	uint32_t data_index[1023]; //Each index is 4B so 1023 entries
} inode_t;

typedef struct data_block{
	uint8_t data[4096];	//4 KiB
} data_block_t;

typedef struct boot_block{
	uint32_t n_dentry;
	uint32_t n_inode;
	uint32_t n_data_block;
	uint8_t reserved[52];
	dentry_t dentry[63];	//Each dentry_t is 64B so there is 63 directory entries
} boot_block_t;

void fs_init(uint32_t start);

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t file_read(int32_t fd , void* buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t file_open(const uint8_t* filename);
int32_t file_close(int32_t fd);

int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t dir_open(const uint8_t* filename);
int32_t dir_close(int32_t fd);

boot_block_t* get_boot();
int32_t file_length(uint32_t inode);
#endif
