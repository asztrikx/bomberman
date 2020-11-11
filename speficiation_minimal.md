# 1. Feladat (Rövid)
Az eredeti bomberman játék alapkoncepcióját utánzó programról van szó.
A játékos a pályán tud mozogni, robbanó tölteteket lerakni, ellenségek erőket vagy falakat robbantani ezáltal.

A játékot lehetőség van bármikor lementeni és visszatölteni, vagy akár újat generálni.

A játékot parancssorból lehet elindítani, onnatól a játékmenet lényegi része grafikus.

# 2. Specifikáció
# Paraméterek
Ahol nem specifikált paraméterek vannak ott a játék élvezhetőségének legoptimálisabb értékeket kell megtalálni. Ilyen például a négyzetméret, ami a játék minden objektumára, karakterére értendő méret

# Indítás
A játék indítása után egy páratlanszor páratlan dimenziójú pályára kerülünk.

# Megjelenítés
A játék természetesen grafikus. A képernyő közepén mindig a játékos legyen, amíg él.

# Pálya (objektumok)
A pálya széleit elpusztíthatatlan objektumok veszik körül (benne van a méretben). A pálya belsejét nézve, ha azt négyzetekre osztva elképzeljük, akkor minden páros számú pozíción ((1,1) -től indítva a számozást) szintén egy elpuszíthatatlan objektum található. Ezen kívül x arányában véletlenszerűen találhatóak elpusztítható blokkok. Az objektumok átjárhatatlanok. Ez alól kivétel a játékos saját bombája, akkor ha annak lerakása után még nem ment ki alóla, illetve a tűz (halál). 2 karakter átmehet egymáson ha az egyik játékos a másik meg ellenség (halál).

# Játékos (karakter)
Indítás után egy nem foglalt, négyzet mezőre kerül a pályán belül. Mozogni a w,a,s,d billentyűkkel tud (fel, balra, le, jobbra). Szintén képes a space billentyűvel bombákat lehelyezni az alatta legközelebb lévő négyzet egységre. Egyszerre csak egy bomba lehet a pályán. A bomba x időn belül felrobban, majd egy 3x3-as + jelben tüzet hagy maga után y ideig. Ha a tűz érint egy elpuszíthatatlan blokkot, vagy karaktert, akkor az elpusztul.

# Ellenségek (karakterek)
A pályában x mennyiségű ellenség van kezdetben, akik mozognak egy irányba. Véletlenszerűen, de nem túl gyakran váltanak irányt. Az irányok lehetnek: jobbra, balra, fel, le. Ezek közül csak az egyik érvényesül és sebességük megegyező méretű mindig.

# Cél
Cél a kijárat megtalálása, ami az egyik elpusztítható objektum alatt lesz. Ebbe akkor lehet belemenni és megnyerni a játékot, ha már minden ellenség meghalt. A játék végét egy sárga képernyő jelzi. Ha a karaktert tűz vagy ellenség éri, akkor a játék végét egy piros képernyő jelzi.

# Kilépés
A játékból az ablakot bezárva lehet kilépni.

# Kinézet
Az egyes objektum típusok kinézetét külön képekből töltse be a program, melyek a program mellett legyenek egy mappában. Bár az objektumok méreteti előre meg lettek adva, azonban ezek csak az ütközés feltételei, a képek lehetnek akármekkora arányban átlátszóak. Egyéb megkötés nincs a kinézetre, ezt nem feladat megtervezni (de lehet).