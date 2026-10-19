# Fejlesztői dokumentáció

## Architektúra

- Tiszta **I/O vs. állapot vs. prezentáció** határ: `TripLoader` / `TripManager` / `MainWindow`.
- **TripManager** jó kompozíciós gyökér: navigáció, ellenőrzés, export, `focusTrip` a hiba→forrás szinkronhoz.
- **QRangeModel + Trip tuple** — kevés kód, három táblanézet ugyanarra az adatsémára.

## Adatséma és export

1. **`columnheaders.h`** — egyetlen fordítási kontextus (`ColumnHeaders`), a riport fejléce a metaadat- és útoszlopokból épül fel. A modellek és a CSV ugyanazokat a fejléceket használják.

2. **Export hibakezelés** — a könyvtárlétrehozás, a fájlmegnyitás és a stream-írás hibái látható figyelmeztetést adnak. A riportmodell az export indításakor feltöltődik, ezért fájlhiba esetén a hibariport fül tartalma megmarad.

   Az export az összesített hibariport mellett minden hibát tartalmazó PDF hibás útjait külön CSV-be írja. A közös `writeCsv()` elkülöníti a fájlírást és fájlonként gyűjti a hibákat.

3. **Build rendszer** — a [CMakeLists.txt](CMakeLists.txt) támogatja a vcpkg toolchaint, a MinGW dynamic tripletet és a Poppler pkg-config felderítését. A nem illeszkedő MinGW triplet konfigurációs hibát ad.

4. **Disztribúció** — Qt resource-ban található alkalmazásikon, Windows ICO erőforrás és telepíthető futtatható állomány készül.

## Felhasználói felület

- Szinkron bal/jobb fájl navigáció, piros jelzés eltérő fájlnévnél.
- Hibasor kiválasztása → bal tábla sor + scroll.
- Részleges PDF betöltés + `loadFailed` lista.
- A `TripMetadataWidget` egységesen jeleníti meg a két panel metaadatait.
- A README beágyazott súgóként F1-gyel érhető el; a Súgó menü középre igazított névjegyet is tartalmaz.

---

## Tudatos designkorlátok

- A koordináta-alapú PDF-parser egy rögzített dokumentumformátumot céloz; nem általános PDF-feldolgozó.
- A formátum manuális ellenőrzése rövid és egyértelmű.
- A `kVisibleColumns{0, 5, 6, 7, 12}` indexei stabil, zárt adatsémára épülnek.
- A `tripFields()`, a `Trip::get<I>` és a `tripColumnHeaders()` mezősorrendje együtt alkotja a PDF-sémához tartozó leképezést. Dokumentumformátum-változáskor ezeket együtt kell módosítani.
- Az `ErrorStorage` `const Trip*` mutatókat tárol a `TripStorage` elemeire. Új betöltéskor a hibák törlése megelőzi az utak cseréjét; ellenőrzés után a forráskonténer nem módosul.
- A betöltés és az ellenőrzés a UI szálon fut. A célzott könyvtárméret mellett a feldolgozási idő rövid.
- Az ablak címe **„KR”**, a CMake projekt `KrEllenorzo`, a csomag `KrEllenorzes`, a teljes terméknév „Kiküldetési Rendelvény Ellenőrző”. Ezek külön szerepet betöltő elnevezések.

## Kódminőség

- A `TripMetadataWidget` egy helyen kezeli a bal és jobb panel azonos metaadat-megjelenítését.
- A `TripChecker` egyszerű C++ osztály; az ellenőrzés közvetlen metódushívással történik.
- Az `ErrorReportCreator` publikus `create()` metódusa közvetlenül kapcsolódik a `TripManager` exportműveletéhez.
- Az export sikeres és részleges eredménye külön felhasználói üzenetet kap; a sikertelen fájlok listája megjelenik.
- A `CMAKE_AUTORCC` biztosítja a súgó és az ikon erőforrásainak beépítését.
