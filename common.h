#pragma once

#include <stdlib.h>

#include <arpa/inet.h>

#define MAX_VALUES 5

// --- TRATAMENTO DE ERROS --- //
void logexit(const char *msg);

// --- TRATAMENTO DE ENDERECOS --- // 
int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage);

// --- TRATAMENTO DE MENSAGENS --- //
struct request_msg{
    char *type;
    int local_id; 
    int dev_id;
    int dev_state[MAX_VALUES]; //São poucos valores na mensagem de request, no max 5
};

void build_msg(struct request_msg *msg_out, int loc, int dev);

void msg2string(char *str_out, struct request_msg *msg_in);