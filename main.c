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

void print_tree(Node* root, int depth) {
    if (root == NULL) return;

    print_tree(root->right, depth + 1);

    for (int i = 0; i < depth; i++) {
        printf("    "); // 4 spaces per level
    }
    printf("%c\n", root->x);

    print_tree(root->left, depth + 1);
}

int main(){
    stack_base=(const uintptr_t*)__builtin_frame_address(0);
    for(size_t i=0;i<=10;i++){
        heap_alloc(i);
    }
    Node* root=generate_tree(0,3,0);
    print_tree(root,0);
    printf("\n-----------------------------\n");
    heap_collect();
    chunk_list_dump(&alloced_chunks,"Alloced");
    chunk_list_dump(&freed_chunks,"freed");
    printf("\n-----------------------------\n");
    root=NULL;
    heap_collect();
    heap_alloc(1);
    chunk_list_dump(&alloced_chunks,"Alloced");
    chunk_list_dump(&freed_chunks,"freed");
    return 0;
}