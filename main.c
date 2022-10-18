#include "node.h"
#include "httpd.h"
#include "chain.h"

// require OpenSSL
// compile flags: -lssl -lcrypto


int main(int c, char** v)
{
    //strcmp(v[],"indexer")
    //strcmp(v[],"node")

    server_listen("11011");
    return 0;
}