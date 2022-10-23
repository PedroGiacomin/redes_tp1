#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

#define MSGSZ 1024
#define MAX_STATE_VALUES 2

// --- TRATAMENTO DE ERROS --- //
void logexit(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

// --- TRATAMENTO DE MENSAGENS --- // 

//Estrutura que guarda uma mensagem de requisicao para o servidor
struct request_msg{
    char *type;
    int local_id; 
    int dev_id;
    int *dev_state;
};

//Funcao pra inicializar mensagem de requisicao (cliente -> servidor)
void build_request_msg(struct request_msg *msg_out, char *tipo, int loc, int dev, int *state_vec){
    msg_out->type = tipo;
    msg_out->dev_id = dev;
    msg_out->local_id = loc;
    msg_out->dev_state = state_vec;

    printf("[log] Msg built\n");
}

// Transforma uma [request_msg] em uma [string] no formato TYPE LOC_ID DEV_ID VALUES 
// Primeiro transforma cada parte da mensagem em string e depois concatena tudo 
void msg2string(char *str_out, struct request_msg *msg_in){

    //[ERRO] - Estava dando segmentation fault por que eu nao estava alocando memoria pras strings auxiliares, depois de usar malloc deu certo
    //[ERRO] - Tava retornando uma string antes, mas nao tinha como desalocar a memoria da string de retorno [vide commit 1.0], entao eu passei a string como 
    //parametro pra resolver, e aloquei localmente onde a string com a mensagem ia ser usada
    
    char *dev_id_aux = malloc(sizeof(msg_in->dev_id));
    char *loc_id_aux = malloc(sizeof(msg_in->dev_id));
    char *values_aux = malloc(sizeof(msg_in->dev_id));
    
    sprintf(dev_id_aux, " %d", msg_in->dev_id);
    sprintf(loc_id_aux, " %d", msg_in->local_id);

    //sizeof(msg_in->dev_state)/sizeof(int) eh a quantidade de elementos do vetor, que tem tamanho variavel
    for(int i = 0; i < sizeof(msg_in->dev_state)/sizeof(int); i++){
        char *aux = malloc(3);
        sprintf(aux, " %d", msg_in->dev_state[i]);
        strcat(values_aux, aux); 
        free(aux);
    }

    //constroi a string completa
    strcat(str_out, msg_in->type);
    strcat(str_out, dev_id_aux);
    strcat(str_out, loc_id_aux);
    strcat(str_out, values_aux);

    //desaloca variaveis auxiliares
    free(dev_id_aux);
    free(loc_id_aux);
    free(values_aux);
}

// Transforma uma [string] no formato TYPE LOC_ID DEV_ID VALUES em uma [request_msg] 
// O vetor tem de valores tem de ser alocado dinamicamente e depois desalocado
void string2msg(char *str_in, struct request_msg *msg_out){

    // A funcao strtok eh usada pra cortar a string pedaco por pedaco de acordo com o " ", e salva o pedaco atual na variavel token
    // Na primeira chamada passamos a string a ser cortada e depois passamos NULL
    char *token = strtok(str_in, " ");
    msg_out->type = token;      // pega o tipo
    
    token = strtok(NULL, " ");
    msg_out->local_id = atoi(token);   // pega o local_id, atoi eh uma funcao parse de string pra inteiro

    token = strtok(NULL, " ");
    msg_out->dev_id =  atoi(token);   // pega o dev_id

    // loop pra pegar cada valor do vetor dev_state, que tem tamanho variavel, e construi-lo
    int i = 0;
    token = strtok(NULL, " ");
    while(token != NULL){
                
        msg_out->dev_state = realloc(msg_out->dev_state, i * sizeof(int));   //aloca memoria pro proximo elemento do vetor
        msg_out->dev_state[i] = atoi(token);                //atribui valor ao proximo elemento do vetor

        token = strtok(NULL, " ");              //corta a string novamente
        i++;
    }
}


// --- DEFINICAO DE ENDERECOS --- // 

//Funcao para passar os enderecos recebidos na chamada do programa
//para a estrutura do POSIX que guarda os enderecos (sockaddr_storage)

//se tiver erro retorna -1, se nao retorna 0
int addrparse(const char *ip_recebido, const char *port_recebido, struct sockaddr_storage *storage){
    if(ip_recebido == NULL || port_recebido == NULL){
        //Sinal pra se der algum erro
        return -1;
    }

    //salva a porta como inteiro
    uint16_t port = (uint16_t)atoi(port_recebido); 
    if(port == 0){
        return -1;
    }
    port = htons(port); // host to network (pra garantir que a endianidade esta padrao com a da rede)

    //Se for passado um IPv4 (AF_INET)
    struct in_addr inaddr4;     //IPv4
    if(inet_pton(AF_INET, ip_recebido, &inaddr4)) {
        // Salva IPv4 e port no storage
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    //Se for passado um IPv6 (AF_INET6)
    struct in_addr inaddr6;     //IPv6
    if(inet_pton(AF_INET6, ip_recebido, &inaddr6)) {
        // Salva IPv6 e port no storage
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

//Funcao que transforma o endereco, que esta em uma estrutura sockaddr, em uma string
void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;

    //Trata diferente se eh IPv4 ou IPv6
    if (addr->sa_family == AF_INET) {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        //inet_ntop eh o que transforma o endereco em formato sockaddr pra string
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            logexit("erro na conversao ntop");
        }
        port = ntohs(addr4->sin_port); // network to host short, transforma a endianidade da rede para do sistema
    } else if (addr->sa_family == AF_INET6) {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            logexit("erro na conversao ntop");
        }
        port = ntohs(addr6->sin6_port); // network to host short
    } else {
        logexit("familia de IP desconhecida");
    }
    //Soh imprime no terminal o endereco (ja em formato de string)
    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}

//Funcao para passar os enderecos recebidos na chamada do programa (fam_ip + port)
//para a estrutura do POSIX que guarda os enderecos (sockaddr_storage)
int server_sockaddr_init(const char *ip_family_recebido, const char *port_recebido,
                         struct sockaddr_storage *storage) {
    
    // Salva a porta do processo como int
    uint16_t port = (uint16_t)atoi(port_recebido);
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short (endianidade)

    memset(storage, 0, sizeof(*storage));

    //Trata diferente se eh v4 ou v6
    if (0 == strcmp(ip_family_recebido, "v4")) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY; //significa que o endereco de IP pode ser qualquer um
        addr4->sin_port = port;
        return 0;
    } else if (0 == strcmp(ip_family_recebido, "v6")) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any;
        addr6->sin6_port = port;
        return 0;
    } else {
        return -1;
    }
}