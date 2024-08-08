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

static FILE *rubrica = NULL;
// static int recordContenuti = 0;

/*

void handle_sigint(int sig);
void handle_death_signal_from_admin(int sig);
void handle_death_signal_from_user(int sig);

void handle_errno(int errorCode, char* errorMessage);
*/

int main()
{
  /* a+ -> file aperto in lettura/(scrittura in aggiunta) creandolo se necessario, o aggiungendovi a partire dalla fine
  e di conseguenza posizionandosi alla fine del file stesso */
  rubrica = fopen("RubricaDB", "a+");
  /*long int k = sizeof(rubrica);
  char * stringa;
  sprintf(stringa,"%d",k);
  printf("%s",stringa);*/

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
      if (risultato < 1)
      {
        generazioneErrore("Lettura fallita\n");
      }

      printf("Lettura effettuata: %d\n", richiesta);

      risultato = -1;
      char *output; // stringa che contterrà ciò che verrà inviato al client
      switch (richiesta)
      {

      case VISUALIZZA_OGNI_RECORD:
        printf("Gestione Richiesta 1: \n");
        output = visualizzaRubrica();

        break;

      case RICERCA_RECORD_CON_COGNOME:
        printf("Gestione Richiesta 2\n");
        output = ricercaRecordConCognome(clientSocket);

        break;

      case RICERCA_RECORD_CON_NOME_COGNOME:
        printf("Gestione Richiesta 3\n");
        output = ricercaRecordConCognomeNome(clientSocket);

        break;

      case AGGIUGI_RECORD:
        printf("Gestione Richiesta 4\n");
        richiestaPassword(clientSocket);
        risultato = aggiungiRecord(clientSocket);
        controlloOutput(risultato, "Aggiunta Record in Rubrica Fallita \n");

        break;

      case RIMUOVI_RECORD:
        printf("Gestione Richiesta 5\n");
        richiestaPassword(clientSocket);
        risultato = rimuoviRecord(clientSocket);
        controlloOutput(risultato, "Rimozione Record in Rubrica Fallita \n");

        break;

      case MODIFICA_TELEFONO:
        printf("Gestione Richiesta 6\n");
        richiestaPassword(clientSocket);
        risultato = modificaTelefono(clientSocket);
        controlloOutput(risultato, "Modifica Telefono Fallita \n");

        break;

      case MODIFICA_INDIRIZZO:
        printf("Gestione Richiesta 7\n");
        richiestaPassword(clientSocket);
        risultato = modificaIndirizzo(clientSocket);
        controlloOutput(risultato, "Modifica Indirizzo Fallita \n");

        break;

      default:
        generazioneErrore("Richiesta non valida\n");
      }

      // Invio della risposta al client
      send(clientSocket, output, strlen(output), 0);
      printf("Risposta inviata: %s\n", output);
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

    printf("La connessione si è conclusa\n");
  }

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
  printf("Password Ricevuta: %s \n", passwordRicevuta);

  if (strcmp(passwordRicevuta, PASSWORD) != 0)
  {
    generazioneErrore("Password errata: l'operazione non può essere eseguita \n");
  }
  else
  {
    printf("Password Accettata \n");
  }
}

void controlloOutput(int risultato, char *messaggio)
{
  if (risultato == 0)
  {
    generazioneErrore(messaggio);
  }
}

char *visualizzaRubrica()
{

  char output[1000];

  fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio
  for (int i = 0; i < 4 * NUM_RECORD_RUBRICA; i++)
  {
    if (i % 4 == 0)
    {
      strcat(output, "Nome: ");
    }
    if (i % 4 == 1)
    {
      strcat(output, "Cognome: ");
    }
    if (i % 4 == 2)
    {
      strcat(output, "Indirizzo: ");
    }
    if (i % 4 == 3)
    {
      strcat(output, "Telefono: ");
    }
    fgets(output, i * MAX_LUNG_CAMPO, rubrica);
    // strcat(output,"\0");
    strcat(output, "------------------------------------------------------------- \n");
  }

  return output;
}

char *ricercaRecordConCognome(int clientSocket)
{

  char richiestaCognome[] = "Richiesto un cognome per poter effettuare la ricerca\n";
  send(clientSocket, richiestaCognome, sizeof(richiestaCognome), 0);
  char cognomeDaRicercare[MAX_LUNG_CAMPO];
  int byteLetti = recv(clientSocket, cognomeDaRicercare, sizeof(cognomeDaRicercare), 0);
  if (byteLetti < 1)
    generazioneErrore("Cognome non ricevuto/valido");

  char output[1000];

  fseek(rubrica, MAX_LUNG_CAMPO, SEEK_SET); // il puntatore del file viene spostato all'inizio del primo cognome
  for (int i = 0; i < NUM_RECORD_RUBRICA; i++)
  {
    char cognome[MAX_LUNG_CAMPO];
    if(read(rubrica, cognome, MAX_LUNG_CAMPO) == 0){
      generazioneErrore("Errore nella lettura");
    }

    if(strcmp(cognome, cognomeDaRicercare) == 0) {
      char recordTrovato[4 * MAX_LUNG_CAMPO];
      fgets(recordTrovato, sizeof(recordTrovato), rubrica);
      strcat(output, recordTrovato);
      strcat(output, "------------------------------------------------------------- \n");
    }

  }

  return output;
}

char *ricercaRecordConCognomeNome(int clientSocket)
{
  return NULL;
}

int aggiungiRecord(int clientSocket)
{
  return 0;
}

int rimuoviRecord(int clientSocket)
{
  return 0;
}

int modificaTelefono(int clientSocket)
{
  return 0;
}

int modificaIndirizzo(int clientSocket)
{
  return 0;
}
