#include <stdio.h>
#include "heap.h"

uintptr_t heap[HEAP_CAP]={0};

chunk_list alloced_chunks={0};
chunk_list freed_chunks={
    .count=1,
    .chunks={[0]={.start=heap,.size=sizeof(heap)}},
};
chunk_list temp_chunks={0};

int chunk_list_find(const chunk_list* list,uintptr_t* ptr){
    int left=0;
    int right=list->count;
    while(left<right){
        int mid=(left+right)/2;
        if(list->chunks[mid].start==ptr){
            return mid;
        }
        if(list->chunks[mid].start>ptr){
            right=mid;
        }
        else if(list->chunks[mid].start<ptr){
            left=mid+1;
        }
    }
    return -1;
}

void chunk_list_insert(chunk_list* list,void* ptr,size_t size){
    if(list->count<CHUNK_LIST_CAP){
        list->chunks[list->count].start=ptr;
        list->chunks[list->count].size=size;

        for(size_t i=list->count;i>0 && list->chunks[i].start<list->chunks[i-1].start;i--){
            const chunk t=list->chunks[i];
            list->chunks[i]=list->chunks[i-1];
            list->chunks[i-1]=t;
        }
        list->count++;
    }
    else{
        printf("Heap is full\n");
    }
}
void chunk_list_remove(chunk_list* list,size_t index){
    if(index<list->count){
        for(size_t i=index;i<list->count-1;i++){
            list->chunks[i]=list->chunks[i+1];
        }
        list->count--;
    }
}

void chunk_list_merge(chunk_list* temp,chunk_list* list){
   temp->count=0;
   chunk_list_insert(temp,list->chunks[0].start,list->chunks[0].size);
   for(size_t i=1;i<list->count;i++){
        const chunk mychunk=list->chunks[i];
        if(temp->count>0){
            chunk* top_chunk=&temp->chunks[temp->count-1];
            if(top_chunk->start+top_chunk->size==mychunk.start){
                top_chunk->size+=mychunk.size;
            }
            else{
                chunk_list_insert(temp,mychunk.start,mychunk.size);
            }
        }
   }
}