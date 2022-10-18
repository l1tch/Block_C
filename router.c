#include "node.h"
#include "httpd.h"
#include "chain.h"
#include "functions.h"
#include <stdlib.h>


void route(Node* self_node, Block* next_block)
{
    ROUTE_START()

    ROUTE_GET("/")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");

        printf("Http routing on %s", request_header("User-Agent"));
    }

    ROUTE_GET("/init_blockchain")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");

        printf("genesis pointer   %p\n", (void*) self_node->main_chain);
        printf("genesis timestamp %s\n", self_node->main_chain->head->timestamp);
        printf("genesis size %d\n", self_node->main_chain->size);
    }

    ROUTE_POST("/")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        
        printf("%d bytes posted \r\n", payload_size);
        printf("%s",payload);
    }

    ROUTE_POST("/post_transaction")
    {

        // { "author" : "hashstring", "data": "transaction", "timestamp" : "datatime" }
        char** keys = malloc(sizeof(char*) * char_count(payload, ':') * 2);
        char** values = malloc(sizeof(char*) * char_count(payload, ':') * 2);

        fprintf(stderr,"genesis pointer   %p\n", (void*) self_node->main_chain);
        fprintf(stderr,"genesis timestamp %s\n", self_node->main_chain->head->timestamp);

        read_json(payload, &keys, &values);

        // fprintf(stderr,"%s\n",);

        Transaction* transaction = init_transaction();
        build_transaction(transaction, &keys, &values);

        // fprintf(stderr,"%s\n",transaction_datas(transaction));
        // add_transaction(self_node->main_chain, transaction);
        // print linked list
        // print_transaction(transaction);

        add_transaction(self_node->main_chain, transaction);

        free(keys);
        free(values);

        printf("HTTP/1.1 200 OK\r\n\r\n");
            
    }

    ROUTE_GET("/mine"){ 
        // send transaction hash + timestamp + previous hash and js page for finding the nonce

        // fprintf(stderr,"trans chain mine %p\n", (void*) self_node->main_chain->unconfirmed_transactions); 

        FILE* difficulty;

        difficulty = fopen("./conf/difficulty.cfg", "r");
        if(difficulty == NULL)
            return;

        fseek(difficulty, 0, SEEK_END);
        long difficulty_size = ftell(difficulty);
        fseek(difficulty, 0L, SEEK_SET);	

        char* confcontent = malloc(difficulty_size);
        fread(confcontent, sizeof(char), difficulty_size, difficulty);

        FILE* page;
        page = fopen("./pages/mine.html", "r");
        if(page == NULL)
            return;
        fseek(page, 0, SEEK_END);
        long page_size = ftell(page);
        fseek(page, 0L, SEEK_SET);	
        char* fcontent = malloc(page_size + 255);
        fread(fcontent, 1, page_size, page);
            
        char* b_contents = block_contents(next_block, self_node->main_chain->unconfirmed_transactions, self_node->main_chain->head->previous_hash);

        fcontent = str_replace(fcontent, "%diff", confcontent);
        fcontent = str_replace(fcontent, "%block", b_contents);

        fprintf(stderr,"block contents -> %s\n",b_contents); 

        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("%s", fcontent);
        free(fcontent);

    }
    
    ROUTE_POST("/mine"){

        // if payload is none? 

        char** keys = malloc(sizeof(char*) * char_count(payload, ':') * 2);
        char** values = malloc(sizeof(char*) * char_count(payload, ':') * 2);

        read_json(payload, &keys, &values); // { "proof" : "000afe56cd6a...", "nonce": "467" }

        build_block(self_node->main_chain, next_block);

        char* nonce = get_val_by_key(&keys, &values, "nonce");
        int nonce_size = strlen(nonce);
        strncpy(next_block->nonce, nonce, nonce_size);
        
        char* proof = strdup(get_val_by_key(&keys, &values, "proof"));

        char* formatted_contents = block_contents(next_block, self_node->main_chain->unconfirmed_transactions, self_node->main_chain->head->hash);
        char * hash = sha256(formatted_contents);

        if(strcmp(proof, hash) == 0){
            strncpy(next_block->hash, proof, 32);
            // insert transactions in block
            next_block->transactions = self_node->main_chain->unconfirmed_transactions;
            self_node->main_chain->unconfirmed_transactions = NULL;

            // insert block in chain
            if(add_block(self_node->main_chain, next_block)){
                //response ok
                printf("HTTP/1.1 200 OK\r\n\r\n");
                printf("index:: %d\r\n",self_node->main_chain->head->index);
                printf("previous hash:: %s\r\n",self_node->main_chain->head->previous_hash);
                printf("hash:: %s\r\n",hash);

                free(next_block);
                next_block = _init_block();
                build_block(self_node->main_chain, next_block);

            }else{
                free(proof);
                free(next_block);
                printf("HTTP/1.1 200 OK\r\n\r\n");
                printf("Error forging block");
            }
        }else{
            free(proof);
            free(next_block);
            printf("HTTP/1.1 200 OK\r\n\r\n");
            printf("No block was mined");

        }

        // last_proof = last_block['proof']
        // last_hash = self.hash(last_block)

        // proof = 0
        // while self.valid_proof(last_proof, proof, last_hash) is False:
        //     proof += 1

        // return proof


        // previous_hash = blockchain.hash(last_block)
        // block = blockchain.next_block(proof, previous_hash)

        // response = {
        //     'message': "New Block Forged",
        //     'index': block['index'],
        //     'transactions': block['transactions'],
        //     'proof': block['proof'],
        //     'previous_hash': block['previous_hash'],
        // }

        free(formatted_contents);
        free(hash);
        free(keys);
        free(values);
    }
  
    ROUTE_END()
}
