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
void msg2string(char *str_out, struct mensagem msg_in){

    //[ERRO] - Estava dando segmentation fault por que eu nao estava alocando memoria pras strings auxiliares, depois de usar malloc deu certo
    //[ERRO] - Tava retornando uma string antes, mas nao tinha como desalocar a memoria da string de retorno [vide commit 1.0], entao eu passei a string como 
    //parametro pra resolver, e aloquei localmente onde a string com a mensagem ia ser usada
    char *dev_id_aux = malloc(sizeof(msg_in.dev_id));
    char *loc_id_aux = malloc(sizeof(msg_in.dev_id));


    sprintf(dev_id_aux, "<dev_id = %d>", msg_in.dev_id);
    sprintf(loc_id_aux, "<loc_id = %d>", msg_in.local_id);

    strcat(str_out, "mensagem> ");
    strcat(str_out, dev_id_aux);
    strcat(str_out, loc_id_aux);

    free(dev_id_aux);
    free(loc_id_aux);
}

int main(){
    // --- TESTES --- // 
    printf("Teste do msg2string\n");
    struct mensagem msg_teste = build_msg(1,1);
    
    //Aloca 4B, que eh mais que suficiente para a mensagem a ser enviada 
    char *str_out = malloc(1024);
    msg2string(str_out, msg_teste);
    puts(str_out);

    return 0;
}