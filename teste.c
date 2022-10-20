#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

//Estrutura que guarda uma mensagem para o servidor
struct mensagem{
    //char *tipo;
    int local_id; 
    int dev_id;
    //int dev_state[10];
};

struct mensagem build_msg(int loc, int dev){
    struct mensagem msg_out;
    msg_out.dev_id = dev;
    msg_out.local_id = loc;

    printf("Msg built\n");

    return msg_out;
}

// Transforma uma [mensagem] em uma [string]
char *msg2string(struct mensagem msg_in){

    //[ERRO] - Estava dando segmentation fault por que eu nao estava alocando memoria pras strings auxiliares, depois de usar malloc deu certo
    //Aloca 4B, que eh mais que suficiente para a mensagem a ser enviada 
    char *str_out = malloc(1024);
    char *dev_id_aux = malloc(sizeof(msg_in.dev_id));
    char *loc_id_aux = malloc(sizeof(msg_in.dev_id));


    sprintf(dev_id_aux, "<dev_id = %d>", msg_in.dev_id);
    sprintf(loc_id_aux, "<loc_id = %d>", msg_in.local_id);

    strcat(str_out, "mensagem> ");
    strcat(str_out, dev_id_aux);
    strcat(str_out, loc_id_aux);

    //COMO DESALOCAR STR_OUT???
    free(dev_id_aux);
    free(loc_id_aux);

    return  str_out;
}

int main(){
    // --- TESTES --- // 
    printf("Teste do msg2string\n");
    struct mensagem msg_teste = build_msg(1,1);
    char *str_out = msg2string(msg_teste);
    puts(str_out);

    return 0;
}