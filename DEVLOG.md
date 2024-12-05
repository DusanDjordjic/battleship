# Implementacija - Pocetna ideja

Napraviti TCP server koji se brine o igracima, trenutnim igrama i rezultatima. TCP server za pocetak treba da bude jednostavan,
koristicemo jedan thread za svaku konekciju sa nekim "globalnim" stanjem servera u kome cemo da cuvamo trenutno konektovane
igrace, igre koje trenutno traju, rezultate igara itd.

Sa client strane cemo da napravimo TUI koristeci ncurses da bi smo dobili sto fukncionalniji UI.

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

# DEVLOG

# 03.12.2024

- Napravljena pocetna struktura projekta
- Dodat make test target za automatsko testiranje koda kao i github action koji ca da build-uje kod i testira ga nakon svakog commit-a
- Dodan ncurses i hello world example program

# 04.12.2024

- Ipak necemo jos da koristimo ncurses
- Naprljen meni sistem sa vise stranica
- Napravljen mock sistem za konektovanje na server, signup, login, logout
- TODO: dodati testove za meni
- TODO: napraviti novi thread za prihvatanje poruka i SIGNINT handler. Koristiti epool za proveu da li je user uneo nesto.
