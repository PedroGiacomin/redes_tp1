#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>


#define BUFSZ 1024

void usage(){
    printf("Chamada correta: ./server <v4/v6> <port>\n");
    exit(EXIT_FAILURE);
}

//argv[1] -> familia IP
//argv[2] -> porta do processo
int main(int argc, char **argv){
    // Garantia de que o programa foi inicializado corretamente
    if(argc < 3){
        usage();
    }

    // ------------- CONEXAO COM CLIENTE ------------- //
    // --- DEFINICAO DE v4|v6 E PORTA --- //
    // Salva a familia IP e a porta na variavel storage que eh do tipo POSIX certo pra guardar endereco
    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage();
    }

    // --- INICIALIZACAO DO SOCKET --- //
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("erro ao inicializar socket");
    }

    //Essa eh apenas uma opcao pra reutilizar a mesma porta em duas execucoes consecutivas sem ter que esperar
    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    // Salva o endereco de storage em addr, que eh do tipo correto (addr_storage nao eh suportado)
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    
    // O bind associa('liga') o socket a uma porta do SO 
    // Recebe o socket e o addr (v4|v6 + porta)
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("erro no bind");
    }

    // --- ESPERAR CONEXAO DO CLIENTE --- //
    // O listen indica que o socket passado eh passivo, i.e., que vai esperar requisicao de conexao 
    // Recebe o socket e o maximo de conexoes pendentes possiveis para aquele socket
    if (0 != listen(s, 10)) {
        logexit("erro no listen");
    }
    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("ouvindo na porta %s\n", addrstr);

    // Aceita e trata conexoes eternamente
    while (1) {
        // --- ACEITAR CONEXAO DO CLIENTE --- //
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        // A funcao accept aceita a conexao pelo socket s e cria OUTRO socket pra comunicar com o cliente 
        // Recebe o socket de conexao, caddr pra salvar o endereco do cliente e caddrlen pro tamanho desse endereco
        int client_sock = accept(s, caddr, &caddrlen);
        if (client_sock == -1) {
            logexit("erro ao conectar com o cliente");
        }

        // So transforma o endereco de sockaddr pra string
        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

        // ------------- TROCA DE MENSAGENS ------------- //
        //SERVER 1.0 - funcionamento: 
        // - recebe mensagem de requisicao do cliente e salva no buffer
        // - envia o endereco do cliente de volta pra ele
        // - encerra conexao com o cliente e volta a esperar novas conexoes
        
        // --- BUFFER --- //
        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);

        // --- RECEBIMENTO DA MENSAGEM DO CLIENTE (request) --- //
        // Recebe mensagem por client_sock e salva no buffer (e printa no terminal o que foi recebido)
        size_t count = recv(client_sock, buf, BUFSZ - 1, 0);
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

        // --- CRIACAO DA MENSAGEM (response) --- //
        //Salva o proprio endereco do cliente no buffer
        //(num primeiro momento a mensagem eh so esse endereco)
        sprintf(buf, "Oi cliente <%.900s>, recebi sua mensagem\n", caddrstr);

        // --- ENVIO DA MENSAGEM (response) --- //
        count = send(client_sock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logexit("erro ao enviar mensagem de resposta");
        }

        // Encerra a conexao com aquele cliente
        close(client_sock);
    }

    exit(EXIT_SUCCESS);
}