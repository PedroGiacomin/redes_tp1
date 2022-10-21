#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "common.h"

#define BUFSZ 1024
#define MSGSZ 1024

//Pra testes
//Terminal 1: ./server v4 5151
//Terminal 2: ./client 127.0.0.1 5151

void usage(){
    printf("Chamada correta: ./client <server IP> <port>\n");
    exit(EXIT_FAILURE);
}

//argv[1] -> IP do servidor
//argv[2] -> porta do processo
int main(int argc, char **argv){
    // Garantia de que o programa foi inicializado corretamente
    if(argc < 3){
        usage();
    }
    
    // ------------- CONEXAO COM SERVIDOR ------------- //

    //--- DEFINICAO DE IP E PORTA ---//
    // sockaddr_storage eh a estrutura de dados do POSIX que permite guardar IPv4 e IPv6, e tambem guarda a porta
    // A funcao addrparse vai salvar a o IP e a porta (que sao recebidas como string) na variavel storage
    // A condicional eh so pra garantir que nao teve erro no parse
    struct sockaddr_storage storage;
    if(0 != addrparse(argv[1], argv[2], &storage)){
        usage();
    }

    //--- INICIALIZACAO DO SOCKET ---//
    //Tanto IPv4 quanto IPv6 a partir do endereco que estah salvo em storage
    //O argumento ss_family guarda a constante AF_INET (v4) ou AF_INET6 (v6)
    int s = socket(storage.ss_family, SOCK_STREAM, 0);
    if(s == -1){
        logexit("erro ao incializar socket");
    }

    //--- ESTABELECENDO CONEXAO COM SERVIDOR --//
    //Salva o endereco de storage em addr, que eh do tipo correto (addr_storage nao eh suportado)
    //Usa a funcao connect passando o
    //      - socket local (s) 
    //      - porta e IP do servidor de destino (addr)
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if(connect(s, addr, sizeof(storage)) != 0){
        logexit("erro ao conectar com servidor");
    }

    //Transforma addr (que tem IP e porta) de volta pra string e printa
    char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

    printf("conectado a %s\n", addrstr);

    // ------------- TROCA DE MENSAGENS ------------- //
    //CLIENT 2.0 - funcionamento: 
    // - passa uma mensagem definida aqui no codigo para o servidor como a struct [request_msg]
    // - depois espera uma resposta do servidor e imprime na tela
    // - encerra o programa
    
    // --- BUFFER --- // 
    //Buffer que vai guardar o conteudo recebido do teclado
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // --- CRIACAO DA MENSAGEM (request) --- // 
    struct request_msg *msg = malloc(MSGSZ); //guarda a msg
    char *tipo = "INS_REQ"; //guarda o tipo
    int valores[2] = {1, 2}; //guarda os valores
    build_request_msg(msg, tipo, 1, 1, valores); //constroi a msg

    // --- ENVIO DA MENSAGEM (request) --- // 
    //Envia a mensagem pro socket em formato de [request_msg], retorna o numero de bytes enviado
    //O segundo argumento de send eh um const void *, que recebe tipos quaisquer, mas tem que ser castado pra ser lido
    int count = send(s, msg, sizeof(msg) + 1, 0);
    if(count != sizeof(msg) + 1){
        logexit("erro ao enviar mensagem pro socket");
    }

    // --- DEESTRUICAO DA MENSAGEM ---//
    free(msg); //desaloca o espaco da msg

    // --- RECEBIMENTO DA MENSAGEM DO SERVER (response) --- //
    //Aguarda chegar mensagem do servidor no socket em formato de string e salva no buffer
    unsigned total = 0;
    while(1){
        //O recv salva o que é recebido byte a byte o e retorna o numero de bytes recebido
        count = recv(s, buf + total, BUFSZ - total, 0);
        if(count == 0){
            //Se nao receber nada, significa que a conexão foi fechada
            break;
        }
        //Desloca o buffer pra receber o proximo byte
        total += count;
    }

    //Fecha o socket ao receber a mensagem toda
    close(s);

    // Imprime a mensagem na tela
    printf("%u received bytes\n", total);
    puts(buf);

    //Termina o programa
    exit(EXIT_SUCCESS);
}