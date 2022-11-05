#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "common.h"

#define BUFSZ 1024
#define MSGSZ 500
#define STR_MIN 8

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
//
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

// Transformar um comando em uma mensagem [string] no formato pronto pra enviar pro servidor
    // A funcao strtok eh usada pra cortar a string pedaco por pedaco de acordo com o " "
    // token guarda a palavra do comando que estah sendo processada no momento
    // retorna 0 se tiver algum erro, 1 se tiver tudo bem
    // testa erros em todas as etapas
    // Pra saber se os inteiros digitados estao certos, testa se atoi == 0. Pra saber se as palavras digitadas estao erradas, testa se strcmp != 0
    // No comando, o dev_id vem antes do loc_id, mas na mensagem vem o loc_id antes do dev_id. Entao tem que inverter a ordem ao converter.
unsigned process_command(char *comando, char *msg_out){
    
    //Inicialmente, token tem a primeira palavra do comando, que eh a "identificacao" dele
    char *token = strtok(comando, " ");
    
    //caso INS_REQ
    if(!strcmp(token, "install")){
        
        strcpy(msg_out, "INS_REQ"); 

        token = strtok(NULL, " "); //token = devId
        if(strcmp(token, "0") && !atoi(token))
            return 0; 
            //primeiro testa se o valor eh 0, se nao for, testa se eh um inteiro (esse problema surge porque atoi
            //retorna 0 se o valor nao for um inteiro, mas nao faz distincao entre o que eh um int 0 e um nao int)

        char hold[BUFSZ];
        strcpy(hold, token);   //guarda o token = devId

        token = strtok(NULL, " "); //token = in
        if(strcmp(token, "in"))
            return 0;

        token = strtok(NULL, ": "); //token = locId:
        if(strcmp(token, "0") && !atoi(token))
            return 0;

        strcat(msg_out, " ");
        strcat(msg_out, token);

        strcat(msg_out, " ");
        strcat(msg_out, hold);  //insere o devId depois do locId

        token = strtok(NULL, " "); //token = value1
        if(strcmp(token, "0") && !atoi(token))
            return 0;
       
        strcat(msg_out, " ");
        strcat(msg_out, token);

        token = strtok(NULL, " "); //token = value2
        if(strcmp(token, "0") && !atoi(token)){
            return 0;
        }
       
        strcat(msg_out, " ");
        strcat(msg_out, token);

        strcat(msg_out, "\n");

        //soh suporta dispositivos com 2 valores 
    }
    
    //caso REM_REQ
    else if(!strcmp(token, "remove")){
    
        strcat(msg_out, "REM_REQ"); 

        token = strtok(NULL, " "); //token = devId
        if(strcmp(token, "0") && !atoi(token))
            return 0; //teste se o valor eh um inteiro
        
        char *hold = malloc(strlen(token));
        strcpy(hold, token);   //guarda o token = devId

        token = strtok(NULL, " "); //token = in
        if(strcmp(token, "in"))
            return 0;
        
        token = strtok(NULL, " "); //token = locId:
        if(strcmp(token, "0") && !atoi(token))
            return 0;
        
        strcat(msg_out, " ");
        strcat(msg_out, token);

        strcat(msg_out, " ");
        strcat(msg_out, hold);  //insere o devId depois do locId
    
        strcat(msg_out, "\n");

        free(hold);
    }

    //caso CH_REQ
    else if(!strcmp(token, "change")){

        strcat(msg_out, "CH_REQ"); 

        token = strtok(NULL, " "); //token = devId
        if(strcmp(token, "0") && !atoi(token))
            return 0; //teste se o valor eh um inteiro

        char *hold = malloc(strlen(token));
        strcpy(hold, token);   //guarda o token = devId

        token = strtok(NULL, " "); //token = in
        if(strcmp(token, "in"))
            return 0;
        
        token = strtok(NULL, ": "); //token = locId:
        if(strcmp(token, "0") && !atoi(token))
            return 0;
        
        strcat(msg_out, " ");
        strcat(msg_out, token);

        strcat(msg_out, " ");
        strcat(msg_out, hold);  //insere o devId depois do locId

        token = strtok(NULL, " "); //token = value1
        if(strcmp(token, "0") && !atoi(token))
            return 0;
        
        strcat(msg_out, " ");
        strcat(msg_out, token);

        token = strtok(NULL, " "); //token = value2
        if(strcmp(token, "0") && !atoi(token))
            return 0;
    
        strcat(msg_out, " ");
        strcat(msg_out, token);

        strcat(msg_out, "\n");

        //soh suporta dispositivos com 2 valores 
        free(hold);
    }

    //caso LOC_REQ || DEV_REQ
    else if(!strcmp(token, "show")){
        //nesse caso, a proxima palavra do comando tem que ser state
        token = strtok(NULL, " "); // token = state
        if(strcmp(token, "state")){
            return 0;
        }

        token = strtok(NULL, " "); // token = in || <devId>
        
        //caso LOC_REQ
        if(!strcmp(token, "in")){
            strcat(msg_out, "LOC_REQ");

            token = strtok(NULL, " "); //token = locId
            if(strcmp(token, "0") && !atoi(token))
                return 0; //teste se o valor eh um inteiro
            
            strcat(msg_out, " ");
            strcat(msg_out, token);

            strcat(msg_out, "\n");
        }
        
        //caso DEV_REQ
        else if(!strcmp(token, "0") || atoi(token)){        
            strcat(msg_out, "DEV_REQ");

            char *hold = malloc(strlen(token));
            strcpy(hold, token);   //guarda o token = devId

            token = strtok(NULL, " "); //token = in
            if(strcmp(token, "in"))
                return 0;

            token = strtok(NULL, " "); //token = locId
            if(strcmp(token, "0") && !atoi(token))
                return 0; //teste se o valor eh um inteiro

            strcat(msg_out, " ");
            strcat(msg_out, token);

            strcat(msg_out, " ");
            strcat(msg_out, hold); //insere o devId depois do locId

            strcat(msg_out, "\n");

            free(hold);
        }
        
        else{
            return 0;
        }
    }

    //caso erro
    else{
        return 0;
    }

    //free(hold);
    return 1;
}

//
void process_resmsg(char *msg_in, char *str_out){


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
    unsigned total = 0;
    while(1){
        // --- RECEBE O COMANDO --- // 
        //buf vai guardar o comando recebido do teclado
        char *buf = malloc(BUFSZ);
        memset(buf, 0, BUFSZ-1);
        fgets(buf, BUFSZ-1, stdin);
        buf = strtok(buf, "\n"); //desconsidera o enter que se da ao acabar de digitar o comando

        // --- RECEBE E CONSTROI A MENSAGEM --- //
        //process_command constroi a mensagem de requisicao em formato de string e a aloca dinamicamente. Retorna 0 se o comando tiver erro
        //msg_buf guarda a mensagem que vai ser enviada ao server
        char *msg_buf = malloc(MSGSZ);
        if(!process_command(buf, msg_buf))
            break;  //se receber mensagem com algum erro, sai do loop
        if(!strcmp(buf, "kill"))
            break;  //se receber o comando kill, sai do loop

        // --- ENVIA A MENSAGEM --- // 
        size_t count = send(s, msg_buf, strlen(msg_buf)+1, 0); //envia como string
        if (count != strlen(msg_buf)+1) {
            logexit("send");
        }

        // --- RECEBE MENSAGEM DO SERVER (response) --- //
        //Aguarda chegar mensagem do servidor no socket em formato de string e salva no buffer
        //O recv salva o que é recebido byte a byte o e retorna o numero de bytes recebido
        char *buf_res = malloc(BUFSZ);
        count = recv(s, buf_res + total, BUFSZ - total, 0);
        if(count == 0){
            //Se nao receber nada, significa que a conexão foi fechada
            break;
        }
        //Desloca o buffer pra receber o proximo byte
        total += count;

        // Imprime a mensagem na tela
        printf("%s", buf_res);
        
        // --- LIBERA A MEMORIA ---//
        free(buf);
        buf = NULL;
        free(msg_buf); //desaloca o espaco da msg
        msg_buf = NULL;
        free(buf_res);
        buf_res = NULL;

        total = 0;
    }
    //Ao sair do loop, fecha o socket e finaliza a conexao
    close(s);

    //Termina o programa
    exit(EXIT_SUCCESS);
}