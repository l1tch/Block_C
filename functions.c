#include "functions.h"
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

char* sha256(char *s){
	unsigned char *d = SHA256(s, strlen(s), 0);
	int i;
    char* buffer = malloc(sizeof(char)*4);
    char* trad_buffer =  malloc(sizeof(char)*64);

	for (i = 0; i < SHA256_DIGEST_LENGTH; i++){
        snprintf(buffer,4,"%02x",d[i]);
        sprintf(trad_buffer + strlen(trad_buffer), buffer);
    }
    
    return trad_buffer;
}

char** str_split(char* a_str, const char a_delim)
{
    char delim[2];
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    delim[0]         = a_delim;
    delim[1]         = 0;


    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

void remove_spaces(char* s) {
    char* d = s;
    do {
        while (*d == ' ' || *d == '\n' ) {
            ++d;
        }
    } while (*s++ = *d++);
}

void remove_char(char* s, char c) {
    char* d = s;
    do {
        while (*d == c ) {
            ++d;
        }
    } while (*s++ = *d++);
}

int char_count(char* s, char c) {
    int count = 0;
    while (*s != '\0' ) {
        if (*s == c)
            ++count;
        s++;
    }
    return count;
}

void read_json(char* payload_string, char*** keys, char*** values){
        
    remove_spaces(payload_string);

    // to modify on defferent depths
    remove_char(payload_string, '{');
    remove_char(payload_string, '}');
    remove_char(payload_string, '"');

    char** keyval;
    char** json;
    json = str_split(payload_string, ',');

    if (json){
        int i,j,k = 0;
        for (i = 0; *(json + i); i++) {
            keyval = str_split( *(json + i), ':');
            for (j = 0; *(keyval + j); j++) {
                if (j%2){
                    * (*(values) + (k-1)/2) = strdup( *(keyval + j) );
                    // fprintf(stderr,("val %d %s \n"), ((k-1)/2),* (*(values) + (k-1)/2) );

                }else{
                    * (*(keys) + (k)/2) = strdup( *(keyval + j) );
                    // fprintf(stderr,("key %d %s \n"), ((k)/2),* (*(keys) + (k-1)/2) );
                }
                k++;
                free(*(keyval + j));
            }
            free(keyval);
            free(*(json + i));
        }
        free(json);
    }

}

char* get_val_by_key( char*** keys, char*** values, char* selection_key ){
    for(int i = 0; *(*(keys)+i); i++){
        if(strcmp(selection_key, *(*(keys)+i)) == 0){
            return *(*(values)+i);
        }
    }
}

// char** substr_json(char* json){ // to complete
//     int i = 0;
//     while(json[i] != '\0'){
//         if (json[i]=='{'){
//             substr_json(json+i);
//         }
//     }
// }

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}