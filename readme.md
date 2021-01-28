# Dokumentacija

## Uvod
Projektna specifikacija za aplikaciju Publisher / Subscriber navodi razvoj tri različita entiteta, a to su PubSubEngine, Publisher i Subscriber. PubSubEngine je potreban za pružanje usluga klijentima primenom interfejsa od tri funkcije. Prva funkcija - funkcija Connect je namenjena za preslušavanje klijentskih veza. Drugo - Subscribe funkcija je namenjena klijentima da se pretplate na određene teme od interesa. Treća funkcija je funkcija objavljivanja i namenjena je klijentima da pošalju svoje podatke za objavljivanje kako bi se mogli preneti klijentima koji su se pretplatili na dotičnu temu. Postoje dve vrste tema - Analogn i Status. Ako je tema Status, tada su dozvoljeni tipovi SVG (rasklopna oprema) i CRB (prekidač). U slučaju Analog teme, dozvoljeni tip je MER (merenje). Izdavači mogu da šalju vrednosti za analogne i digitalne tačke. Pretplatnik treba da potvrdi poruke koje dobije od PubSubEngine-a tako što će proveriti da li vrednost odgovara njegovoj temi - Analogna vrednost ili Digitalna vrednost. Digitalne vrednosti su 0 i 1. Analogne vrednosti su nepotpisane int vrednosti.
Cilj ovog projekta je pokazati kako se arhitektura klijenta / servera može paralelno paralelizovati, koristeći TCP protokol za slanje paketa podataka.

[! [1.jpg] (https://i.postimg.cc/bNjV04FKS/1.jpg)] (https://postimg.cc/RkKST4pBG)

TCP je prokotol transportnog sloja koji je connection-oriented, odnosno pre slanja samih podataka potrebno je otvoriti i odrzavati vezu izmedju stranaka. TCP garantuje pouzdan transfer podataka primenom detekcije i retransmisije ispalih paketa. 

## Arhitektura i dizajn
Sistem se sastoji od tri odvojena entiteta. Prva je servis - PubSubEngine. Servis je pruža usluge dvoma tipova klijenata: Publisher i Subscriber. Opšta arhitektura sistema je klijent - server arhitektura sa N brojem klijenata (izdavača i pretplatnika) i jednom uslugom. Za komunikaciju se koristi protokol TCP. Uvedena je zasebna statička biblioteka - TCPLib i njegove funkcionalnosti koriste se za TCP komunikaciju između komponenti. Model podataka koji se koristi za čuvanje podataka je struktura i generička lista. Generička lista se koristi za čuvanje  paketa i utičnica za klijentske veze u odvojenim listama.

[! [2.png] (https://i.postimg.cc/3JNKSMBkS/2.png)] (https://postimg.cc/56cILvbL)

## Logika niti
Pokrenuta je nit za preslušavanje kako bi mogla da preuzme veze klijenta i sačuva ih na listama klijenata.
Spiskovi klijenata su:
`` c
NODE * publisherList = NULL;
NODE * subscriberList = NULL;
``
/// DEJO OPISATI

## Strukture podataka
### Struktura merenja
Struktura ima tri polja: temu, tip i vrednost. Tip teme je nabrajanje sa vrednostima Analog i Status. Tip tipova je nabrajanje sa vrednostima SVG, MER i CRB. Vrednost je ceo broj.
`` c
tipedef enum MeasurmentTopic {Analog = 0, Status} Tema;

tipedef enum MeasurmentTipe {SVG = 0, CRB, MER} Tip;

tipedef struct _msgFormat {
    Tema teme;
    Tipe tipe;
    int value;

} Merenje;
``
Datoteka zaglavlja Measurment.h sadrži nekoliko pomoćnih funkcija:
`` c
const char * GetStringFromEnumHelper (Tema teme);
const char * GetStringFromEnumHelper (tip tipa);
void PrintMeasurment (merenje * m);
``
Prve dve funkcije su pomoćne funkcije pretvarača nabrajanja, a treća je jednostavna funkcija ispisa koja ispisuje vrednosti Measurment strukture.

### Thread Safe Generic Linked List
Ovo je lista koja se može koristiti za čuvanje bilo koje vrste podataka, jer njeni čvorovi sadrže void pokazivač.
Čvor liste ima strukturu:
`` c
tipedef struct Node
{
    // Bilo koji tip podataka može biti uskladišten u ovom čvoru
    void * data;
    HANDLE mutex;
    struct Node * next;
} NODE;
``
Mutex koji ima svaki čvor osigurava sigurnost niti na nivou čvora liste. Sigurnost niti na nivou liste se primenjuje korišćenjem kritičnih sekcija u kojima se koriste liste (PubSubEngine). U svakoj pomoćnoj funkciji liste nit će čekati na oslobađanju mutex-a za manipulaciju čvorom.

Implementacija liste gura novi element na početak u O(1) vremenu. 

Podaci koje usluga prima čuvaju se u odvojenim listama na osnovu teme. Lista je korišćena kao struktura podataka jer, kada se podaci čitaju tokom prenosa pretplatnicima, svi podaci će se svakako čitati uzastopno.

### PubSubEngine
Servis namenjen servisiranju izdavačkih / pretplatničkih klijenata. Prihvata veze u niti napravljenoj za slušanje. Veze izdavača čuvaju se na generičkoj listi koja je inicirana za izdavače, a one pretplatničke na generičkoj listi pretplatnika. Nakon predstavljanja klijenata, u zavisnosti od toga koji je tip, počinje logika izdavača / pretplatnika. Za izdavače postoji spremište niti koje prihvata Measurment pakete poslate od različitih izdavača. Paket se zatim čuva u generičkim listama, u zavisnosti od teme koju ima - Analog ili Status. *opisi thread pool*

### Publisher
Klijentska komponenta koja je namenjena objavljivanju Measurment paketa.
