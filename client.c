#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024

void usage(){
    printf("./ client <server IP> <port>");
    exit(EXIT_FAILURE);
}

void logexit(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

//argv[1] -> IP do servidor
//argv[2] -> porta do processo
void main(int argc, int **argv){
    // Garantia de que o programa foi inicializado corretamente
    if(argc < 3){
        usage();
    }

    //Inicializacao do socket
    int s_v4 = socket(AF_INET, SOCK_STREAM, 0);
    if(s_v4 == -1){
        logexit("socket IPv4");
    }

    //Inicia a conexao com o servidor passa o socket e o IP do servidor
    if(connect(s_v4, server_addr, sizeof(server_addr)) != 0){
        logexit("connect to server");
    }

    //--- client1.0: recebe uma mensagem do terminal, salva em buf, e envia pro servidor--//
    
    //String que vai guardar o conteudo da mensagem inicialmente
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    //Pega mensagem do teclado e salva no buffer
    printf("mensagem> ");
    fgets(buf, BUFSZ - 1, stdin);

    //Envia a mensagem pro socket, retorna o numero de bytes enviado
    int count = send(s_v4, buf, strlen(buf) + 1, 0);
    if(count != strlen(buf) + 1){
        logexit("send data to socket");
    }

    memset(buf, 0, BUFSZ);
    unsigned total = 0;
    
    //Aguarda chegar mensagem no socket e salva no buffer
    while(1){
        //O recv salva o que é recebido byte a byte o numero de bytes recebido
        count = recv(s_v4, buf + total, BUFSZ - total, 0);
        if(count == 0){
            //Se nao receber nada, significa que a conexão foi fechada
            break;
        }
        //Desloca o buffer pra receber o proximo byte
        total += count;
    }

    //Fecha o socket IPv4
    close(s_v4);

    printf("%u received bytes", total);
    puts(buf);

    //Termina o programa
    exit(EXIT_SUCCESS);

}