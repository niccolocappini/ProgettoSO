#ifndef FUNZIONI_SERVER_H
#define FUNZIONI_SERVER_H

int loginUtente(int clientSocket);
void logoutUtente(int clientSocket);
void richiestaPassword(int clientSocket);
void controlloOutput(int risultato, char *messaggio);
void riceviDatiDaClient(int clientSocket, char *datoDaRicevere, int dimensioneDato, char *messaggio);
void normalizzaRecord(recordRub *recordDaAggiungere);

/* Questi metodi restituiranno una stringa contenente ciò che è stato richiesto*/
void visualizzaRubrica(char **output);
int controlloRubricaVuota(char **output);
void ricercaRecordConCognome(int clientSocket, char **output);
void ricercaRecordConCognomeNome(int clientSocket, char **output);

/* Questi metodi restituiranno un intero (codice) che in caso di successo sarà 0 altrimenti 1*/
int aggiungiRecord(int clientSocket, char **output);
int rimuoviRecord(int clientSocket, char **output);
int modificaTelefono(int clientSocket, char **output);
int modificaIndirizzo(int clientSocket, char **output);

#endif