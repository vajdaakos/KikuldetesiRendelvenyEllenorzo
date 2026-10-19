# KR

A program kiküldetési rendelvény PDF-eket tölt be egy könyvtárból, megmutatja az utakat, és kiszűri azokat, amelyeknél a kiindulópont és a cél megegyezik, az összköltség pedig nagyobb nullánál.

## Könyvtár megnyitása

A **Fájl → Könyvtár megnyitása…** menüponttal, vagy a `Ctrl+O` billentyűkombinációval válasszon könyvtárat. A tallózóban a mappák és a PDF-fájlok látszanak.

A program a kiválasztott könyvtár PDF-jeit nyitja meg, az almappákat nem. Ha egy fájl nem nyitható meg, a többi ettől még betöltődik, a sikertelen fájlokról pedig figyelmeztetés jelenik meg. Ha a könyvtárban nincs PDF, a program ezt külön jelzi.

A betöltés után az állapotsor mutatja a feldolgozott PDF-ek és az utak számát. Minden panelen az aktuális fájl adatai látszanak: sorszám, munkavállaló, törzsszám és fájlnév.

## Ellenőrzés

Az **Ellenőrzés** fül két, egyforma méretű panelből áll.

A bal panel a betöltött utakat mutatja, egyszerre egy fájlt. Az **Előző** és a **Következő** gombbal, vagy a közöttük lévő legördülő listából lehet a fájlok között váltani. A táblázat oszlopai: azonosító, honnan, hova, távolság és összesen. A hosszú szövegek több sorba törnek.

A **Betöltöttek ellenőrzése** gomb csak akkor aktív, ha van betöltött út. Az ellenőrzés után:

- ha nincs hiba, egy ablak közli ezt;
- ha van hiba, a jobb panel az első hibás fájl első hibás sorára ugrik.

A jobb panel csak a hibás utakat mutatja. Az **Előző** és a **Következő** gomb, valamint a közöttük lévő legördülő lista a hibátlan fájlokat kihagyja. Egy hibás sorra kattintva a bal panel arra a fájlra és arra a sorra ugrik, majd odagörget.

Ha a két panel nem ugyanazt a fájlt mutatja, a bal panel **Előző** és **Következő** gombja közötti legördülő lista piros és félkövér.

## Hibariport

A **Hibák exportálása** gomb csak akkor aktív, ha legalább egy hiba van. A gomb a betöltött könyvtárban létrehozza a `report` mappát, ha az még nincs meg, és kétféle CSV-t ír bele.

Az összesített hibariport neve a készítés időpontját tartalmazza, például `report_2029_12_18_19-05.csv`. Ebben csak a hibás utak szerepelnek. Az első négy oszlop a hibát tartalmazó fájl metaadata: fájlnév, sorszám, munkavállaló és törzsszám. Utána az út mezői következnek, az azonosítótól az összesenig. Ugyanezek a sorok a **Hibariport** fülön is megjelennek. A fejléc félkövér, a cellák középre igazítottak, a hosszú mezők pedig több sorba törnek. Az oszlopok elég szélesek ahhoz, hogy a fejlécek olvashatók legyenek.

Emellett minden betöltött hibát tartalmazó PDF saját CSV-t kap a hibás utakról. A fájlnév a munkavállaló nevéből és a forrásfájl nevéből áll, a szóközök helyén aláhúzásjel van, például `Munkavállaló_neve-kiküldetési_rendelvény.csv`.

A kész riportról egy ablak tájékoztat. Ha egy PDF CSV-je nem írható ki, ez az ablak figyelmeztetés lesz, és felsorolja a sikertelen fájlokat.
