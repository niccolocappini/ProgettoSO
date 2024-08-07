#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "definizioniComuni.h"
#include "funzioniServer.h"

#define PASSWORD "PROGETTOSO"
// #define dimBuffer 2048 // dimensione buffer per la comunicazione tramite socket

static recordRub *rubrica = NULL;

/*
int search_records(recordRub entries[], int entriesCount, recordRub query,
recordRub queryResults[]); int add_new_record(recordRub entries[], int*
entriesCount, recordRub newDataEntry); int delete_record(recordRub entries[],
int entriesCount, recordRub entryToDelete); int edit_record(recordRub entries[],
int entriesCount, recordRub entryToEdit, recordRub editedEntry);

int search_record_position(recordRub entries[], int entriesCount, recordRub
query); void send_entries(int clientSocket, recordRub entries[], int
entriesCount); void update_runtime_database(recordRub newRuntimeDatabase[], int*
newRuntimeEntriesCount);

void handle_sigint(int sig);
void handle_death_signal_from_admin(int sig);
void handle_death_signal_from_user(int sig);

void handle_errno(int errorCode, char* errorMessage);
*/

int main()
{
  /* a+ -> file aperto in lettura/(scrittura in aggiunta) creandolo se necessario, o aggiungendovi a partire dalla fine
  e di conseguenza posizionandosi alla fine del file stesso */
  rubrica = (recordRub *)fopen("RubricaDB", "a+");

  int serverSocket, clientSocket;
  struct sockaddr_in indirizzo;
  int lunghezzaIndirizzo = sizeof(indirizzo);

  /*
    char input[dimBuffer] = {0};  // buffer utilizzato per le richieste dei
    client char output[dimBuffer] = {0}; // buffer utilizzato per le risposte
    del server
  */

  // Creazione del socket
  serverSocket = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
  if (serverSocket < 0)
  {
    generazioneErrore("Creazione socket fallita \n");
  }

  // Configurazione di "indirizzo" per poter effettuare il binding correttamente
  indirizzo.sin_family = AF_INET;
  indirizzo.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  indirizzo.sin_port = htons(PORTA);

  // Binding del socket alla porta specificata in "indirizzo"
  if (bind(serverSocket, (struct sockaddr *)&indirizzo, lunghezzaIndirizzo) < 0)
  {
    generazioneErrore("Errore durante il binding \n");
  }
  listen(serverSocket, 10);

  printf("Menù delle operazione che possono essere richieste dal client: \n"
         "1) Visualizzazzione tutti i record della rubrica \n"
         "2) Ricerca record tramite cognome \n"
         "3) Ricerca record tramite coppia nome-cognome \n"
         "4) Aggiunta Record \n"
         "5) Eliminazione Record \n"
         "6) Modifica Numero di Telefono \n"
         "7) Modifica Indirizzo \n");

  while (1)
  {

    printf("Server in ascolto sulla porta %d\n", PORTA);

    // Accetta una connessione in entrata
    if ((clientSocket = accept(serverSocket, (struct sockaddr *)&indirizzo,
                               (socklen_t *)&lunghezzaIndirizzo)) < 0)
    {
      generazioneErrore("Errore durante l'accettazione di una richiesta \n");
    }

    printf("Accettata connessione da un client\n");

    // Gestione richiesta tramite fork
    int pid = fork();

    if (pid == 0)
    {
      printf("Sono un figlio generato per gestire la richiesta appena arrivata\n");

      // Login utente
      int richiesta = 0;
      // si leggono 4 byte = dimensione intero
      int risultato = recv(clientSocket, &richiesta, sizeof(richiesta), 0);
      if (risultato < 1){
        generazioneErrore("Lettura fallita\n");
      }

      printf("Lettura effettuata: %d\n", richiesta);

      int outputFunzioneServer = -1;
      switch (richiesta)
      {

        case VISUALIZZA_OGNI_RECORD:
          printf("Gestione Richiesta 1: \n");
          visualizzaRubrica(clientSocket);

        break;

        case RICERCA_RECORD_CON_COGNOME:
          printf("Richiesta 2\n");
          ricercaRecordConCognome(clientSocket);

        break;

        case RICERCA_RECORD_CON_NOME_COGNOME:
          printf("Richiesta 3\n");
          ricercaRecordConCognomeNome(clientSocket);

        break;

        case AGGIUGI_RECORD:
          printf("Richiesta 4\n");
          richiestaPassword(clientSocket);
          outputFunzioneServer = aggiungiRecord(clientSocket);
          controlloOutput(outputFunzioneServer,"Aggiunta Record in Rubrica Fallita \n");

        break;

        case RIMUOVI_RECORD:
          printf("Richiesta 5\n");
          richiestaPassword(clientSocket);
          outputFunzioneServer = rimuoviRecord(clientSocket);
          controlloOutput(outputFunzioneServer,"Rimozione Record in Rubrica Fallita \n");

        break;

        case MODIFICA_TELEFONO:
          printf("Richiesta 6\n");
          richiestaPassword(clientSocket);
          outputFunzioneServer = modificaTelefono(clientSocket);
          controlloOutput(outputFunzioneServer,"Modifica Telefono Fallita \n");

        break;

        case MODIFICA_INDIRIZZO:
          printf("Richiesta 7\n");
          richiestaPassword(clientSocket);
          outputFunzioneServer = modificaIndirizzo(clientSocket);
          controlloOutput(outputFunzioneServer,"Modifica Indirizzo Fallita \n");

        break;

        default:
          generazioneErrore("Richiesta non valida\n");
          
        break;
      }

      // Invio della risposta al client
      // send(clientSocket, output, strlen(output), 0);
      // printf("Risposta inviata: %s\n", output);
      close(clientSocket);
      exit(EXIT_SUCCESS);
    }
    else
    {
      printf("Sono il processo che continua ad accettare richieste\n");
      close(clientSocket);
    }

    // Chiudi la connessione con il client
    // logoutUtente(clientSocket);

    //printf("La connessione si è conclusa\n");
  }

  printf("La connessione si è conclusa\n");

  return 0;
}

int loginUtente(int clientSocket) { return 0; }

void logoutUtente(int clientSocket) {}

void richiestaPassword(int clientSocket)
{
  char richiesta[] = "Richiesta password per effettuare operazioni di modifica della rubrica\n";
  send(clientSocket, richiesta, sizeof(richiesta), 0);

  char passwordRicevuta[sizeof(PASSWORD)];
  recv(clientSocket, passwordRicevuta, sizeof(PASSWORD), 0);
  printf("Password Ricevuta: %s \n",passwordRicevuta);

  if (strcmp(passwordRicevuta, PASSWORD) != 0){
    generazioneErrore("Password errata: l'operazione non può essere eseguita \n");
  }
  else{
    printf("Password Accettata \n");
  }
}

void controlloOutput(int risultato, char * messaggio){
  if(risultato == 0){
    generazioneErrore(messaggio);
  }
}

char * visualizzaRubrica(int clientSocket){
  return NULL;
}

char * ricercaRecordConCognome(int clientSocket){
  return NULL;
}

char * ricercaRecordConCognomeNome(int clientSocket){
  return NULL;
}

int aggiungiRecord(int clientSocket){
  return 0;
}

int rimuoviRecord(int clientSocket){
  return 0;
}

int modificaTelefono(int clientSocket){
  return 0;
}

int modificaIndirizzo(int clientSocket){
  return 0;
}
