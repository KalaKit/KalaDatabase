//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include "data/kdb_field.hpp"
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
                "KalaDatabase field error",
                "Failed to " + action + " because caller '" 
                + to_string(callerID) + "' had an invalid group! Reason: " + err);
        }
    }

    outValue = { u, g };

    return "";
}

namespace KalaDatabase::Data
{
	static KalaDatabaseRegistry<Field> registry{};

	KalaDatabaseRegistry<Field>& Field::GetRegistry() { return registry; }

    bool Field::IsVerboseLoggingEnabled() { return isVerboseLoggingEnabled; }
    void Field::SetVerboseLoggingState(bool newValue) { isVerboseLoggingEnabled = newValue; }

    u32 Field::GetRegistryIDByPersistentID(u32 persistentID)
    {
        for (Field* f : registry.GetAllContent())
        {
            if (f->persistentID == persistentID) return f->ID;
        }

        KalaDatabaseCore::LogPrint(
            "Failed to get registry ID for field '" 
            + to_string(persistentID) + "' because it was invalid!",
            "KDB_FIELD",
            LogType::LOG_WARNING);

        return 0;
    }
    u32 Field::GetPersistentIDByRegistryID(u32 registryID)
    {
        for (Field* f : registry.GetAllContent())
        {
            if (f->ID == registryID) return f->persistentID;
        }

        KalaDatabaseCore::LogPrint(
            "Failed to get persistent ID for field '" 
            + to_string(registryID) + "' because it was invalid!",
            "KDB_FIELD",
            LogType::LOG_WARNING);

        return 0;
    }

    Field* Field::Initialize(
        u32 callerID,
        u32 persistentID,
        string_view fieldName,
        string_view fieldValue,
        u32 tableID)
    {

    }

    u32 Field::GetID() const { return ID; }

    u32 Field::GetPersistentID() const { return persistentID; }
    void Field::SetPersistentID(
        u32 callerID,
        u32 newValue)
    {

    }

    u32 Field::GetTableID() const { return tableID; }
    void Field::SetTableID(
        u32 callerID,
        u32 newValue)
    {

    }

    const string& Field::GetFieldName(u32 callerID) const
    {

    }
    void Field::SetFieldName(
        u32 callerID,
        string_view newValue)
    {

    }

    const string& Field::GetFieldValue(u32 callerID) const
    {

    }
    void Field::SetFieldValue(
        u32 callerID,
        string_view newValue)
    {

    }

    u64 Field::GetFieldRange(u32 callerID) const
    {

    }
    void Field::SetFieldRange(
        u32 callerID,
        u64 newValue)
    {

    }

    FieldEditType Field::GetFieldEditType(u32 callerID) const
    {

    }
    void Field::SetFieldEditType(
        u32 callerID,
        FieldEditType newValue)
    {

    }

    FieldValueType Field::GetFieldValueType(u32 callerID) const
    {

    }
    void Field::SetFieldValueType(
        u32 callerID,
        FieldValueType newValue)
    {

    }

    bool Field::IsUnsignedOnly(u32 callerID) const
    {

    }
    void Field::SetUnsignedOnly(
        u32 callerID,
        bool newValue)
    {

    }

    void Field::Destroy(u32 callerID)
    {

    }

    Field::~Field()
    {

    }
}