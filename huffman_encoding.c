/*
 * Encodes and decodes files using huffman coding
 *
 * Author: Aaron Mayer
 *
 */

/*
 * TODO:
 * fix Heap; current algorithmic complexity is not ideal
 * refactor code so it's not spaghetti
 * make data structures more generic
 * naming conventions used in this file are not consistent -fix that
 * make function to deallocate a tree
 *      -Currently huffman trees are never deallocated
 *          which could pose significant memory issues if this is ever extended to decode/encode more than 1 file
 * make headers more efficient space efficient
 *      -saving symbol frequencies at the start of encoded files - not ideal
 * provide better documentation and perhaps switch from a Makefile to CMake
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIRST_CHILD(index) ((index * 2) + 1)
#define SECOND_CHILD(index) ((index * 2) + 2)

#define PARENT(index) ((index -1) / 2)

typedef struct Node {
    unsigned char symbol;
    int freq;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct Heap {
    Node** data;
    int count;
} Heap;

typedef struct Encoding {
    unsigned int encoding;
    unsigned char bits;
} Encoding;

typedef struct BitWriter{
    unsigned char buff;
    int used;
    FILE* file;
} BitWriter;

typedef struct BitReader {
    int buff;
    int unread;
    FILE* file;
} BitReader;

void init_BitWriter(BitWriter* w, FILE* f){
    w->buff = 0;
    w->used = 0;
    w->file = f;
}
void init_BitReader(BitReader* r, FILE* f){
    r->buff = 0;
    r->unread = 0;
    r->file = f;
}

void writeBit(BitWriter* w, unsigned char data){
    w->buff = (w->buff << 1) | (0x1 & data);
    if(++w->used == 8){
        fputc(w->buff, w->file);
        w->buff = 0;
        w->used = 0;
    }
}
char readBit(BitReader* r){
    if(!r->unread){
        r->buff = fgetc(r->file);
        r->unread = 8;
    }
    if(r->buff == EOF) return EOF;
    unsigned char ret = r->buff & 0x80;
    r->buff = r->buff << 1;
    r->unread--;
    if(ret) ret = 1;
    return ret;

}
void writeData(BitWriter* w, unsigned int data, int size){
    int i;
    for(i = size - 1; i >= 0; i--){
        writeBit(w, data >> i);
    }
}
//swaps two elements in the given heap
void swap(Heap* heap, int index1, int index2){
    Node* temp = heap->data[index1];
    heap->data[index1] = heap->data[index2];
    heap->data[index2] = temp;
}

//moves an out of place item up to the proper spot
void percolateUp(Heap* heap, int index){
    while(heap->data[PARENT(index)]->freq > heap->data[index]->freq){
        swap(heap,index, PARENT(index));
        index = PARENT(index);
    }
}


void heapify(Heap* heap){
    for(int i = 0; i < heap->count; i++){
        percolateUp(heap, i);
    }
}
void printHeapAsArray(Heap* heap){
    printf("[");
    for(int i = 0; i < heap->count - 1; i++){
        printf("%d, ", heap->data[i]->freq);
    }
    printf("%d]\n", heap->data[heap->count - 1]->freq);
}
void printHeapAsTree(Heap* heap){
    int num = 1;
    int inc = 2;
    int i;
    for(i = 0; i < heap->count; i++){
        printf("%d ",heap->data[i]->freq);
        if(i + 1 == num) {
            num += inc;
            inc *= 2;
            printf("\n");
        }
    }
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
    return  min;
}


//counts the frequency of symbols in a file;
void count_symbols(int* counts, char* filename){
    //initialize values
    int i;
    for(i = 0; i < 256; i++){
        counts[i] = 0;
    }

    FILE* f = fopen(filename, "r");

    //count symbols
    int c;
    while((c = fgetc(f)) != EOF){
        counts[c]++;
    }
    fclose(f);

}
Node* constructHuffmanTree(int* counts){

    //count nonzero
    int count = 0;
    int i;
    for(i = 0; i < 256; i++){
        if(counts[i] > 0) count++;
    }

    Heap* heap = malloc(sizeof(Heap));
    heap->data = malloc(count*sizeof(Node*));
    heap->count = count;

    //fill heap
    int free_index = 0;
    for(i = 0; i < 256; i++){
        if(counts[i] > 0){
            Node* n = malloc(sizeof(Node));
            n->freq = counts[i];
            n->symbol = (char) i;
            n->left = NULL;
            n->right = NULL;
            heap->data[free_index++] = n;
        }
    }
    heapify(heap);

    while(heap->count >=  2){
        Node* node = malloc(sizeof(Node));
        node->left = dequeue(heap);
        node->right = dequeue(heap);
        node->freq = node->left->freq + node->right->freq;
        enqueue(heap, node);
    }
    return dequeue(heap);
}
void createEncTable(Encoding* enc_table, Node* huff, unsigned int prev, unsigned int num){
    //leaf
    if(huff->left == NULL && huff->right == NULL){
        enc_table[huff->symbol].encoding = prev;
        enc_table[huff->symbol].bits = num;
        return;
    }

    //not leaf
    if(huff->left != NULL ){
        createEncTable(enc_table, huff->left, (prev << 1) | 1, num + 1);
    }
    if(huff->right != NULL ){
        createEncTable(enc_table, huff->right, (prev << 1), num + 1);
    }
}
void encode(Encoding* enc_table, FILE* in, FILE* out){
    BitWriter w;
    init_BitWriter(&w, out);

    int c;
    while((c = fgetc(in)) != EOF){
        writeData(&w, enc_table[c].encoding, enc_table[c].bits);
    }
}
void decode(Node* huff, FILE* in, FILE* out){
    BitReader r;
    init_BitReader(&r, in);

    Node* cur = huff;
    int c;
    while((c = readBit(&r)) != EOF){
        if(c){
            cur = cur->left;
        }
        else{
            cur = cur->right;
        }
        if(cur->left == NULL && cur->right == NULL){
            fputc(cur->symbol, out);
            cur = huff;
        }
    }
}
void encodeHeader(FILE* f, int* counts){
    int i;
    for(i = 0; i < 256; i++){
        putw(counts[i], f);
    }
}
Node* decodeHeader(FILE* f){
    int counts[256];
    int i;
    for(i = 0; i < 256; i++){
        counts[i] =getw(f);
    }
    return constructHuffmanTree(counts);
}
void encodeFile(char* inputDir, char* outputDir){
    int counts[256];
    count_symbols(counts, inputDir);
    Node* n = constructHuffmanTree(counts);
    Encoding encodings[256];
    createEncTable(encodings, n, 0, 0);
    FILE* f = fopen(inputDir, "r");
    FILE* out = fopen(outputDir, "w");
    encodeHeader(out, counts);
    encode(&encodings, f, out);

    fclose(f);
    fclose(out);
}
void decodeFile(char* inputDir, char* outputDir){
    FILE* in = fopen(inputDir, "r");
    FILE* out = fopen(outputDir, "w");
    Node* huff = decodeHeader(in);
    decode(huff, in, out);
    fclose(in);
    fclose(out);
}

int main(int argc, char** argv){
    if(argc == 5){
        if(!strcmp(argv[3], "-o")){
            if(!strcmp(argv[1], "-e")){
                encodeFile(argv[2], argv[4]);
            }
            else if(!strcmp(argv[1], "-d")){
                decodeFile(argv[2], argv[4]);
            }
        }
    }

}
