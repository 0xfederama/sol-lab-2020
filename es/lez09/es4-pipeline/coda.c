#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <coda.h>

//Implementazione coda FIFO con struct a primo e ultimo elemento per rendere piu veloce il processo


coda* initCoda (coda*q) {
	q=malloc(sizeof(coda));
	q->first=NULL;
	q->last=NULL;
	q->size=0;
	return q;
}

void push (coda*q, char *str) {
	node *ins=malloc(sizeof(node));
	ins->info=str;
	ins->repeat=1;
	ins->next=NULL;
	if (q->first==NULL) {
		q->first=ins;
		q->last=ins;
	} else {
		(q->last)->next=ins;
		q->last=ins;
	}
	(q->size)++;
}

char* pop (coda*q) {
	node*del=q->first;
	if (q->first==NULL || q->last==NULL) {
		return NULL;
	}
	char*info=del->info;
	if (q->first==q->last) {
		q->first=NULL;
		q->last=NULL;
	} else {
		q->first=(q->first)->next;
	}
	(q->size)--;
	free(del);
	return info;
}

int updateRepeat(coda*q, char*str) {
	node*finder=q->first;
	while (finder!=NULL) {
		if (finder->info!=EOS) {
			if (strcmp(finder->info, str)==0) {
				(finder->repeat)++;
				int rep=finder->repeat;
				return rep;
			}
		} else {
			printf("EOS\n");
			break;
		}
		
		finder=finder->next;
	}
	free(finder);
	return 0;
}

void deleteCoda (coda*q) {
	node *killer=q->first;
	while (killer!=NULL) {
		node*tmp=killer;
		killer=killer->next;
		free(tmp);
	}
	free(killer);
	free(q);
}

void stampaCoda (coda*q) {
	node* printer=q->first;
	printf("Le parole che compaiono una sola volta sono:\n");
	while (printer!=NULL) {
		if (printer->info!=EOS) {
			if (printer->repeat==1) printf("%s\n", printer->info);
		} else break;
		printer=printer->next;
	}
	free(printer);
}