#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define MSGSZ 1024

void usage(){
    printf("Chamada correta: ./server <v4/v6> <port>\n");
    exit(EXIT_FAILURE);
}

//Cada dispositivo tem um ID, um estado de on/off e um estado de dados
struct dispositivo{
    unsigned id;
    unsigned ligado;
    unsigned dado;
};

//Cada local tem no maximo 5 dispositivos
struct local{
    unsigned id;
    struct dispositivo dispositivos[5];
};

// [OBS] - Como as mensagens de controle tem apenas dois parametros, optei por já criar direto como string sem uma struct intermediaria
//Funcoes pra construir mensagens de controle ERROR e OK jah em formato de string, fornecendo apenas o codigo
void build_error_msg(char *msg_out, unsigned codigo){
    
    //parse int->str
    char *code_aux = malloc(sizeof(3));
    sprintf(code_aux, "%02u", codigo);
    
    strcat(msg_out, "ERROR ");
    strcat(msg_out, code_aux);
    strcat(msg_out, "\n");

    free(code_aux);
}
void build_ok_msg(char *msg_out, unsigned codigo){
    
    //parse int->str
    char *code_aux = malloc(sizeof(3));
    sprintf(code_aux, "%02u", codigo);
    
    strcat(msg_out, "OK ");
    strcat(msg_out, code_aux);
    strcat(msg_out, "\n");

    free(code_aux);
}

//Funcao pra testar se o dispositivo tem um id valido, retorna 1 ou 0
unsigned is_dev_id_valid(int dev_id){
    if(dev_id >= 1 && dev_id <= 5){
        return 1;
    }
    return 0;
}

// [OBS] - Não usei um switch case pra nao ter que ficar transformando o tipo de msg de requisicao de enum pra string
//Funcao que trata a mensagem que chega do cliente e retorna a mensagem de controle para o cliente na variavel msg_out
void process_request(struct request_msg *msg, struct dispositivo dispositivos[5], char *msg_out){
    //Caso INS_REQ
    if(strcmp(msg->type, "INS_REQ")){
        if(!is_dev_id_valid(msg->dev_id)){
            //Envia ERROR 03
            build_error_msg(msg_out, 3);
            return;
        }
        
        dispositivos[msg->dev_id].id = msg->dev_id;
        dispositivos[msg->dev_id].ligado = msg->dev_state[0];
        dispositivos[msg->dev_id].dado = msg->dev_state[1];
        
        //Envia OK 01
        build_ok_msg(msg_out, 1);
    }

    // else if(strcmp(msg->type, "REM_REQ")){

    // }
    // else if(strcmp(msg->type, "CH_REQ")){

    // }
    // else if(strcmp(msg->type, "DEV_REQ")){

    // }
    // else if(strcmp(msg->type, "LOC_REQ")){

    // }
    else{

    }
 
}

//argv[1] -> familia IP
//argv[2] -> porta do processo
int main(int argc, char **argv){
    // Garantia de que o programa foi inicializado corretamente
    if(argc < 3){
        usage();
    }

    // ------------- DB DO SERVER ------------- //
    //cada local por padrao tem um vetor de 5 dispositivos
    struct local locais[5];

    //inicializa o banco de dados com os ids 0
    for(int i = 0; i < 5; i++){
        locais[i].id = 0;
        for(int j = 0; j < 5; j++){
            locais[i].dispositivos[j].id = 0;
        }
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
        // --- BUFFER --- //
        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);

        // --- RECEBIMENTO DA MENSAGEM DO CLIENTE (request) --- //
        //Recebe msg em formato de [string] e salva no msg_buf
        char msg_buf[BUFSZ];
        memset(msg_buf, 0, BUFSZ);
        size_t count = recv(client_sock, msg_buf, MSGSZ - 1, 0);
    
        //Transforma a mensagem em [request_msg] e guarda em msg_recebida
        struct request_msg *msg_recebida = malloc(MSGSZ);
        string2msg(msg_buf, msg_recebida);

        //Printa o conteudo da msg na tela
        printf("tipo: %s\n", msg_recebida->type);
        printf("loc_id: %d\n", msg_recebida->local_id);
        printf("dev_id: %d\n", msg_recebida->dev_id);
        for(int i = 0; i < sizeof(msg_recebida->dev_state)/ sizeof(int); i++){
            printf("dev_state[%d]: %d\n", i, msg_recebida->dev_state[i]);
        }

        //Processa a mensagem e guarda a mensagem de controle de resposta ao cliente em msg_buf
        memset(msg_buf, 0, BUFSZ);
        process_request(msg_recebida, locais->dispositivos, msg_buf);

        //Desaloca a mensagem recebida do cliente
        free(msg_recebida->dev_state); //desaloca o vetor de valores
        free(msg_recebida);

        // --- CRIACAO DA MENSAGEM (response) --- //
        //Salva o proprio endereco do cliente no buffer
        //(num primeiro momento a mensagem eh so esse endereco)
        sprintf(buf, "%.900s", msg_buf);

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