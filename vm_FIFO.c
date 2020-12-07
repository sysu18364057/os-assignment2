#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#define PAGESIZE 256
#define nothing -1

struct node{
	int key;
	int value;
	struct node* prev;
	struct node* next;
};//双向链表

struct hash{
	struct node* unused;//未使用计量
	struct hash* next;//解决哈希冲突
};//哈希表

typedef struct{
	int size;
	int capacity;
	struct hash* table;
	struct node* recent;
	struct node* unrecent;
} LRUCache;

struct hash* HashMap(struct hash* table,int key,int capacity){
	int mod = key % capacity;
	return &table[mod];
}//找哈希地址

void HeadInser(struct node* head, struct node* cur)
{
	if(cur->prev == NULL && cur->next == NULL)
	{
		//该节点不再链表中
		//插入
		cur->prev = head;
		cur->next = head->next;
		head->next->prev = cur;
		head->next = cur;
	}
	else{
		struct node* temp = head->next;
		if(temp != cur){
			//cur节点不在第一个
			//取出
			cur->prev->next = cur->next;
			cur->next->prev = cur->prev;
			//插入
			cur->next = temp;
			cur->prev = head;
			head->next = cur;
			temp->prev = cur;
		}
	}
}


LRUCache* LRUCacheCreate(int capacity){
	//分配空间
	LRUCache* a = (LRUCache*)malloc(sizeof(LRUCache));
	a->table = (struct hash*)malloc(sizeof(struct hash)*capacity);
	memset(a->table,0,capacity*sizeof(struct hash));
	a->recent = (struct node*)malloc(sizeof(struct node));
	a->unrecent = (struct node*)malloc(sizeof(struct node));
	//初始化
	a->recent->prev = NULL;
	a->unrecent->next  = NULL;
	a->recent->next = a->unrecent;
	a->unrecent->prev = a->recent;

	a->size = 0;
	a->capacity = capacity;
	return a;
}

int Get_LRUCache(LRUCache* a,int key){
	struct hash* addr = HashMap(a->table,key,a->capacity);
	addr = addr->next;
	if(addr = NULL){
		return nothing;
	}
	while(addr->next != NULL && addr->unused->key != key){
		addr = addr->next;
	}
	if(addr->unused->key == key)
	{
		HeadInser(a->recent,addr->unused);
		return addr->unused->value;
	}
	return nothing;
}

void Put_LRUCache(LRUCache* a,int key, int value){
	struct hash* addr = HashMap(a->table,key,a->capacity);
	if(Get_LRUCache(a,key) == nothing){
		if(a->size >= a->capacity){
			struct node* last = a->unrecent->prev;
			struct hash* remove = HashMap(a->table,last->key,a->capacity);
			struct hash* ptr = remove;
			remove = remove->next;
			while(remove->unused->key != last->key){
				ptr = remove;
				remove = remove->next;
			}
			ptr->next = remove->next;
			remove->next = NULL;
			remove->unused = NULL;
			free(remove);
			struct hash* new_node = (struct hash*)malloc(sizeof(struct hash));
			new_node->next = addr->next;
			addr->next = new_node;
			new_node->unused = last;
			last->key = key;
			last->value = value;
			HeadInser(a->recent,last);
			}
		else{
			struct hash* new_node = (struct hash*)malloc(sizeof(struct hash));
			new_node->unused = (struct node*)malloc(sizeof(struct node));
			new_node->next = addr->next;
			addr->next = new_node;
			new_node->unused->prev = NULL;
			new_node->unused->next = NULL;
			new_node->unused->key = key;
			new_node->unused->value = value;
			HeadInser(a->recent,new_node->unused);
			++(a->size);
		}
	}
	else{
		a->recent->next->value = value;
	}
}

void LRUCacheFree(LRUCache* a){
	free(a->table);
	free(a->recent);
	free(a->unrecent);
	free(a);

}





int main(int argc,char* argv[])
{
	FILE *rd,*fpt;
	int i=0,fp_temp,count=0;
	int phymemory,value;
	int page_table[PAGESIZE];
	char memory[PAGESIZE/2][PAGESIZE];
	int page_num,offset;
	char* buffer;

	//初始化页表,-1为未存取
	for(i = 0;i < PAGESIZE;i++){
		page_table[i] = -1;
	}

	rd = fopen("addresses.txt","r");//只读传入的address
	struct node TLB[16];
	int TLBHitTimes = 0;
	int TLB_point = 0;
	int page_defult = 0;
	int update = 1;
	int x = 0;
	int virtual_ad = 0;
	int va = 0;
	while(fscanf(rd,"%d",&fp_temp) != EOF)
	{
		update = 1;
		virtual_ad = fp_temp;
		page_num = fp_temp>>8;//右移8位获得页码
		va = page_num;
		offset = fp_temp & 255;//后8位比较得出偏移量
		int w=0;
		for(w = 0;w<16;w++){
			if(TLB[w].key == page_num){
				TLBHitTimes++;
				page_num = TLB[w].value;
				goto out;
			}
		}
		if(page_table[page_num] == -1){//缺页
			page_defult++;
			if(count/128){
				//页满了，要置换
				for(w=0;w<PAGESIZE;w++)
					if(page_table[w] == (count % 128))
						page_table[w] = -1;
				for(w=0;w<16;w++){
					if(TLB[w].key == count/128){
						TLB[w].value = page_num;
						update = 0;//已经更新,不用置换
					}
				}
			}
			page_table[page_num] = count % 128;
			fpt = fopen("BACKING_STORE.bin","rb");
			fseek(fpt,PAGESIZE*page_num,0);
			buffer = (char*)malloc(PAGESIZE);
			fread(buffer,1,PAGESIZE,fpt);
			for(x=0;x<PAGESIZE;x++)
				memory[count%128][x] = buffer[x];
			free(buffer);
			page_num = count++%128;
			
		}
		else
			page_num = page_table[page_num];

		if(update){
			if(TLB_point == 16)
				TLB_point = 0;
			TLB[TLB_point].key = va;
			TLB[TLB_point].value = page_num;
			TLB_point++;
		}

		value = memory[page_num][offset];
out:
		page_num = page_num<<8;
		phymemory = page_num + offset;//物理地址
		printf("virtual memory is %d,physical memory is %d,value is %d\n ",virtual_ad,phymemory,value);

		
		
	}
	printf("hit is %d ,defult is  %d\n",TLBHitTimes,page_defult);
	fclose(rd);
	fclose(fpt);
	return 0;
}

