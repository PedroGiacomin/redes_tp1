#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define MSGSZ 500
#define STR_MIN 8
#define MIN_DEV_ID 1
#define MAX_DEV_ID 5
#define MIN_LOC_ID 1
#define MAX_LOC_ID 5

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

//Funcoes pra construir mensagens de controle ERROR e OK ja em formato de string, fornecendo apenas o codigo
void build_error_msg(char *msg_out, unsigned codigo){
    
    //parse int->str
    char *code_aux = malloc(sizeof(STR_MIN));
    sprintf(code_aux, "%02u", codigo);

    strcpy(msg_out, "ERROR ");
    strcat(msg_out, code_aux);
    strcat(msg_out, "\n");

    free(code_aux);
}
void build_ok_msg(char *msg_out, unsigned codigo){
    
    //parse int->str
    char *code_aux = malloc(sizeof(STR_MIN));
    sprintf(code_aux, "%02u", codigo);

    strcpy(msg_out, "OK ");
    strcat(msg_out, code_aux);
    strcat(msg_out, "\n");

    free(code_aux);
}

//Funcao pra testar se o dispositivo tem um id valido, retorna 1 ou 0 
unsigned is_dev_id_valid(int dev_id){
    if(dev_id >= MIN_DEV_ID && dev_id <= MAX_DEV_ID){
        return 1;
    }
    return 0;
}

//Funcao pra testar se o dispositivo tem um id valido, retorna 1 ou 0 
unsigned is_loc_id_valid(int loc_id){
    if(loc_id >= MIN_LOC_ID && loc_id <= MAX_LOC_ID){
        return 1;
    }
    return 0;
}

// [ERRO] - Tive problema ao passar o database por referencia para a funcao process_request, mas deu certo usando esses colchetes

// --- LOGICA DO DB --- //
// - O banco de dados do servidor eh simplesmente um vetor de 'struct local' com 5 posicoes. 
// - A posicao i do vetor eh resrevada ao local de id i + 1, segundo TABELA 2: 
//      ID     LOCAL    i
//      1      quarto   0
//      2      suite    1 
//      3      sala     2
//      4      cozinha  3
//      5      banheiro 4
// 
// - Cada local tem em sua estrutura um vetor de 5 dispositivos
// - A posicao j do vetor eh reservada ao dispositivo de ID j + 1, segundo TABELA 1:
//      ID     DISPOSITIVO  j
//      1      AC           0
//      2      TV           1
//      3      lampada      2
//      4      porta        3
//      5      camera       4
// - O valor default das posicoes do vetor de dispositivos eh 0, assim, se o dispositivo na posicao j do vetor eh 0, significa
// que o dispositivo de ID j nao estah instalado naquele local

//Funcao que trata a mensagem que chega do cliente e retorna a mensagem de controle (OK ou ERROR) ou de RESPONSE para o cliente na variavel msg_out
void process_request(char *request, struct local locais[], char *response){

    char *token = strtok(request, " "); //token = type
    unsigned req_type = parse_msg_type(token);
    int loc_id = 0;
    int dev_id = 0;
    int ligado = 0;
    int dado = 0;

    switch (req_type){
        case INS_REQ:
            // INS_REQ <locId> <devId> <ligado> <dado>
            token = strtok(NULL, " "); //token = locId
            loc_id = atoi(token);
            token = strtok(NULL, " "); //token = devId
            dev_id = atoi(token);
            token = strtok(NULL, " "); //token = ligado
            ligado = atoi(token);
            token = strtok(NULL, " "); //token = dado
            dado = atoi(token);

            //Primeiro testa se o id do dispositivo eh invalido
            if(!is_dev_id_valid(dev_id)){
                build_error_msg(response, 3);
                return;
            }

            //Instala o dispositivo 
            locais[loc_id - 1].dispositivos[dev_id - 1].id = dev_id;
            locais[loc_id - 1].dispositivos[dev_id - 1].ligado = ligado;
            locais[loc_id - 1].dispositivos[dev_id - 1].dado = dado;

            //Resposta de sucesso
            build_ok_msg(response, 1); 
        break;

        case REM_REQ:
            // REM_REQ <locId> <devId> 
            token = strtok(NULL, " "); //token = locId
            loc_id = atoi(token);
            token = strtok(NULL, " "); //token = devId
            dev_id = atoi(token);
      
            //Primeiro testa se o id do dispositivo eh invalido
            if(!is_dev_id_valid(dev_id)){
                build_error_msg(response, 3); 
                return;
            }

            //testa se o dispositivo estah instalado, i.e., se o id do dispositivo na sua posicao correspondente eh != 0
            if(locais[loc_id - 1].dispositivos[dev_id - 1].id == 0){
                build_error_msg(response, 1);
                return;
            }
            
            //se estiver instalado, desinstala (zerando todos os seus atributos no vetor do DB)
            locais[loc_id - 1].dispositivos[dev_id - 1].id = 0;
            locais[loc_id - 1].dispositivos[dev_id - 1].ligado = 0;
            locais[loc_id - 1].dispositivos[dev_id - 1].dado = 0;

            //Resposta de sucesso
            build_ok_msg(response, 2); 

        break;

        case CH_REQ:
            // CH_REQ <locId> <devId> <ligado> <dado>
            token = strtok(NULL, " "); //token = locId
            loc_id = atoi(token);
            token = strtok(NULL, " "); //token = devId
            dev_id = atoi(token);
            token = strtok(NULL, " "); //token = ligado
            ligado = atoi(token);
            token = strtok(NULL, " "); //token = dado
            dado = atoi(token);

            //Primeiro testa se o id do dispositivo eh invalido
            if(!is_dev_id_valid(dev_id)){
                build_error_msg(response, 3); 
                return;
            }
            
            //Testa se o dispositivo estah instalado no local
            if(locais[loc_id - 1].dispositivos[dev_id - 1].id == 0){
                build_error_msg(response, 1);
                return;
            }

            //se estiver instalado, muda seu estado
            locais[loc_id - 1].dispositivos[dev_id - 1].ligado = ligado;
            locais[loc_id - 1].dispositivos[dev_id - 1].dado = dado;
            
            //resposta de sucesso
            build_ok_msg(response, 3);
            
        break;

        case DEV_REQ:
            // DEV_REQ <locId> <devId> 
            token = strtok(NULL, " "); //token = locId
            loc_id = atoi(token);
            token = strtok(NULL, " "); //token = devId
            dev_id = atoi(token);

            //Primeiro testa se o id do dispositivo eh invalido
            if(!is_dev_id_valid(dev_id)){
                build_error_msg(response, 3); 
                return;
            }

            //Testa se o dispositivo estah instalado no local
            if(locais[loc_id - 1].dispositivos[dev_id - 1].id == 0){
                build_error_msg(response, 1);
                return;
            }

            //Constroi a resposta com os dados como uma string dinamicamente
            //DEV_RES <ligado> <dado>  
            strcpy(response, "DEV_RES");

            char *aux = malloc(STR_MIN);
            //Consulta o DB e insere as infos na string da msg de resposta 
            sprintf(aux, " %d", locais[loc_id - 1].dispositivos[dev_id - 1].ligado);
            strcat(response, aux);
            
            sprintf(aux, " %d", locais[loc_id - 1].dispositivos[dev_id - 1].dado);
            strcat(response, aux);

            strcat(response, "\n");
            
            free(aux);
            puts(response);
        break;

        case LOC_REQ:
            // LOC_REQ <locId>  
            token = strtok(NULL, " "); //token = locId
            loc_id = atoi(token);
            
            strcpy(response, "LOC_RES");

            //Primeiro testa se o id do local eh invalido
            if(!is_loc_id_valid(loc_id)){
                build_error_msg(response, 4); 
                return;
            }
            
            char aux2[STR_MIN];
            unsigned dev_count = 0;
            //Percorre todos os dispositivos do vetor
            for (int i = MIN_DEV_ID; i <= MAX_DEV_ID; i++){
                if(locais[loc_id - 1].dispositivos[i - 1].id != 0){
                    //significa que o dispositivo i estah instalado, adiciona seus estados na LOC_RES
                    sprintf(aux2, " %d", i);
                    strcat(response, aux2); // dev i

                    sprintf(aux2, " %d", locais[loc_id - 1].dispositivos[i - 1].ligado);
                    strcat(response, aux2); // ligado/desligado

                    sprintf(aux2, " %d", locais[loc_id - 1].dispositivos[i - 1].dado);
                    strcat(response, aux2); //dado

                    dev_count++;
                }
            }
            strcat(response, "\n");

            //testa se nao tem nenhum dispositivo instalado
            if(!dev_count){
                build_error_msg(response, 2); 
                return;
            }

        break;
        
        default:

        break;
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
    struct local database[MAX_DEV_ID];
    //inicializa o banco de dados com os ids 0
    for(int i = MIN_LOC_ID ; i <= MAX_LOC_ID; i++){
        database[i - 1].id = i;
        for(int j = MIN_DEV_ID - 1; j <= MAX_DEV_ID - 1; j++){
            database[i].dispositivos[j].id = 0;
            database[i].dispositivos[j].dado = 0;
            database[i].dispositivos[j].ligado = 0;
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

    while (1) {
        // ------------- TROCA DE MENSAGENS ------------- //

        // --- RECEBIMENTO DA MENSAGEM DO CLIENTE (request) --- //
        //Recebe msg em formato de [string] e salva no numa string
        char req_msg[MSGSZ];
        memset(req_msg, 0, MSGSZ);
        size_t count = recv(client_sock, req_msg, MSGSZ - 1, 0);
        if(count == 0){
            //Se nao receber nada, significa que a conexão foi fechada
            close(client_sock); // Encerra a conexao com aquele cliente
            break;
        }
        printf("%s", req_msg); //Imprime a msg na tela
        
        // --- ACAO E CONSTRUCAO DA MENSAGEM DE RESPOSTA --- //
        //Processa a mensagem e guarda a mensagem de resposta ao cliente numa string
        char *res_msg = malloc(MSGSZ);
        process_request(req_msg, database, res_msg);

        // --- ENVIO DA MENSAGEM (response) --- //
        count = send(client_sock, res_msg, strlen(res_msg) + 1, 0);
        if (count != strlen(res_msg) + 1) {
            logexit("erro ao enviar mensagem de resposta");
        }

        free(res_msg);
        res_msg = NULL;
    }

    exit(EXIT_SUCCESS);
}