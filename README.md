<!--
Kiküldetési Rendelvény Ellenőrző — kiküldetési rendelvények ellenőrzése.
Copyright (C) 2026 Vajda Ákos Péter <vajdaakos@alu-fon.hu>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
-->

# Kiküldetési Rendelvény Ellenőrző

A program kiküldetési rendelvény PDF-eket tölt be egy könyvtárból, megmutatja az utakat, és kiszűri azokat, amelyeknél a kiindulópont és a cél megegyezik, az összköltség pedig nagyobb nullánál.

## Könyvtár megnyitása

A **Fájl → Könyvtár megnyitása…** menüponttal, vagy a `Ctrl+O` billentyűkombinációval válasszon könyvtárat. A tallózóban a mappák és a PDF-fájlok látszanak.

A program a kiválasztott könyvtár PDF-jeit nyitja meg, az almappákat nem. Ha egy fájl nem nyitható meg, a többi ettől még betöltődik, a sikertelen fájlokról pedig figyelmeztetés jelenik meg. Ha a könyvtárban nincs PDF, a program ezt külön jelzi.

A betöltés után az állapotsor mutatja a feldolgozott PDF-ek és az utak számát. Minden panelen az aktuális fájl adatai látszanak: sorszám, munkavállaló, törzsszám és fájlnév.

## Ellenőrzés

Az **Ellenőrzés** fül két, egyforma méretű panelből áll.

A bal panel a betöltött utakat mutatja, egyszerre egy fájlt. Az **Előző** és a **Következő** gombbal, vagy a közöttük lévő legördülő listából lehet a fájlok között váltani. A bal panel csak az utat tartalmazó fájlokat mutatja, az út nélküli PDF-eket kihagyja. A táblázat oszlopai: azonosító, honnan, hova, távolság és összesen. A hosszú szövegek több sorba törnek.

A **Betöltöttek ellenőrzése** gomb csak akkor aktív, ha van betöltött út. Az ellenőrzés után:

- ha nincs hiba, egy ablak közli ezt;
- ha van hiba, a jobb panel az első hibás fájl első hibás sorára ugrik, a bal panelen pedig a hibás sorok háttere piros.

A jobb panel csak a hibás utakat mutatja. Az **Előző** és a **Következő** gomb, valamint a közöttük lévő legördülő lista a hibátlan fájlokat kihagyja. Egy hibás sorra kattintva a bal panel arra a fájlra és arra a sorra ugrik, majd odagörget.

Ha a két panel nem ugyanazt a fájlt mutatja, a bal panel **Előző** és **Következő** gombja közötti legördülő lista piros és félkövér.

## Exportálási beállítások

A **Beállítások → Exportálási beállítások** menüpont modális ablakot nyit. Két csoport van benne.

- A **Metaadatok:** a fájlnév, a sorszám, a munkavállaló és a törzsszám.
- Az **Adatok:** az út mezői, az azonosítótól az összesenig.

A csoport címe melletti pipa a csoport minden jelölőnégyzetét be- vagy kikapcsolja. A csoport elemei ettől függetlenül egyenként is kapcsolhatók. Az ablak a betöltött pipákat mutatja. Ha nincs mentett fájl, a fájlnév és a forrásfájl nincs bejelölve, a többi oszlop igen.

Az **Ok** a pipákat ettől a pillanattól az exportálásra alkalmazza, és a program mappájába, a `settings.json` fájlba írja. A régi fájl csak a sikeres írás után cserélődik le. Ha a mentés nem sikerül, és a korábbi fájl megvan, figyelmeztetés közli, hogy az változatlan marad, és a következő indulás abból tölt. Ha még nincs fájl, a figyelmeztetés azt mondja, hogy a következő induláskor az alapértelmezett beállítások lépnek életbe. A **Mégsem** és az Escape elveti a módosítást.

> **Figyelem!** Ha a két csoportban minden jelölőnégyzetet kikapcsol, a program ettől még elkészíti az összesített és a PDF-enkénti CSV-fájlokat, de ezek üresek lesznek: sem fejlécet, sem adatot nem tartalmaznak, csak üres sorokat. A **Hibariport** fül ilyenkor oszlopok nélküli táblát mutat. Exportálás előtt legalább egy oszlopot jelöljön be.

Induláskor a program beolvassa a mappájában lévő `settings.json` fájlt, és ezekkel a pipákkal exportál. Ha a fájl hiányzik, a JSON nem olvasható, vagy a gyökér nem objektum, modális figyelmeztetés jelenik meg, középre igazított szöveggel: a beállító fájl betöltése sikertelen, a program az alapértelmezett beállításokkal indul.

## Hibariport

A **Hibák exportálása** gomb csak akkor aktív, ha legalább egy hiba van. A gomb a betöltött könyvtárban létrehozza a `report` mappát, ha az még nincs meg, és kétféle CSV-t ír bele.

Az összesített hibariport neve a készítés időpontját tartalmazza, például `report_2029_12_18_19-05.csv`. Ebben csak a hibás utak szerepelnek. A metaadatok és az útmezők közül csak a bejelölt oszlopok kerülnek a fejlécbe és a sorokba. Ugyanezek a sorok a **Hibariport** fülön is megjelennek, a fejléccel együtt. A későbbi beállításváltás ezt a kész riportot nem rendezi át: az új oszlopok a következő exportáláskor jelennek meg. A fejléc félkövér, a cellák középre igazítottak, a hosszú mezők pedig több sorba törnek. Az oszlopok elég szélesek ahhoz, hogy a fejlécek olvashatók legyenek.

Emellett minden betöltött hibát tartalmazó PDF saját CSV-t kap a hibás utakról. A fejléce megegyezik az összesített hibariport fejlécével, a sorok pedig ugyanezekből a bejelölt oszlopokból állnak. A fájlnév a munkavállaló nevéből és a forrásfájl nevéből áll, a szóközök helyén aláhúzásjel van, például `Munkavállaló_neve-kiküldetési_rendelvény.csv`.

A kész riportról egy ablak tájékoztat. Ha egy PDF CSV-je nem írható ki, ez az ablak figyelmeztetés lesz, és felsorolja a sikertelen fájlokat.
