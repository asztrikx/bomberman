# Állapot
A specifikációban meghatározott dolgok közül a pálya lementése, ellenséges erők és a kijárat nincs megvalósítva. Tekintettel a projekt méretére a főbb függvények leírásán megyek végig. A belső működés megtalálható a kommentekben.

# Felépítés
- type
	> Itt található minden típushoz kötődő dolog.  
	  > A célja, hogy az objektumok létrehozása és felszabadítása ne ismétlődjön, azok alapértelmezett értékei egységesek legyenek.  
	  > Előnye az, hogy hibákat is ki lehet iktatni, ha csak egy helyen kell helyesen megírni a dolgokat
- resource
	> Itt találhatóak a betöltendő képek a típusnak megfelelő mappában 0-tól kezdődő indexeléssel, png formátumban
- main
	> Ez tartalmazza a modulok indításának vezérlését
- client
	> Ez az egység felelős a megjelenítésért, illetve a bemenetek kezeléséért
- server
	> Ez az egység felelős a pálya létrehozásáért, illetve annak következő állapotának kiszámításáért.
- SDL
	> Itt találhatóak az SDL-hez kötődő dolgok.
- geometry
	> Itt minden a pályához szerkezetéhez kapcsolódot függvények tartoznak.
- config
	> A nevéből is adódóan itt a játék működését lehet szabályozni.
- network
	> Bár nincs valódi hálózat a játékban, de későbbi fejlesztést megkönnyíti egy absztrakt modul. Itt lényegében a client és a server modul közötti konverzió történik.

# Függvény leírás
- type
	- ...New()
		> Ezek olyan függvények, amik az adott típust hozzák létre
	- ...Delete(Type* type)
		> Ezek az előbb említett függvények által létrehozott memóriaterületeket szabadítják fel
	- list
		> Generikus lista függvényei
	- array
		> Generikus tömb függvényei
- client
	- ClientStart
		> Modult inicializálja
	- ClientStop
		> Modult leállítja
	- ClientDraw
		> A pályát kirajzolja
	- ClientDraw*
		> A ClientDraw alfüggvényei
	- clientDrawCharacterFind
		> generikus lista segédfüggvénye
	- ClientConnect
		> regisztrálja magát a server modulnál, hogy a pálya állapotváltozásáról kapjon éretsítést
	- ClientEventKey
		> Billentyű eventeket regisztrálja
	- ClientTick
		> a client "órajele", ez értesíti a szervert a jelenleg lenyomott billentyűkről
- server
	- ServerStart
		> Modult inicializálja
	- ServerStop
		> Modult leállítja
	- ServerReceive
		> A client modul éretesítéseit fogadja
	- ServerConnect
		> A client felcsatlakozását fogadja
	- serverTick
		> A client "órajele", ennek az alfüggvényei fogják kiszámítani a pálya új állapotát, illetve a clienteket értesíteni erről
	- fireDestroy
		> Minden a megadott tűzobjektummal érintkező objektumot töröl. Ha nem tűzobjektum akkor nem csinál semmit.
	- bombExplode
		> A megadott bombát felrobbantja, helyére tüzet rak. Ha nem bomba objektum akkor nem csinál semmit.
	- ServerAuthCreate
		> Létrehoz egy véletlenszerű méretben biztonságos azonosítót
	- ServerAuthFind
		> Megkeresi az authnak megfelelő UserServer-t
	- keyBomb
		> A karakternél megnézi, hogy akar-e bombát lerakni és ha igen megteszi.
	- keyMovement
		> A karakternél megnézi, hogy akar-e mozogni és ha igen megteszi.
	- keyMovementCollisionDetect*
		> Segédfüggvény a collision eldöntésére
	- characterFind*
		> Generikus listában kereséshez segédfüggvény
	- worldGenerate
		> Létrehoz a feladatnak megfelelő pályát
- network
	- networkConnectServer
		> A client felcsatlakozását a serverhez valósítja meg
	- networkSendServer
		> Client serverhez való adatküldését valósítja meg
	- networkSendClient
		> Server clienthez való adatküldését valósítja meg
	- *Start, *Stop
		> Absztrakt függvények
- geometry
	- SpawnGet
		> A paramétereknek megfelelő négyzetrácsban lévő pozíciót ad vissza.
	- CollisionFreeCountObjectGet*
		> A pozícióból elérhető szabad pozíciót számolja meg
	- CollisionPoint
		> 2 pont ütközését állapítja meg
	- CollisionPointAll*Get
		> Az adott objektum más objektumokkal való ütközését állapítja meg. Kivétel az objektum és a paraméterben megadott függvény által kezelt esetek.
	- CollisionLinePositionGet
		> Két pozíció között a vektor által megadott irány alapján az ütközéseket figyelembe véve megállapítja hol van a valós pozíció
