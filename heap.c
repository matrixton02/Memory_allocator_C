#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "heap.h"

uintptr_t heap[HEAP_CAP]={0};
bool reachable_chunks[CHUNK_LIST_CAP]={0};
void* to_free[CHUNK_LIST_CAP]={0};
size_t to_free_count=0;

const uintptr_t* stack_base=0;
chunk_list alloced_chunks={0};
chunk_list freed_chunks={
    .count=1,
    .chunks={[0]={.start=heap,.size=HEAP_CAP}},
};
chunk_list temp_chunks={0};

void chunk_list_dump(const chunk_list* list,const char* name){
    printf("%s Chunk (%zu):\n",name,list->count);
    for(size_t i=0;i<list->count;i++){
        printf("Start: %p, size:%zu\n",(void*)list->chunks[i].start,list->chunks[i].size);
    }
}

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

void* heap_alloc(size_t size_bytes){
    const size_t size_words=(size_bytes+sizeof(uintptr_t)-1)/sizeof(uintptr_t);
    if(size_words>0){
        chunk_list_merge(&temp_chunks,&freed_chunks);
        freed_chunks=temp_chunks;
        for(size_t i=0;i<freed_chunks.count;i++){
            const chunk mychunk=freed_chunks.chunks[i];
            if(mychunk.size>=size_words){
                chunk_list_remove(&freed_chunks,i);
    
                const size_t rest_size=mychunk.size-size_words;
                chunk_list_insert(&alloced_chunks,mychunk.start,size_words);
                if(rest_size>0){
                    chunk_list_insert(&freed_chunks,mychunk.start+size_words,rest_size);
                }
                return (void*) mychunk.start;
            }
        }
    }
    return NULL;
}
void heap_free(void* ptr){
    if(ptr!=NULL){
        const int index=chunk_list_find(&alloced_chunks,ptr);
        if(index>=0 && ptr==alloced_chunks.chunks[index].start){
            chunk_list_insert(&freed_chunks,alloced_chunks.chunks[index].start,alloced_chunks.chunks[index].size);
            chunk_list_remove(&alloced_chunks,(size_t)index);
        }
    }
}
static void mark_region(const uintptr_t *start,const uintptr_t* end){
    for(;start<end;start+=1){
        const uintptr_t* p=(const uintptr_t*)*start;
        for(size_t i=0;i<alloced_chunks.count;i++){
            chunk Chunk=alloced_chunks.chunks[i];
            if(Chunk.start<=p && p<Chunk.start+Chunk.size){
                if(!reachable_chunks[i]){
                    reachable_chunks[i]=true;
                    mark_region(Chunk.start,Chunk.start+Chunk.size);
                }
            }
        }
    }
}
void heap_collect(){
    const uintptr_t* stack_start=(const uintptr_t*)__builtin_frame_address(0);
    memset(reachable_chunks,0,sizeof(reachable_chunks));
    mark_region(stack_start,stack_base+1);

    to_free_count=0;
    for(size_t i=0;i<alloced_chunks.count;i++){
        // printf("Start: %p size %zu reachable %s\n",alloced_chunks.chunks[i].start,alloced_chunks.chunks[i].size,reachable_chunks[i]?"true":"false");
        if(!reachable_chunks[i]){
            to_free[to_free_count++]=alloced_chunks.chunks[i].start;
        }
    }

    for(size_t i=0;i<to_free_count;i++){
        heap_free((void*)to_free[i]);
    }
}