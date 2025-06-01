#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "heap.h"

void* ptrs[10]={0};
int main(){
    for(int i=1;i<=10;i++){
        ptrs[i]=heap_alloc(i);
        printf("heap alloc(%d)\n",i);
    }

    // for(int i=0;i<=10;i++){
    //     if(i%2==0){
    //         heap_free(ptrs[i]);
    //     }
    // }

    void* ptr=heap_alloc(10);
   
    chunk_list_dump(&alloced_chunks);
    chunk_list_dump(&freed_chunks);
    return 0;
}