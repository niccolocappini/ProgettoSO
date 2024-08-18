#ifndef FUNZIONI_CLIENT_H
#define FUNZIONI_CLIENT_H

#include "definizioniComuni.h"

// Procedure di Supporto
void stampaOutputDalServer(int clientSocket);
void inserimentoDatiRecord(recordRub *record);
void invioRecord(int clientSocket);

// Procedure relative alle opearzioni di lettura
void visualizzaRubrica();
void ricercaRecordCognome(int clientSocket);
void ricercaRecordNomeCognome(int clientSocket);

// Procedure relative alle operazioni di scrittura
void aggiungiRecord(int clientSocket);
void rimuoviRecord(int clientSocket);
void modificaTelefono(int clientSocket);
void modificaIndirizzo(int clientSocket);

// Gestione Segnali
void handle_sigint(int sig);

#endif
