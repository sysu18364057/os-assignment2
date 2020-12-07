#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#define PAGESIZE 256







int main(int argc,char* argv[])
{
	FILE *rd,*fpt;
	int i=0,fp_temp,count=0;
	int phymemory,value;
	int page_table[PAGESIZE];
	char memory[PAGESIZE][PAGESIZE];
	int page_num,offset;
	char* buffer;

	//初始化页表,-1为未存取
	for(i = 0;i < PAGESIZE;i++){
		page_table[i] = -1;
	}

	rd = fopen("addresses.txt","r");//只读传入的address
	
	int x = 0;
	int virtual_ad = 0;
	while(fscanf(rd,"%d",&fp_temp) != EOF)
	{
		virtual_ad = fp_temp;
		page_num = fp_temp>>8;//右移8位获得页码
		offset = fp_temp & 255;//后8位比较得出偏移量
		if(page_table[page_num] == -1){
			page_table[page_num] = count;
			fpt = fopen("BACKING_STORE.bin","rb");
			fseek(fpt,PAGESIZE*page_num,0);
			buffer = (char*)malloc(PAGESIZE);
			fread(buffer,1,PAGESIZE,fpt);
			for(x=0;x<PAGESIZE;x++){
				memory[count][x] = buffer[x];
			}
			free(buffer);
			page_num = count++;
			
		}
		else
			page_num = page_table[page_num];

		value = memory[page_num][offset];
		page_num = page_num<<8;
		phymemory = page_num + offset;//物理地址
		printf("virtual memory is %d,physical memory is %d,value is %d\n ",virtual_ad,phymemory,value);

		
		
	}
	fclose(rd);
	fclose(fpt);
	return 0;
}

