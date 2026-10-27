/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef ASCENSION_CLIENT_DBC_STREAM_H
#define ASCENSION_CLIENT_DBC_STREAM_H

#include "Common.h"
#include "WorldPacket.h"
#include <array>
#include <string>
#include <vector>

class Player;

// Rows of the world tables gameobjectdisplayinfo_dbc, map_dbc and mapdifficulty_dbc, streamed at login to
// clients whose Extensions.dll is built with ASCENSION_DBC_STREAM. The same rows the server's own stores load,
// so server and client agree on them. See docs/coa/client-dbc-stream.md.
constexpr uint16 AscensionClientDbcGameObjectDisplayInfoOpcode = 0x8F3;
constexpr uint16 AscensionClientDbcMapOpcode = 0x8F4;
constexpr uint16 AscensionClientDbcMapDifficultyOpcode = 0x8F5;

// Packet limits: a batch never holds more rows or bytes than this (a single larger row still goes alone).
constexpr std::size_t AscensionClientDbcPacketRows = 512;
constexpr std::size_t AscensionClientDbcPacketBytes = 16 * 1024;

enum class AscensionClientDbcColumnKind : uint8
{
    Int,
    Float,
    String,
    LocalizedString,
};

struct AscensionClientDbcColumn
{
    char const* Name;
    AscensionClientDbcColumnKind Kind;
};

struct AscensionClientDbcTable
{
    uint16 Opcode;
    char const* SqlTable;
    std::vector<AscensionClientDbcColumn> Columns;   // the client's in-memory row, field by field
};

struct AscensionClientDbcRow
{
    std::vector<uint32> Values;                                    // one per column; 0 for strings
    std::vector<std::array<std::string, TOTAL_LOCALES>> Strings;   // one per string column, in order
};

std::vector<AscensionClientDbcTable> const& GetAscensionClientDbcTables();

// The batches for these rows: u32 count, then per row each column as a u32 or a u32-sized string. A localized
// column sends the locale's text, else enUS; a plain string column holds its text in slot 0.
std::vector<WorldPacket> BuildAscensionClientDbcPackets(AscensionClientDbcTable const& table,
    std::vector<AscensionClientDbcRow> const& rows, LocaleConstant locale);

// Sends every table's rows (loaded from the world database on first use); returns the bytes sent.
std::size_t SendAscensionClientDbcStream(Player* player, uint32& rowsSent);

#endif
