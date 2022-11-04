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
enum MSG_TYPE {INS_REQ = 1, REM_REQ, CH_REQ, DEV_REQ, LOC_REQ, DEV_RES, LOC_RES, ERROR, OK};
unsigned parse_msg_type(const char *msg_type_in);