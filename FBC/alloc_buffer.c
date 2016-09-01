//#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/stat.h>
#include <string.h>

#define FLEX_BLK 16  //一个大group中的小group数量（多少个Bitmap是连续的）
#define EXTENT 0x0001  //最大extent数量
#define BUFF_SIZE 1024*4096/2  //inode buffer大小
#define BLK_SIZE 4096  //块大小
#define BLK_NUM 1024*16/2   //data buff中有多少个块
#define INODE_SIZE 256  //inode 大小
#define TOTAL_BLK BLK_SIZE*8*FLEX_BLK  //一个bitmap涵盖的块数量
#define BIT_NUM 763   //文件系统总块数量



void main()
{
	int start_point;
	int mark;
	int fd,fd1;//file pos
	FILE* alloc;
	FILE* file;
	FILE* bitmap;
	int b_buf,i_buf,blk_num;//counting
	int n,m;//return value
	int i,j,k,l;
	char name[128];
	long long int inode_order[BUFF_SIZE/16];
	int blk_order_size[BLK_NUM];	//记录每个文件的大小
	int blk_order_num[BLK_NUM];		//记录每个文件所需的块
	memset(blk_order_size,0,sizeof(blk_order_size));
	memset(blk_order_num,0,sizeof(blk_order_num));
	long long int blk,pos;//inode position
	int offset;
	b_buf=0,i_buf=0;
	file = fopen("/home/yl/file.txt","r");
	alloc = fopen("/home/yl/test.txt","r");
	
	int tar=0;//block position
	int tar_bit=0;//bitmap position
	char bit_map[BLK_SIZE*FLEX_BLK]; //bitmap buffer



	unsigned short int inode_buff[BUFF_SIZE/2];		//inode  buff
	char *data_buff = (char *)malloc(BLK_NUM*BLK_SIZE);		//data buffer
	struct stat buf;
	fd = open("/dev/sdb3",O_RDWR|O_NONBLOCK);

	
	//init bitmap
	start_point = 1;
	int bit_blk,final_bit;
	bitmap = fopen("/home/yl/bitmap.txt","r");
	
	

	i=0;
	while((fscanf(bitmap,"%d",&bit_blk)>0)&&tar_bit<FLEX_BLK)
	{
		lseek(fd, (int64_t)bit_blk*BLK_SIZE, SEEK_SET);
		read(fd, bit_map+tar_bit*BLK_SIZE, BLK_SIZE);
		tar_bit++;
		tar++;
	}
	
	while(1)
	{
		
		//复制结束时写
		if(fscanf(alloc,"%lld %x",&blk,&offset)<=0)
		{
			printf("---------------------------------------------------------------------------------------------------\n");
			start_point = alloc_bitmap(b_buf, bit_map);
			//printf("start_point:%d\n",start_point);
/* 			while(start_point == 0)
			{
				lseek(fd, (int64_t)(bit_blk-15)*BLK_SIZE, SEEK_SET);
				//write(fd, bit_map, BLK_SIZE*FLEX_BLK);
				tar_bit = 0;
				while((fscanf(bitmap,"%d",&bit_blk)>0)&&i<FLEX_BLK)
				{
					lseek(fd,(int64_t) bit_blk*BLK_SIZE, SEEK_SET);
					read(fd, bit_map+i*BLK_SIZE, BLK_SIZE);
					tar_bit++;
					tar++;
					final_bit = bit_blk;
				}
				start_point = alloc_bitmap(b_buf, bit_map);
				if(i!=FLEX_BLK)
				{
					bit_blk = final_bit;
					goto no_full;
				}
			} */

			lseek(fd, (int64_t)((tar - FLEX_BLK)*TOTAL_BLK  + start_point)*BLK_SIZE, SEEK_SET);
			
			//write(fd, data_buff, b_buf*BLK_SIZE);
			
			//write inode
			for(i=0;i<=i_buf;i++)
			{
				alloc_inode(inode_buff,blk_order_size[i],i*INODE_SIZE,(int64_t)((tar - FLEX_BLK)*TOTAL_BLK  + start_point));
				lseek(fd,(int64_t)inode_order[i],SEEK_SET);
				start_point=start_point+blk_order_num[i];
				//write(fd,inode_buff+i*INODE_SIZE,INODE_SIZE);
			}
			goto no_full;
		}
		fscanf(file,"%s",name);
		
		
		stat(name,&buf);
		
		blk_num = size_to_block(buf.st_size);
		
		
		fd1 = open(name,O_RDWR|O_NONBLOCK);
		pos = blk*BLK_SIZE + offset;
		
		//Buffer满的时候写
		if((blk_num+b_buf)>=BLK_NUM||i_buf+1>=(BUFF_SIZE/INODE_SIZE))
		{	
			//write blk
			start_point = alloc_bitmap(b_buf, bit_map);
			while(start_point==0)
			{
				lseek(fd, (int64_t)(bit_blk-15)*BLK_SIZE, SEEK_SET);
				//write(fd, bit_map, BLK_SIZE*FLEX_BLK);
				tar_bit = 0;
				while((fscanf(bitmap,"%d",&bit_blk)>0)&&tar_bit<FLEX_BLK)
				{
					//fclose(bitmap);
					lseek(fd, (int64_t)bit_blk*BLK_SIZE, SEEK_SET);
					read(fd, bit_map+tar_bit*BLK_SIZE, BLK_SIZE);
					tar_bit++;
					tar++;
					final_bit = bit_blk;
				}
				start_point = alloc_bitmap(b_buf, bit_map);
				if(tar_bit!=FLEX_BLK)
				{
					bit_blk = final_bit;
					goto no_full;
				}
			}
			//printf("pos:%lld\n",(int64_t)(tar - FLEX_BLK)*TOTAL_BLK  + start_point);
			lseek(fd, (int64_t)((tar - FLEX_BLK)*TOTAL_BLK  + start_point)*BLK_SIZE, SEEK_SET);
			//write(fd, data_buff, b_buf*BLK_SIZE);
			
		
			//write inode
			k =0;
			for(i=0;i<=i_buf;i++)
			{
				alloc_inode(inode_buff,blk_order_size[i],i*INODE_SIZE,(int64_t)((tar - FLEX_BLK)*TOTAL_BLK  + start_point));
				lseek(fd,(int64_t)inode_order[i],SEEK_SET);
				start_point=start_point+blk_order_num[i];
				//write(fd,inode_buff+i*INODE_SIZE,INODE_SIZE);
			}
			
			k=0;
			b_buf = 0;
			i_buf = 0;
			memset(inode_buff,0,sizeof(inode_buff));
			memset(data_buff,0,sizeof(data_buff));
			//goto no_full;
		}
		
		//read new inode
		//printf("pos:%lld\n",(int64_t)pos);
		lseek(fd,(int64_t)pos,SEEK_SET);

		n = read(fd, inode_buff+(unsigned short int)i_buf*INODE_SIZE,INODE_SIZE);		
		blk_order_size[i_buf] =buf.st_size;
		blk_order_num[i_buf] = blk_num;
		inode_order[i_buf++] = pos;
		
		
		//read new data
		m = read(fd1, data_buff+b_buf*BLK_SIZE,buf.st_size);
		b_buf = b_buf + blk_num;
	}
no_full:	
	free(data_buff);
	fclose(bitmap);
	fclose(file);
	fclose(alloc);
	lseek(fd, (int64_t)(bit_blk-tar_bit)*BLK_SIZE, SEEK_SET);
	//write(fd, bit_map, BLK_SIZE*tar_bit);
}

int size_to_block(long int size)
{
	int i=0;
	while(size>0)
	{
		i++;
		size = size - BLK_SIZE;
	}
	return i;
}

int alloc_inode(unsigned short int *buff , int size , int num,long long int pos)
{
	//update size;
	int blocknum = size_to_block(size);
	buff[num+2] = size;
	buff[num+3] = (size>>16);
	//update block number
	buff[num+14] = blocknum;
	buff[num+15] = blocknum>>16;
	
	//update block position;
	buff[num+21] = EXTENT;
	buff[num+28] = blocknum;
	buff[num+29] = (pos>>32); 
	//upper 16 bit, big file system need change; 
	buff[num+31] = (pos>>16);   //middle 16 bit;
	buff[num+30] = pos;       //low 16 bit;
	return 0;
}


int alloc_bitmap(int blk_num, char *bitmap)
{
	struct pos{
		int start;
		int stop;
	} pos1;
	int tar = 0 ;
	int j;
	int	i=0, k=0;
	int m ,n;
	pos1.start = 0, pos1.stop = 0;
	while(i<TOTAL_BLK/8)
	{
		for(j=0;j<8;j++)
		{
			if(((bitmap[i]>>j)&01)==0)
				k++;
			else
			{
				tar++;
				k = 0;
				if(j<7)
					pos1.start = i, pos1.stop =j+1;
				else
					pos1.start = i+1, pos1.stop =0;
				
			}
			if(k == blk_num)
			{
				m = pos1.start, n = pos1.stop;
				while(pos1.start!=i||pos1.stop!=j)
				{
					bitmap[pos1.start] = bitmap[pos1.start]|(01<<pos1.stop);
					if(pos1.stop<7)
						pos1.stop++;
					else
					{
						pos1.start++;
						pos1.stop = 0;
					}
				}
				bitmap[pos1.start] = bitmap[pos1.start]|(01<<pos1.stop);

				return tar+1; 
			}
		}
		i++;
	}
	return 0;
}

