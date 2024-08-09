#ifndef FUNZIONI_CLIENT_H
#define FUNZIONI_CLIENT_H

#include "definizioniComuni.h"

void stampaOutputDalServer(int clientSocket, char *str);
/*void ricercaRecordCognome(char cognome[]);
void ricercaRecordNomeCognome(char nome[], char cognome[]);

void aggiungiRecord(recordRub record);
void rimuoviRecord(recordRub record);
void modificaTelefono(char vecchioTelefono[], char nuovoTelefono[]);
void modificaIndirizzo(char vecchioIndirizzo[], char nuovoIndirizzo[]);*/

void visualizzaRubrica();
void ricercaRecordCognome();
void ricercaRecordNomeCognome();

void aggiungiRecord();
void rimuoviRecord();
void modificaTelefono();
void modificaIndirizzo();

#endif
