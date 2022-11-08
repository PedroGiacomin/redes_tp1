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

// Transforma um comando em uma mensagem [string] no formato pronto pra enviar pro servidor
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
    
        strcpy(msg_out, "REM_REQ"); 

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

        strcpy(msg_out, "CH_REQ"); 

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
    //Para o cliente imprimir na tela a resposta a esses chamados, eles retornam 2.
    else if(!strcmp(token, "show")){
        //nesse caso, a proxima palavra do comando tem que ser state
        token = strtok(NULL, " "); // token = state
        if(strcmp(token, "state")){
            return 0;
        }

        token = strtok(NULL, " "); // token = in || <devId>
        
        //caso LOC_REQ
        if(!strcmp(token, "in")){
            strcpy(msg_out, "LOC_REQ");

            token = strtok(NULL, " "); //token = locId
            if(strcmp(token, "0") && !atoi(token))
                return 0; //teste se o valor eh um inteiro
            
            strcat(msg_out, " ");
            strcat(msg_out, token);

            strcat(msg_out, "\n");
        }
        
        //caso DEV_REQ
        else if(!strcmp(token, "0") || atoi(token)){        
            strcpy(msg_out, "DEV_REQ");

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

    return 1;
}

//Transforma uma mensagem OK, ERROR em um aviso no terminal do cliente
void process_res_msg(char *msg_in, char *str_out, char *req_msg){
    char *token = strtok(msg_in, " "); //token = type
    int type = parse_msg_type(token);
    unsigned code = 0;
    char *ligado_aux = malloc(STR_MIN);
    char *dado_aux = malloc(STR_MIN);
    char *loc_aux = malloc(STR_MIN);
    char *dev_aux = malloc(STR_MIN);
    int i = 0;
    int count = 0;

    switch (type){

        case ERROR:
            token = strtok(NULL, " "); //token = codigo do erro
            code = atoi(token);

            switch (code){
                case 1:
                    strcpy(str_out,"device not installed\n");
                break;

                case 2:
                    strcpy(str_out,"no devices\n");
                break;

                case 3:
                    strcpy(str_out,"invalid device\n");
                break;

                case 4:
                    strcpy(str_out,"invalid local\n");
                break;

                default:
                    
                break;
            }
        break;

        case OK:
            token = strtok(NULL, " "); //token = codigo do erro
            code = atoi(token);

            switch (code){

                case 1:
                    strcpy(str_out,"successful installation\n");
                break;

                case 2:
                    strcpy(str_out,"successful removal\n");
                break;

                case 3:
                    strcpy(str_out,"successful change\n");
                break;

                default:
                    
                break;
            }
        break;

        case DEV_RES:
            // Desmonta o DEV_RES
            token = strtok(NULL, " "); //token = ligado
            strcpy(ligado_aux, token);
            token = strtok(NULL, " "); //token = dado
            strcpy(dado_aux, token);

            // Desmonta o DEV_REQ
            // req_msg = DEV_REQ <locId> <devId>
            token = strtok(req_msg, " "); //token = DEV_REQ
            token = strtok(NULL, " "); //token = loc_id
            strcpy(loc_aux, token);

            token = strtok(NULL, " "); //token = dev_id
            strcpy(dev_aux, token);
            dev_aux = strtok(dev_aux, "\n");
            
            strcpy(str_out, "device ");
            strcat(str_out, dev_aux);
            strcat(str_out, " in ");
            strcat(str_out, loc_aux);
            strcat(str_out, ": ");
            strcat(str_out, ligado_aux);
            strcat(str_out, " ");
            dado_aux = strtok(dado_aux, "\n");
            strcat(str_out, dado_aux);
            strcat(str_out, "\n");

        break;

        case LOC_RES:
            // Desmonta o LOC_RES
            //LOC_RES <dev_1> <lig1> <dado1> <dev_2> <lig2> <dado2>\n
            //percorre toda a mensagem e conta quanto espacos ela tem, todo dispositivo tem 3 espacos
            while (msg_in[i] != '\n'){
                //Por algum motivo tem 2 valores pro espaco no ascii
                if(msg_in[i] ==  0 || msg_in[i] == 32){
                    count++;
                   
                }
                i++;
            }

            char aux[MSGSZ] = "";
            for(int j = 0; j < count/3; j++){
                token = strtok(NULL, " "); //token = dev[i]
                strcat(aux, token);
                strcat(aux, " (");

                token = strtok(NULL, " "); //token = ligado[i]
                strcat(aux, token);
                strcat(aux, " ");
                
                token = strtok(NULL, " "); //token = dado[i] || dado[i]\n
                if(j == (count/3) - 1){
                    token = strtok(token, "\n"); //se for o ultimo item da LOC_RES, tira o \n
                }
                strcat(aux, token);
                strcat(aux, ") ");
            }
            
            token = strtok(req_msg, " "); //token = LOC_REQ
            token = strtok(NULL, " "); //token = loc_id 
            strcpy(loc_aux, token);
            loc_aux = strtok(loc_aux, "\n");
            
            strcpy(str_out, "local ");
            strcat(str_out, loc_aux);
            strcat(str_out, ": ");

            strcat(str_out, aux);
            strcat(str_out, "\n");

        break;

        default:
            break;
    }

    free(loc_aux);
    loc_aux = NULL;
    free(dev_aux);
    dev_aux = NULL;
    free(ligado_aux);
    ligado_aux = NULL;
    free(dado_aux);
    dado_aux = NULL;
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
        char *buf = malloc(BUFSZ);
        memset(buf, 0, BUFSZ-1);
        fgets(buf, BUFSZ-1, stdin);
        buf = strtok(buf, "\n"); //desconsidera o enter que se da ao acabar de digitar o comando

        // --- CONSTROI A MENSAGEM DE REQUISICAO --- //
        //msg_buf guarda a mensagem que vai ser enviada ao server
        char *msg_buf = malloc(MSGSZ);
        unsigned correto = process_command(buf, msg_buf);
        if(!correto)
            break;  //se receber mensagem com algum erro, sai do loop
        if(!strcmp(buf, "kill"))
            break;  //se receber o comando kill, sai do loop

        // --- ENVIA A MENSAGEM DE REQUISICAO --- // 
        size_t count = send(s, msg_buf, strlen(msg_buf)+1, 0); //envia como string
        if (count != strlen(msg_buf)+1) {
            logexit("send");
        }

        // --- RECEBE MENSAGEM DE RESPOSTA  --- //
        //O recv salva o que é recebido byte a byte o e retorna o numero de bytes recebido
        char *buf_res = malloc(MSGSZ);
        count = recv(s, buf_res + total, BUFSZ - total, 0);
        if(count == 0){
            //Se nao receber nada, significa que a conexão foi fechada
            break;
        }
        total += count; //Desloca o buffer pra receber o proximo byte

        // --- TRATA MENSAGEM DE RESPOSTA E IMPRIME AVISO NA TELA --- //
        char *warn = malloc(BUFSZ);
        process_res_msg(buf_res, warn, msg_buf);
        
        printf("%s", warn);

        // --- LIBERA A MEMORIA ---//
        free(buf);
        buf = NULL;
        free(msg_buf); //desaloca o espaco da msg
        msg_buf = NULL;
        free(buf_res);
        buf_res = NULL;
        free(warn);
        warn = NULL;

        total = 0;
    }
    //Ao sair do loop, fecha o socket e finaliza a conexao
    close(s);

    //Termina o programa
    exit(EXIT_SUCCESS);
}