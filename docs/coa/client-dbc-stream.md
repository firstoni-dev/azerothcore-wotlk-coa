# Streamed client DBC rows

The world tables `gameobjectdisplayinfo_dbc`, `map_dbc` and `mapdifficulty_dbc` already add rows to (or
replace rows in) the server's own `GameObjectDisplayInfo.dbc`, `Map.dbc` and `MapDifficulty.dbc`. With the
display-patch stream on (`CoA.SendDisplayPatches`), the same rows are now also sent to each player
at login. Server and client read them from one place.

Code: `src/server/coa/AscensionClientDbcStream.{h,cpp}`, called from the login patch stream in
`AscensionCompat.cpp`.

## Client requirement

The genuine Ascension client has no handler for these tables. The rows apply only on a client whose
`Extensions.dll` is built with `ASCENSION_DBC_STREAM` (client-ext `AscDbcStream`). Other clients ignore the
opcodes.

| Opcode | Table | Client row |
|---|---|---|
| `0x8F3` | GameObjectDisplayInfo | 19 fields |
| `0x8F4` | Map | 18 fields |
| `0x8F5` | MapDifficulty | 7 fields |

A packet is a batch: u32 count, then each row's fields in table column order. Numbers are u32 (floats as
their bits). Strings are a u32 length plus bytes. Localized columns send the player's locale (the
`*_Lang_*` columns are positional, slot n = `LocaleConstant` n), else enUS. A batch holds at most 512 rows
or 16 KB. The client grows each table's index once per batch, so streams of many new rows stay cheap.

## What streaming a row does not do

- **New maps need much more than a `map_dbc` row.** The client needs the map's terrain (ADT/WDT/WMO) in its
  archives. The server needs `maps/`, `vmaps/` and `mmaps/` extracted from those same archives, plus
  `instance_template`, area triggers, graveyards, spawns and scripts. A Map row alone gives a map
  the client cannot load.
- **Objects already spawned keep their display.** A changed `gameobjectdisplayinfo_dbc` row applies to
  objects the client creates after the stream, for example on leaving and re-entering view.
- **The map being loaded at login** has already read its Map row; a change applies at the next load.
- Rows are loaded once, on the first login after the server starts. Restart the worldserver after editing
  the tables; the server's own stores also read them only at startup.
