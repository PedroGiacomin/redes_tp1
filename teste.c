#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MSGSZ 1024
#define MAX_STATE_VALUES 2

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
void string2msg(char *str_in, struct request_msg *msg_out){
    
}

int main(){
    // --- TESTES --- // 
    printf("Teste do string2msg\n");

    
    char palavra[50] = {"REQ 13 90 12 22"};
    
    //Sao as variaveis que vao guardar os valores dos parametros da mensagem lida da string, para construi-la depois
    char *buf_tipo;
    int buf_loc_id;
    int buf_dev_id;
    int *buf_values;

    // A funcao strtok eh usada pra cortar a string pedaco por pedaco de acordo com o " ", e salva o pedaco atual na variavel token
    // Na primeira chamada passamos a string a ser cortada e depois passamos NULL
    char *token = strtok(palavra, " ");
    buf_tipo = token;      // pega o tipo
    
    token = strtok(NULL, " ");
    buf_loc_id = atoi(token);   // pega o loc_id, atoi eh uma funcao parse de string pra inteiro

    token = strtok(NULL, " ");
    buf_dev_id =  atoi(token);   // pega o dev_id

    // loop pra pegar cada valor do vetor dev_state, que tem tamanho variavel, e construi-lo
    int i = 0;
    token = strtok(NULL, " ");
    while(token != NULL){
                
        buf_values = realloc(buf_values, i * sizeof(int));   //aloca memoria pro proximo elemento do vetor
        buf_values[i] = atoi(token);            //atribui valor ao proximo elemento do vetor

        token = strtok(NULL, " ");              //corta a string novamente
        i++;
    }

    printf("buf_tipo : %s\n", buf_tipo);
    printf("buf_loc_id : %d\n", buf_loc_id);
    printf("buf_dev_id : %d\n", buf_dev_id);
    for(int i = 0; i < sizeof(buf_values)/sizeof(int); i++){
        printf("buf_values[%d] = %d\n", i, buf_values[i]);
    }


    free(buf_values);   //libera

    //Aloca 4B, que eh mais que suficiente para a mensagem a ser enviada 
    // struct request_msg *msg_teste = malloc(MSGSZ);
    // char *tipo = "INS_REQ"; 
    // int *vec;
    // int vec_aux[2] = {2, 3};
    // vec = vec_aux;
    // char *str_out = malloc(MSGSZ);

    //build_msg(msg_teste, tipo, 13, 90, vec);

    // msg2string(str_out, msg_teste);

    // puts(str_out);
    
    // free(msg_teste);
    // free(str_out);

    return 0;
}
