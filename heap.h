#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stdlib.h>
#define HEAP_CAP_BYTES 640000
#define HEAP_CAP (HEAP_CAP_BYTES/sizeof(uintptr_t))
extern uintptr_t heap[HEAP_CAP];

#define todo\
    do{\
        fprintf(stderr,"%s:%d: TODO:%s is not yet implemented\n",__FILE__,__LINE__,__func__);\
        abort();\
    }while(0);

void *heap_alloc(size_t size_bytes);
void heap_free(void* ptr);
void heap_collect();

#define CHUNK_LIST_CAP 1024
#define HEAP_FREED_CAP 1024

typedef struct{
    uintptr_t* start;
    size_t size;
}chunk;

typedef struct{
    size_t count;
    chunk chunks[CHUNK_LIST_CAP];
}chunk_list;

extern chunk_list alloced_chunks;
extern chunk_list freed_chunks;
extern chunk_list temp_chunks;

void chunk_list_dump(const chunk_list* list);
int chunk_list_find(const chunk_list* list,uintptr_t* ptr);
void chunk_list_insert(chunk_list* list,void* ptr,size_t size);
void chunk_list_remove(chunk_list* list,size_t index);
void chunk_list_merge(chunk_list* temp,chunk_list* list);
#endif