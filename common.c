#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#include <arpa/inet.h>


void logexit(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

//Funcao para passar os enderecos recebidos na chamada da funcao
//para a estrutura do POSIX que guarda os enderecos (sockaddr_storage)
int addrparse(const char *ip_addr, const char *port_addr, struct sockaddr_storage *storage){
    if(ip_addr == NULL || port_addr == NULL){
        //Sinal pra se der algum erro
        return -1;
    }

    uint16_t port = (uint16_t)atoi(port_addr); //salva a porta como inteiro
    if(port == 0){
        return -1;
    }
    port = htons(port); // host to network (pra garantir que a endianidade esta padrao com a da rede)

    //Se for passado um IPv4 (AF_INET)
    struct in_addr inaddr4;     //IPv4
    if(inet_pton(AF_INET, ip_addr, &inaddr4)) {
        // Salva IPv4 e port no storage
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    //Se for passado um IPv6 (AF_INET6)
    struct in_addr inaddr6;     //IPv6
    if(inet_pton(AF_INET6, ip_addr, &inaddr6)) {
        // Salva IPv6 e port no storage
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET;
        addr6->sin6_port = port;
        memcpy(&(addr6->sin6_addr), inaddr6, sizeof(inaddr6));
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
            logexit("conversao ntop");
        }
        port = ntohs(addr4->sin_port); // network to host short, transforma a endianidade da rede para do sistema
    } else if (addr->sa_family == AF_INET6) {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            logexit("conversao ntop");
        }
        port = ntohs(addr6->sin6_port); // network to host short
    } else {
        logexit("unknown protocol family.");
    }
    //Soh imprime no terminal o endereco (ja em formato de string)
    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}