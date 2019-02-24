// NAME: Bradley Mont
// EMAIL: bradleymont@gmail.com
// ID: 804993030

#include <stdio.h> //for fprintf(3)
#include <unistd.h> //for open(2)
#include <fcntl.h> //for open(2)
#include <stdlib.h> //for exit(2)
#include <errno.h> //to get errno for error messages
#include <math.h>  //for ceil
#include <time.h>
#include "ext2_fs.h"


const int superBlockOffset = 1024; //the superblock is located at an offset of 1024 bytes from the start of the device
const int groupDescTableOffset = 2048; //the offset is 2048 bytes since the superblock starts at an offset of 1024 bytes, and the size of the superblock is 1024 bytes
const int FREE = 0; //if a bit in the block bit map or inode bit map is zero, that means the corresponding block/inode is free

//for finding free entries
const int BLOCK = 1;
const int INODE = 2;

struct ext2_super_block superBlock; //global super block struct variable
struct ext2_group_desc groupDescTable; //global group descriptor table variable
int imageFD; //global variable for file system image file descriptor

//pread wrapper function (inspired by Arpaci-Dusseau textbook)
void Pread(int fd, void* buf, size_t count, off_t offset)
{
    ssize_t numBytesRead = pread(fd, buf, count, offset);
    
    if (numBytesRead < (ssize_t) count)
    {
        fprintf(stderr, "Error: Call to pread(2) failed.\n");
        exit(2);
    }
}

void superBlockSummary()
{
    //the superblock is located at an offset of 1024 bytes from the start of the device
    Pread(imageFD, &superBlock, sizeof(struct ext2_super_block), superBlockOffset);
    
    __u32 bsize = EXT2_MIN_BLOCK_SIZE << superBlock.s_log_block_size;
    
    printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n",
           superBlock.s_blocks_count, //total number of blocks (decimal)
           superBlock.s_inodes_count, //total number of i-nodes (decimal)
           bsize, //block size (in bytes, decimal)
           superBlock.s_inode_size, //i-node size (in bytes, decimal)
           superBlock.s_blocks_per_group, //blocks per group (decimal)
           superBlock.s_inodes_per_group, //i-nodes per group (decimal)
           superBlock.s_first_ino  //first non-reserved i-node (decimal)
           );
}

void groupSummary()
{
    Pread(imageFD, &groupDescTable, sizeof(struct ext2_group_desc), groupDescTableOffset);
    
    int groupNumber = 0;    //assuming 1 group
    
    printf("GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n",
           groupNumber, //group number (decimal, starting from zero)
           superBlock.s_blocks_count, //total number of blocks in this group (decimal)
           superBlock.s_inodes_count, //total number of i-nodes in this group (decimal)
           groupDescTable.bg_free_blocks_count,  //number of free blocks (decimal)
           groupDescTable.bg_free_inodes_count,  //number of free i-nodes (decimal)
           groupDescTable.bg_block_bitmap,   //block number of free block bitmap for this group (decimal)
           groupDescTable.bg_inode_bitmap,   //block number of free i-node bitmap for this group (decimal)
           groupDescTable.bg_inode_table     //block number of first block of i-nodes in this group (decimal)
           );
}

//one common function for the block bitmap and inode bitmap since they are structured very similarl
void freeEntries(int bitmap)
{
    __u32 bitmapOffset;
    char bitMapType;
    __u32 maxBytes;
    
    if (bitmap == BLOCK)
    {
        bitMapType = 'B';
        bitmapOffset = groupDescTable.bg_block_bitmap * EXT2_MIN_BLOCK_SIZE;
        maxBytes = ceil(superBlock.s_blocks_count / 8);
    }
    else //INODE
    {
        bitMapType = 'I';
        bitmapOffset = groupDescTable.bg_inode_bitmap * EXT2_MIN_BLOCK_SIZE;
        maxBytes = ceil(superBlock.s_inodes_count / 8);
    }
    
    for (__u32 byteNum = 0; byteNum < maxBytes; byteNum++) //read one byte at a time
    {
        //Note: the first block/inode of the group is represented by bit 0 of byte 0, the second by bit 1 of byte 0,
        //the 8th block/inode is represented by bit 7 of byte 0, the 9th block/inode is represented by bit 0 of byte 1, etc.
        __u8 currentByte;
        Pread(imageFD, &currentByte, sizeof(__u8), bitmapOffset + byteNum);
        
        //the purpose of this bit mask is to bitwise AND it with each bit, and if it equals 0, then the corresponding block/inode is free
        int bitMask = 1;
        
        //check each bit of the current byte to see if it's free (FREE = 0, ALLOCATED = 1)
        for (int bitNum = 0; bitNum < 8; bitNum++)
        {
            if ((bitMask & currentByte) == FREE)
            {
                int blockNum = (byteNum * 8) + (bitNum + 1);
                printf("%cFREE,%u\n", bitMapType, blockNum);
            }
            bitMask <<= 1;  //shift the mask to the left to check the next bit
        }
    }
}

void formatTime(time_t rawTime, char* result)
{
    struct tm timeStruct = *(gmtime(&rawTime));
    strftime(result, 20, "%m/%d/%y %H:%M:%S", &timeStruct);
}

void inodeSummary()
{
    //__u32 inodeTableSize = superBlock.s_inodes_count * superBlock.s_inode_size;
    __u32 inodeTableOffset = groupDescTable.bg_inode_table * EXT2_MIN_BLOCK_SIZE;
    
    for (__u32 inodeNum = 1; inodeNum <= superBlock.s_inodes_count; inodeNum++)
    {
        struct ext2_inode currInode;
        Pread(imageFD, &currInode, superBlock.s_inode_size, inodeTableOffset + (inodeNum - 1) * superBlock.s_inode_size);
        
        //don't scan unallocated inodes
        if (currInode.i_mode == 0 || currInode.i_links_count == 0) continue;
        
        char fileType = '?';
        if (currInode.i_mode & 0x8000)
        {
            fileType = 'f';
        }
        else if (currInode.i_mode & 0x4000)
        {
            fileType = 'd';
        }
        else if (currInode.i_mode & 0xA000)
        {
            fileType = 's';
        }
        
        char creationTime[20];
        char modificationTime[20];
        char accessTime[20];
        formatTime(currInode.i_ctime, creationTime);
        formatTime(currInode.i_mtime, modificationTime);
        formatTime(currInode.i_atime, accessTime);
        
        printf("INODE,%u,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u",
               inodeNum, //inode number (decimal)
               fileType, //file type ('f' for file, 'd' for directory, 's' for symbolic link, '?" for anything else)
               currInode.i_mode & 0x0FFF,    //mode (low order 12-bits, octal ... suggested format "%o")
               currInode.i_uid, //owner (decimal)
               currInode.i_gid, //group (decimal)
               currInode.i_links_count, //link count (decimal)
               creationTime, //time of last I-node change (mm/dd/yy hh:mm:ss, GMT)
               modificationTime, //modification time (mm/dd/yy hh:mm:ss, GMT)
               accessTime, //time of last access (mm/dd/yy hh:mm:ss, GMT)
               currInode.i_size, //file size (decimal)
               currInode.i_blocks //number of (512 byte) blocks of disk space (decimal) taken up by this file
               );
        
        if (fileType == '?' || (fileType == 's' && currInode.i_size <= 60))
        {
            printf("\n");
            continue;
        }
        
        for (int i = 0; i < EXT2_N_BLOCKS; i++)
        {
            printf(",%u", currInode.i_block[i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    //check for only one command-line parameter: the file system image
    if (argc != 2)
    {
        fprintf(stderr, "Error: Incorrect number of parameters.\nUsage: ./lab3a [fileSystemImage]\n");
        exit(1);
    }
    
    //open file system image
    imageFD = open(argv[1], O_RDONLY);
    
    if (imageFD < 0) //if an error occurred opening the file
    {
        fprintf(stderr, "Error opening file system image.\n");
    }
    
    superBlockSummary();
    groupSummary();
    freeEntries(BLOCK);
    freeEntries(INODE);
    inodeSummary();
}
