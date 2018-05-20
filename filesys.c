#include "filesys.h"
#include "types.h"
#include "lib.h"
#include "pcb.h"

boot_block_t* boot;
inode_t* inodes_ptr;
data_block_t* data_blocks_ptr;

 /* 
  * fs_init()
  *	  DESCRIPTION: Initialize pointers to relevant addresses
  *	  INPUT: Start address of boot block
  *   OUTPUT: none
  *	  RETURN VALUE: none
  *	  SIDE EFFECTS: none
  */
void fs_init(uint32_t start)
{
	boot = (boot_block_t*)start;
	inodes_ptr = (inode_t*)(&boot[1]);
	data_blocks_ptr = (data_block_t*)(start + BLOCK_SIZE*(boot->n_inode + 1));
}

 /* 
  * read_dentry_by_name()
  *	  DESCRIPTION: Copies the directory entry of the given file name
  *	  INPUT: filename and dentry object 
  *   OUTPUT: dentry object
  *	  RETURN VALUE: 0 - success; -1 - failure
  *	  SIDE EFFECTS: none
  */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
	int i = 0;
	while(strncmp((int8_t*)fname, (int8_t*)boot->dentry[i].fname, 32) != 0){
		if(i++ > 63)
			return -1;
    if(boot->dentry[i].fname[0] == '\0')
      return -1;
	}
  int j;
  for(j = 0; j < 32; j++)
    dentry->fname[j] = boot->dentry[i].fname[j];
  //strncpy((int8_t*)dentry->fname, (int8_t*)boot->dentry[i].fname, 32);
  //dentry->fname = &boot->dentry[i].fname;
  dentry->ftype = boot->dentry[i].ftype;
  dentry->n_inode = boot->dentry[i].n_inode;
  //dentry->reserved = &boot->dentry[i].reserved;
	//memcpy(dentry, boot->dentry[i], 64);
	//*dentry = boot->dentry[i];
	return 0;
}

 /* 
  * read_dentry_by_index()
  *	  DESCRIPTION: Copies the directory entry of the given directory index
  *	  INPUT: index and dentry object 
  *   OUTPUT: dentry object
  *	  RETURN VALUE: 0 - success; -1 - failure
  *	  SIDE EFFECTS: none
  */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
	if(index > 63 || boot->dentry[index].fname[0] == '\0')
		return -1;

  strncpy((int8_t*)dentry->fname, (int8_t*)boot->dentry[index].fname, 32);
  //dentry->fname = &boot->dentry[i].fname;
  dentry->ftype = boot->dentry[index].ftype;
  dentry->n_inode = boot->dentry[index].n_inode;
	//memcpy(&dentry, &boot->dentry[index], 64);
	//*dentry = boot->dentry[index];
	return 0;
}

 /* 
  * read_data()
  *	  DESCRIPTION: Reads the data of size length into a buffer from a data block
  *	  INPUT: inode - inode index of the file
  *			 offset - position in data block to start reading from
  *			 buf - buffer to copy data into
  *			 length - number of bytes to copy
  *   OUTPUT: buffer of copie data
  *	  RETURN VALUE: number of bytes read
  *	  SIDE EFFECTS: none
  */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	if(inode >= boot->n_inode)
		return -1;
	inode_t* inode_block;
	inode_block = (inode_t*)(&boot[inode+1]);
	uint32_t filesize;
	filesize = inode_block->length;
/* 	printf("Length: %d\n", inode_block->length);
	printf("Data Block 1: %d\n", inode_block->data_index[0]);
	printf("Data Block 2: %d\n", inode_block->data_index[1]);
	printf("Data Block 3: %d\n", inode_block->data_index[2]); */

	if(length > filesize - offset)
		length = filesize - offset;
	
	int32_t bytes_read = 0;
	int32_t position = offset;
	data_block_t* curr_dblock;

	if(offset >= filesize)
		return 0;
	
	while(bytes_read < length){
		if(inode_block->data_index[position/BLOCK_SIZE] >= boot->n_data_block)
			return bytes_read;
		curr_dblock = (data_block_t*)(&boot[boot->n_inode+1+inode_block->data_index[position/BLOCK_SIZE]]);
		//memcpy(&buf[bytes_read], &curr_dblock->data[position%BLOCK_SIZE], 1);
		buf[bytes_read] = curr_dblock->data[position%BLOCK_SIZE];
		bytes_read++; position++;
	}
	return bytes_read;
}

 /* 
  * file_read()
  *	  DESCRIPTION: Reads the data in a file given a file name; Calls read_data
  *	  INPUT: fname - file to read
  *			 buf - buffer to copy into
  * 		 nbytes - number of bytes to copy
  *   OUTPUT: buffer of copied data
  *	  RETURN VALUE: number of bytes read
  *	  SIDE EFFECTS: none
  */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes)
{
  pcb_t* pcb = get_pcb();
	//if fd past eof, return 0
  if(pcb->files[fd].file_pos >= file_length(pcb->files[fd].inode))
    return 0;

	//read data until eof or eobuffer(whichever occurs first)
  int32_t bytes_read = 0;
	bytes_read = read_data(pcb->files[fd].inode, pcb->files[fd].file_pos, buf, nbytes);
  pcb->files[fd].file_pos += bytes_read;
  return bytes_read;
}

 /* 
  * file_write()
  *	  DESCRIPTION: File system is read-only; Does nothing
  *	  INPUT: none
  *	  RETURN VALUE: -1
  *	  SIDE EFFECTS: none
  */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes)
{
	return -1;
}

 /* 
  * file_open()
  *	  DESCRIPTION: Opens a file
  *	  INPUT: filename to open
  *	  RETURN VALUE: 0
  *	  SIDE EFFECTS: none
  */
int32_t file_open(const uint8_t* filename)
{
	
	return 0;
}

 /* 
  * file_close()
  *	  DESCRIPTION: Closes a file; Undoes file_open()
  *	  INPUT: file descriptor of file to close
  *	  RETURN VALUE: 0
  *	  SIDE EFFECTS: none
  */
int32_t file_close(int32_t fd)
{
	return 0;
}

 /* 
  * dir_read()
  *	  DESCRIPTION: Writes to a buffer the names of all the files in the directory
  *	  INPUT: fd - file descriptor of directory
  *			 buf - buffer to write file names to
  *	  RETURN VALUE: nbytes
  *	  SIDE EFFECTS: none
  */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes)
{
	dentry_t dentry;
	int i;
  nbytes = 32;
	int8_t temp_buf[nbytes];
  pcb_t* pcb = get_pcb();

	if(read_dentry_by_index(pcb->files[fd].file_pos, &dentry) == 0){
		for(i = 0; i < nbytes; i++)
			temp_buf[i] = dentry.fname[i];
		strncpy(buf, temp_buf, nbytes);
		pcb->files[fd].file_pos++;
		return nbytes;
	}
	return 0;
}

 /* 
  * dir_write()
  *	  DESCRIPTION: File system is read-only; Does nothing
  *	  INPUT: none
  *	  RETURN VALUE: -1
  *	  SIDE EFFECTS: none
  */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes)
{
	return -1;
}

 /* 
  * dir_open()
  *	  DESCRIPTION: File system is flat; Does nothing
  *	  INPUT: filename to open
  *	  RETURN VALUE: 0
  *	  SIDE EFFECTS: none
  */
int32_t dir_open(const uint8_t* filename)
{
	return 0;
}

 /* 
  * dir_close()
  *	  DESCRIPTION: File system is flat; Does nothing
  *	  INPUT: filename to open
  *	  RETURN VALUE: 0
  *	  SIDE EFFECTS: none
  */
int32_t dir_close(int32_t fd)
{
	return 0;
}

/* Returns boot block */
boot_block_t* get_boot()
{
	return boot;
}

/* 
  * file_length(uint32_t inode)
  *   DESCRIPTION: returns file length
  *   INPUT: inode
  *   RETURN VALUE: file_length
  *   SIDE EFFECTS: none
  */


int32_t file_length(uint32_t inode){
  inode_t* inode_block;
  inode_block = (inode_t*)(&boot[inode+1]);
  return (int32_t)inode_block->length;
}
