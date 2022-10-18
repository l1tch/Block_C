#include "chain.h"
#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

Transaction* init_transaction(){
    Transaction* transaction = (Transaction*) malloc(sizeof(Transaction));
    transaction->key =  malloc(sizeof(char));
    transaction->value =  malloc(sizeof(char));
    transaction->key[0] = '\0';
    transaction->next = NULL;
    return transaction;
}

void build_transaction(Transaction* transaction, char*** keys, char*** values){

    FILE* transaction_objdef;
    transaction_objdef = fopen("transaction.objdef", "r");

    if (transaction_objdef == NULL){
        return;
    }

    char* buffer = NULL;
    size_t len = 0;
    ssize_t read;
    int inserted = 0;

    if(transaction == NULL){
        return;
    }

    // can i make a function of this listed insertion?

    for(int i = 0; *(*(keys)+i); i++){
        inserted = 0;
        rewind(transaction_objdef);
        while ((read = getline(&buffer, &len, transaction_objdef)) != -1) {
            remove_char(buffer, '\n');
            if(strcmp(buffer, *(*(keys)+i))==0){
                while(*(*(keys)+i) != NULL){
                    if(transaction->key[0] == '\0' || strcmp(transaction->key, *(*(keys)+i)) == 0){
                        transaction->key = strdup(*(*(keys)+i));
                        if (transaction->value[0] != '\0'){
                            int strsize = sizeof(transaction->value);
                            char* tmp = malloc(strsize);
                            memcpy(tmp, transaction->value,strsize);
                            free(transaction->value);
                            transaction->value = malloc ( sizeof(*(*(values)+i)) + strsize );
                            memcpy( transaction->value,tmp,strsize);
                            strcat(transaction->value, *(*(values)+i));

                        }else{
                            transaction->value = strdup(*(*(values)+i));
                        }
                        inserted = 1;
                        // free(**(keys+i));
                        // **(keys+i) = NULL;

                        // free(**(values+i));
                        // **(values+i) = NULL;

                        break;

                    }else{
                        inserted = 0;
                        if(transaction->next == NULL){
                            transaction->next = (Transaction*) init_transaction();
                            // fprintf(stderr,("ptr %p \n"), ( void * ) transaction->next);
                            transaction = (Transaction*) transaction->next;
                        }else{
                            // fprintf(stderr,("ptr %p \n"), ( void * ) transaction->next);
                            transaction  = (Transaction*) transaction->next;
                        }
                    }
                }
                if (inserted){
                    inserted = 0;
                    break;
                }
            }
        }
    }
    free(buffer);
    fclose(transaction_objdef);
}

void add_transaction(Blockchain* main_c, Transaction* pack){

    if(main_c->unconfirmed_transactions == NULL){
        main_c->unconfirmed_transactions = malloc(sizeof(struct Transaction *));
        main_c->unconfirmed_transactions[0] = malloc(sizeof(Transaction));
        memcpy(main_c->unconfirmed_transactions[0], pack, sizeof(pack));
    } else {
        if (main_c->unconfirmed_transactions[0]->key[0] == '\0'){
            main_c->unconfirmed_transactions[0]->key = pack->key;
            main_c->unconfirmed_transactions[0]->value = pack->value;
            main_c->unconfirmed_transactions[0]->next = pack->next;
        } else{
            Transaction** tmp = (Transaction **) malloc(sizeof(Transaction*) * sizeof(main_c->unconfirmed_transactions));
            memcpy(tmp, main_c->unconfirmed_transactions, sizeof(main_c->unconfirmed_transactions));
            // free(main_c->unconfirmed_transactions);
            int newsize = (sizeof(main_c->unconfirmed_transactions)+1);
            main_c->unconfirmed_transactions = (Transaction **) malloc(sizeof(Transaction*) * newsize);
            memcpy(main_c->unconfirmed_transactions, tmp, sizeof(tmp));
            memcpy(main_c->unconfirmed_transactions+newsize, pack, sizeof(pack));
        }
    }
}   

void print_transaction(Transaction* transaction_print){
    do{
        fprintf(stderr, "%s -> %s\n", transaction_print->key, transaction_print->value);
        transaction_print = transaction_print->next;

    }while(transaction_print != NULL);
}

char* transaction_datas(Transaction* transaction_print){

    char* tran_datas = malloc(strlen(transaction_print->key)+strlen(transaction_print->value) + 6);

    sprintf(tran_datas,"%s\n%s\n\n", transaction_print->key,transaction_print->value);
    do{
        transaction_print = transaction_print->next;
        if (transaction_print != NULL){
            int strsize = strlen(tran_datas);
            char* tmp = malloc(strsize);
            memcpy(tmp, tran_datas, strsize);
            free(tran_datas);
        
            tran_datas = malloc(strlen(transaction_print->key) + strlen(transaction_print->value) + strsize + 6 );
            sprintf(tran_datas,"%s%s\n%s\n\n", tmp, transaction_print->key,transaction_print->value);
        }
    }while(transaction_print != NULL);

    return tran_datas;
}

Block* _init_block(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    Block* block = (Block*) malloc(sizeof(Block));
    block->index =  0;
    memset(block->nonce, '\0', sizeof(block->nonce));
    memset(block->transactions_hash, '0', sizeof(block->transactions_hash));
    memset(block->previous_hash, '0', sizeof(block->previous_hash));
    memset(block->hash, '\0', sizeof(block->hash));

    // sprintf(block->timestamp, "test");
    sprintf(block->timestamp, "%ld", tv.tv_sec);
    block->previous_block = NULL;
    return block;
}

void build_block(Blockchain* chain, Block* block) {
    block->index = chain->head->index + 1;
    strcpy(block->previous_hash, chain->head->previous_hash);
}

char* block_contents(Block* block, Transaction** transactions, char* previous_hash){
    Transaction** trs = transactions;
    char* trans_datas;
    char* tmpdatas;
    int sizetrs = sizeof(trs) / sizeof(Transaction**);
    int i = 0;
    do{
        tmpdatas = transaction_datas(trs[i]);
        trans_datas = (char *) realloc(trans_datas, sizeof(transaction_datas)+sizeof(tmpdatas));
        strcat(trans_datas, tmpdatas);
        i++;
    }while(i != sizetrs);

    char* transactions_hash = sha256(trans_datas);

    char* contents = malloc(strlen(block->previous_hash) + strlen(transactions_hash) +strlen(block->timestamp) + strlen(block->nonce) + 6);
    // hash::transaction_datas::timestamp::nonce

    if (block->nonce[0] != '\0')
        sprintf(contents,"%s::%s::%s::%s", block->previous_hash,transactions_hash,block->timestamp,block->nonce);
    else
        sprintf(contents,"%s::%s::%s::", block->previous_hash,transactions_hash,block->timestamp);

    free(trans_datas);
    return contents;
}

int is_valid(Block* block){ return 1; }

int add_block(Blockchain* chain, Block* block){

    ///////////////////// todo 

    if (chain->size > 0){
        if (strcmp(block->previous_hash, chain->head->previous_hash)){
            strcpy(block->previous_hash, chain->head->hash);
            block->previous_block = (struct  Block*) chain->head;
            chain->head = block;

            ////// TODO
            // int blocksize = sizeof(block);
            // unsigned char* bytearray = malloc(blocksize);
            // memcpy(bytearray, block, blocksize);


            // memcpy(tmp, chain.chain, chain.size);
            // free(chain.chain);
            // chain.chain = tmp;
            // chain.chain[chain.size] = block;
            return 1;
        }
    }
    return 0;
}

Blockchain* _init_blockchain(){
    Blockchain* chain = malloc(sizeof(Blockchain));
    chain->unconfirmed_transactions = malloc(sizeof(struct Transaction *));
    chain->unconfirmed_transactions[0] = init_transaction();
    chain->head = _init_block();
    chain->size = 1;
    return chain;
}