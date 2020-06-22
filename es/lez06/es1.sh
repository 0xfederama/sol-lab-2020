#!/bin/bash

if [ $# -ne 1 ]; then                        # Controllo se il numero di argomenti passati e' !=0
    echo usa: $(basename $0) nomedirectory   # Stampo l'uso dello script
    exit -1
fi
dir=$1
if [ ! -d $dir ]; then                       # Controllo che la directory esista e sia una directory
    echo "L'argomento $dir non e' una directory"   
    exit 1;   
fi

bdir=$(basename $dir)
if [ -w $bdir ]; then                     	# il file esiste ed e' scrivibile
    echo -n "il file $bdir.tar.gz esiste gia', sovrascriverlo (S/N)?"
    read yn                                 # Leggo in input
    if [ x$yn != x"S" ]; then               # Se la risposta non e' S esco, senno' sovrascrivo
          exit 0;
    fi
    rm -f $bdir.tar.gz
fi
echo "creo l'archivio con nome $bdir.tar.gz"

tar cf $bdir.tar $dir 2>> ./error.txt       # appende l’output sullo std-error nel file error.txt   
if [ $? -ne 0 ]; then                         # controlla che il comando sia andato a buon fine
    echo "Errore nella creazione dell'archivio"
    exit 1
fi
gzip $bdir.tar  2>> ./errori.txt            # appende l’output sullo std-error nel file error.txt
if [ $? -ne 0 ]; then                         # controlla che il comando sia andato a buon fine
    echo
    echo "Errore nella compressione dell'archivio"
    exit 1
fi

echo "archivio creato con successo, il contenuto dell’archivio e':"
tar tzvf $bdir.tar.gz  2>&1           # redirige lo std-error sullo std-output
exit 0

