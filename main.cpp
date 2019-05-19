#define _CRT_SECURE_NO_WARNINGS

#include<ctime>
#include<iostream>
#include<cstring>
#include<cstdio>
#include<vector>
#include<algorithm>
#include<cstdlib>
#include"os.h"
using namespace std;

void initial();
void dir(FILE*,Inode*);
int createFile(SuperBlock&,Inode&,FILE*);
void cat(FILE*,Inode*);
void createDir(SuperBlock&,Inode&,FILE*);
int find_item(FILE* f,Inode* node,string filename,Inode* openFile);

vector<string> split(string str,string pattern);

SuperBlock superB;
SuperBlock s_block;
int main() {
     
     /*string path;
     while(cin>>path){
         for(auto i:split(path, "/")){
             cout<<i<<endl;
         }
     }*/

	cout << sizeof(SuperBlock) << endl;
	cout << sizeof(Inode) << endl;
	cout << sizeof(Dir_item) << endl;
	
	SuperBlock s_block;
    Inode root_Inode;
    Inode cur_Dir;
	initial();

    FILE* file;
	file = fopen("file system.date", "rwb+");
    fseek(file, 0, SEEK_SET);
    fread(&s_block, sizeof(SuperBlock), 1, file);

    fseek(file, INODE_STARTADDR, SEEK_SET);
    fread(&root_Inode, sizeof(Inode), 1, file);
    fseek(file, INODE_STARTADDR, SEEK_SET);
    fread(&cur_Dir, sizeof(Inode), 1, file);

    string command;
    while(cin>>command){
        if(command=="show"){
            s_block.info_show();
        }
        else if(command=="c"){
            createFile(s_block,cur_Dir,file);
        }
        else if(command=="cd"){
            createDir(s_block,cur_Dir,file);
        }
        else if(command=="dir"){
            cur_Dir.info_show();
            dir(file,&cur_Dir);
        }
        else if(command=="cat"){
            cat(file,&cur_Dir);
        }
    }
    
    std::fclose(file);
}

//定义用于初始化的对象，用于写入disk文件
Inode inode_list[MAX_INODE_NUM];

int createFile(SuperBlock& s_block,Inode& cur_Dir,FILE* file){
    string fileName;
    int fileSize;
    int b_num;
    vector<int>block_id;
    int inode_id;
    cin>>fileName>>fileSize;
    if(s_block.Total_Free_Inode_Num<=0){
        cout<<"There is not enough free Inode."<<endl;
        return -1;
    }
    else if(fileSize>BLOCK_SIZE*s_block.Total_Free_Block_Num || fileSize>MAX_FILE_SIZE){
        cout<<"File size is too lagre. It should not be more than "<<1024*s_block.Total_Free_Block_Num<<"Bytes."<<endl;
        return -1;    
    }
    else if(fileName.size()>MAX_FILE_NAME_LENGTH){
        cout<<"File name is too long. It should be smaller than "<<MAX_FILE_NAME_LENGTH<<"Bytes."<<endl;
        return -1;
    }
    // find the path
    
    // create the file node
    else{
        b_num=fileSize/BLOCK_SIZE;
        if(fileSize)   b_num=b_num+1;
        cout<<b_num<<endl;
        inode_id=s_block.get_Inode(); 
        block_id=s_block.get_block(b_num);
        char fill_in=rand()%26+'A';
        Inode node;
        node.create_time=time(NULL);
        node.file_size=fileSize;
        strcpy(node.filename,fileName.c_str());
        node.fill_in=fill_in;
        node.Inode_Id=inode_id;
        node.isDir=false;
        node.occupy_block_num=b_num;
        if(b_num<=DIRECT_BLOCK_NUM){
            for(int i=0;i<b_num;++i){
                node.direct_block[i]=block_id[i]*BLOCK_SIZE+BLOCK_STARTADDR;
            }
            node.undirect_pointer_block=-1;
            fseek(file,INODE_STARTADDR+inode_id*INODE_SIZE,SEEK_SET);
            fwrite(&node,sizeof(node),1,file);
            char buffer[BLOCK_SIZE];
            for(int i=0;i<BLOCK_SIZE;++i) buffer[i]=fill_in;
            for(int i=0;i<node.occupy_block_num;++i){
                int c_size=min(BLOCK_SIZE,node.file_size-i*BLOCK_SIZE);
                fseek(file,node.direct_block[i],SEEK_SET);
                cout<<node.direct_block[i]<<endl;
                fwrite(buffer,sizeof(char),c_size,file);
            }
            cout<<"We fill it by a random string : "<<fill_in<<endl;
        }
        
        //add item into cur_Dir
        Dir_item newFile(fileName,node.Inode_Id,false);
        int offset=cur_Dir.file_size%BLOCK_SIZE;  //是否占满了最后一个block
        if(offset>0){   //没占满了最后一个block
            if(cur_Dir.undirect_pointer_block==-1){ //最后一个block在直接块处
                int b_id=cur_Dir.direct_block[cur_Dir.occupy_block_num-1];
                cout<<b_id<<"  "<<offset<<endl;
                fseek(file,b_id+offset,SEEK_SET);
                fwrite(&newFile,sizeof(Dir_item),1,file);
                cur_Dir.file_size+=DIRITEM_SIZE;
            }
            else{

            }
        }
        else{    //占满了最后一个block
            if(cur_Dir.occupy_block_num<10){    //直接块仍有空余
                int new_addr=BLOCK_STARTADDR+s_block.get_block(1)[0]*BLOCK_SIZE;
                cur_Dir.direct_block[cur_Dir.occupy_block_num]=new_addr;
                cur_Dir.occupy_block_num+=1;
                fseek(file,new_addr,SEEK_SET);
                fwrite(&newFile,sizeof(Dir_item),1,file);
                cur_Dir.file_size+=DIRITEM_SIZE;
            }
        }
    }
}

void createDir(SuperBlock& s_block,Inode& cur_Dir ,FILE* file){
    string fileName;
    cin>>fileName;
    if(s_block.Total_Free_Inode_Num<=0){
        cout<<"There is not enough free Inode."<<endl;
        return;
    }
    else if(s_block.Total_Free_Block_Num<=0){
        cout<<"File size is too lagre. It should not be more than "<<1024*s_block.Total_Free_Block_Num<<"Bytes."<<endl;
        return; 
    }
    else if(fileName.size()>MAX_FILE_NAME_LENGTH){
        cout<<"File name is too long. It should be smaller than "<<MAX_FILE_NAME_LENGTH<<"Bytes."<<endl;
        return;
    }
    else{
        Inode node;
        node.Inode_Id=s_block.get_Inode();
        node.file_size=2*DIRITEM_SIZE;
        node.direct_block[0]=(s_block.get_block(1))[0]*BLOCK_SIZE+BLOCK_STARTADDR;
        node.occupy_block_num=1;
        node.undirect_pointer_block=-1;
        node.create_time=time(NULL);
        node.isDir=true;
        strcpy(node.filename,fileName.c_str());
        fseek(file, INODE_STARTADDR+node.Inode_Id*INODE_SIZE, SEEK_SET);
        fwrite(&node, sizeof(Inode), 1, file);
        fflush(file);

        Dir_item node_last("..",cur_Dir.Inode_Id,true);
        Dir_item node_cur(".",node.Inode_Id,true);
        fseek(file, node.direct_block[0], SEEK_SET);
        fwrite(&node_last, sizeof(Dir_item), 1, file);
        fseek(file, node.direct_block[0]+DIRITEM_SIZE, SEEK_SET);
        fwrite(&node_cur, sizeof(Dir_item), 1, file);
        fflush(file);

        Dir_item newFile(fileName,node.Inode_Id,false);
        int offset=cur_Dir.file_size%BLOCK_SIZE;  //是否占满了最后一个block
        if(offset>0){   //没占满了最后一个block
            if(cur_Dir.undirect_pointer_block==-1){ //最后一个block在直接块处
                int b_id=cur_Dir.direct_block[cur_Dir.occupy_block_num-1];
                cout<<b_id<<"  "<<offset<<endl;
                fseek(file,b_id+offset,SEEK_SET);
                fwrite(&newFile,sizeof(Dir_item),1,file);
                cur_Dir.file_size+=DIRITEM_SIZE;
            }
            else{

            }
        }
        else{    //占满了最后一个block
            if(cur_Dir.occupy_block_num<10){    //直接块仍有空余
                int new_addr=BLOCK_STARTADDR+s_block.get_block(1)[0]*BLOCK_SIZE;
                cur_Dir.direct_block[cur_Dir.occupy_block_num]=new_addr;
                cur_Dir.occupy_block_num+=1;
                fseek(file,new_addr,SEEK_SET);
                fwrite(&newFile,sizeof(Dir_item),1,file);
                cur_Dir.file_size+=DIRITEM_SIZE;
            }
        }
    }
}


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
	for(int i=0;i<INODE_BITMAP_SIZE;++i) superB.Inode_bitmap[i]=0;
    for(int i=0;i<BLOCK_BITMAP_SIZE;++i) superB.Block_bitmap[i]=0; 
	//cout << sizeof(inode_list) << endl;  //92*3048=280416

	//初始化inode表
	for (int i = 0; i < MAX_INODE_NUM; i++)
	{
		inode_list[i].Inode_Id = i;
	}

	//写入disk
	//超级块写入

    //初始化根目录 inode和block
    Inode root;
    root.Inode_Id=superB.get_Inode();
    root.file_size=2*DIRITEM_SIZE;
    root.direct_block[0]=(superB.get_block(1))[0]*BLOCK_SIZE+BLOCK_STARTADDR;
    cout<<root.direct_block[0]<<endl;
    root.occupy_block_num=1;
    root.undirect_pointer_block=-1;
    root.create_time=time(NULL);
    root.isDir=true;
    strcpy(root.filename,"root");

	fseek(file, SUPERBLOCK_STARTADDR, SEEK_SET);
	fwrite(&superB, sizeof(SuperBlock), 1, file);
	fflush(file);


    fseek(file, INODE_STARTADDR, SEEK_SET);
	fwrite(&root, sizeof(Inode), 1, file);
	fflush(file);

    Dir_item root_last("..",root.Inode_Id,true);
    Dir_item root_cur(".",root.Inode_Id,true);
    fseek(file, BLOCK_STARTADDR, SEEK_SET);
	fwrite(&root_last, sizeof(Dir_item), 1, file);
    fseek(file, BLOCK_STARTADDR+DIRITEM_SIZE, SEEK_SET);
    fwrite(&root_cur, sizeof(Dir_item), 1, file);
    fflush(file);

    std::fclose(file);

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
                if( strcmp(item.filename,".")==0 ||  strcmp(item.filename,"..")==0){
                    continue;
                }
                fseek(f,item.Inode_Id*INODE_SIZE+INODE_STARTADDR,SEEK_SET);
                fread(&attr, sizeof(Inode), 1, f);
                attr.info_show();
            }
        }
    }
}

void cat(FILE*f,Inode* node){
    string filename;
    cin>>filename;
    Inode openFile;
    int result=find_item(f,node,filename,&openFile);
    if(result==1){
        if(openFile.undirect_pointer_block==-1){
            for(int i=0;i<openFile.occupy_block_num;++i){
                char content[BLOCK_SIZE];
                int c_size=min(BLOCK_SIZE,openFile.file_size-i*BLOCK_SIZE);
                fseek(f,openFile.direct_block[i],SEEK_SET);
                fread(content,sizeof(char),c_size,f);
                for(int i=0;i<c_size;++i)
                cout<<content[i];
            }
        }
    }
    cout<<endl<<"Print end"<<endl;
    return;
                
    
}

int find_item(FILE* f,Inode* node,string filename,Inode* openFile){
    int item_num=node->file_size/DIRITEM_SIZE;
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
                if(strcmp(filename.c_str(),item.filename)==0){
                    int node_id=item.Inode_Id;
                    fseek(f,INODE_STARTADDR+node_id*INODE_SIZE,SEEK_SET);
                    fread(&openFile,sizeof(Inode),1,f);
                    return 1;
                }
            }
        }
    }
    return 0;
}

void find_path(Inode* cur_Node,string path){
    vector<string> p=split(path,"/");
    for(int i=0;i<p.size()-1;++i){
        string filename=p[0];
        
    }
}

vector<string> split(string str,string pattern) {   //分割字符串，用于寻找路径
  string::size_type pos;
  vector<string> result;
  str+=pattern;
  int size=str.size();
 
  for(int i=0; i<size; i++)
  {
    pos=str.find(pattern,i);
    if(pos<size)
    {
      string s=str.substr(i,pos-i);
      result.push_back(s);
      i=pos+pattern.size()-1;
    }
  }
  return result;
}
