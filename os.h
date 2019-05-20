
#define BLOCK_SIZE 1024
#define INODE_SIZE 128
#define SUPERBLOCK_SIZE 3072
#define DIRITEM_SIZE 32

#define FILE_SIZE	 16*1024*1024  //单位Byte

#define MAX_BLOCK_NUM 16000
#define MAX_INODE_NUM 3048    //381*1024/128=3048

#define MAX_FILE_SIZE 1024*(10+256)
#define MAX_FILE_NAME_LENGTH 20
#define SUPERBLOCK_STARTADDR 0  
#define INODE_STARTADDR 3072    //3*1024
#define BLOCK_STARTADDR 393216  //384*1024
#define DIRECT_BLOCK_NUM 10
#define INT_SIZE 32
#define INODE_BITMAP_SIZE 96
#define BLOCK_BITMAP_SIZE 500
#define UNDIRECT_POINTER_BLOCK 1024/4

#include<ctime>
#include<string>
#include<iostream>
#include<iomanip>
#include<string>
#include<cstdio>
#include<climits>
#include<vector>

using namespace std;
class SuperBlock{ //3072  3 Blocks 实际为2424
public:
    int Total_Inode_Num;
    int Total_Block_Num;
    int Total_Free_Block_Num;
    
    int Total_Free_Inode_Num;
    
    int Block_Size;
    int Inode_Size;
    int SuperBlock_Size;
    int Dir_Item_Size;

    int SuperBlock_StartAddr;
	int Inode_StartAddr;
	int Block_StartAddr;

   
	unsigned int Block_bitmap[500];
	unsigned int Inode_bitmap[96];//381*8/32
    void info_show(){
        cout<<"Block size : "<<Block_Size<<endl;
        cout<<"Inode size : "<<Inode_Size<<endl;
        cout<<"SuperBlock_Size : "<<SuperBlock_Size<<endl;
        cout<<"DirItem size : "<<Dir_Item_Size<<endl;
        cout<<"SuperBlock_StartAddr : "<<SuperBlock_StartAddr<<endl;
        cout<<"Inode_StartAddr : "<<Inode_StartAddr<<endl;
        cout<<"Block_StartAddr : "<<Block_StartAddr<<endl;
        cout<<"Free Inode number/Total Inode Number : "<<Total_Free_Inode_Num<<"/"<<Total_Inode_Num<<endl;
        cout<<"Free Block number/Total Inode Number : "<<Total_Free_Block_Num<<"/"<<Total_Block_Num<<endl;
    }
    int get_Inode(){
        for(int i=0;i<INODE_BITMAP_SIZE;++i){
            for(int j=31;j>=0;--j){
                //cout<<"Inode : "<<Inode_bitmap[i]<<"  j :"<<j<<endl;
                if((Inode_bitmap[i]>>j)%2==0){
                    Inode_bitmap[i]=Inode_bitmap[i]+(1<<(j));
                    Total_Free_Inode_Num--;
                    return 31-j+i*32;
                }
            }
        }
    }
    vector<int> get_block(int num){
        vector<int>block_id;
        for(int i=0;i<BLOCK_BITMAP_SIZE;++i){
            for(int j=31;j>=0;--j){
                if((Block_bitmap[i]>>j)%2==0){
                    Block_bitmap[i]=Block_bitmap[i]+(1<<j);
                    Total_Free_Block_Num--;
                    num--;
                    block_id.push_back(31-j+i*32);
                    if(num<=0) 
                    return block_id;
                }
            }
        }
    }
   
};


 class Inode{     //128  实际为112   92
 public:
    int Inode_Id;
    int direct_block[DIRECT_BLOCK_NUM];
    int undirect_pointer_block;
    
    int file_size;
    int occupy_block_num;
    time_t create_time;
    char filename[MAX_FILE_NAME_LENGTH];
    char fill_in;
    bool isDir;
    bool isRoot;
    int para_Inode_id;
    /*Inode(const Inode* other){
        Inode_Id=other->Inode_Id;
        for(int i=0;i<DIRECT_BLOCK_NUM;++i) direct_block[i]=other->direct_block[i];
        undirect_pointer_block=other->undirect_pointer_block;
        file_size=other->file_size;
        occupy_block_num=other->occupy_block_num;
        create_time=other->create_time;
        strcpy(filename,other->filename);
        fill_in=other->fill_in;
        isDir=other->isDir;
    }*/
    Inode(const Inode& other){
        Inode_Id=other.Inode_Id;
        for(int i=0;i<DIRECT_BLOCK_NUM;++i) direct_block[i]=other.direct_block[i];
        undirect_pointer_block=other.undirect_pointer_block;
        file_size=other.file_size;
        occupy_block_num=other.occupy_block_num;
        create_time=other.create_time;
        strcpy(filename,other.filename);
        fill_in=other.fill_in;
        isDir=other.isDir;
        isRoot=other.isRoot;
        para_Inode_id=other.para_Inode_id;
    }
    Inode(){

    }
    void info_show(){
        string isDir_s;
        if(isDir) isDir_s="True";
        else isDir_s="False";
        cout<<setiosflags(ios::left)<<setw(25)<<filename<<setw(10)<<Inode_Id<<setw(10)<<file_size
        <<setw(10)<<isDir_s<<setw(20)<<occupy_block_num;
        struct tm time_out=*localtime(&create_time);
        printf("%d年%d月%d日%d时%d分%d秒  星期%d",time_out.tm_year+1900,
        time_out.tm_mon+1,time_out.tm_mday,time_out.tm_hour,time_out.tm_min,
        time_out.tm_sec,time_out.tm_wday);
        cout<<endl;
    }

    int get_offset(){
        if(file_size<BLOCK_SIZE){
            return direct_block[0]*BLOCK_SIZE+BLOCK_STARTADDR+file_size;
        }
        else{
            int off=file_size%BLOCK_SIZE;
            if(occupy_block_num<=10){

            }
        }
    }
};

 class Dir_item{   //32 实际为28
 public:
    char filename[MAX_FILE_NAME_LENGTH];
    int Inode_Id;
    bool isDir;
    Dir_item(string name,int id,bool isdir){
        if(name.size()>MAX_FILE_NAME_LENGTH){
            cout<<"File name too long"<<endl;
        }
        strcpy(filename,name.c_str());
        Inode_Id=id;
        isDir=isdir;
    }
    Dir_item(){

    }
    void info_show(){
        
        cout<<"filename : "<<filename<<endl;
        cout<<"Inode_Id : "<<Inode_Id<<endl;
        cout<<"isDir : "<<isDir<<endl;
    }
};


