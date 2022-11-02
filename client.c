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

// --- LOGICA DA INTERFACE --- //
// - A interface do cliente vai pegar um COMANDO do teclado e transformar numa mensagem de requisicao, segundo:
//  <devId>, <locId> e <valuei> sao int 
//  MENSAGEM     COMANDO      
//  INS_REQ      install <devId> in <locId>: <value1> <value2>
//  REM_REQ      remove <devId> in <locId>    
//  CH_REQ       change <devId> in <locId>: <value1> <value2>
//  DEV_REQ      show state <devId> in <locId>
//  LOC_REQ      show state in <locId>
// - Percebe-se que todos os comandos tem uma identificacao inicial: install, remove, change ou show.
// - Precisara ser feita uma funcao que interpreta o comando de uma forma diferente para cada identificacao inicial.
// - No caso do show, ainda, eh necessario uma condicional para saber se eh uma consulta de device ou de local.
// - Todas as palavras do comando devem ser testadas, pois um comando invalido gera uma desconexao do servidor


// A funcao strtok eh usada pra cortar a string pedaco por pedaco de acordo com o " "
// token guarda a palavra do comando que estah sendo processada no momento
// retorna 0 se tiver algum erro, 1 se tiver tudo bem
// testa erros em todas as etapas
// Pra saber se os inteiros digitados estao certos, testa se atoi == 0. Pra saber se as palavras digitadas estao erradas, testa se strcmp != 0

// Transformar um comando em uma mensagem [string] no formato pronto pra enviar pro servidor
// A string que vai ser enviada eh alocada dinamicamente
unsigned process_command(char *comando, char *msg_out){
    
    //Inicialmente, token tem a primeira palavra do comando, que eh a "identificacao" dele
    char *token = strtok(comando, " ");
    if(!strcmp(token, "install")){
        msg_out = realloc(msg_out, sizeof(msg_out) + strlen("INS_REQ"));
        strcat(msg_out, "INS_REQ"); 

        token = strtok(NULL, " "); //token = devId
        if(strcmp(token, "0") && !atoi(token))
            return 0; 
            //primeiro testa se o valor eh 0, se nao for, testa se eh um inteiro (esse problema surge porque atoi
            //retorna 0 se o valor nao for um inteiro, mas nao faz distincao entre o que eh um int 0 e um nao int)

        msg_out = realloc(msg_out, sizeof(msg_out) + strlen(token));
        strcat(msg_out, " ");
        strcat(msg_out, token);

        token = strtok(NULL, " "); //token = in
        if(strcmp(token, "in"))
            return 0;
        
        token = strtok(NULL, ": "); //token = locId:
        if(strcmp(token, "0") && !atoi(token))
            return 0;
        msg_out = realloc(msg_out, sizeof(msg_out) + strlen(token));
        strcat(msg_out, " ");
        strcat(msg_out, token);

        token = strtok(NULL, " "); //token = value1
        if(strcmp(token, "0") && !atoi(token))
            return 0;
        msg_out = realloc(msg_out, sizeof(msg_out) + strlen(token));
        strcat(msg_out, " ");
        strcat(msg_out, token);

        token = strtok(NULL, " "); //token = value2
        if(strcmp(token, "0") && !atoi(token)){
            return 0;
        }
        msg_out = realloc(msg_out, sizeof(msg_out) + strlen(token));
        strcat(msg_out, " ");
        strcat(msg_out, token);

        msg_out = realloc(msg_out, sizeof(msg_out) + strlen("\n"));
        strcat(msg_out, "\n");

        //soh suporta dispositivos com 2 valores 
    }
    
    else if(!strcmp(token, "remove")){
        msg_out = realloc(msg_out, sizeof(msg_out) + strlen("REM_REQ"));
        strcat(msg_out, "REM_REQ"); 

        token = strtok(NULL, " "); //token = devId
        if(strcmp(token, "0") && !atoi(token))
            return 0; //teste se o valor eh um inteiro
        msg_out = realloc(msg_out, sizeof(msg_out) + strlen(token));
        strcat(msg_out, " ");
        strcat(msg_out, token);

        token = strtok(NULL, " "); //token = in
        if(strcmp(token, "in"))
            return 0;
        
        token = strtok(NULL, " "); //token = locId:
        if(strcmp(token, "0") && !atoi(token))
            return 0;
        msg_out = realloc(msg_out, sizeof(msg_out) + strlen(token));
        strcat(msg_out, " ");
        strcat(msg_out, token);

        msg_out = realloc(msg_out, sizeof(msg_out) + strlen("\n"));
        strcat(msg_out, "\n");
    }

    else if(!strcmp(token, "change")){
        msg_out = realloc(msg_out, sizeof(msg_out) + strlen("CH_REQ"));
        strcat(msg_out, "CH_REQ"); 

        token = strtok(NULL, " "); //token = devId
        if(strcmp(token, "0") && !atoi(token))
            return 0; //teste se o valor eh um inteiro

        msg_out = realloc(msg_out, sizeof(msg_out) + strlen(token));
        strcat(msg_out, " ");
        strcat(msg_out, token);

        token = strtok(NULL, " "); //token = in
        if(strcmp(token, "in"))
            return 0;
        
        token = strtok(NULL, ": "); //token = locId:
        if(strcmp(token, "0") && !atoi(token))
            return 0;
        msg_out = realloc(msg_out, sizeof(msg_out) + strlen(token));
        strcat(msg_out, " ");
        strcat(msg_out, token);

        token = strtok(NULL, " "); //token = value1
        if(strcmp(token, "0") && !atoi(token))
            return 0;
        msg_out = realloc(msg_out, sizeof(msg_out) + strlen(token));
        strcat(msg_out, " ");
        strcat(msg_out, token);

        token = strtok(NULL, " "); //token = value2
        if(strcmp(token, "0") && !atoi(token))
            return 0;
        msg_out = realloc(msg_out, sizeof(msg_out) + strlen(token));
        strcat(msg_out, " ");
        strcat(msg_out, token);

        msg_out = realloc(msg_out, sizeof(msg_out) + strlen("\n"));
        strcat(msg_out, "\n");

        //soh suporta dispositivos com 2 valores 
    }

    else if(!strcmp(token, "show")){
        //nesse caso, a proxima palavra do comando tem que ser state
        token = strtok(NULL, " "); // token = state
        if(strcmp(token, "state")){
            return 0;
        }

        token = strtok(NULL, " "); // token = in || <devId>
        if(!strcmp(token, "in")){
            //nesse caso eh uma LOC_REQ
            msg_out = realloc(msg_out, sizeof(msg_out) + strlen("LOC_REQ"));
            strcat(msg_out, "LOC_REQ");

            token = strtok(NULL, " "); //token = locId
            if(strcmp(token, "0") && !atoi(token))
                return 0; //teste se o valor eh um inteiro
            msg_out = realloc(msg_out, sizeof(msg_out) + strlen(token));
            strcat(msg_out, " ");
            strcat(msg_out, token);

            msg_out = realloc(msg_out, sizeof(msg_out) + strlen("\n"));
            strcat(msg_out, "\n");
        }
        else if(!strcmp(token, "0") || atoi(token)){
            //nesse caso eh uma DEV_REQ
            msg_out = realloc(msg_out, sizeof(msg_out) + strlen("DEV_REQ"));
            strcat(msg_out, "DEV_REQ");

            msg_out = realloc(msg_out, sizeof(msg_out) + strlen(token));
            strcat(msg_out, " ");
            strcat(msg_out, token);

            token = strtok(NULL, " "); //token = in
            if(strcmp(token, "in"))
                return 0;
        
            token = strtok(NULL, " "); //token = locId
            if(strcmp(token, "0") && !atoi(token))
                return 0; //teste se o valor eh um inteiro
            msg_out = realloc(msg_out, sizeof(msg_out) + strlen(token));
            strcat(msg_out, " ");
            strcat(msg_out, token);

            msg_out = realloc(msg_out, sizeof(msg_out) + strlen("\n"));
            strcat(msg_out, "\n");
        }
        else{
            return 0;
        }
    }

    else{
        return 0;
    }

    return 1;
}

// - A interface vai receber uma mensagem de RESPOSTA do servidor e imprimir um aviso na tela segundo:
// MENSAGEM     PRINT
// ERROR 01     device not installed
// ERROR 02     no devices
// ERROR 03     invalid device
// ERROR 04     invalid local
// OK 01        successful installation
// OK 02        successful removal
// OK 03        successful change
// DEV_RES      device <devId> in ComId: <valor1> <valor>
// LOC_RES      local <locId>: <devId> <valor1> <valor2> <devId2> <valor1> <valor2>

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
    
    // --- RECEBE O COMANDO --- // 
    //Buffer que vai guardar o comando recebido do teclado
    char *buf = malloc(BUFSZ);
	fgets(buf, BUFSZ-1, stdin);
    buf = strtok(buf, "\n"); //desconsidera o enter que se da ao acabar de digitar o comando

    // --- CONSTROI A MENSAGEM --- //
    //process_command constroi a mensagem de requisicao em formato de string e a aloca dinamicamente. Retorna 0 se o comando tiver erro
    char *msg_buf = malloc(0);
    printf("ACERTO = %d\n", process_command(buf, msg_buf));

    // --- ENVIA A MENSAGEM --- // 
	size_t count = send(s, msg_buf, strlen(msg_buf)+1, 0); //envia como string
	if (count != strlen(msg_buf)+1) {
		logexit("send");
	}

    // --- LIBERA A MEMORIA ---//
    free(buf);
    free(msg_buf); //desaloca o espaco da msg

    // --- RECEBE MENSAGEM DO SERVER (response) --- //
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