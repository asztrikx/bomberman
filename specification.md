# pálya
- n*m-es
- négyzetekre osztva
- elpuszíthatatlan blokkok
	- pálya szélein
	- minden második pozicíon (y és x tengelyen egyszerre)
- elpuszítható blokkok
	- random generálva a maradék területen
- járható területek
	- a maradék terület
- kritériumok
	- start pont
		- nem lehetetlen a játék megcsinálása

# karakterek
- user
	- négyzet egység
	- w, a, s, d mozgás
	- képesség
		- bomba lehelyezés
- enemy
	- négyszög egység
	- random mozgás
	- képesség
		- félig a userben megöli

# képességek
- sebesség növelőt
- több bomba lerakót
- bomba hatókör növelő
- távirányítót (utolsó bombát)
- bomba lökőt
- fal átjáró
- bomba átjáró
- ?(bomba immunitás)
- ?(ideiglenes immunitás)

# objektumok (nem átjárhatóak)
- elpuszítható blokkok
	- bomba hatására elpusztulnak
	- tartalmazhatnak képességeket
- bomba
	- felrobbanás
		- x másodperc után
		- l hosszú tüzet okozva
	- felrobbantás
		- player által
		- másik bomba által

# kijárat használata
- összes enemy megölése
- kijárat megtalálása
	- elpuszítható blokk alatt
