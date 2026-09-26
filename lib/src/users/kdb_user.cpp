//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include "users/kdb_user.hpp"
#include "users/kdb_group.hpp"
#include "core/kdb_core.hpp"

using KalaHeaders::KalaLog::LogType;

using KalaDatabase::Users::User;
using KalaDatabase::Users::Group;
using KalaDatabase::Core::KalaDatabaseCore;

using std::string;
using std::to_string;
using std::pair;

static bool isVerboseLoggingEnabled{};

static constexpr u32 ROOT_USER_PERSISTENT_ID = 1;

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
                "KalaDatabase user error",
                "Failed to " + action + " because caller '" 
                + to_string(callerID) + "' had an invalid group! Reason: " + err);
        }
    }

    outValue = { u, g };

    return "";
}

namespace KalaDatabase::Users
{
	static KalaDatabaseRegistry<User> registry{};

	KalaDatabaseRegistry<User>& User::GetRegistry() { return registry; }

    bool User::IsVerboseLoggingEnabled() { return isVerboseLoggingEnabled; }
    void User::SetVerboseLoggingState(bool newValue) { isVerboseLoggingEnabled = newValue; }

    u32 User::GetRegistryIDByPersistentID(u32 persistentID)
    {
        for (User* u : registry.GetAllContent())
        {
            if (u->persistentID == persistentID) return u->ID;
        }

        KalaDatabaseCore::LogPrint(
            "Failed to get registry ID for user '" 
            + to_string(persistentID) + "' because it was invalid!",
            "KDB_USER",
            LogType::LOG_WARNING);

        return 0;
    }
    u32 User::GetPersistentIDByRegistryID(u32 registryID)
    {
        for (User* u : registry.GetAllContent())
        {
            if (u->ID == registryID) return u->persistentID;
        }

        KalaDatabaseCore::LogPrint(
            "Failed to get persistent ID for user '" 
            + to_string(registryID) + "' because it was invalid!",
            "KDB_USER",
            LogType::LOG_WARNING);

        return 0;
    }

    u32 User::GetRootUserRegistryID()
    { 
        return GetRegistryIDByPersistentID(ROOT_USER_PERSISTENT_ID);
    }
    u32 User::GetRootUserPersistentID() { return ROOT_USER_PERSISTENT_ID; }

    User* User::Initialize(
        u32 callerID,
        u32 persistentID,
        u32 groupID,
        string_view username,
        string_view password)
    {

    }

    u32 User::GetID() const { return ID; }

    u32 User::GetPersistentID() const { return persistentID; }
    void User::SetPersistentID(
        u32 callerID,
        u32 newValue)
    {

    }

    u32 User::GetGroupID() const { return groupID; }
    void User::SetGroupID(
        u32 callerID,
        u32 newValue)
    {

    }

    const vector<u32>& User::GetOwnedTableIDs() const { return ownedTableIDs; }

    const string& User::GetUsername() const { return username; }
    void User::SetUsername(
        u32 callerID,
        string_view newValue)
    {

    }

    const pair<string, string>& User::GetPassword(u32 callerID) const
    {

    }
    void User::SetPassword(
        u32 callerID,
        string_view newValue)
    {

    }
    bool User::IsValidPassword(
        u32 callerID,
        string_view password)
    {

    }

    void User::Destroy(u32 callerID)
    {
        
    }

    User::~User()
    {

    }
}