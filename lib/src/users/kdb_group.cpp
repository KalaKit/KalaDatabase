//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include "users/kdb_group.hpp"
#include "users/kdb_user.hpp"
#include "core/kdb_core.hpp"

using KalaHeaders::KalaLog::LogType;

using KalaDatabase::Users::User;
using KalaDatabase::Users::Group;
using KalaDatabase::Core::KalaDatabaseCore;

using std::string;
using std::to_string;
using std::pair;

static bool isVerboseLoggingEnabled{};

static constexpr u32 EVERYONE_GROUP_PERSISTENT_ID = 2;

static constexpr u8 MAX_GROUP_PERMISSIONS_RANGE = 31;
static constexpr u8 MAX_USER_PERMISSIONS_RANGE = 127;
static constexpr u8 MAX_TABLE_PERMISSIONS_RANGE = 127;

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
                "KalaDatabase group error",
                "Failed to " + action + " because caller '" 
                + to_string(callerID) + "' had an invalid group! Reason: " + err);
        }
    }

    outValue = { u, g };

    return "";
}

namespace KalaDatabase::Users
{
	static KalaDatabaseRegistry<Group> registry{};

	KalaDatabaseRegistry<Group>& Group::GetRegistry() { return registry; }

    bool Group::IsVerboseLoggingEnabled() { return isVerboseLoggingEnabled; }
    void Group::SetVerboseLoggingState(bool newValue) { isVerboseLoggingEnabled = newValue; }

    u32 Group::GetRegistryIDByPersistentID(u32 persistentID)
    {
        for (Group* g : registry.GetAllContent())
        {
            if (g->persistentID == persistentID) return g->ID;
        }

        KalaDatabaseCore::LogPrint(
            "Failed to get registry ID for group '" 
            + to_string(persistentID) + "' because it was invalid!",
            "KDB_GROUP",
            LogType::LOG_WARNING);

        return 0;
    }
    u32 Group::GetPersistentIDByRegistryID(u32 registryID)
    {
        for (Group* g : registry.GetAllContent())
        {
            if (g->ID == registryID) return g->persistentID;
        }

        KalaDatabaseCore::LogPrint(
            "Failed to get persistent ID for group '" 
            + to_string(registryID) + "' because it was invalid!",
            "KDB_GROUP",
            LogType::LOG_WARNING);

        return 0;
    }

    u32 Group::GetEveryoneGroupRegistryID()
    { 
        return GetRegistryIDByPersistentID(EVERYONE_GROUP_PERSISTENT_ID);
    }
    u32 Group::GetEveryoneGroupPersistentID() { return EVERYONE_GROUP_PERSISTENT_ID; }

    Group* Group::Initialize(
        u32 callerID,
        u32 persistentID,
        string_view groupName,
        u8 groupPermissions,
        u8 userPermissions,
        u8 tablePermissions)
    {
        
    }

    u32 Group::GetID() const { return ID; }

    u32 Group::GetPersistentID() const { return persistentID; }
    void Group::SetPersistentID(
        u32 callerID,
        u32 newValue)
    {

    }

    const vector<u32>& Group::GetUserIDs() const { return userIDs; }
    const vector<u32>& Group::GetTableIDs() const { return tableIDs; }

    const string& Group::GetGroupName(u32 callerID) const
    {

    }
    void Group::SetGroupName(
        u32 callerID,
        string_view newValue)
    {

    }

    u8 Group::GetGroupPermissions(u32 callerID) const { return groupPermissions; }
    void Group::SetGroupPermissions(
        u32 callerID,
        u8 newValue)
    {

    }

    u8 Group::GetUserPermissions(u32 callerID) const { return userPermissions; }
    void Group::SetUserPermissions(
        u32 callerID,
        u8 newValue)
    {
        
    }

    u8 Group::GetTablePermissions(u32 callerID) const { return tablePermissions; }
    void Group::SetTablePermissions(
        u32 callerID,
        u8 newValue)
    {
        
    }

    void Group::Destroy(u32 callerID)
    {

    }

    Group::~Group()
    {

    }
}