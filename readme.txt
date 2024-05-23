Buliga Theodor Ioan
323 CA

Tema 2 - Aplicatie client-server TCP si UDP pentru gestionarea mesajelor

Descriere:
Tema implementeaza o aplicatie ce respecta modelul client-server pentru 
gestionarea mesajelor. In implementarea temei se regasesc trei fisiere: 
unul pentru server, unul pentru client si unul pentru functiile ajutatoare 
folosite si structurile de date auxiliare.

Am definit o structura care sa ma ajute sa tin grupati socketii udp 
si tcp pentru fisierul server si care ma va ajuta mai departe sa 
modularizez codul. Urmeaza apoi structurile pentru datele trimise 
clientului tcp, respectiv udp. Apoi am definit structura de pentru 
client si cea pentru pachet. Apoi, urmeaza cateva functii ajutatoare, 
pe care le voi detalia odata cu server.c, respecitv subscriber.c.

In implementarea serverului, creez prima oara cei doi socketi, tcp 
si udp. Setez portul si adresele, fac bind-ul si apoi listen in cazul
in care un client se conecteaza. Creez 
multimea de file descriptori, caut maximul si apoi fac selectul.
Verific daca am primit o cerere de conexiune noua de la un client tcp.
In functie de tipul la care se incadreaza, nou, reconectat sau online 
si fac afisarile dorite. In cazul in care este reconectat trimit pachetele 
primite cat timp a fost inactiv. Daca nu exista client conectat creez unul 
nou si il adaug la vectorul de clienti. Altfel, inchid socketul.
Daca primesc o cerere de tip udp, verific ce tip de date am primit si fac
afisarile. Verific apoi, daca am primit input de la stdin. Daca am primit, 
analizez si de ce tip este. Daca primesc exit voi iesi din bucla
si voi inchide serverul. Daca primesc subscribe voi adauga topicul 
in vectorul de topic al clientului. Daca primesc unsubscribe, 
elimin topicul din vector.

Pentru subscriber, folosesc doar un socket tcp, il initializez aproape 
la fel ca in subscriber(aici nu am mai folosit structura pentru cei doi
socketi), pornesc conexiunea cu serverul, iar apoi voi trimite catre 
server id-ul clientului care se va conecta. Verific intr-o
bucla continua mesajele primite de  la stdin, daca sunt "exit", 
"subscribe" sau "unsubscribe" si trimit mesajele specifice fiecarui caz. 
Pentru exit, voi inchide si bucla, iar pentru celelalte 2 cazuri am ales 
sa modularizez putin, impartirea in tokeni fiind aproape similara. Daca 
primesc un mesaj de la server, voi afisa portul, topic-ul, tipul de date 
si informatia(valoarea mesajului) in conformitate cu cerinta.

Comentarii asupra temei:
Mi s-a parut o tema destul de grea, totusi ceva mai usoara decat prima tema.
Laboratoarele m-au ajutat destul de mult si le-am si folosit din plin. 
Checker-ul mi s-a parut putin dificil de folosit si cel putin local, 
am mai avut probleme. Per total, a fost o tema interesanta la care am 
lucrat cu placere.

Resurse / Bibliografie:
Laboratoarele de PCOM