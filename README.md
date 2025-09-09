# Potapanje brodova - ORM projekat

[Uputsvo za odbranu](https://www.rt-rk.uns.ac.rs/sites/default/files/materijali/lab/Odbrana%20projektnih%20zadataka%20-%20Uputstvo_0.pdf)

## Zadatak

Potrebno je implementirati TCP klijent-server aplikaciju “podmornice”.

### Pravila igre

Svakom igraču je dodeljena tabela od 8x8 polja, koja predstavlja oblast mora u kojoj su sakrivene igračeve podmornice.
Kolone se obeležavaju slovima abecede od A do H, a vrste brojevima od 1 do 8.
Svaki igrač na početku bitke raspolaže sledećim podmornicama:

- Podmornice veličine 1 polja X 4,
- Podmornice veličine 2 polja X 3.
- Podmornice veličine 3 polja X 2.
- Podmornice veličine 4 polja X 1.

Podmornice je moguće orijentisati samo vertikalno i horizontalno (sva polja koja zauzima jedna podmornica su poravnata duž jedne linije).
Podmornice smeju da se dodiruju. Podmornice su nepokretne u toku igre.
Svaki igrač, pre početka igre, raspoređuje svoje podmornice, tako da protivnički igrač ne vidi njihov raspored.
U toku igre, igrači naizmenično „gađaju“ podmornice drugog igrača, svaki u toku svog poteza.
Gađanje se obavlja navođenjem koordinate u ranije opisanoj tabeli gde igrač pretpostavlja da se nalaze protivnikove podmornice.
U toku jednog poteza igrač gađa sve dok pogađa protivnikove podmornice.
Prvi put kada promaši, na red dolazi drugi igrač.
Gađanje polja koje je već bilo gađano se tretira kao promašaj.
Protivnički igrač je dužan da saopšti da li je podmornica pogođena, ali ne i da li je potopljena,
ukoliko nisu pogođeni svi delovi iste. Igra se završava kada neki od igrača izgubi sve svoje podmornice.
Prikazati trenutno stanje igre u vizuelnom formatu pogodnom za čitanje od strane ljudi, odnosno igrača.
U svakom trenutku treba omogućiti prelazak sa ekrana za igru na ekran sa glavnim menijem
(postavka koordinata podmornica, početak i kraj igre, nastavak započete igre) i obrnuto.

## Kako pokrenuti projekat

1. Pokrenuti `make build`. Ova komanda ce da napravi bin/server.out and bin/client.out programe
2. pokrenuti server `./bin/server.out 9000` 
3. pokrenuti vise klijenata `./bin/client.out 127.0.0.1 9000`
