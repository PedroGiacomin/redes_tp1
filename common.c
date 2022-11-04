#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

#define MSGSZ 1024
#define MAX_STATE_VALUES 2
#define STR_MIN 8

// --- TRATAMENTO DE ERROS --- //
void logexit(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

// --- LÓGICA DAS MENSAGENS --- // 
//Funcao que pega uma string com o tipo da mensagem e devolve uma variavel do enum
enum MSG_TYPE {INS_REQ = 1, REM_REQ, CH_REQ, DEV_REQ, LOC_REQ, DEV_RES, LOC_RES, ERROR, OK};
unsigned parse_msg_type(const char *msg_type_in){
    if(!strcmp(msg_type_in, "INS_REQ"))
        return INS_REQ;
    else if (!strcmp(msg_type_in, "REM_REQ"))
        return REM_REQ;
    else if (!strcmp(msg_type_in, "CH_REQ"))
        return CH_REQ;
    else if (!strcmp(msg_type_in, "DEV_REQ"))
        return DEV_REQ;
    else if (!strcmp(msg_type_in, "LOC_REQ"))
        return LOC_REQ;
    else if (!strcmp(msg_type_in, "DEV_RES"))
        return DEV_RES;
    else if (!strcmp(msg_type_in, "LOC_RES"))
        return LOC_RES;
    else if (!strcmp(msg_type_in, "ERROR"))
        return ERROR;
    else if (!strcmp(msg_type_in, "OK"))
        return OK;
    else 
        return 0;
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