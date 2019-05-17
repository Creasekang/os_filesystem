#define _CRT_SECURE_NO_WARNINGS

#include<ctime>
#include<iostream>
#include<cstring>
#include<cstdio>
#include"os.h"
using namespace std;

void initial();
void dir(FILE*,Inode*);

int main() {
	cout << sizeof(SuperBlock) << endl;
	cout << sizeof(Inode) << endl;
	cout << sizeof(Dir_item) << endl;
	
	SuperBlock t;
    Inode root_Inode;
	initial();

    FILE* file;
	file = fopen("file system.date", "rb+");
    fseek(file, 0, SEEK_SET);
    fread(&t, sizeof(SuperBlock), 1, file);
    t.info_show();
    fseek(file, INODE_STARTADDR, SEEK_SET);
    fread(&root_Inode, sizeof(Inode), 1, file);
    dir(file,&root_Inode);
    root_Inode.info_show();
    fclose(file);
}

//定义用于初始化的对象，用于写入disk文件
SuperBlock superB;
Inode inode_list[MAX_INODE_NUM];



void initial(){

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

    //初始化根目录 inode和block
    Inode root;
    root.Inode_Id=0;
    root.file_size=2*DIRITEM_SIZE;
    root.direct_block[0]=0+BLOCK_STARTADDR;
    root.occupy_block_num=1;
    root.undirect_pointer_block=-1;
    root.create_time=time(NULL);
    root.isDir=true;
    strcpy(root.filename,"root");
    fseek(file, INODE_STARTADDR, SEEK_SET);
	fwrite(&root, sizeof(Inode), 1, file);
	fflush(file);

    Dir_item root_last("..",0,true);
    Dir_item root_cur(".",0,true);
    fseek(file, BLOCK_STARTADDR, SEEK_SET);
	fwrite(&root_last, sizeof(Dir_item), 1, file);
    fseek(file, BLOCK_STARTADDR+DIRITEM_SIZE, SEEK_SET);
    fwrite(&root_cur, sizeof(Dir_item), 1, file);
    fflush(file);

    fclose(file);

}

void dir(FILE* f,Inode* node){   //半成品，仅适用于未采用间接块的部分
    int item_num=node->file_size/DIRITEM_SIZE;
    cout<<setiosflags(ios::left)<<setw(25)<<"Filename"<<setw(10)<<"Inode_Id"<<setw(10)<<"File_size"
    <<setw(10)<<"isDir_s"<<setw(20)<<"Occupy_block_num"<<setw(15)<<"Create_time"<<endl;
    if(node->undirect_pointer_block==-1){
        for(int i=0;i<node->occupy_block_num;++i){
            int block_st=node->direct_block[i];
            int num=0;
            if(i==node->occupy_block_num-1) num=item_num%32;
            else num=32;
            for(int j=0;j<num;++j){
                Dir_item item;
                Inode attr;
                fseek(f, block_st+j*DIRITEM_SIZE, SEEK_SET);
                fread(&item, sizeof(Dir_item), 1, f);
                if(item.filename=="." || item.filename==".."){
                    continue;
                }
                fseek(f,item.Inode_Id*INODE_SIZE+INODE_STARTADDR,SEEK_SET);
                fread(&attr, sizeof(Inode), 1, f);
            }
        }
    }
}

