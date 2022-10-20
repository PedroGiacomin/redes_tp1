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

//Funcao pra inicializar mensagem
void build_msg(struct mensagem *msg_out, int loc, int dev){
    msg_out->dev_id = dev;
    msg_out->local_id = loc;

    printf("Msg built\n");
}

// Transforma uma [mensagem] em uma [string] no formato <loc_id> <dev_id> 
void msg2string(char *str_out, struct mensagem *msg_in){

    //[ERRO] - Estava dando segmentation fault por que eu nao estava alocando memoria pras strings auxiliares, depois de usar malloc deu certo
    //[ERRO] - Tava retornando uma string antes, mas nao tinha como desalocar a memoria da string de retorno [vide commit 1.0], entao eu passei a string como 
    //parametro pra resolver, e aloquei localmente onde a string com a mensagem ia ser usada (O MESMO OCORREU PRA MENSAGENS)
    char *dev_id_aux = malloc(sizeof(msg_in->dev_id));
    char *loc_id_aux = malloc(sizeof(msg_in->dev_id));

    
    sprintf(dev_id_aux, "<dev_id = %d>", msg_in->dev_id);
    sprintf(loc_id_aux, "<loc_id = %d>", msg_in->local_id);

    strcat(str_out, "mensagem> ");
    strcat(str_out, dev_id_aux);
    strcat(str_out, loc_id_aux);

    free(dev_id_aux);
    free(loc_id_aux);
}

int main(){
    // --- TESTES --- // 
    printf("Teste do msg2string\n");

    //Aloca 4B, que eh mais que suficiente para a mensagem a ser enviada 
    struct mensagem *msg_teste = malloc(1024);
    build_msg(msg_teste, 1, 1);
    
    //Aloca 4B, que eh mais que suficiente para a mensagem a ser enviada 
    char *str_out = malloc(1024);
    msg2string(str_out, msg_teste);
    puts(str_out);

    free(str_out);
    free(msg_teste);

    return 0;
}