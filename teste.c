#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MSGSZ 500
#define MAX_STATE_VALUES 2
#define STR_MIN 8


//Estrutura que guarda uma mensagem de controle (erro ou sucesso)
struct control_msg{
    char *type;
    unsigned code;
};

//Estrutura que guarda uma mensagem para o servidor
struct request_msg{
    char *type;
    int local_id; 
    int dev_id;
    int *dev_state;
};

//Funcao pra inicializar mensagem
void build_msg(struct request_msg *msg_out, char *tipo, int loc, int dev, int *state_vec){
    msg_out->type = tipo;
    msg_out->dev_id = dev;
    msg_out->local_id = loc;
    msg_out->dev_state = state_vec;

    printf("Msg built\n");
}

//Funcao para desalocar os vetores de uma req_msg
void free_string2msg(struct request_msg *msg){
    
    //free(msg->type);
    free(msg->dev_state); 
}

// [OBS] - Como as mensagens de controle tem apenas dois parametros, optei por já criar direto como string sem uma struct intermediaria
//Funcao pra construir mensagem de ERROR jah em formato de string, fornecendo apenas o codigo
void build_error_msg(char *msg_out, unsigned codigo){
    
    //parse int->str
    char *code_aux = malloc(sizeof(3));
    sprintf(code_aux, " %02u", codigo);
    
    strcat(msg_out, "ERROR ");
    strcat(msg_out, code_aux);
    strcat(msg_out, "\n");

    free(code_aux);
}

//Funcao pra construir mensagem de OK jah em formato de string, fornecendo apenas o codigo
void build_ok_msg(char *msg_out, unsigned codigo){
    
    //parse int->str
    char *code_aux = malloc(sizeof(3));
    sprintf(code_aux, " %02u", codigo);
    
    strcat(msg_out, "OK ");
    strcat(msg_out, code_aux);
    strcat(msg_out, "\n");

    free(code_aux);
}

// Transforma uma [control_msg] em uma [string] no formato TYPE CODE 
void control_msg2string(char *str_out, struct control_msg *msg_in){

    char *code_aux = malloc(sizeof(msg_in->code));
    sprintf(code_aux, " %u", msg_in->code);

    strcat(str_out, msg_in->type);
    strcat(str_out, code_aux);

    free(code_aux);
}


// Transforma uma [request_msg] em uma [string] no formato TYPE LOC_ID DEV_ID VALUES 
// Primeiro transforma cada parte da mensagem em string e depois concatena tudo 
void msg2string(char *str_out, struct request_msg *msg_in){

    //[ERRO] - Estava dando segmentation fault por que eu nao estava alocando memoria pras strings auxiliares, depois de usar malloc deu certo
    //[ERRO] - Tava retornando uma string antes, mas nao tinha como desalocar a memoria da string de retorno [vide commit 1.0], entao eu passei a string como 
    //parametro pra resolver, e aloquei localmente onde a string com a mensagem ia ser usada
    
    char *dev_id_aux = malloc(sizeof(msg_in->dev_id));
    char *loc_id_aux = malloc(sizeof(msg_in->dev_id));
    char *values_aux = malloc(sizeof(msg_in->dev_id));
    
    sprintf(dev_id_aux, " %d", msg_in->dev_id);
    sprintf(loc_id_aux, " %d", msg_in->local_id);

    //sizeof(msg_in->dev_state)/sizeof(int) eh a quantidade de elementos do vetor, que tem tamanho variavel
    for(int i = 0; i < sizeof(msg_in->dev_state)/sizeof(int); i++){
        char *aux = malloc(3);
        sprintf(aux, "% d", msg_in->dev_state[i]);
        strcat(values_aux, aux); 
        free(aux);
    }

    strcat(str_out, "request_msg> ");
    strcat(str_out, msg_in->type);
    strcat(str_out, dev_id_aux);
    strcat(str_out, loc_id_aux);
    strcat(str_out, values_aux);

    free(dev_id_aux);
    free(loc_id_aux);
    free(values_aux);
}

// Transforma uma [string] no formato TYPE LOC_ID DEV_ID VALUES em uma [request_msg] 
// O vetor tem de valores tem de ser alocado dinamicamente e depois desalocado
void string2msg(char *str_in, struct request_msg *msg_out){

    // A funcao strtok eh usada pra cortar a string pedaco por pedaco de acordo com o " ", e salva o pedaco atual na variavel token
    // Na primeira chamada passamos a string a ser cortada e depois passamos NULL
    char *token = strtok(str_in, " ");
    msg_out->type = token;      // pega o tipo
    
    token = strtok(NULL, " ");
    msg_out->local_id = atoi(token);   // pega o local_id, atoi eh uma funcao parse de string pra inteiro

    token = strtok(NULL, " ");
    msg_out->dev_id =  atoi(token);   // pega o dev_id

    // loop pra pegar cada valor do vetor dev_state, que tem tamanho variavel, e construi-lo
    int i = 0;
    token = strtok(NULL, " ");
    while(token != NULL){
                
        msg_out->dev_state = realloc(msg_out->dev_state, i * sizeof(int));   //aloca memoria pro proximo elemento do vetor
        msg_out->dev_state[i] = atoi(token);                //atribui valor ao proximo elemento do vetor

        token = strtok(NULL, " ");              //corta a string novamente
        i++;
    }
}

struct reqres_msg{
    char *type;
    int *info;
};

//Funcao pra inicializar mensagem de reqres (cliente -> servidor)
void build_reqres_msg(struct reqres_msg *msg_out, char *tipo, int *info_vec){
    msg_out->type = tipo;
    msg_out->info = info_vec;

    printf("[log] Request_msg built\n");
}

// Transforma uma [reqres_msg] em uma [string] no formato TYPE INFO[0] INFO[1] ...
// Aloca a string da mensagem final dinamicamente
void reqres_msg2string(char *str_out, struct reqres_msg *msg_in){

    //comeca a construir a string pelo tipo
    //o reallloc pode mandar pra uma posicao nova, mas retorna o ponteiro para a nova memoria alocada
    str_out = realloc(str_out, strlen(msg_in->type));
    strcat(str_out, msg_in->type); 

    //parse int[] -> string
    //sizeof(msg_in->dev_state)/sizeof(int) eh a quantidade de elementos do vetor, que tem tamanho variavel
    for(int i = 0; i < sizeof(msg_in->info)/sizeof(int); i++){
        char *aux = malloc(STR_MIN);
        sprintf(aux, " %d", msg_in->info[i]); //parse int -> str
        str_out = realloc(str_out, strlen(aux));
        strcat(str_out, aux); 
        free(aux);
    }

    //as mensagens devem terminar em \n
    str_out = realloc(str_out, strlen("\n"));
    strcat(str_out, "\n"); 
}


int main(){
    // --- TESTES --- // 
    printf("Teste do reqres_msg2string\n");

    char *tipo = "INS_REQ";
    int vec_aux[2] = {1, 2};

    struct reqres_msg *msg_teste = malloc(MSGSZ);
    build_reqres_msg(msg_teste, tipo, vec_aux);

    char *str_out = malloc(0);
    reqres_msg2string(str_out, msg_teste);
    printf("%s\n", str_out);
    
    return 0;
}
