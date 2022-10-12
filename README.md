# TP 1 - Sistema controle de dispositivos em Smart Homes

O trabalho consiste em desenvolver uma aplicação com arquitetura Cliente-Servidor para controle de diversos dispositivos inteligentes em uma casa inteligente. Para isso, deve ser implementado o protocolo especificado em no PDF presente em ``/Intro``.

**Data de entrega** : 07/11/22

## Product Roadmap

- Semana 1 (**12/10 - 15/10**): 
  + Organização do roadmap
  + Conexão inicial cliente-servidor
  + Troca de mensagem inicial e teste no Wireshark
- Semana 2 (**16/10 - 22/10**):
  + Implementação da mensagem INS_REQ 
    - É a requisição de instalação de um dispositivo em um cômodo, no formato ``INS_REQ 2 2 1 30``
  + Desenvolvimento da interface de requisição no terminal
    - Manipular strings para que seja possível enviar a mensagem apenas usando a string ``install <devId> in <locId>: <valor1>, <valor2>`` 
  + Implementação da mensagem de OK
  + Implementação do kill
- Semana 3 (**23/10 - 29/10**):
  + Implementação das mensagens usando o modelo da que foi desenvolvida na semana anterior
    - REM_REQ
    - CH_REQ
    - DEV_REQ
    - DEV_RES
    - ERROR
   + Aprimoramento da interface de requisição no terminal usando o modelo da que foi desenvolvida na semana anterior
   + Makefile
- Semana 4 (**30/10 - 06/11**):
  + Implementação das mensagens
    - LOC_REQ
    - LOC_RES 
  + Aprimoramento da interface de requisição


