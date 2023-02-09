# ArduinoLedGames
Progetto di Sistemi Embedded che porta un sistema in grado di far giocare a 3 minigiochi tramite l'utilizzo di 4 led con delle rispettive fotoresistenze. Il circuito, con anche l'eventuale simulazione (sconsigliata per la gestione delle fotoresistenze) è disponibile al seguente link: [Tinkercad](https://www.tinkercad.com/things/5BtaArf9GgU?sharecode=XZpf0lvjkRXBo49i21X9vVDXMpyeQTdTWDjt0rkrPhM)

## Hit That Led
Obiettivo del gioco è coprire la fotoresistenza proposta dallo schermo lcd (numerate da 1 a 4) nel momento in cui il corrispettivo led è acceso. I led si accenderanno in sequenza, uno dopo l'altro. Man mano che si andrà avanti, la velocità di cambiamento tra un led e l'altro aumenterà.

![alt text](https://github.com/maurotella/ArduinoLedGames/blob/main/gif/HTL.gif?raw=true)

## Memory Game
Obiettivo del gioco è osservare la sequenza di led casuale proposta, ricordarsela e indicarla. La lunghezza della sequenza sarà crescente man mano che si andrà avanti. Per indicare la sequenza si dovrà coprire la fotoresistenza, relativa al led che si vuole indicare, finchè non uscirà la scritta "Giusto" o "Sbagliato" per poi ripetere la stessa operazione fino al completamento della sequenza.

![alt text](https://github.com/maurotella/ArduinoLedGames/blob/main/gif/MG.gif?raw=true)

## Acchiappa Il Led
Obiettivo del gioco è coprire più fotoresistenze possibili quando il relativo led è attivo. I led si accenderanno e spegneranno in maniera casuale a velocità casuale. Il gioco durerà 15 secondi.

![alt text](https://github.com/maurotella/ArduinoLedGames/blob/main/gif/AIL.gif?raw=true)
