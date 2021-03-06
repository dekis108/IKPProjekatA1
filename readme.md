# Dokumentacija

## Uvod

Projektna specifikacija za aplikaciju Publisher / Subscriber navodi razvoj tri različita entiteta, a to su PubSubEngine, Publisher i Subscriber. PubSubEngine je potreban za pružanje usluga klijentima primenom interfejsa od tri funkcije. Prva funkcija - funkcija Connect je namenjena za preslušavanje klijentskih veza. Drugo - Subscribe funkcija je namenjena klijentima da se pretplate na određene teme od interesa. Treća funkcija je funkcija objavljivanja i namenjena je klijentima da pošalju svoje podatke za objavljivanje kako bi se mogli preneti klijentima koji su se pretplatili na dotičnu temu. Postoje dve vrste tema - Analog i Status. Ako je tema Status, tada su dozvoljeni tipovi SVG (rasklopna oprema) i CRB (prekidač). U slučaju Analog teme, dozvoljeni tip je MER (merenje). Izdavači mogu da šalju vrednosti za analogne i digitalne tačke. Pretplatnik treba da potvrdi poruke koje dobije od PubSubEngine-a tako što će proveriti da li vrednost odgovara njegovoj temi - Analogna vrednost ili Digitalna vrednost. Digitalne vrednosti su 0 i 1. Analogne vrednosti su nepotpisane int vrednosti.
Cilj ovog projekta je pokazati kako se arhitektura klijenta / servera može paralelno paralelizovati, koristeći TCP protokol za slanje paketa podataka.

![1.jpg](https://i.postimg.cc/bNjV04FX/1.jpg)

# TCP
TCP je prokotol transportnog sloja koji je connection-oriented, odnosno pre slanja samih podataka potrebno je otvoriti i odrzavati vezu izmedju stranaka. TCP garantuje pouzdan transfer podataka primenom detekcije i retransmisije ispalih paketa. Između ostalih servisa koje nudi, neki su: pouzdanost, efikasna kontrola toka podataka, operisanje u ful-dupleksu (istovremeno slanje i primanje podataka) i multipleksiranje koje omogućava istovremen rad niza procesa sa viših slojeva putem jedne konekcije. TCP vrši transfer podataka kao nestrukturisan niz bajtova koji se identifikuju sekvencom. Ovaj protokol grupiše bajtove u segmente dodeli im broj sekvence, aplikacijama dodeli broj porta i prosledi ih IP protokolu. TCP obezbeđuje pouzdanost pokretanjem algoritama koji pre razmene podataka prvo uspostave konekciju između korisnika, a potom obezbeđuje i niz mehanizama kao što je slanje ACK broja. Strana koja prima podatke šalje broj sekvence bajta koje je primio, u slučaju da destinacija ne pošalje ACK da je primio određenu sekvencu bajtova u određenom vremenskom intervalu ona biva naknadno ponovo poslata. Mehanizmi pouzdanosti kod TCP-a omogućuju uređajima da se nose sa gubicima, kašnjenjima, dupliciranjem ili pogrešnim iščitavanjem paketa. Time-out mehanizam omogućuje uređaju da detektuje izgubljene pakete i da zahteva njihovu ponovnu transmisiju. Ovako izgleda TCP zaglavlje:

[![tcp.jpg](https://i.postimg.cc/xd5r9Zf8/tcp.jpg)](https://postimg.cc/94DgpJqH)

Polja:

    Izvorišni port - dodeljen broj (16 bita), identifikuje aplikaciju koja je inicijator komunikacije
    Odredišni port - port koji identifikuje serversku aplikaciju (16 bita)
    Broj segmenta (SEQ) - redni broj segmenta u odnosu na početni (broj bajta u odnosu na inicijalni)(32 bita)
    Broj sledećeg bajta (ACK) - redni broj bajta poslat predajnoj strani koji očekuje da primi (32 bita)
    Dužina - Dužina zaglavlja
    Rezervisana polja
    URG, ACK, PSH, RST, SYN, FIN predstavljaju kontrolne bite
    Veličina dinamičkog prozora - broj okteta koje je moguće slati bez potvrde o njihovom prijemu
    Čeksuma - Provera bitskih grešaka, komplement sume TCP zaglavlja
    Pokazivač prioriteta (URG) - pokazuje važnost poruke koja se šalje
    Opcije - Opciona informacija
    Podatak - ako postoji opciona informacija bitovi počevši sa 192 predstavljaju podatak, inače od 160. bita

## Arhitektura i dizajn
Sistem se sastoji od tri odvojena entiteta. Prva je servis - PubSubEngine. Servis je pruža usluge dvoma tipova klijenata: Publisher i Subscriber. Opšta arhitektura sistema je klijent - server arhitektura sa N brojem klijenata (izdavača i pretplatnika) i jednom uslugom. Za komunikaciju se koristi protokol TCP. Uvedena je zasebna statička biblioteka - TCPLib i njegove funkcionalnosti koriste se za TCP komunikaciju između komponenti. Model podataka koji se koristi za čuvanje podataka je struktura i generička lista. Generička lista se koristi za čuvanje  paketa i utičnica za klijentske veze u odvojenim listama.

![2.png](https://i.postimg.cc/3JNXMBqS/2.png)

## Thread Pool

U okviru servisa se prilikom inicijalizacije započne rad konfigurisanog broja radnih niti. Ove niti su u aktivnom stanju tokom celog rada servisa. Definisana je struktura lista zadataka koju niti periodično proveravaju i u slučaju da postoji zadatak (dogadjaj na klijentu), nit ga preuzima i započinje njegovo izvršavanje. Tokom preuzimanja novog zahteva niti se moraju sinhronizovati da ne bi došlo do situacije da više niti preuzme na sebe isti zadatak, a nakon preuzetog zadatka je moguće njihovo potpuno paralelno izvršavanje. Ako ne postoji zadatak za neku nit, ona će se pauzirati na neko prekonfigurisano vreme da bi omogućila procesoru da izvrši zamenu konteksta.

## Strukture podataka
### Struktura merenja
Struktura ima tri polja: temu, tip i vrednost. Tip teme je nabrajanje sa vrednostima Analog i Status. Tip tipova je nabrajanje sa vrednostima SVG, MER i CRB. Vrednost je ceo broj.

```c
typedef enum MeasurmentTopic {Analog = 0, Status} Topic;

typedef enum MeasurmentType {SVG = 0, CRB, MER} Type;

typedef struct _msgFormat {
    Topic topic;
    Type type;
    int value;

} Measurment;
```

Datoteka zaglavlja Measurment.h sadrži nekoliko pomoćnih funkcija:
```c
const char * GetStringFromEnumHelper (Topic topic);
const char * GetStringFromEnumHelper (Type type);
void PrintMeasurment (Measurment * m);
```
Prve dve funkcije su pomoćne funkcije pretvarača nabrajanja, a treća je jednostavna funkcija ispisa koja ispisuje vrednosti Measurment strukture.

### Thread Safe Generic Linked List
Ovo je lista koja se može koristiti za čuvanje bilo koje vrste podataka, jer njeni čvorovi sadrže void pokazivač.
Čvor liste ima strukturu:

```c
tipedef struct Node
{
    // Bilo koji tip podataka može biti uskladišten u ovom čvoru
    void * data;
    HANDLE mutex;
    struct Node * next;
} NODE;
```
Mutex koji ima svaki čvor osigurava sigurnost niti na nivou čvora liste. Sigurnost niti na nivou liste se primenjuje korišćenjem kritičnih sekcija u kojima se koriste liste (PubSubEngine). U svakoj pomoćnoj funkciji liste nit će čekati na oslobađanju mutex-a za manipulaciju čvorom.

Implementacija liste gura novi element na početak u O(1) vremenu. 

Podaci koje usluga prima čuvaju se u odvojenim listama na osnovu teme. Lista je korišćena kao struktura podataka jer, kada se podaci čitaju tokom prenosa pretplatnicima, svi podaci će se svakako čitati uzastopno.

### PubSubEngine
Servis namenjen servisiranju izdavačkih / pretplatničkih klijenata. Prihvata veze u niti napravljenoj za slušanje. Veze izdavača čuvaju se na generičkoj listi koja je inicirana za izdavače, a one pretplatničke na generičkoj listi pretplatnika. Nakon predstavljanja klijenata, u zavisnosti od toga koji je tip, počinje logika izdavača / pretplatnika. Za izdavače postoji spremište niti koje prihvata Measurment pakete poslate od različitih izdavača. Paket se zatim čuva u generičkim listama, u zavisnosti od teme koju ima - Analog ili Status.

PubSubEngine nakon pokretanja i konektovanja dva različita klijenta:

[![1ps.jpg](https://i.postimg.cc/W3QwCMD8/1ps.jpg)](https://postimg.cc/wyQNsRQR)

PubSubEngine nakon gasenja:

[![2ps.jpg](https://i.postimg.cc/x1xHBKhm/2ps.jpg)](https://postimg.cc/mP9tHFbZ)


### Publisher
Klijentska komponenta koja je namenjena objavljivanju Measurment paketa. Pre objavljivanja predstavlja se PubSubEngine-u kao Publisher klijent. Omogućen je ručni unos podataka kao i automatsko generisanje. U slučaju automatskog generisanja potrebno je navesti interval generisanja. Neke od automatski generisanih vrednosti namerno ostaju nevažeće, tako da pretplatnik može da koristi validaciju nad njima. Koristi funkciju TCPSend iz TCPLib. Nema trajno čuvanje podataka.

Primer pokrenutog Publisher-a sa nasumično generisanim paketima:

[![pub.jpg](https://i.postimg.cc/50cHMp3X/pub.jpg)](https://postimg.cc/WDnpGGxV)

### Subscriber
Klijentska komponenta koja je namenjena za primanje Measurment paketa koji odgovaraju temi pretplate klijenata. Klijent nakon povezivanja sa PubSubEngine predstavlja se kao pretplatnički klijent. Nakon uvoda bira temu ili teme na koje će se pretplatiti. Za pretplaćenu temu odmah dobija u odgovor sve podatke koje servis već ima. Nakon pretplate stvara nit za primanje paketa. Ova nit osluškuje na klijentskoj utičnici pakete koji se šalju. Za svaki paket Subscriber mora izvršiti validaciju nad njemu i na konzoli napisati njegov status validacije. Nema trajno čuvanje podataka.

Primer pokrenutog Subscriber-a sa primanjem paketa od obe vrste topica:

[![sub.jpg](https://i.postimg.cc/wj73yn4t/sub.jpg)](https://postimg.cc/4YRXj2gZ)

### TCPLib
Statična biblioteka koja ima odvojene funkcije za slanje i primanje na TCP protokolu. TCPSend funkcija ima dve verzije, jedna je namenjena slanju Measurment paketa, a druga je namenjena uvođenju tipa klijenta u uslugu. TCPRecieve je funkcija koja koristi standardnu funkciju recv za primanje bilo kog tipa paketa, jer će TCPRecieve upisati u primljene bajtove u recvbuf - reciveve bafer, a zadatak pozivaoca je da ga prebaci na očekivani tip.

### Common
Common biblioteka sadrži Measurment.h koji sadrži definiciju strukture Measurment i njegove pomoćne funkcije i GenericList.h. Mernu strukturu čini enum MeasurmentTopic koji ima vrednosti Analog i Status, zatim MeasurmentTipe enum koji ima vrednosti SVG, CRB, MER i na kraju celobrojnu vrednost. GenericList sadrži definicije i pomoćne funkcije za Generičku listu koja se sastoji oda ko čvorovji mogu uzeti bilo koji tip podataka da se u njih uskladišti, jer ima void pokazivač u strukturi čvora.

## Funkcionalnost i upotreba
Prvo se mora pokrenuti PubSubEngine. Nakon toga, klijenti mogu da se pokrenu. Nije važno kojim redosledom se započinju klijenti. Nakon pokretanja Publisher-a, podaci o objavljivanju se šalju PubSubEngine-u i putem mehanizma se preusmeravaju na pretplatnike koji su pretplaćeni na temu paketa.

## Testiranje 

Za potrebe testiranja je podešen sistem od 10 Publisher-a koji na intervalu od 5 ms šalju podatak na servis, kao i 6 Subscriber-a kojima je servis te podatke morao prosledjivati.

![t1-perfomance-pocetak-rada.png](https://i.postimg.cc/ydHVmmYC/t1-perfomance-pocetak-rada.png)
![t1-pocetak-rada.png](https://i.postimg.cc/MHZq4zQs/t1-pocetak-rada.png)
Trenutak 1: Stanje na početku rada, servis pauziran na samom početku main funkcije. 

![t2-nakon-inita.png](https://i.postimg.cc/RCGMtJk2/t2-nakon-inita.png)
Trenutak 2: Stanje nakon ušpesne inicijalizacije servisa. Broj niti (Handle count) skače jer se u ovom momenti inicijalizuju sve radne niti.

![t2-5-neki-trenutak-tokom-rada-perfomance.png](https://i.postimg.cc/fTLMfTZy/t2-5-neki-trenutak-tokom-rada-perfomance.png)
Trenutak 3: Ubrzo nakon što su se zakačili klijenti Publisher-i. Servis počinje da zauzima dodatni prostor radi smeštanja novih podataka.

![t3-pauza.png](https://i.postimg.cc/Dw4FRjSK/t3-pauza.png)
![t3-tredovi.png](https://i.postimg.cc/QMLsDQ7K/t3-tredovi.png)
Trenutak 4: Nakon 6 minuta. Uvid u stanja niti. 

![t4-druga-pauza.png](https://i.postimg.cc/ZRHh6XM6/t4-druga-pauza.png)
Trenutak 5: Nakon 26 minuta. Zauzeće memorije zauzima oko 277 mb.

![t5-pocetak-gasenja.png](https://i.postimg.cc/xCn2J9t0/t5-pocetak-gasenja.png)
Trenutak 6: Početak gašenja servisa, klijenti se diskonektuju. 

![t6-ugasen.png](https://i.postimg.cc/Zqrm5WP7/t6-ugasen.png)
Trenutak 7: Završenje Shutdown funkcije. Oslobodjenje radne memorije.

## Zaključak

Mogućnost servisa da opsluzi klijente koji su postavljali zahteve veoma brzo (reda milisekundi) i gotovo konstantno zazeće procesorske moći (od oko 20%) ukazuju na relativno optimizovanu implementaciju servisa. Nakon nagomilane količine prikupljenih podataka na servisu nije se primetila promena u perfomansi. Thread pool omogućava raspodelu posla preko više niti i samimm tim preko više fizičkih procesora. Kao što se može primetiti na slici 'Trenutak 7' nakon poziva Shutdown funkcije sva memorija se uspesno oslobadja, nema tkzv. curenja memorije.
 
## Moguća poboljšanja

Trenutno implementiran servis ne pruža nikakvu administrativnu kontrolu na sledeće predmete koji bi mogli biti od interesa: definisanje kako, gde i koliko da se čuvaju pristigli podaci (kao i korišćenje relacione baze podataka), uvid u stanje klijenata i mogućnost (ne)dozvoljavanja njihovih operacija, oporavak od ispada tokom rada i slično. U realnom sistemu ove funkcionalnosti bi morale postojati. Klijentske aplikacije su minimalističke, služe samo edukativnom korišćenju servisa. Pravi klijentski softver bi morao da se sastoji od tkzv. user-friendly interfejsa.
