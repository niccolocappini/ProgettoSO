#ifndef FUNZIONI_CLIENTI_H
#define FUNZIONI_CLIENTI_H

#include "definizioniComuni.h"

void visualizzazioneRubrica();
void ricercaRecordCognome(char cognome[]);
void ricercaRecordNomeCognome(char nome[], char cognome[]);
void aggiungiRecord(recordRub record);
void rimuoviRecord(recordRub record);
void modificaTelefono(char vecchioTelefono[], char nuovoTelefono[]);
void modificaIndirizzo(char vecchioIndirizzo[], char nuovoIndirizzo[]);

#endif
