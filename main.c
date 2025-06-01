#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "heap.h"

typedef struct Node Node;

struct Node{
    char x;
    Node* left;
    Node* right;
};

Node* generate_tree(size_t level_cur,size_t level_max,int a){
    if(level_cur<level_max){
        Node* root=(Node*) heap_alloc(sizeof(*root));
        root->x=(char)(level_cur+a+97);
        root->left=generate_tree(level_cur+1,level_max,a+1);
        root->right=generate_tree(level_cur+1,level_max,a+2);
        return root;
    }
    else{
        return NULL;
    }
}

void print_tree(Node* root,size_t level_cur){
    if(root==NULL){
        return;
    }
    printf("%c ",root->x);
    print_tree(root->left,level_cur+1);
    print_tree(root->right,level_cur+1);
}
int main(){
    Node* root=generate_tree(0,3,0);
    print_tree(root,0);
    printf("-----------------------------\n");
    size_t heap_ptr_count=0;
    for(size_t i=0;i<alloced_chunks.count;i++){
        for(size_t j=0;j<alloced_chunks.chunks[i].size;j++){
            uintptr_t* p=(uintptr_t*)alloced_chunks.chunks[i].start[j];
            if(heap<=p && heap+HEAP_CAP){
                printf("Detected heap pointer: %p\n",(void*)p);
                heap_ptr_count+=1;
            }
        }
    }
    printf("Detected heap pointes %d \n",heap_ptr_count); 
    
    return 0;
}