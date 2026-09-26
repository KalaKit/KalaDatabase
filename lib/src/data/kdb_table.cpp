//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include "data/kdb_table.hpp"
#include "users/kdb_group.hpp"
#include "users/kdb_user.hpp"
#include "core/kdb_core.hpp"

using KalaHeaders::KalaLog::LogType;

using KalaDatabase::Users::Group;
using KalaDatabase::Users::User;
using KalaDatabase::Core::KalaDatabaseCore;

using std::string;
using std::to_string;
using std::pair;

static bool isVerboseLoggingEnabled{};

static string GetUserAndGroup(
    u32 callerID,
    string action,
    pair<User*, Group*>& outValue)
{
    User* u{};
    string err = User::GetRegistry().GetContent(
        User::GetRegistryIDByPersistentID(callerID), 
        u);
    if (!err.empty())
    {
        return "Failed to " + action + " because caller ID was invalid! Reason: " + err;
    }

    Group* g{};

    if (callerID != User::GetRootUserPersistentID())
    {
        err = Group::GetRegistry().GetContent(u->GetGroupID(), g);
        if (!err.empty())
        {
            KalaDatabaseCore::ForceClose(
                "KalaDatabase table error",
                "Failed to " + action + " because caller '" 
                + to_string(callerID) + "' had an invalid group! Reason: " + err);
        }
    }

    outValue = { u, g };

    return "";
}

namespace KalaDatabase::Data
{
	static KalaDatabaseRegistry<Table> registry{};

	KalaDatabaseRegistry<Table>& Table::GetRegistry() { return registry; }

    bool Table::IsVerboseLoggingEnabled() { return isVerboseLoggingEnabled; }
    void Table::SetVerboseLoggingState(bool newValue) { isVerboseLoggingEnabled = newValue; }

    u32 Table::GetRegistryIDByPersistentID(u32 persistentID)
    {
        for (Table* t : registry.GetAllContent())
        {
            if (t->persistentID == persistentID) return t->ID;
        }

        KalaDatabaseCore::LogPrint(
            "Failed to get registry ID for table '" 
            + to_string(persistentID) + "' because it was invalid!",
            "KDB_TABLE",
            LogType::LOG_WARNING);

        return 0;
    }
    u32 Table::GetPersistentIDByRegistryID(u32 registryID)
    {
        for (Table* t : registry.GetAllContent())
        {
            if (t->ID == registryID) return t->persistentID;
        }

        KalaDatabaseCore::LogPrint(
            "Failed to get persistent ID for table '" 
            + to_string(registryID) + "' because it was invalid!",
            "KDB_TABLE",
            LogType::LOG_WARNING);

        return 0;
    }

    Table* Table::Initialize(
        u32 callerID,
        u32 persistentID,
        string_view tableName)
    {

    }

    u32 Table::GetID() const { return ID; }

    u32 Table::GetPersistentID() const { return persistentID; }
    void Table::SetPersistentID(
        u32 callerID,
        u32 newValue)
    {
        
    }

    const vector<u32>& Table::GetTableIDs() const { return tableIDs; }
    void Table::AddTableID(
        u32 callerID,
        u32 newValue)
    {

    }
    void Table::RemoveTableID(
        u32 callerID,
        u32 targetTableID)
    {

    }

    const vector<u32>& Table::GetFieldIDs() const { return fieldIDs; }

    u32 Table::GetOwnerID() const { return ownerID; }
    void Table::SetOwnerID(
        u32 callerID,
        u32 newValue)
    {

    }

    const string& Table::GetTableName(u32 callerID) const
    { 

    }
    void Table::SetTableName(
        u32 callerID,
        string_view newValue)
    {

    }

    void Table::Destroy(u32 callerID)
    {
        
    }

    Table::~Table()
    {

    }
}