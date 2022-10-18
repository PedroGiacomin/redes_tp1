#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "common.h"

#define BUFSZ 1024

void usage(){
    printf("./client <server IP> <port>");
    exit(EXIT_FAILURE);
}

//argv[1] -> IP do servidor
//argv[2] -> porta do processo
void main(int argc, int **argv){
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

    printf("connected to %s\n", addrstr);

    // ------------- TROCA DE MENSAGENS ------------- //
    //CLIENT 1.0 - funcionamento: 
    // - pega uma string do teclado e envia para o servidor
    // - depois espera uma resposta do servidor e imprime na tela
    // - encerra o programa
    
    // --- BUFFER --- // 
    //Buffer que vai guardar o conteudo recebido do teclado
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // --- CRIACAO DA MENSAGEM (request) --- // 
    //Pega conteudo do teclado e salva no buffer 
    //(num primeiro momento a mensagem eh so esse conteudo)
    printf("mensagem> ");
    fgets(buf, BUFSZ - 1, stdin);

    // --- ENVIO DA MENSAGEM (request) --- // 
    //Envia a mensagem pro socket, retorna o numero de bytes enviado
    int count = send(s, buf, strlen(buf) + 1, 0);
    if(count != strlen(buf) + 1){
        logexit("erro ao enviar mensagem pro socket");
    }
    //reseta o buffer
    memset(buf, 0, BUFSZ);

    // --- RECEBIMENTO DA MENSAGEM DO SERVER (response) --- //
    //Aguarda chegar mensagem do servidor no socket e salva no buffer
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
    printf("%u received bytes", total);
    puts(buf);

    //Termina o programa
    exit(EXIT_SUCCESS);
}