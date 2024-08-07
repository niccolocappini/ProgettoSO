#ifndef FUNZIONI_SERVER_H
#define FUNZIONI_SERVER_H

int loginUtente(int clientSocket);
void logoutUtente(int clientSocket);
void richiestaPassword(int clientSocket);

void stampaRubrica(int clientSocket);
void ricercaRecordConCognome(int clientSocket);
void ricercaRecordConCognomeNome(int clientSocket);

void aggiungiRecord(int clientSocket);
void rimuoviRecord(int clientSocket);
void modificaTelefono(int clientSocket);
void aggiungiIndirizzo(int clientSocket);

#endif