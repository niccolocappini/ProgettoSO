#ifndef FUNZIONI_SERVER_H
#define FUNZIONI_SERVER_H

int loginUtente(int clientSocket);
void logoutUtente(int clientSocket);
void richiestaPassword(int clientSocket);
void controlloOutput(int risultato, char * messaggio);

/* Questi metodi restituiranno */
char * visualizzaRubrica(int clientSocket);
char * ricercaRecordConCognome(int clientSocket);
char * ricercaRecordConCognomeNome(int clientSocket);

/* Questi metodi restituiranno un intero (codice) che in caso di successo sarà 1 altrimenti 0*/
int aggiungiRecord(int clientSocket);
int rimuoviRecord(int clientSocket);
int modificaTelefono(int clientSocket);
int modificaIndirizzo(int clientSocket);

#endif