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

Inode node[MAX_INODE_NUM];


void initial();
void dir(FILE*,Inode*);
int createFile(SuperBlock&,Inode&,FILE*);
void cat(FILE*,Inode*);
void createDir(SuperBlock&,Inode&,FILE*);
int find_item(FILE* f,Inode* node,string filename,Inode* openFile);//用于寻找当前目录是否有 “filename”项
int find_path(FILE* f,Inode* cur_Node,string path,Inode* last,string&);
vector<string> split(string str,string pattern); //分割字符串，用于寻找路径
int changeDir(FILE*f,Inode* node);
string getPath(FILE*f,Inode* node);
SuperBlock superB;
SuperBlock s_block;

int main() {
     

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
    string cur_path="/root";
    
    cout<<cur_path<<">";
    while(cin>>command){
        cout<<endl;
        if(command=="show"){
            s_block.info_show();
        }
        else if(command=="c"){
            createFile(s_block,cur_Dir,file);
        }
        else if(command=="md"){
            createDir(s_block,cur_Dir,file);
        }
        else if(command=="dir"){
            //cur_Dir.info_show();
            dir(file,&cur_Dir);
        }
        else if(command=="cat"){
            cat(file,&cur_Dir);
        }
        else if(command=="cd"){
           int result= changeDir(file,&cur_Dir);
           cur_path=getPath(file,&cur_Dir);
        }
        fseek(file, cur_Dir.Inode_Id*INODE_SIZE+INODE_STARTADDR, SEEK_SET);
        fread(&cur_Dir, sizeof(Inode), 1, file);
        fflush(file);
        cout<<cur_path<<">";
    }
    
    std::fclose(file);
}

//定义用于初始化的对象，用于写入disk文件
Inode inode_list[MAX_INODE_NUM];

int createFile(SuperBlock& s_block,Inode& cur_Dir,FILE* file){
    string path_str;
    string fileName;
    int fileSize;
    cin>>path_str>>fileSize;
    Inode path;
    int result=find_path(file,&cur_Dir,path_str, &path, fileName);  //find the path
    if(result!=1){
        cout<<"The path does not exist."<<endl;
        return -1;
    }
    int b_num;
    vector<int>block_id;
    int inode_id;
   
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
    
    // create the file node
    else{
        b_num=fileSize/BLOCK_SIZE;
        if(fileSize%BLOCK_SIZE)   b_num=b_num+1;
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
        node.isRoot=false;
        node.para_Inode_id=path.Inode_Id;
        node.occupy_block_num=b_num;

        char buffer[BLOCK_SIZE];
        for(int i=0;i<BLOCK_SIZE;++i) 
            buffer[i]=fill_in;
        vector<int>block_addr;
        if(b_num<=DIRECT_BLOCK_NUM){
            for(int i=0;i<b_num;++i){
                node.direct_block[i]=block_id[i]*BLOCK_SIZE+BLOCK_STARTADDR;
            }
            node.undirect_pointer_block=-1;
            cout<<"We fill it by a random string : "<<fill_in<<endl;
        }
        else{
            for(int i=0;i<DIRECT_BLOCK_NUM;++i){
                int c_size=min(BLOCK_SIZE,node.file_size-i*BLOCK_SIZE);
                node.direct_block[i]=block_id[i]*BLOCK_SIZE+BLOCK_STARTADDR;
                fseek(file,node.direct_block[i],SEEK_SET);
                fwrite(buffer,sizeof(char),c_size,file);
                fflush(file);
            }
            int t_block[UNDIRECT_POINTER_BLOCK];
            node.undirect_pointer_block=s_block.get_block(1)[0]*BLOCK_SIZE+BLOCK_STARTADDR;
            for(int i=DIRECT_BLOCK_NUM;i<b_num;++i){
                t_block[i-DIRECT_BLOCK_NUM]=block_id[i]*BLOCK_SIZE+BLOCK_STARTADDR;
            }
            fseek(file,node.undirect_pointer_block,SEEK_SET);
            fwrite(&t_block,sizeof(int),b_num-DIRECT_BLOCK_NUM,file);
            fflush(file);
        }
        for(int i=0;i<block_id.size();++i){
            int block_addr=block_id[i]*BLOCK_SIZE+BLOCK_STARTADDR;
            int c_size=min(BLOCK_SIZE,node.file_size-i*BLOCK_SIZE);
            fseek(file,block_addr,SEEK_SET);
            fwrite(buffer,sizeof(char),c_size,file);
            fflush(file);
        }
        fseek(file,INODE_STARTADDR+inode_id*INODE_SIZE,SEEK_SET);
        fwrite(&node,sizeof(node),1,file);
        fflush(file);
        //add item into cur_Dir
        Dir_item newFile(fileName,node.Inode_Id,false);
        int offset=path.file_size%BLOCK_SIZE;  //是否占满了最后一个block
        if(offset>0){   //没占满了最后一个block
            int b_id;
            if(path.undirect_pointer_block==-1){ //最后一个block在直接块处
                b_id=path.direct_block[path.occupy_block_num-1];
                fseek(file,b_id+offset,SEEK_SET);
                fwrite(&newFile,sizeof(Dir_item),1,file);
            }
            else{
                int buffer[UNDIRECT_POINTER_BLOCK];
                fseek(file,path.undirect_pointer_block,SEEK_SET);
                fread(&buffer,sizeof(int),path.occupy_block_num-DIRECT_BLOCK_NUM,file);
                fflush(file);
                b_id=buffer[path.occupy_block_num-DIRECT_BLOCK_NUM-1];
            }
            fseek(file,b_id+offset,SEEK_SET);
            fwrite(&newFile,sizeof(Dir_item),1,file);
            fflush(file);
        }
        else{    //占满了最后一个block
            int new_addr=BLOCK_STARTADDR+s_block.get_block(1)[0]*BLOCK_SIZE;
            fseek(file,new_addr,SEEK_SET);
            fwrite(&newFile,sizeof(Dir_item),1,file);
            fflush(file);
            if(path.occupy_block_num<DIRECT_BLOCK_NUM){    //直接块仍有空余
                path.direct_block[path.occupy_block_num]=new_addr;
                path.occupy_block_num+=1;
            }
            else{
                if(path.undirect_pointer_block==-1){
                    path.undirect_pointer_block=s_block.get_block(1)[0]*BLOCK_SIZE+BLOCK_STARTADDR;
                }
                int offset=sizeof(int)*(path.occupy_block_num-DIRECT_BLOCK_NUM);
                fseek(file,path.undirect_pointer_block+offset,SEEK_SET);
                fwrite(&new_addr,sizeof(int),1,file);
                fflush(file);
                path.occupy_block_num+=1;
            }
        }
        path.file_size+=DIRITEM_SIZE;
        fseek(file,INODE_STARTADDR+path.Inode_Id*INODE_SIZE,SEEK_SET);
        fwrite(&path,sizeof(Inode),1,file);
        fflush(file);
    }
}

void createDir(SuperBlock& s_block,Inode& cur_Dir ,FILE* file){
    string path_str;
    string fileName;
    cin>>path_str;
    Inode path;
    int result=find_path(file,&cur_Dir,path_str, &path, fileName);  //find the path
    
    if(result!=1){
        cout<<"The path does not exist."<<endl;
        return ;
    }
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
        node.isRoot=false;
        node.para_Inode_id=path.Inode_Id;
        strcpy(node.filename,fileName.c_str());
        fseek(file, INODE_STARTADDR+node.Inode_Id*INODE_SIZE, SEEK_SET);
        fwrite(&node, sizeof(Inode), 1, file);
        fflush(file);
        

        Dir_item node_last("..",path.Inode_Id,true);
        Dir_item node_cur(".",node.Inode_Id,true);
        fseek(file, node.direct_block[0], SEEK_SET);
        fwrite(&node_last, sizeof(Dir_item), 1, file);
        fseek(file, node.direct_block[0]+DIRITEM_SIZE, SEEK_SET);
        fwrite(&node_cur, sizeof(Dir_item), 1, file);
        fflush(file);

        Dir_item newFile(fileName,node.Inode_Id,false);
        int offset=path.file_size%BLOCK_SIZE;  //是否占满了最后一个block
        if(offset>0){   //没占满了最后一个block
            int b_id;
            if(path.undirect_pointer_block==-1){ //最后一个block在直接块处
                b_id=path.direct_block[path.occupy_block_num-1];
                fseek(file,b_id+offset,SEEK_SET);
                fwrite(&newFile,sizeof(Dir_item),1,file);
            }
            else{
                int buffer[UNDIRECT_POINTER_BLOCK];
                fseek(file,path.undirect_pointer_block,SEEK_SET);
                fread(&buffer,sizeof(int),path.occupy_block_num-DIRECT_BLOCK_NUM,file);
                fflush(file);
                b_id=buffer[path.occupy_block_num-DIRECT_BLOCK_NUM-1];
            }
            fseek(file,b_id+offset,SEEK_SET);
            fwrite(&newFile,sizeof(Dir_item),1,file);
            fflush(file);
        }
        else{    //占满了最后一个block
            int new_addr=BLOCK_STARTADDR+s_block.get_block(1)[0]*BLOCK_SIZE;
            fseek(file,new_addr,SEEK_SET);
            fwrite(&newFile,sizeof(Dir_item),1,file);
            fflush(file);
            if(path.occupy_block_num<DIRECT_BLOCK_NUM){    //直接块仍有空余
                path.direct_block[path.occupy_block_num]=new_addr;
                path.occupy_block_num+=1;
            }
            else{
                if(path.undirect_pointer_block==-1){
                    path.undirect_pointer_block=s_block.get_block(1)[0]*BLOCK_SIZE+BLOCK_STARTADDR;
                }
                int offse=sizeof(int)*(path.occupy_block_num-DIRECT_BLOCK_NUM);
                fseek(file,path.undirect_pointer_block+offse,SEEK_SET);
                fwrite(&new_addr,sizeof(int),1,file);
                fflush(file);
                path.occupy_block_num+=1;
            }
        }
        path.file_size+=DIRITEM_SIZE;
        fseek(file,INODE_STARTADDR+path.Inode_Id*INODE_SIZE,SEEK_SET);
        fwrite(&path,sizeof(Inode),1,file);
        fflush(file);
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
    root.occupy_block_num=1;
    root.undirect_pointer_block=-1;
    root.create_time=time(NULL);
    root.isDir=true;
    root.isRoot=true;
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
    vector<int>block_addr;
    if(node->undirect_pointer_block==-1){
        for(int i=0;i<node->occupy_block_num;++i)
        block_addr.push_back(node->direct_block[i]);
    }
    else{
        for(int i=0;i<DIRECT_BLOCK_NUM;++i)
        block_addr.push_back(node->direct_block[i]);
        int buffer[UNDIRECT_POINTER_BLOCK];
        fseek(f,node->undirect_pointer_block,SEEK_SET);
        fread(&buffer,sizeof(int),node->occupy_block_num-DIRECT_BLOCK_NUM,f);
        fflush(f);
        for(int i=0;i<node->occupy_block_num-DIRECT_BLOCK_NUM;++i){
            block_addr.push_back(buffer[i]);
        }
    }
    for(int i=0;i<node->occupy_block_num;++i){
        int block_st=block_addr[i];
        int num=0;
        if(i==node->occupy_block_num-1) num=item_num%32;
        else num=32;
        for(int j=0;j<num;++j){
            Dir_item item;
            Inode attr;
            fseek(f, block_st+j*DIRITEM_SIZE, SEEK_SET);
            fread(&item, sizeof(Dir_item), 1, f);
            fflush(f);
            if( strcmp(item.filename,".")==0 ||  strcmp(item.filename,"..")==0){
                continue;
            }
            fseek(f,item.Inode_Id*INODE_SIZE+INODE_STARTADDR,SEEK_SET);
            fread(&attr, sizeof(Inode), 1, f);
            fflush(f);
            attr.info_show();
        }
    }
    
    
    cout<<endl<<"Print end"<<endl;
}

void cat(FILE*f,Inode* node){
    string path_str;
    string filename;
    cin>>path_str;
    Inode path;
    Inode openFile;
    int result=find_path(f,node,path_str,&path,filename);
    if(result==1){
        int result2=find_item(f,&path,filename,&openFile);
        if(result2==1){
            vector<int>block_addr;
            if(openFile.undirect_pointer_block==-1){
                for(int i=0;i<openFile.occupy_block_num;++i)
                block_addr.push_back(openFile.direct_block[i]);
            }
            else{
                for(int i=0;i<DIRECT_BLOCK_NUM;++i)
                block_addr.push_back(openFile.direct_block[i]);
                int buffer[UNDIRECT_POINTER_BLOCK];
                fseek(f,openFile.undirect_pointer_block,SEEK_SET);
                fread(&buffer,sizeof(int),openFile.occupy_block_num-DIRECT_BLOCK_NUM,f);
                fflush(f);
                for(int i=0;i<openFile.occupy_block_num-DIRECT_BLOCK_NUM;++i)
                block_addr.push_back(buffer[i]);

            }
            for(int i=0;i<openFile.occupy_block_num;++i){
                char content[BLOCK_SIZE];
                int c_size=min(BLOCK_SIZE,openFile.file_size-i*BLOCK_SIZE);
                fseek(f,block_addr[i],SEEK_SET);
                fread(content,sizeof(char),c_size,f);
                fflush(f);
                cout<<"The "<<i+1<<" block locate at "<<block_addr[i]<<endl<<"The content is : "<<endl;
                for(int i=0;i<c_size;++i)
                cout<<content[i];
                cout<<"("<<c_size<<"*"<<content[0]<<")"<<endl<<"Print of "<<i+1<<" block end"<<endl<<endl;
            }
            
        }
    }
    cout<<endl<<"Print end"<<endl;
    return;
                
    
}

int changeDir(FILE*f,Inode* node){
    string path_str;
    string filename;
    cin>>path_str;
    Inode path;
    Inode openFile;
    int result=find_path(f,node,path_str,&path,filename);
    if(result==1){
        int result2=find_item(f,&path,filename,&openFile);
        if(result2==1){
            *node=openFile;
            return 1;
        }
    }
    
    return 0;
                
}

int find_item(FILE* f,Inode* node,string filename,Inode* openFile){ //用于寻找当前目录是否有 “filename”项
    int item_num=node->file_size/DIRITEM_SIZE;
    vector<int>block_addr;
    if(node->undirect_pointer_block==-1){
        for(int i=0;i<node->occupy_block_num;++i)
        block_addr.push_back(node->direct_block[i]);
    }
    else{
        for(int i=0;i<DIRECT_BLOCK_NUM;++i)
        block_addr.push_back(node->direct_block[i]);
        int buffer[UNDIRECT_POINTER_BLOCK];
        fseek(f,node->undirect_pointer_block,SEEK_SET);
        fread(&buffer,sizeof(int),node->occupy_block_num-DIRECT_BLOCK_NUM,f);
        fflush(f);
        for(int i=0;i<node->occupy_block_num-DIRECT_BLOCK_NUM;++i){
            block_addr.push_back(buffer[i]);
        }
    }
    if(node->undirect_pointer_block==-1){
        for(int i=0;i<node->occupy_block_num;++i){
            int block_st=block_addr[i];
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
                    fread(openFile,sizeof(Inode),1,f);
                    return 1;
                }
            }
        }
    }
    return 0;
}

int find_path(FILE* f,Inode* cur_Node,string path,Inode* last,string& filename){
    vector<string> p=split(path,"/");
    Inode node(*cur_Node);
    for(int i=1;i<p.size()-1;++i){
        Inode next;
        string filename=p[i];
        if(find_item(f,&node,p[i],&next)==1){
            node=next;
        }
        else{
            return -1;
        }
    }
    *last=node;
    filename=p.back();
    return 1;
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

string getPath(FILE*f,Inode* node){
    bool isRoot=node->isRoot;
    vector<string> path;
    string ans="";
    int id;
    path.push_back(node->filename);
    if(!isRoot) 
    id=node->para_Inode_id;
    while(!isRoot){
        Inode para;
        fseek(f,id*INODE_SIZE+INODE_STARTADDR,SEEK_SET);
        fread(&para,sizeof(Inode),1,f);
        fflush(f);
        path.push_back(para.filename);
        isRoot=para.isRoot;
        if(!isRoot) 
        id=para.para_Inode_id;
    }
    for(int i=path.size()-1;i>=0;--i)
      ans=ans+"/"+path[i];
    return ans;
}

