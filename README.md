# Potapanje brodova - ORM projekat

[Uputsvo za odbranu](https://www.rt-rk.uns.ac.rs/sites/default/files/materijali/lab/Odbrana%20projektnih%20zadataka%20-%20Uputstvo_0.pdf)

## Zadatak

Potrebno je implementirati TCP klijent-server aplikaciju “podmornice”.

### Pravila igre

Svakom igraču je dodeljena tabela od 3x3 polja, koja predstavlja oblast mora u kojoj su sakrivene igračeve podmornice.
Kolone se obeležavaju slovima abecede od A do C, a vrste celim brojevima od 1 do 3.
Svaki igrač na početku bitke raspolaže sledećim podmornicama:

- podmornica veličine 1 polja,
- podmornica veličine 2 polja.

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

## Implementacija - Pocetna ideja

Napraviti TCP server koji se brine o igracima, trenutnim igrama i rezultatima. TCP server za pocetak treba da bude jednostavan,
koristicemo jedan thread za svaku konekciju sa nekim "globalnim" stanjem servera u kome cemo da cuvamo trenutno konektovane
igrace, igre koje trenutno traju, rezultate igara itd.

Sa client strane cemo da napravimo TUI koristeci nccurses da bi smo dobili sto fukncionalniji UI.

Sa strane klijenta igranje same igre bi izgledalao nekako ovako:

1. Login - kada se client konektuje na server treba da napravi usera tako sto ce da izabere svoje ime koje ce biti sacuvano na serveru.
2. Pozove nekog od drugih igraca da duel ili ga neko pozove na duel
3. Kada prihvati duel ili izazvani igrac prihvati duel igra pocinje
4. Kada jedan od igraca pobedi igra se zavrsava i oni mogu da izadju iz igre ili da igraju ponovo
5. Rezultati svake partije se cuvaju radi rang liste.

Sa strane servera to bi izgledalo ovako:

1. Prihvatamo konekcije svih klijenata i radimo login sa njima
2. Kada neko izazove nekog drugog na duel i duel bude prihvacen pravimo Game objekat koji dalje koristimo
3. Nakon zavrsetka igre cuvamo rezultate i nastavljamo dalje
