# Progetto-2020-2021

Alessio Minoi, matricola 899161

Compilazione: make
Compilazione ed esecuzione: make run

INIZIALIZZAZIONE:
Il master alloca in memoria condivisa una struct maps_config->my_mp che contiene i vari dati estratti dal file "init.conf" che verrano utilizzati dalle varie struct quali Taxi, Source, Maps. Successivamente alloca in memoria condivisia la struct maps come un array di dimensione base*altezza (valori estratti da "init.conf") di caselle, alloca una coda di messaggi, utilizzata dai processi per comunicare tra loro grazie a un tipo di messaggio specifico per tipo di comunicazione TAXI_TO_MASTER, SOURCE_TO_MASTER, TAXI_TO_SOURCE, SOURCE_TO_TAXI. 
per la sincroniccazione vengono utilizzati 2 set di semafori, il semaforo "sem_sync" viene utilizzato per la sincronizzazione tra il master e i taxi per avviare il movimento dei taxi; il "sem_id" viene applicato uno su ogni singola cella della mappa, in questo modo si potrà gestire la quantità di taxi al suo interno (quantità presa da init.conf).
Tutti gli id precedentemente ottenuti con le chiamate (gc_id_shm, mp_id_shm,so_id_shm,tx_id_shm,key_id_shm,sem_sync,msgq_id_sm, msgq_id,msgq_id_so,msgq_id_ds,msgq_id_end) per la generazione di ipcs sono inseriti in una struct keys_storage->my_ks in memoria condivisa in modo da essere disponibili a tutti i processi in ogni momento.

Prima della generazione dei vari processi taxi e source, viene utilizzata la funzione randomize_hole() dove varie caselle della mappa saranno bloccate e non accessibili impostando il sem_id a 0.

GENERAZIONE DEI PROCESSI FIGLIO:
terminata l'inizializzazione, il master fa un fork() ed esegue un execve() per generare i processi taxi, che verranno posizionati radnomicamente su tutta la mappa (funzione randomize_taxi). Terminata la generazione da parte del master i taxi faranno un attach alla memoria grazie agli id presenti nel keys_storage (consentita con key_id_shm) condivisa attraverso le variabili passate dal Master con args_tx[] nell'execve. 
Viene fatta la stessa cosa nel caso dei processi Source,il master fa un fork() ed esegue un execve() per generare i processi source, posizionati randomicamente all'interno della mappa(grazie alla funzione randomize_source()). Terminata la generazione da parte del master i source faranno un attach alla memoria grazie agli id presenti nel keys_storage (consentita con key_id_shm) condivisa attraverso le variabili passate dal Master con args_so[] nell'execve. 
A questo punto il master aspetta che i processi source e taxi siano inizializzati.
Viene richiesto un input da tastiera per decidere quante richieste Source leggere nella coda di messaggi.

INIZIO:
Ogni secondo verra stampata la mappa segnalando la posizione dei processi taxi e delle source.
Si avvia un timer per terminare il ciclo.
mentre i processi Taxi attendono di essere sbloccati, il master invia un segnale di tipo SIGUSR1 ai processi Source che invieranno una richiesta nella coda di messaggi dedicata tra il source e il master; il master legge la coda di messaggio e inserisce la richiesta in un array pos_source, che successivamente sarà utilizzato dalla funzione compute_target() per decidere quale sarà il Taxi più vicino e con meno tragitto per raggiungere la richiesta, modificando il valore target per i Taxi che sono stati scelti. 
Al termine della funzione compute_target, il master sblocca con un WAIT e poi con un START i processi Taxi per andare alla ricerca delle source designate dal target!=-1 provanda a muoversi, all'interno della mappa, fino a raggiungere la source, nel caso in cui il target==-1 i Taxi rimangono fermi dove si trovano; tutti i Taxi si bloccano su END.  
I processi taxi che hann un target!=-1 possono provare a raggiungere la source designata, possono esserci 3 possibili messaggi che il taxi invierà al master:
    - messaggio = 1 nel caso in cui il Taxi è riuscito a raggiungere la source nel modo corretto e portare la richiesta a destinazione, master continua il proprio ciclo
    - messaggio = -1 nel caso in cui il taxi non sia riuscito a raggiungere la source ed è rimasto fermo, per un certo tempo superiore al timeout, nella stessa casella, così il Taxi effettua un detach dalla memoria condivisa e muore con un exit(), il master genera un processo Taxi con la funzione create_new_taxi() in una posizione differente dal Taxi morto, master continua il proprio ciclo
    - messaggio = 0 nel caso in cui il taxi sia riuscito a raggiungere la richiesta, ma non è riuscito a raggiungere la destinazione desiderata dalla richiesta ed è rimasto fermo, per un certo tempo superiore al timeout, nella stessa casella, così il Taxi effettua un detach dalla memoria condivisa e muore con un exit(), il master genera un processo Taxi con la funzione create_new_taxi() in una posizione differente dal Taxi morto, master continua il proprio ciclo

Tutti i Taxi si mettono in attesa END.


A questo punto il master attende di ricevere tanti messaggi dai Taxi quante sono le richieste estratte dalla coda dei messaggi, infine attende che tutti i processi Taxi siano sincronizzati sul semaforo END.
Il ciclo riprende con la lettura di nuove richieste Source, compute_target e continua fino a quando non termina il timer di 20 secondi.

NOTE:
Ogni taxi ha un proprio timer che si avvia allo START del master,ad ogni movimento questo master viene resettato, nel caso in cui non avvenga nessun movimento per più di un certo tempo si attiva la funzione timecheck() che andrà ad inviare i vari messaggi in base al target!=-1 e dest!=-1 e farà morire il taxi. Nel caso in cui il taxi abbia i valori target==-1 e dest==-1, allora ho deciso di non avvisare il master con segnali, in modo da evitare possibili perdite di sengali e non generare tanti nuovi taxi quanti sono quelli morti a causa del timeout, cosi ho deciso di fare una semplice modifica della variabile my_pid=-1 in modo tale da poter riconoscere chi è morto e chi no. Così quando rinizia il ciclo, al di fuori della sincronizzazione, il master utilizza la funzione check_taxi che genererà tanti taxi quanti sono quelli con my_pid=-1.
