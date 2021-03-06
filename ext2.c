#include <stdio.h>
#include <string.h>
const int bz=0x400;//blocksize
const int groups=8;//(blockcount)/(block per group)
const int ipg=2016;//inode per group
const int si=128;//size of inode_table
const int sgd=0x20;//size of group decribers
FILE* fp;
struct dir{
  int inode;
  int rec_len;
  int file_type;
  int name_len;
  char name[256];
};
const int xz[3]={0x100,0x10000,0x1000000};
long long toint(unsigned char *a,int s,int off)
{if(off==1)return (a[s+off]<<8)+a[s+off-1];
 if(off==3)return (a[s+off]<<24)+(a[s+off-1]<<16)+(a[s+off-2]<<8)+a[s+off-3];
}
struct dir parsedir()
{  struct dir res;
   unsigned char tmp[4];
   memset(res.name,0,sizeof(res.name));
   fread(tmp,4,1,fp);
   res.inode=toint(tmp,0,3);
   fread(tmp,2,1,fp);
   res.rec_len=toint(tmp,0,1);
   fread(tmp,1,1,fp);
   res.name_len=tmp[0];
   fread(tmp,1,1,fp);
   res.file_type=tmp[0];
   fread(res.name,res.name_len,1,fp);
   return res;

}
void writeblock(int block,char *buffer,int size)
{char out[bz];
 memset(out,0,bz);
 memcpy(out,buffer,size);
 fseek(fp,bz*block,SEEK_SET);
 fwrite(out,bz,1,fp);
}
int inodeoffset(int inode)
{int num=inode/ipg;
 int i;
 fseek(fp,bz*2,SEEK_SET);
 fseek(fp,sgd*num,SEEK_CUR);//group decribers
 unsigned char gd[sgd];
 fread(gd,sgd,1,fp);
 short pos=toint(gd,8,1);
 return bz*pos+128*(inode-num*ipg-1);//
}
int xunzhi(int *blocks,int s,int x)
{int i;
 int cnt=0;
 if(s)fseek(fp,bz*s,SEEK_SET);
 for(i=0;i<x;++i){
    char tmp[4];
    fread(tmp,4,1,fp);
    int x=toint(tmp,0,3);
    if(x==0)break;
    blocks[cnt++]=x;
    

 }
 return cnt;

}
/*To be continued*/
void echo(int inode,char *buffer)
{int blocks[65536];
 int cnt=itob(blocks,inode);
 int off=inodeoffset(inode);
 fseek(fp,off+4,SEEK_SET);
 //printf("%s",buffer);
 int i;char sizet[4];
 int size=strlen(buffer);
 writeblock(blocks[0],buffer,size);
 /*char tmp[4];
 fread(tmp,1,4,fp);
 printf("%X %X",tmp[0],tmp[1]);*/
 for(i=0;i<4;++i){
   sizet[i]=size&0xff;
   size>>=8;

 }
 fwrite(sizet,1,4,fp);
 //printf("%d",(int)strlen(buffer));
 //printf("%X %X",blocks[0],off);
}
void mkdir(int inode,char *name)
{

}
void touch()
{

}
void delete(int block,char *name)
{

}
void basicinfo()
{fseek(fp,bz,SEEK_SET);
 unsigned char superblock[bz*1];
 fread(superblock,bz,1,fp);

}
/*To be continued*/
int itob(int *blocks,int inode)//inode to block
{int i;
 int offset=inodeoffset(inode);
 fseek(fp,offset+40,SEEK_SET);
 int cnt=0;
 cnt=xunzhi(blocks,0,12);
 unsigned int uxz[3];//jian jie xun zhi
 if(cnt==12){
    for(i=0;i<3;++i){
       unsigned char tmp[4];
       fread(tmp,4,1,fp);
       uxz[i]=toint(tmp,0,3);
    }
    for(i=0;i<3;++i)if(uxz[i])cnt+=xunzhi(blocks+cnt,uxz[i],xz[i]);
  }
  return cnt;
}

void dir(int block)
{fseek(fp,block*bz,SEEK_SET);
 printf("%X",0x400*block);
 while(1){
   struct dir tmpdir=parsedir();
   if(tmpdir.inode)printf("%6d: %12d%12d %12s\n",tmpdir.inode,tmpdir.rec_len,tmpdir.file_type,tmpdir.name);
   if(tmpdir.rec_len>=256)break;
   int seek=tmpdir.rec_len-8-tmpdir.name_len;
   fseek(fp,seek,SEEK_CUR);
 }
}
void backup(int inode)
{int blocks[65536];
 int cnt=itob(blocks,inode);
 int i;
 FILE *bu=fopen("aa.bak","wb+");
 for(i=0;i<cnt;++i){
   unsigned char buffer[bz];
   memset(buffer,0,bz);
   if(i==0)fseek(fp,blocks[0]*bz,SEEK_SET);
   else fseek(fp,(blocks[i]-blocks[i-1]-1)*bz,SEEK_CUR);
   fread(buffer,bz,1,fp);
   //printf("%s",buffer);
   fwrite(buffer,bz,1,bu);

 }
 fclose(bu);

}
void cat(int inode)
{int blocks[65536];
 int cnt=itob(blocks,inode);
 int i;
 for(i=0;i<cnt;++i){
   unsigned char buffer[bz];
   if(i==0)fseek(fp,blocks[0]*bz,SEEK_SET);
   else fseek(fp,(blocks[i]-blocks[i-1]-1)*bz,SEEK_CUR);
   fread(buffer,bz,1,fp);
   printf("%s",buffer);
   
 }
 printf("\n");
}
void ls(int inode)
{printf(" inode number | rec_len | file_type | name\n");
 printf("======================================================================\n");
 int blocks[233];
 int cnt=itob(blocks,inode);
 int i;
 for(i=0;i<cnt;++i)dir(blocks[i]);

}

//debug
int ctoi(char *a)
{int res=0;
 int i;
 for(i=0;i<strlen(a);++i)res=res*10+a[i]-'0';
 return res;
}
main(int argc, char** argv)
{int i;
 fp=fopen("bean3","r+");
 if(strcmp(argv[1],"ls")==0)ls(ctoi(argv[2]));
 if(strcmp(argv[1],"cat")==0)cat(ctoi(argv[2]));
 if(strcmp(argv[1],"backup")==0)backup(ctoi(argv[2]));
 if(strcmp(argv[1],"echo")==0)echo(ctoi(argv[3]),argv[2]);

}
