#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

#include "definizioniComuni.h"
#include "funzioniServer.h"

#define PASSWORD "ProgettoSO"

static FILE *rubrica = NULL;
static int recordContenuti = NUM_RECORD_RUBRICA; // è obbligatorio avviare generatoreRubrica prima di accettare le richieste

static int continuaEsecuzione;
static int idProcessoPadre;

int main()
{
  signal(SIGINT, handle_sigint);
  idProcessoPadre = getpid();

  int serverSocket, clientSocket;
  struct sockaddr_in indirizzo;
  int lunghezzaIndirizzo = sizeof(indirizzo);

  // Creazione del socket
  serverSocket = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
  if (serverSocket < 0)
    generazioneErrore("Creazione socket fallita \n");

  // Configurazione di "indirizzo" per poter effettuare il binding correttamente
  indirizzo.sin_family = AF_INET;
  indirizzo.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  indirizzo.sin_port = htons(PORTA);

  // Binding del socket alla porta specificata in "indirizzo"
  if (bind(serverSocket, (struct sockaddr *)&indirizzo, lunghezzaIndirizzo) < 0)
    generazioneErrore("Errore durante il binding \n");

  listen(serverSocket, 10);

  rubrica = fopen("RubricaDB", "w+");
  if (rubrica == NULL)
    generazioneErrore("Rubrica non aperta correttamente \n");

  printf("Menù delle operazioni che possono essere richieste dal client: \n"
         "1) Visualizzazione tutti i record della rubrica \n"
         "2) Ricerca record tramite cognome \n"
         "3) Ricerca record tramite coppia nome-cognome \n"
         "4) Aggiunta Record \n"
         "5) Eliminazione Record \n"
         "6) Modifica Indirizzo \n"
         "7) Modifica Numero di Telefono \n\n");

  continuaEsecuzione = 1;
  while (continuaEsecuzione == 1)
  {

    printf("Server in ascolto sulla porta %d \n", PORTA);

    // Accetta una connessione in entrata
    if ((clientSocket = accept(serverSocket, (struct sockaddr *)&indirizzo, (socklen_t *)&lunghezzaIndirizzo)) < 0)
      generazioneErrore("Errore durante l'accettazione di una richiesta \n");

    printf("Accettata connessione da un client \n");

    // Gestione richiesta tramite fork()
    int pid = fork();

    if (pid == 0)
    {
      printf("Sono un figlio generato per gestire la richiesta del client \n");

      // Login utente
      int richiesta;
      if (recv(clientSocket, &richiesta, sizeof(richiesta), 0) < 1)
        generazioneErrore("Lettura della richiesta fallita \n");

      printf("Lettura effettuata: %d \n", richiesta);

      char *output;                                                                                                 // stringa che conterrà ciò che verrà inviato al client
      int dimensioneMassima = recordContenuti * 4 * MAX_LUNG_CAMPO + 4 * recordContenuti * 2 + recordContenuti * 2; // nel caso pessimo si devono stampare tutti i record
      int risultato;
      switch (richiesta)
      {

      case VISUALIZZA_OGNI_RECORD:
        printf("Gestione Richiesta 1: \n");
        output = (char *)malloc(dimensioneMassima);
        visualizzaRubrica(&output);

        break;

      case RICERCA_RECORD_CON_COGNOME:
        printf("Gestione Richiesta 2: \n");
        output = (char *)malloc(dimensioneMassima);
        ricercaRecordConCognome(clientSocket, &output);

        break;

      case RICERCA_RECORD_CON_NOME_COGNOME:
        printf("Gestione Richiesta 3: \n");
        output = (char *)malloc(dimensioneMassima);
        ricercaRecordConNomeCognome(clientSocket, &output);

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
        controlloOutput(clientSocket, risultato, output);

        break;

      case MODIFICA_INDIRIZZO:
        printf("Gestione Richiesta 6: \n");
        richiestaPassword(clientSocket);
        risultato = modificaIndirizzo(clientSocket, &output);
        controlloOutput(clientSocket, risultato, output);

        break;

      case MODIFICA_TELEFONO:
        printf("Gestione Richiesta 7: \n");
        richiestaPassword(clientSocket);
        risultato = modificaTelefono(clientSocket, &output);
        controlloOutput(clientSocket, risultato, output);

        break;

      default:
        generazioneErrore("Richiesta non valida \n");
      }

      // Invio della risposta al client

      while (write(clientSocket, output, strlen(output) + 1) < 0)
      {
      }

      char statoServer[MAX_LUNG_MESSAGGIO];
      if(continuaEsecuzione == 0)
        strcpy(statoServer, "Esecuzione del server interrotta: ulteriori richieste non verranno soddisfatte \n");
      else
        strcpy(statoServer, "Server in esecuzione: in attesa di ulteriori richieste \n");

      while (write(clientSocket, statoServer, strlen(statoServer) + 1) < 0)
      {
      }
      printf("Risposta inviata \n"); 
      printf("Richiesta eseguita: terminazione di questo figlio \n\n");
      close(clientSocket);

      exit(EXIT_SUCCESS);
    }
    else
    {
      int status = 0;
      printf("Sono il processo che continua ad accettare richieste \n");
      close(clientSocket);
      wait(&status);
    }

    printf("La connessione si è conclusa \n\n");
  }

  printf("Esecuzione terminata \n");
  return 0;
}

void richiestaPassword(int clientSocket)
{
  char passwordRicevuta[MAX_LUNG_PASSWORD];
  if (recv(clientSocket, passwordRicevuta, sizeof(passwordRicevuta), 0) < 0)
    generazioneErrore("Lettura Password Fallita \n");
  printf("Password Ricevuta: %s \n", passwordRicevuta);

  char rispostaPassword[MAX_LUNG_MESSAGGIO];
  if (strcmp(passwordRicevuta, PASSWORD) != 0)
  {
    strcpy(rispostaPassword, "Password Errata \n");
    while (send(clientSocket, rispostaPassword, MAX_LUNG_MESSAGGIO, 0) < 0)
    {
    }
    generazioneErrore("Password errata: l'operazione non può essere eseguita \n");
  }
  else
  {
    strcpy(rispostaPassword, "Password Corretta \n");
    while (send(clientSocket, rispostaPassword, MAX_LUNG_MESSAGGIO, 0) < 0)
    {
    }
    printf("Password Accettata \n");
  }
}

void controlloOutput(int clientSocket, int risultato, char *messaggio)
{
  if (risultato == ESITO_NEGATIVO)
  {
    while (write(clientSocket, messaggio, strlen(messaggio) + 1) < 0)
    {
    }
    generazioneErrore(messaggio);
  }
}

void riceviCampoDaClient(int clientSocket, char *datoDaRicevere, int dimensioneDato, char *messaggioDiErrore)
{
  if (recv(clientSocket, datoDaRicevere, dimensioneDato, 0) < 1)
  {
    while (write(clientSocket, messaggioDiErrore, strlen(messaggioDiErrore) + 1) < 0)
    {
    }
    generazioneErrore(messaggioDiErrore);
  }
}

void riceviRecordDaClient(int clientSocket, recordRub *recordDaRicevere, int dimensioneRecord, char *messaggioDiErrore)
{
  if (recv(clientSocket, recordDaRicevere, dimensioneRecord, 0) < 1)
  {
    while (write(clientSocket, messaggioDiErrore, strlen(messaggioDiErrore) + 1) < 0)
    {
    }
    generazioneErrore(messaggioDiErrore);
  }
}

int controlloRubricaVuota(char **output) // restituisce -1 se la rubrica è vuota e in tal caso scrive già in output la stringa corretta
{
  fseek(rubrica, 0, SEEK_END);
  long int posizioneFinale = ftell(rubrica);
  if (posizioneFinale == 0)
  {
    *output = "La rubrica al momento è vuota \n";
    return ESITO_NEGATIVO;
  }
  return posizioneFinale;
}

int normalizzaNomeCognome(char campo[MAX_LUNG_CAMPO])
{
  char carattere;
  for (int i = 0; i < MAX_LUNG_CAMPO; i++)
  {
    carattere = campo[i];
    if (carattere == '\0')
    {
      break;
    }
    if (isalpha(carattere) == 0 && isspace(carattere) == 0)
    {
      return ESITO_NEGATIVO;
    }

    if (i == 0 || isspace(campo[i - 1]))
    {
      campo[i] = toupper(carattere);
    }
    else
    {
      campo[i] = tolower(carattere);
    }
  }
  return 0;
}

int normalizzaIndirizzo(char campo[MAX_LUNG_CAMPO])
{
  char carattere;
  for (int i = 0; i < MAX_LUNG_CAMPO; i++)
  {
    carattere = campo[i];
    if (carattere == '\0')
    {
      break;
    }
    if (isalpha(carattere) == 0 && isspace(carattere) == 0 && isdigit(carattere) == 0)
    {
      return ESITO_NEGATIVO;
    }

    if (i == 0 || isspace(campo[i - 1]))
    {
      campo[i] = toupper(carattere);
    }
    else
    {
      campo[i] = tolower(carattere);
    }
  }
  return 0;
}

int normalizzaTelefono(char campo[MAX_LUNG_CAMPO])
{
  char carattere;
  for (int i = 0; i < MAX_LUNG_CAMPO; i++)
  {
    carattere = campo[i];
    if (carattere == '\0')
    {
      break;
    }
    if (isdigit(carattere) == 0)
    {
      return ESITO_NEGATIVO;
    }
  }
  return 0;
}

int normalizzaRecord(recordRub *record)
{
  int risultato = normalizzaNomeCognome((*record).nome);
  if (risultato == ESITO_NEGATIVO)
  {
    return ESITO_NEGATIVO;
  }

  risultato = normalizzaNomeCognome((*record).cognome);
  if (risultato == ESITO_NEGATIVO)
  {
    return ESITO_NEGATIVO;
  }

  risultato = normalizzaIndirizzo((*record).indirizzo);
  if (risultato == ESITO_NEGATIVO)
  {
    return ESITO_NEGATIVO;
  }

  risultato = normalizzaTelefono((*record).telefono);
  if (risultato == ESITO_NEGATIVO)
  {
    return ESITO_NEGATIVO;
  }

  return 0;
}

long int ricercaRecord(recordRub *recordDaRicercare)
{
  long int posizioneRecord;

  fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio
  int recordTrovato = 0;
  char campoLetto[MAX_LUNG_CAMPO];
  int uscita = 0;
  while (uscita == 0)
  {
    recordTrovato = 0;
    for (int j = 0; j < 4; j++)
    {
      if (j == 0)
        posizioneRecord = ftell(rubrica);

      if (fread(campoLetto, MAX_LUNG_CAMPO, 1, rubrica) < 1)
      {
        uscita = 1;
        break;
      }

      if (j == 0 && strcmp(campoLetto, recordDaRicercare->nome) == 0)
        recordTrovato++;

      if (j == 1 && strcmp(campoLetto, recordDaRicercare->cognome) == 0)
        recordTrovato++;

      if (j == 2 && strcmp(campoLetto, recordDaRicercare->indirizzo) == 0)
        recordTrovato++;

      if (j == 3 && strcmp(campoLetto, recordDaRicercare->telefono) == 0)
        recordTrovato++;
    }

    if (recordTrovato == 4)
      return posizioneRecord;
  }
  return ESITO_NEGATIVO; // si ritorna -1 se il record non è presente, altrimenti la posizione del record
}

void visualizzaRubrica(char **output)
{

  if (controlloRubricaVuota(output) == ESITO_NEGATIVO)
    return;

  char supporto[MAX_LUNG_CAMPO];
  int i = 0;
  int contatore = 0;
  char fineStringa[MAX_LUNG_CAMPO];

  fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio del file
  while (1)
  {
    i = fread(supporto, MAX_LUNG_CAMPO, 1, rubrica);
    if (i < 1)
      break;

    if (strcmp(supporto, "\0") != 0)
    {
      strcat(*output, supporto);
      if (contatore % 4 != 3)
        strcat(*output, ", ");

      else
      {
        strcat(*output, "\n");
      }
    }
    contatore++;
  }
}

/* Casi di errore: Record non Trovato*/
void ricercaRecordConCognome(int clientSocket, char **output)
{
  char cognomeDaRicercare[MAX_LUNG_CAMPO];

  printf("In attesa del cognome da ricercare... \n");
  riceviCampoDaClient(clientSocket, cognomeDaRicercare, sizeof(cognomeDaRicercare), "Cognome non ricevuto o non valido \n");

  if (controlloRubricaVuota(output) == ESITO_NEGATIVO)
    return;

  fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio
  int recordTrovato;
  char recordCorrente[4 * MAX_LUNG_CAMPO + 100];
  char campoLetto[MAX_LUNG_CAMPO];
  int uscita = 0;
  while (1)
  {
    recordTrovato = 1;
    strcpy(recordCorrente, "");
    for (int j = 0; j < 4; j++)
    {
      if (fread(campoLetto, MAX_LUNG_CAMPO, 1, rubrica) < 1)
      {
        uscita = 1;
        break;
      }

      strcat(recordCorrente, campoLetto);
      if (j != 3)
        strcat(recordCorrente, ", ");

      if (j == 1 && strcmp(campoLetto, cognomeDaRicercare) == 0)
        recordTrovato = 0;
    }

    if (recordTrovato == 0)
    {
      strcat(*output, recordCorrente);
      strcat(*output, "\n");
    }

    if (uscita == 1)
      break;
  }

  if (strlen(*output) == 0)
  {
    strcat(*output, "Nella rubrica non è presente nessun record con il cognome ");
    strcat(*output, cognomeDaRicercare);
    strcat(*output, "\n");
  }
}

/* Casi di errore: Record non Trovato*/
void ricercaRecordConNomeCognome(int clientSocket, char **output)
{
  char nomeDaRicercare[MAX_LUNG_CAMPO];
  char cognomeDaRicercare[MAX_LUNG_CAMPO];

  printf("In attesa del nome da ricercare... \n");
  riceviCampoDaClient(clientSocket, nomeDaRicercare, sizeof(nomeDaRicercare), "Nome non ricevuto o non valido \n");

  printf("In attesa del cognome da ricercare... \n");
  riceviCampoDaClient(clientSocket, cognomeDaRicercare, sizeof(cognomeDaRicercare), "Cognome non ricevuto o non valido \n");

  if (controlloRubricaVuota(output) == ESITO_NEGATIVO)
    return;

  fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio
  int recordTrovato = 0;
  char recordCorrente[4 * MAX_LUNG_CAMPO + 100];
  char campoLetto[MAX_LUNG_CAMPO];
  int uscita = 0;
  while (1)
  {
    recordTrovato = 0;
    strcpy(recordCorrente, "");
    for (int j = 0; j < 4; j++)
    {
      if (fread(campoLetto, MAX_LUNG_CAMPO, 1, rubrica) < 1)
      {
        uscita = 1;
        break;
      }

      strcat(recordCorrente, campoLetto);
      if (j != 3)
        strcat(recordCorrente, ", ");

      if (j == 0 && strcmp(campoLetto, nomeDaRicercare) == 0)
        recordTrovato++;

      if (j == 1 && strcmp(campoLetto, cognomeDaRicercare) == 0)
        recordTrovato++;
    }

    if (recordTrovato == 2)
    {
      strcat(*output, recordCorrente);
      strcat(*output, "\n");
    }

    if (uscita == 1)
      break;
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

/* Casi di errore: Aggiunta non riuscita*/
int aggiungiRecord(int clientSocket, char **output)
{
  recordRub recordDaAggiungere;

  printf("In attesa del record da inserire... \n");
  riceviRecordDaClient(clientSocket, &recordDaAggiungere, sizeof(recordDaAggiungere), "Record non ricevuto o non valido \n");

  if (normalizzaRecord(&recordDaAggiungere) == ESITO_NEGATIVO)
  {
    *output = "Record Formattato Scorrettamente \n";
    return ESITO_NEGATIVO;
  }

  printf("Record da aggiungere: %s, %s, %s, %s \n", recordDaAggiungere.nome, recordDaAggiungere.cognome, recordDaAggiungere.indirizzo, recordDaAggiungere.telefono);

  if (ricercaRecord(&recordDaAggiungere) != ESITO_NEGATIVO)
  {
    *output = "Record già presente in Rubrica \n";
    return ESITO_NEGATIVO;
  }

  fseek(rubrica, 0, SEEK_END);

  if (fwrite(&recordDaAggiungere, sizeof(recordDaAggiungere), 1, rubrica) < 1)
  {
    *output = "Aggiunta Record Fallita \n";
    return ESITO_NEGATIVO;
  }

  *output = "Aggiunta Record andata a buon fine \n";
  recordContenuti++;
  return 0;
}

/* Casi di errore: Eliminazione non riuscita*/
int rimuoviRecord(int clientSocket, char **output)
{
  recordRub recordDaRimuovere;

  printf("In attesa del record da rimuovere... \n");
  riceviRecordDaClient(clientSocket, &recordDaRimuovere, sizeof(recordDaRimuovere), "Record non ricevuto o non valido \n");

  if (controlloRubricaVuota(output) == ESITO_NEGATIVO)
  {
    return ESITO_NEGATIVO;
  }

  if (normalizzaRecord(&recordDaRimuovere) == ESITO_NEGATIVO)
  {
    *output = "Record Formattato Scorrettamente \n";
    return ESITO_NEGATIVO;
  }

  printf("Record da rimuovere: %s, %s, %s, %s \n", recordDaRimuovere.nome, recordDaRimuovere.cognome, recordDaRimuovere.indirizzo, recordDaRimuovere.telefono);

  long int posizioneRecordDaRimuovere = ricercaRecord(&recordDaRimuovere);
  if (posizioneRecordDaRimuovere < 0)
  {
    *output = "Record non trovato \n";
    return ESITO_NEGATIVO;
  }

  printf("Record trovato: inizio rimozione... \n");

  fseek(rubrica, posizioneRecordDaRimuovere, SEEK_SET);

  char fineStringa[4 * MAX_LUNG_CAMPO] = "\0";
  if (fwrite(fineStringa, 4 * MAX_LUNG_CAMPO, 1, rubrica) < 1)
  {
    *output = "Rimozione Record Fallita \n";
    return ESITO_NEGATIVO;
  }

  *output = "Rimozione Record andata a buon fine \n";
  recordContenuti--;
  return 0;
}

int modificaCampoRubrica(int clientSocket, char **output, int campoScelto)
{
  recordRub recordDaModificare;
  char nuovoValore[MAX_LUNG_CAMPO];
  char *messaggioDiErrore = "Errore nella ricezione del nuovo valore \n";

  printf("In attesa del record da modificare... \n");

  riceviRecordDaClient(clientSocket, &recordDaModificare, sizeof(recordDaModificare), "Record non ricevuto o non valido \n");

  riceviCampoDaClient(clientSocket, nuovoValore, sizeof(nuovoValore), messaggioDiErrore);

  if (controlloRubricaVuota(output) == ESITO_NEGATIVO)
    return ESITO_NEGATIVO;

  if (normalizzaRecord(&recordDaModificare) == ESITO_NEGATIVO)
  {
    *output = "Record Formattato Scorrettamente \n";
    return ESITO_NEGATIVO;
  }

  printf("Record da modificare: %s, %s, %s, %s \n", recordDaModificare.nome, recordDaModificare.cognome, recordDaModificare.indirizzo, recordDaModificare.telefono);

  long int posizioneRecordDaModificare = ricercaRecord(&recordDaModificare);
  if (posizioneRecordDaModificare < 0)
  {
    *output = "Record non trovato \n";
    return ESITO_NEGATIVO;
  }

  printf("Record da modificare presente in rubrica \n");

  char nomeCampo[10];
  switch (campoScelto)
  {
  case 3:
    if (normalizzaIndirizzo(nuovoValore) == ESITO_NEGATIVO)
    {
      *output = "Record Formattato Scorrettamente \n";
      return ESITO_NEGATIVO;
    }
    strcpy(nomeCampo, "Indirizzo");

    break;

  case 4:
    if (normalizzaTelefono(nuovoValore) == ESITO_NEGATIVO)
    {
      *output = "Record Formattato Scorrettamente \n";
      return ESITO_NEGATIVO;
    }
    strcpy(nomeCampo, "Telefono");

    break;

  default:
    generazioneErrore("Campo scelto non valido \n");
  }

  printf("Nuovo %s: %s \n", nomeCampo, nuovoValore);

  if (modificaCampoRecord(posizioneRecordDaModificare, campoScelto, nuovoValore) < 1)
  {
    strcat(*output, nomeCampo);
    strcat(*output, " non modificato \n");
    return ESITO_NEGATIVO;
  }

  *output = "Modifica andata a buon fine \n";
  return 0;
}

/* Casi di errore: vecchioIndirizzo non trovato, modifica non riuscita*/
int modificaIndirizzo(int clientSocket, char **output)
{
  return modificaCampoRubrica(clientSocket, output, 3);
}

/* Casi di errore: vecchioTelefono non trovato, modifica non riuscita*/
int modificaTelefono(int clientSocket, char **output)
{
  return modificaCampoRubrica(clientSocket, output, 4);
}

int modificaCampoRecord(int posizioneRecordDaModificare, int campoScelto, char *nuovoValore)
{
  fseek(rubrica, posizioneRecordDaModificare + (campoScelto - 1) * MAX_LUNG_CAMPO, SEEK_SET);
  return fwrite(nuovoValore, MAX_LUNG_CAMPO, 1, rubrica);
}

void handle_sigint(int sig) 
{
  continuaEsecuzione = 0;
  if (idProcessoPadre == getpid())
    printf("\nSegnale di interruzione rilevato dal processo padre: il processo terminerà dopo l'esecuzione di un'ultima richiesta \n");
  else
    printf("\nSegnale di interruzione rilevato dal processo figlio: arresto dell'esecuzione al termine della gestione della richiesta attuale \n");
}
