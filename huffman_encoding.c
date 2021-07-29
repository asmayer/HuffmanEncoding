#include <stdio.h>

#define FIRST_CHILD(index) ((index * 2) + 1)
#define SECOND_CHILD(index) ((index * 2) + 2)

#define PARENT(index) ((index -1) / 2)

typedef struct Node {
  char key;
  int value;
  struct Node* left;
  struct Node* right;
} Node;

typedef struct Heap {
  Node** data;
  int count;
} Heap;

//swaps two elements in the given heap
void swap(Heap* heap, int index1, int index2){
  Node* temp = heap->data[index1];
  heap->data[index1] = heap->data[index2];
  heap->data[index2] = temp;
}

//moves an out of place item up to the proper spot
void percolateUp(Heap* heap, int index){
  while(heap->data[PARENT(index)]->value > heap->data[index]->value){
    swap(heap,index, PARENT(index));
    index = PARENT(index);
  }
}

void heapify(Heap* heap){
  for(int i = 0; i < heap->count; i++){
    percolateUp(heap, i);
  }
}
void printHeap(Heap* heap){
  printf("[");
  for(int i = 0; i < 255; i++){
    printf("%d, ", heap->data[i]->value);
  }
  printf("%d]\n", heap->data[255]->value);
}

//enquees a node on the heap
void enqueue(Heap* heap, Node* node){
  heap->data[heap->count] = node;
  percolateUp(heap, heap->count++);
}

Node* dequeue(Heap* heap){
  Node* min = heap->data[0];
  heap->data[0] = heap->data[heap->count - 1];
  heap->count--;
  heapify(heap);
}

Node* constructHuffmanTree(Heap* heap){
  while(heap->count >=  2){
    Node* node = malloc(sizeof(Node));
    node->left = dequeue(heap);
    node->right = dequeue(heap);
    node->value = node->left->value + node->right->value;
    enqueue(heap, node);
  }
  return dequeue(heap);
}
//counts the frequency of symbols in a file;
Heap* count_symbols(char* filename){
  Node** symbols = malloc(256*sizeof(Node*));
  for(int i = 0; i < 256; i++){
    symbols[i] = malloc(sizeof(Node));
    symbols[i]->value = 0;
    symbols[i]->key = (char) i;
    symbols[i]->left = NULL;
    symbols[i]->right = NULL;
  }

  FILE* f = fopen(filename, "r");
  char c;
  while((c = fgetc(f)) != EOF){
    symbols[c]->value++;
  }

  Heap* heap = malloc(sizeof(Heap));
  heap->data = symbols;
  heap->count = 256;
  heapify(heap);

  return heap;
}

int main(int argc, char** argv){
  Heap* heap = count_symbols("README");
  printHeap(heap);
}
