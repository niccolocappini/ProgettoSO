#ifndef FUNZIONI_SERVER_H
#define FUNZIONI_SERVER_H

void richiestaPassword(int clientSocket);
void controlloOutput(int clientSocket, int risultato, char *messaggio);
void riceviCampoDaClient(int clientSocket, char *datoDaRicevere, int dimensioneDato, char *messaggioDiErrore);
void riceviRecordDaClient(int clientSocket, recordRub *recordDaRicevere, int dimensioneRecord, char *messaggioDiErrore);
int controlloRubricaVuota(char **output);
int normalizzaRecord(recordRub *recordDaAggiungere);
long int ricercaRecord(recordRub *recordDaRicercare);
// long int ricercaPosizioneRecordConSingoloCampo(char *valoreDaRicercare, int campoScelto);
int modificaCampo(int posizioneRecordDaModificare, int campoScelto, char *nuovoValore);

/* Questi metodi restituiranno una stringa contenente ciò che è stato richiesto*/
void visualizzaRubrica(char **output);
void ricercaRecordConCognome(int clientSocket, char **output);
void ricercaRecordConNomeCognome(int clientSocket, char **output);

/* Questi metodi restituiranno un intero (codice) che in caso di successo sarà 1 altrimenti 0*/
int aggiungiRecord(int clientSocket, char **output);
int rimuoviRecord(int clientSocket, char **output);
int modificaTelefono(int clientSocket, char **output);
int modificaIndirizzo(int clientSocket, char **output);

#endif