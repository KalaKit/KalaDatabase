//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include "core/kdb_database.hpp"
#include "users/kdb_user.hpp"
#include "users/kdb_group.hpp"

using KalaHeaders::KalaCore::ContainsValue;

using KalaDatabase::Core::KalaDatabaseCore;
using KalaDatabase::Users::User;
using KalaDatabase::Users::Group;

using std::string;
using std::string_view;
using std::to_string;
using std::vector;
using std::pair;
using std::filesystem::path;

static path loadedDatabasePath{};

static vector<string_view> loggedInUsers{};

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
                "KalaDatabase database error",
                "Failed to " + action + " because caller '" 
                + to_string(callerID) + "' had an invalid group! Reason: " + err);
        }
    }

    outValue = { u, g };

    return "";
}

namespace KalaDatabase::Core
{
    const vector<UserData>& Database::GetUserList(u32 callerID)
    {

    }
    void Database::SaveUserList(u32 callerID)
    {

    }
    void Database::LoadUserList(u32 callerID)
    {

    }

    bool Database::IsUserLoggedIn(
        u32 callerID,
        string_view username)
    {
        User* u{};
        string err = User::GetRegistry().GetContent(callerID, u);
        if (!err.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to get logged in state because user ID was invalid! Reason: " + err,
                "KDB_CORE",
                LogType::LOG_WARNING);

            return false;
        }

        Group* g{};
        err = Group::GetRegistry().GetContent(u->groupID, g);
        if (!err.empty())
        {
            KalaDatabaseCore::ForceClose(
                "KalaDatabase core error",
                "Failed to get logged in state because user '" 
                + to_string(callerID) + "' had an invalid group! Reason: " + err);
        }

        KalaDatabaseCore::LogPrint(
            "User '" + u->username + "' with ID '" + to_string(callerID) 
            + "' in group '" + g->groupName + "' requested login state for user '" + string(username) + "'",
            "KDB_CORE",
            LogType::LOG_INFO);

        return ContainsValue(loggedInUsers, username);
    }

    void Database::Login(
        u32 callerID,
        string_view username,
        string_view password)
    {

    }

    void Database::Logout(
        u32 callerID,
        string_view username)
    {

    }

    const path& Database::GetLoadedDatabasePath() { return loadedDatabasePath; }

    void Database::Save(
        u32 callerID,
        const path& targetPath)
    {

    }

    void Database::Load(
        u32 callerID,
        const path& targetPath)
    {

    }

    void Database::Unload(u32 callerID)
    {

    }
}