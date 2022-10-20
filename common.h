#pragma once

#include <stdlib.h>

#include <arpa/inet.h>

// --- TRATAMENTO DE ERROS --- //
void logexit(const char *msg);

// --- TRATAMENTO DE ENDERECOS --- // 
int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage);

// --- TRATAMENTO DE MENSAGENS --- //
struct mensagem{
    //char *tipo;
    int local_id; 
    int dev_id;
    //int dev_state[10];
};

struct mensagem build_msg(int loc, int dev);

void msg2string(char *str_out, struct mensagem msg_in);