//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <string>
#include <vector>

#include "core_utils.hpp"

#include "core/kdb_registry.hpp"

namespace KalaDatabase::Core
{
    class KalaDatabaseCore;
    class Database;
}

namespace KalaDatabase::Data
{
    using std::string;
    using std::string_view;
    using std::vector;
    using std::default_delete;

    using KalaDatabase::Core::KalaDatabaseRegistry;

    static constexpr u8 MIN_TABLE_NAME_SIZE = 4;
    static constexpr u8 MAX_TABLE_NAME_SIZE = 16;

    class LIB_API Table
    {
    friend class KalaDatabase::Core::KalaDatabaseCore;
    friend class KalaDatabase::Core::Database;
	friend struct default_delete<Table>;
    public:
        KNODISCARD
		static KalaDatabaseRegistry<Table>& GetRegistry();

        static bool IsVerboseLoggingEnabled();
        static void SetVerboseLoggingState(bool newValue);

        //Does not need caller ID, only persistent ID whose registry ID is needed
        static u32 GetRegistryIDByPersistentID(u32 persistentID);
        //Does not need caller ID, only registry ID whose persistent ID is needed
        static u32 GetPersistentIDByRegistryID(u32 registryID);

        //Create a new table, leave persistent ID as 0 if you want it to be auto-assigned,
        //each persistent ID must be unique, they cannot be shared across groups, users, tables and fields
        KNODISCARD
		static Table* Initialize(
            u32 callerID,
            u32 persistentID,
            string_view tableName);

        u32 GetID() const;

        KNODISCARD
        u32 GetPersistentID() const;
        void SetPersistentID(
            u32 callerID,
            u32 newValue);

        const vector<u32>& GetTableIDs() const;
        void AddTableID(
            u32 callerID,
            u32 newValue);
        void RemoveTableID(
            u32 callerID,
            u32 targetTableID);

        const vector<u32>& GetFieldIDs() const;

        //Get the current owner ID, owner simply means a regular user can read and write to this table
        //meanwhile root or users with correct permissions can always write to this table
        u32 GetOwnerID() const;
        void SetOwnerID(
            u32 callerID,
            u32 newValue);

        const string& GetTableName(u32 callerID) const;
        void SetTableName(
            u32 callerID, 
            string_view newValue);

        //Destroying a table destoys its fields and removes it from the database,
        //must save database to apply changes on disk
        void Destroy(u32 callerID);
    private:
        ~Table();

        u32 ID{};
        u32 persistentID{};
        vector<u32> tableIDs{};
        vector<u32> fieldIDs{};
        u32 ownerID{};

        string tableName{};
    };
}