#ifndef FUNZIONI_CLIENT_H
#define FUNZIONI_CLIENT_H

#include "definizioniComuni.h"

void stampaOutputDalServer(int clientSocket);
/*void ricercaRecordCognome(char cognome[]);
void ricercaRecordNomeCognome(char nome[], char cognome[]);

void aggiungiRecord(recordRub record);
void rimuoviRecord(recordRub record);
void modificaTelefono(char vecchioTelefono[], char nuovoTelefono[]);
void modificaIndirizzo(char vecchioIndirizzo[], char nuovoIndirizzo[]);*/

void visualizzaRubrica();
void ricercaRecordCognome(int clientSocket);
void ricercaRecordNomeCognome(int clientSocket);

void aggiungiRecord(int clientSocket);
void rimuoviRecord(int clientSocket);
void modificaTelefono(int clientSocket);
void modificaIndirizzo(int clientSocket);

#endif
