#include "node.h"
#include "chain.h"
#include "functions.h"

void _init_node(Node* self_node, Block* next_block){
    
    self_node->main_chain = _init_blockchain();

    build_block(self_node->main_chain, next_block);

}


