#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long isNumber(const char* s) {
   char* e = NULL;
   long val = strtol(s, &e, 0);
   if (e != NULL && *e == '\0') return val; 
   return -1;
}

int main (int argc, char *argv[]) {
	if (argc==1) {
		printf("nessun argomento passato alla funzione\n");
		return 0;
	}
	int foundn=0, foundm=0, founds=0;
	int argn=-1, argm=-1; char*args=NULL;
	int i=0;
	for (i=1; i<argc; i++) {
		char *str=strdup(argv[i]);
		int j=0;
		while (str[j]=='-') j++;
		
		switch (str[j]) {
			case 'n': {
				//Ho trovato n, se ho qualcosa di attaccato e quello e' un numero allora lo salvo per stamparlo, altrimenti se non e' un numero do un errore. Se non ho niente attaccato, con un nuovo indice controllo l'elemento successivo e se e' un numero ok altrimenti break. Setto foundn a 1 e copio il valore in argn. 
				foundn=1; j++;
				int lenrimasta=strlen(str)-j;
				//Se la lunghezza rimasta e' >0 allora guardo quello che c'e dopo e se e' un int lo salvo in argn, se invece e' 0, significa che dopo non ho niente e devo controllare l'argv successivo
				if (lenrimasta>0) {
					char*aux=(char*)malloc((1+lenrimasta)*sizeof(char));
					strncpy(aux, str+j, lenrimasta);
					aux[lenrimasta+1]='\0';
					argn=isNumber(aux);
				} else { //Controllo l'argv successivo
					if (i+1 == argc) break;
					char*str2=strdup(argv[i+1]);
					char*aux=(char*)malloc((1+strlen(str2))*sizeof(char));
					strcpy(aux, str2);
					aux[strlen(str2)+1]='\0';
					argn=isNumber(aux);
					if (argn!=-1) i++;
				}
				
				if (argn==-1) { //Se quello successivo non e' un numero do un errore
					foundn=0;
					printf ("argomento n non valido\n");
				}
			} break;
						
			case 'm': {
				foundm=1; j++;
				int lenrimasta=strlen(str)-j;
				//Se la lunghezza rimasta e' >0 allora guardo quello che c'e dopo e se e' un int lo salvo in argn, se invece e' 0, significa che dopo non ho niente e devo controllare l'argv successivo
				if (lenrimasta>0) {
					char*aux=(char*)malloc((1+lenrimasta)*sizeof(char));
					strncpy(aux, str+j, lenrimasta);
					aux[lenrimasta+1]='\0';
					argn=isNumber(aux);
				} else { //Controllo l'argv successivo									
					if (i+1 == argc) break;
					char*str2=strdup(argv[i+1]);
					char*aux=(char*)malloc((1+strlen(str2))*sizeof(char));
					strcpy(aux, str2);
					aux[strlen(str2)+1]='\0';
					argm=isNumber(aux);
					if (argm!=-1) i++;
				}
				
				if (argm==-1) { //Se quello successivo non e' un numero do un errore
					foundm=0;
					printf ("argomento n non valido\n");
				}
			} break;
			
			case 's': {
				founds=1; j++;
				int lenrimasta=strlen(str)-j;
				if (lenrimasta>0) {
					char*aux=(char*)malloc((1+lenrimasta)*sizeof(char));
					strncpy(aux, str+j, lenrimasta);
					aux[lenrimasta+1]='\0';
					args=aux;
				} else {
					args=argv[i+1];
					i++;
				}
			} break;
			
			case 'h': {
				printf("usage: %s -n <numero> -s <stringa> -m <numero> -h\n", argv[0]+2);
				return 0;
			} break;
			
			default : 
			printf("argomento %c non riconosciuto\n", str[j]);
		}
	}
	
	if (foundn) printf("-n: %d\n", argn);
	if (foundm) printf("-m: %d\n", argm);
	if (founds) printf("-s: %s\n", args);
	
	return 0;
}










