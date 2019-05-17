#define _CRT_SECURE_NO_WARNINGS


#include<iostream>
#include"os.h"
using namespace std;

void initial();

int main() {
	cout << sizeof(SuperBlock) << endl;
	cout << sizeof(Inode) << endl;
	cout << sizeof(Dir_item) << endl;
	
	
	initial();
}


//定义用于初始化的对象，用于写入disk文件
SuperBlock superB;
Inode inode_list[MAX_INODE_NUM];

 



void initial()
{

	char*buff = new char[FILE_SIZE];
	
	FILE* file;
	file = fopen("file system.date", "wb+");
	if (file==NULL)
	{
		printf("file system file creat failed !\n");
		exit(0);
	}
	fseek(file, 0, SEEK_SET);
	fwrite(buff, FILE_SIZE, 1, file);
	fflush(file);
	//fclose(file);

	//初始化superBlock
	superB.Total_Inode_Num = MAX_INODE_NUM;
	superB.Total_Block_Num = MAX_BLOCK_NUM;
	superB.Total_Free_Block_Num = MAX_BLOCK_NUM;
	superB.Total_Free_Inode_Num = MAX_INODE_NUM;

	superB.SuperBlock_StartAddr = SUPERBLOCK_STARTADDR;
	superB.Inode_StartAddr = INODE_STARTADDR;
	superB.Block_StartAddr = BLOCK_STARTADDR;

	superB.SuperBlock_Size = SUPERBLOCK_SIZE;
	superB.Inode_Size = INODE_SIZE;
	superB.Dir_Item_Size = DIRITEM_SIZE;
	superB.Block_Size = BLOCK_SIZE;
			//初始化bitmap
	//cout << sizeof(superB.Block_bitmap) << endl << sizeof(superB.Inode_bitmap); //2000, 384
	memset(superB.Block_bitmap, 0, sizeof(superB.Block_bitmap));
	memset(superB.Inode_bitmap, 0, sizeof(superB.Inode_bitmap));

	

	//cout << sizeof(inode_list) << endl;  //92*3048=280416

	//初始化inode表
	for (int i = 0; i < MAX_INODE_NUM; i++)
	{
		inode_list[i].Inode_Id = i;

	}


	//写入disk
	

	//超级块写入
	fseek(file, SUPERBLOCK_STARTADDR, SEEK_SET);
	fwrite(&superB, sizeof(SuperBlock), 1, file);
	fflush(file);


	//inode list 写入
	for (int i = 0; i < MAX_INODE_NUM; i++)
	{
		fseek(file, INODE_STARTADDR+i*INODE_SIZE, SEEK_SET);
		fwrite(&inode_list[i], sizeof(Inode), 1, file);
		fflush(file);
	}


	//BLOCK *16000 写入
	char block[1];  // 1 Byte
	memset(block,0,sizeof(char));
	for (int i = 0; i < MAX_BLOCK_NUM; i++)
	{
		fseek(file, BLOCK_STARTADDR + i * BLOCK_SIZE, SEEK_SET);
		fwrite(block, BLOCK_SIZE, 1, file);
		
	}


	//初始化root目录




}

