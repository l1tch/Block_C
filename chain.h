#ifndef _CHAIN_H___
#define _CHAIN_H___
#define getName(var)  #var

// note macro
// #define PRINT_INT(X) printf(#X"-->%d\n",X)
// #define PRINT_OBJ(X) printf(#X".x-->%d " #X".y-->%d\n" ,X.x ,X.y)

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// TRANSACTION SHOULD BE A LINKED LIST

typedef struct Transaction{
	struct Transaction* next;
	char* value;
	char* key; 
}Transaction;

Transaction* init_transaction();
void build_transaction(Transaction* transaction, char*** keys, char*** values);
void print_transaction(Transaction* transaction_print);
char* transaction_datas(Transaction* transaction_print);

typedef struct {
	int index;
    char timestamp[32]; // DD/MM/YYYY HH:MM:SS
	char nonce[32];
    char transactions_hash[32];
    char previous_hash[32];
    char hash[32];
	Transaction** transactions;
	struct Block* previous_block;
}Block;

Block* _init_block();

typedef struct {
	Block* head;
	int size;
	Transaction** unconfirmed_transactions;
}Blockchain;


typedef struct {
	Blockchain* main_chain;
	// others ...
}Node;

void build_block(Blockchain* chain, Block* block);
char* block_contents(Block* block, Transaction** transaction, char* previous_hash);

// hash
char* transaction_hashing(Transaction pack);
char* block_hashing(Block block);

// genesis block
Blockchain* _init_blockchain();
void genesis(Blockchain* chain);

// addblock
int add_block(Blockchain* chain, Block* block);

// add transaction
void add_transaction(Blockchain* main_c, Transaction* pack);

// is valid
int is_valid(Block* block);
// mine

// Node* self_node;
// Block* next_block;

#endif

