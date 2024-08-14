#ifndef FUNZIONI_CLIENT_H
#define FUNZIONI_CLIENT_H

#include "definizioniComuni.h"

void stampaOutputDalServer(int clientSocket);
void inserimentoDatiRecord(recordRub *record);
void invioRecord(int clientSocket);
void visualizzaRubrica();
void ricercaRecordCognome(int clientSocket);
void ricercaRecordNomeCognome(int clientSocket);

void aggiungiRecord(int clientSocket);
void rimuoviRecord(int clientSocket);
void modificaTelefono(int clientSocket);
void modificaIndirizzo(int clientSocket);

#endif
