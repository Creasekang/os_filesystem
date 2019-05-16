
#define BLOCK_SIZE 1024
#define INODE_SIZE 128
#define SUPERBLOCK_SIZE 3072
#define DIRITEM_SIZE 32

#define MAX_BLOCK_NUM 16000
#define MAX_INODE_NUM 3048

#define SUPERBLOCK_STARTADDR 0  
#define INODE_STARTADDR 3072    //3*1024
#define BLOCK_STARTADDR 393216  //384*1024
#include<ctime>


class SuperBlock{ //3072  3 Blocks 实际为2424
    int Total_Inode_Num;
    int Total_Block_Num;
    int Total_Free_Block_Num;
    int Block_Size;
    int Inode_Size;
    int SuperBlock_Size;
    int Dir_Item_Size;

    int SuperBlock_StartAddr;
	int Inode_StartAddr;
	int Block_StartAddr;

    int Block_bitmap[500];
    int Inode_bitmap[96];//381*8/32
};


class Inode{     //128  实际为112
    int Inode_Id;
    int direct_block[10];
    int undirect_pointer_block;
    
    int file_size;
    int occupy_block_num;
    struct tm create_time;
};

class Dir_item{   //32 实际为28
    char filename[20];
    int Inode_Id;
    bool isDir;
};
