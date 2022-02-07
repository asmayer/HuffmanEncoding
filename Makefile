all: huffman_encoding

huffman_encoding: huffman_encoding.c
	gcc -g huffman_encoding.c -o huffman_encoding

clean:
