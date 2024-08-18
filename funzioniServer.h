#ifndef FUNZIONI_SERVER_H
#define FUNZIONI_SERVER_H

// Procedure e Funzioni di Supporto
void richiestaPassword(int clientSocket);
void controlloOutput(int clientSocket, int risultato, char *messaggio);
void riceviCampoDaClient(int clientSocket, char *datoDaRicevere, int dimensioneDato, char *messaggioDiErrore);
void riceviRecordDaClient(int clientSocket, recordRub *recordDaRicevere, int dimensioneRecord, char *messaggioDiErrore);
int controlloRubricaVuota(char **output);
int normalizzaRecord(recordRub *record);
int normalizzaNomeCognome(char campo[MAX_LUNG_CAMPO]);
int normalizzaIndirizzo(char campo[MAX_LUNG_CAMPO]);
int normalizzaTelefono(char campo[MAX_LUNG_CAMPO]);
long int ricercaRecord(recordRub *recordDaRicercare);
int modificaCampoRubrica(int clientSocket, char **output, int campoScelto);
int modificaCampoRecord(int posizioneRecordDaModificare, int campoScelto, char *nuovoValore);

// Procedure che modificano una stringa, facendogli contenere ciò che è stato richiesto
void visualizzaRubrica(char **output);
void ricercaRecordConCognome(int clientSocket, char **output);
void ricercaRecordConNomeCognome(int clientSocket, char **output);

// Queste funzioni restituiranno un intero (codice) che in caso di successo sarà 0 altrimenti -1
int aggiungiRecord(int clientSocket, char **output);
int rimuoviRecord(int clientSocket, char **output);
int modificaTelefono(int clientSocket, char **output);
int modificaIndirizzo(int clientSocket, char **output);

//Gestione segnali
void handle_sigint(int sig);

#endif