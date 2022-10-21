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
void msg2string(char *str_out, struct request_msg *msg_in){

    //[ERRO] - Estava dando segmentation fault por que eu nao estava alocando memoria pras strings auxiliares, depois de usar malloc deu certo
    //[ERRO] - Tava retornando uma string antes, mas nao tinha como desalocar a memoria da string de retorno [vide commit 1.0], entao eu passei a string como 
    //parametro pra resolver, e aloquei localmente onde a string com a mensagem ia ser usada
    
    //rever o tamanho desses maloc aq
    char *dev_id_aux = malloc(sizeof(msg_in->dev_id));
    char *loc_id_aux = malloc(sizeof(msg_in->dev_id));
    char *values_aux = malloc(sizeof(msg_in->dev_id));
    
    sprintf(dev_id_aux, " %d", msg_in->dev_id);
    sprintf(loc_id_aux, " %d", msg_in->local_id);

    //Transforma vetor em string, sizeof(msg_in->dev_state)/sizeof(int) eh a quantidade de elementos do vetor, que tem tamanho variavel
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

int main(){
    // --- TESTES --- // 
    printf("Teste do build_msg\n");

    //Aloca 4B, que eh mais que suficiente para a mensagem a ser enviada 
    struct request_msg *msg_teste = malloc(MSGSZ);
    char *tipo = "INS_REQ"; 
    int *vec;
    int vec_aux[2] = {2, 3};
    vec = vec_aux;
    char *str_out = malloc(MSGSZ);

    build_msg(msg_teste, tipo, 13, 90, vec);

    msg2string(str_out, msg_teste);

    puts(str_out);
    
    free(msg_teste);
    free(str_out);

    return 0;
}