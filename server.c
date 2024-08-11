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

static FILE *rubrica = NULL;
static int recordContenuti = NUM_RECORD_RUBRICA; // da aggiornare ad ogni aggiunta ed eliminazione

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

  int serverSocket, clientSocket;
  struct sockaddr_in indirizzo;
  int lunghezzaIndirizzo = sizeof(indirizzo);

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

  printf("Menù delle operazioni che possono essere richieste dal client: \n"
         "1) Visualizzazzione tutti i record della rubrica \n"
         "2) Ricerca record tramite cognome \n"
         "3) Ricerca record tramite coppia nome-cognome \n"
         "4) Aggiunta Record \n"
         "5) Eliminazione Record \n"
         "6) Modifica Numero di Telefono \n"
         "7) Modifica Indirizzo \n\n");

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
      printf("Sono un figlio generato per gestire la richiesta del client\n");

      // Login utente
      int richiesta = 0;
      int risultato = recv(clientSocket, &richiesta, sizeof(richiesta), 0);
      if (risultato < 1)
      {
        generazioneErrore("Lettura fallita\n");
      }

      printf("Lettura effettuata: %d\n", richiesta);

      risultato = -1;
      char *output;                                                                                          // stringa che contterrà ciò che verrà inviato al client
      int dimensione = recordContenuti * 4 * MAX_LUNG_CAMPO + 4 * recordContenuti * 2 + recordContenuti * 2; // nel caso pessimo si devono stampare tutti i record
      switch (richiesta)
      {

      case VISUALIZZA_OGNI_RECORD:
        printf("Gestione Richiesta 1\n");
        output = (char *)malloc(dimensione);
        visualizzaRubrica(&output);

        break;

      case RICERCA_RECORD_CON_COGNOME:
        printf("Gestione Richiesta 2\n");
        output = (char *)malloc(dimensione);
        ricercaRecordConCognome(clientSocket, &output);

        break;

      case RICERCA_RECORD_CON_NOME_COGNOME:
        printf("Gestione Richiesta 3\n");
        output = (char *)malloc(dimensione);
        ricercaRecordConCognomeNome(clientSocket, &output);

        break;

      case AGGIUNGI_RECORD:
        printf("Gestione Richiesta 4: \n");
        richiestaPassword(clientSocket);
        risultato = aggiungiRecord(clientSocket, &output);
        controlloOutput(clientSocket, risultato, output); 
    
        break;

      case RIMUOVI_RECORD:
        printf("Gestione Richiesta 5: \n");
        richiestaPassword(clientSocket);
        risultato = rimuoviRecord(clientSocket, &output);
        controlloOutput(clientSocket, risultato, "Rimozione Record in Rubrica Fallita \n");

        break;

      case MODIFICA_TELEFONO:
        printf("Gestione Richiesta 6: \n");
        richiestaPassword(clientSocket);
        risultato = modificaTelefono(clientSocket, &output);
        controlloOutput(clientSocket, risultato, "Modifica Telefono Fallita \n");

        break;

      case MODIFICA_INDIRIZZO:
        printf("Gestione Richiesta 7: \n");
        richiestaPassword(clientSocket);
        risultato = modificaIndirizzo(clientSocket, &output);
        controlloOutput(clientSocket, risultato, "Modifica Indirizzo Fallita \n");

        break;

      default:
        generazioneErrore("Richiesta non valida\n");
      }

      // Invio della risposta al client
      write(clientSocket, output, strlen(output) + 1);
      printf("Risposta inviata: \n%s \n", output); // da togliere prima della consegna
      printf("Richiesta eseguita: terminazione di questo figlio\n\n");
      close(clientSocket);
      // free(output);
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
  char passwordRicevuta[MAX_LUNG_PASSWORD];
  recv(clientSocket, passwordRicevuta, sizeof(passwordRicevuta), 0);
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

void controlloOutput(int clientSocket, int risultato, char *messaggio)
{
  if (risultato == 0)
  {
    write(clientSocket, messaggio, strlen(messaggio) + 1);
    generazioneErrore(messaggio);
  }
}

void riceviDatiDaClient(int clientSocket, char *datoDaRicevere, int dimensioneDato, char *messaggio)
{
  int byteLetti;

  byteLetti = recv(clientSocket, datoDaRicevere, dimensioneDato, 0);
  if (byteLetti < 1)
  {
    generazioneErrore(messaggio);
  }
}

int controlloRubricaVuota(char **output) // restituisce 0 se la rubrica è vuota e in tal caso scrive già in output la stringa corretta
{
  fseek(rubrica, 0, SEEK_END);
  long int posizioneFinale = ftell(rubrica);
  if (posizioneFinale == 0)
  {
    strcat(*output, "La rubrica al momento è vuota\n");
    return 0;
  }
  return 1;
}

void normalizzaRecord(recordRub *recordDaAggiungere)
{
}

int ricercaRecord(recordRub *recordDaRicercare) // metodo generale di ricerca record da usare nelle ricerche settoriali
{
  fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio
  int recordTrovato = 0;
  char campoLetto[MAX_LUNG_CAMPO];
  for (int i = 0; i < recordContenuti; i++)
  {
    recordTrovato = 0;
    for (int j = 0; j < 4; j++)
    {
      if (fread(campoLetto, MAX_LUNG_CAMPO, 1, rubrica) == 0)
      {
        generazioneErrore("Errore nella lettura\n");
      }

      if (j == 0 && strcmp(campoLetto, (const char *)recordDaRicercare->nome) == 0)
      {
        recordTrovato++;
      }

      if (j == 1 && strcmp(campoLetto, (const char *)recordDaRicercare->cognome) == 0)
      {
        recordTrovato++;
      }

      if (j == 2 && strcmp(campoLetto, (const char *)recordDaRicercare->indirizzo) == 0)
      {
        recordTrovato++;
      }

      if (j == 3 && strcmp(campoLetto, (const char *)recordDaRicercare->telefono) == 0)
      {
        recordTrovato++;
      }
    }

    if (recordTrovato == 4)
    {
      return 1;
    }
  }
  return 0; // si ritorna 0 se il record non è presente, 1 se il record è presente
}

void visualizzaRubrica(char **output)
{
  if (controlloRubricaVuota(output) != 0)
  {
    char supporto[MAX_LUNG_CAMPO];
    int i = 0;
    int contatore = 0;

    fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio del file
    while (1)
    {
      i = fread(supporto, MAX_LUNG_CAMPO, 1, rubrica);
      if (i <= 0)
      {
        break;
      }

      strcat(*output, supporto);
      if (contatore % 4 != 3)
      {
        strcat(*output, ", ");
      }
      else
      {
        strcat(*output, "\n");
      }

      contatore++;
    }
  }
}

/* Casi di errore: Record non Trovato*/
void ricercaRecordConCognome(int clientSocket, char **output)
{
  if (controlloRubricaVuota(output) != 0)
  {
    char cognomeDaRicercare[MAX_LUNG_CAMPO];

    printf("In attesa del cognome da ricercare... \n");
    riceviDatiDaClient(clientSocket, cognomeDaRicercare, sizeof(cognomeDaRicercare), "Cognome non ricevuto o non valido\n");

    fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio
    int recordTrovato;
    char recordCorrente[4 * MAX_LUNG_CAMPO + 100];
    char campoLetto[MAX_LUNG_CAMPO];

    for (int i = 0; i < recordContenuti; i++)
    {
      recordTrovato = 1;
      strcpy(recordCorrente, "");
      for (int j = 0; j < 4; j++)
      {
        if (fread(campoLetto, MAX_LUNG_CAMPO, 1, rubrica) == 0)
        {
          generazioneErrore("Errore nella lettura\n");
        }

        strcat(recordCorrente, campoLetto);
        if (j != 3)
        {
          strcat(recordCorrente, ", ");
        }

        if (j == 1 && strcmp(campoLetto, cognomeDaRicercare) == 0)
        {
          recordTrovato = 0;
        }
      }

      if (recordTrovato == 0)
      {
        strcat(*output, recordCorrente);
        strcat(*output, "\n");
      }
    }

    if (strlen(*output) == 0)
    {
      strcat(*output, "Nella rubrica non è presente nessun record con il cognome ");
      strcat(*output, cognomeDaRicercare);
      strcat(*output, "\n");
    }
  }
}

/* Casi di errore: Record non Trovato*/
void ricercaRecordConCognomeNome(int clientSocket, char **output)
{
  if (controlloRubricaVuota(output) != 0)
  {

    char nomeDaRicercare[MAX_LUNG_CAMPO];
    char cognomeDaRicercare[MAX_LUNG_CAMPO];

    printf("In attesa del nome da ricercare... \n");
    riceviDatiDaClient(clientSocket, nomeDaRicercare, sizeof(nomeDaRicercare), "Nome non ricevuto o non valido\n");

    printf("In attesa del cognome da ricercare... \n");
    riceviDatiDaClient(clientSocket, cognomeDaRicercare, sizeof(cognomeDaRicercare), "Cognome non ricevuto o non valido\n");

    fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio
    int recordTrovato = 0;
    char recordCorrente[4 * MAX_LUNG_CAMPO + 100];
    char campoLetto[MAX_LUNG_CAMPO];
    for (int i = 0; i < recordContenuti; i++)
    {
      recordTrovato = 0;
      strcpy(recordCorrente, "");
      for (int j = 0; j < 4; j++)
      {
        if (fread(campoLetto, MAX_LUNG_CAMPO, 1, rubrica) == 0)
        {
          generazioneErrore("Errore nella lettura\n");
        }

        strcat(recordCorrente, campoLetto);
        if (j != 3)
        {
          strcat(recordCorrente, ", ");
        }

        if (j == 0 && strcmp(campoLetto, nomeDaRicercare) == 0)
        {
          recordTrovato++;
        }

        if (j == 1 && strcmp(campoLetto, cognomeDaRicercare) == 0)
        {
          recordTrovato++;
        }
      }

      if (recordTrovato == 2)
      {
        strcat(*output, recordCorrente);
        strcat(*output, "\n");
      }
    }

    if (strlen(*output) == 0)
    {
      strcat(*output, "Nella rubrica non è presente nessun record con nome-cognome ");
      strcat(*output, nomeDaRicercare);
      strcat(*output, " ");
      strcat(*output, cognomeDaRicercare);
      strcat(*output, "\n");
    }
  }
}

/* Casi di errore: Aggiunta non riuscita*/
int aggiungiRecord(int clientSocket, char **output)
{
  recordRub recordDaAggiungere;
  int byteLetti,byteScritti;

  printf("In attesa del record da inserire... \n");

  byteLetti = recv(clientSocket, &recordDaAggiungere, sizeof(recordDaAggiungere), 0);
  if (byteLetti < 1)
  {
    generazioneErrore("Record non ricevuto o non valido\n");
  }

  if(ricercaRecord(&recordDaAggiungere) == 1)
  {
    *output = "Record Presente in Rubrica\n";
    return 0;
  }

  normalizzaRecord(&recordDaAggiungere);

  fseek(rubrica, 0, SEEK_END);
  byteScritti = fwrite(&recordDaAggiungere, sizeof(recordDaAggiungere), 1, rubrica);
  if(byteScritti <= 0)
  {
    *output = "Aggiunta Record Fallita\n";
    return 0;
  }
  *output = "Aggiunta Record andata a buon fine\n";
  recordContenuti++;
  return 1;
}

/* Casi di errore: Eliminazione non riuscita
  Gestire il ricompattamento del file dopo l'eliminazione del record*/
int rimuoviRecord(int clientSocket, char **output)
{
  if (controlloRubricaVuota(output) != 0)
  {
    recordRub recordDaRimuovere;
    char recordStr[4 * MAX_LUNG_CAMPO];

    printf("In attesa del record da inserire... \n");
    riceviDatiDaClient(clientSocket, recordStr, sizeof(recordStr), "Record non ricevuto o non valido\n");

    printf("%s", recordStr);
    fseek(rubrica, 0, SEEK_END);

    long int posizioneRecordDaRimuovere = ricercaRecord(&recordDaRimuovere);
    if (posizioneRecordDaRimuovere < 0)
      return 0;

    fseek(rubrica, posizioneRecordDaRimuovere, 0);

    for (int i = 0; i < 4; i++)
    {
      fwrite("\0", MAX_LUNG_CAMPO, 1, rubrica);
    }

    recordContenuti--;
    return 1;
  }
}

/* Casi di errore: vecchioTelefono non trovato, modifica non riuscita*/
int modificaTelefono(int clientSocket, char **output)
{
  if (controlloRubricaVuota(output) != 0)
  {
    char telefonoVecchio[MAX_LUNG_CAMPO];

    long int posizioneRecordDaModificare = ricercaPosizioneRecordConTelefono(telefonoVecchio);
    if (posizioneRecordDaModificare < 0)
    {
      return 0;
    }
    printf("Il record da modificare è il %ldesimo \n", posizioneRecordDaModificare / (4 * MAX_LUNG_CAMPO) + 1);
    fseek(rubrica, posizioneRecordDaModificare, SEEK_SET);
    char telefonoNuovo[MAX_LUNG_CAMPO];
    riceviDatiDaClient(clientSocket, telefonoNuovo, sizeof(telefonoNuovo), "Errore nella ricezione del nuovo numero di telefono\n");

    // modifica del record

    return 0;
  }
}

long int ricercaPosizioneRecordConTelefono(char *telefonoDaRicercare)
{

  fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio
  int recordTrovato = 1;
  char campoLetto[MAX_LUNG_CAMPO];

  for (int i = 0; i < recordContenuti; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      if (fread(campoLetto, MAX_LUNG_CAMPO, 1, rubrica) == 0)
      {
        generazioneErrore("Errore nella lettura\n");
      }

      if (j == 4 && strcmp(campoLetto, telefonoDaRicercare) == 0)
      {
        recordTrovato = 0;
      }
    }

    if (recordTrovato == 0)
    {
      return ftell(rubrica) - 4 * MAX_LUNG_CAMPO; // la sottrazione è perché il record è stato letto, quindi il puntatore è stato spostato alla sua fine
    }
  }
}

/* Casi di errore: vecchioIndirizzo non trovato, modifica non riuscita*/
int modificaIndirizzo(int clientSocket, char **output)
{
  if (controlloRubricaVuota(output) != 0)
  {
    char indirizzoVecchio[MAX_LUNG_CAMPO];

    // ricerca indirizzo vecchio

    char indirizzoNuovo[MAX_LUNG_CAMPO];
    riceviDatiDaClient(clientSocket, indirizzoNuovo, sizeof(indirizzoNuovo), "Errore nella ricezione del nuovo indirizzo\n");
    return 0;
  }
}