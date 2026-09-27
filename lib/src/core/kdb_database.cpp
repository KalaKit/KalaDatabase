//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include "password_hasher.hpp"
#include "file_utils.hpp"
#include "string_utils.hpp"

#include "core/kdb_database.hpp"
#include "users/kdb_user.hpp"
#include "users/kdb_group.hpp"

using KalaHeaders::KalaCore::ContainsValue;
using KalaHeaders::KalaCore::HasDuplicates;

using KalaHeaders::KalaPasswordHasher::VerifyPassword;

using KalaHeaders::KalaFile::WriteLinesToFile;
using KalaHeaders::KalaFile::ReadLinesFromFile;

using KalaHeaders::KalaString::ContainsSpace;
using KalaHeaders::KalaString::SplitString;

using KalaDatabase::Core::KalaDatabaseCore;
using KalaDatabase::Core::UserData;
using KalaDatabase::Users::USER_ROOT;
using KalaDatabase::Users::User;
using KalaDatabase::Users::GroupPermissions;
using KalaDatabase::Users::UserPermissions;
using KalaDatabase::Users::TablePermissions;
using KalaDatabase::Users::Group;

using std::string;
using std::string_view;
using std::to_string;
using std::vector;
using std::pair;
using std::filesystem::path;

static bool isVerboseLoggingEnabled{};

static path loadedUserListPath{};
static path loadedDatabasePath{};

//username, hashed password, password salt
static vector<UserData> allUsers{};
//registry user IDs
static vector<u32> loggedInUsers{};

static string GetUserAndGroup(
    u32 callerID,
    string action,
    pair<User*, Group*>& outValue)
{
    User* u{};
    string err = User::GetRegistry().GetContent(User::GetRegistryIDByPersistentID(callerID), u);
    if (!err.empty())
    {
        return "Caller ID was invalid! Reason: " + err;
    }

    Group* g{};
    if (u->GetUsername() != USER_ROOT)
    {
        err = Group::GetRegistry().GetContent(u->GetGroupID(), g);
        if (!err.empty())
        {
            KalaDatabaseCore::ForceClose(
                "KalaDatabase core error",
                "Failed to " + action + " because caller '" 
                + to_string(callerID) + "' had an invalid group! Reason: " + err);
        }
    }

    outValue = { u, g };

    return "";
}

namespace KalaDatabase::Core
{
    bool Database::IsVerboseLoggingEnabled() { return isVerboseLoggingEnabled; }
    void Database::SetVerboseLoggingState(bool newValue) { isVerboseLoggingEnabled = newValue; }

    const path& Database::GetLoadedUserListPath() { return loadedUserListPath; }

    const vector<UserData>& Database::GetUserList(u32 callerID)
    {
        static const vector<UserData> empty{};

        pair<User*, Group*> userData{};

        string err = GetUserAndGroup(
            callerID,
            "get user list",
            userData);
        if (!err.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to get user list! Reason: " + err, 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return empty;
        }

        bool isRoot = userData.first->username == USER_ROOT;

        if (!isRoot)
        {
            KalaDatabaseCore::LogPrint(
                "User '" + userData.first->username + "' with ID '" + to_string(callerID) 
                + "' in group '" + userData.second->groupName + "' and insufficient 'root' permission "
                "requested user list!",
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return empty;
        }

        if (isVerboseLoggingEnabled)
        {
            KalaDatabaseCore::LogPrint(
                "User '" + userData.first->username 
                + "' with ID '" + to_string(callerID) 
                + "' requested user list.",
                "KDB_DATABASE",
                LogType::LOG_VERBOSE);
        }

        return allUsers;
    }
    void Database::SaveUserList(
        u32 callerID,
        const path& userListPath,
        bool relativeToExe,
        bool override)
    {
        if (allUsers.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to save user list because there are no users!",
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        pair<User*, Group*> userData{};

        string err = GetUserAndGroup(
            callerID,
            "save user list",
            userData);
        if (!err.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to save user list! Reason: " + err, 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        bool isRoot = userData.first->username == USER_ROOT;

        if (!isRoot)
        {
            KalaDatabaseCore::LogPrint(
                "User '" + userData.first->username + "' with ID '" + to_string(callerID) 
                + "' in group '" + userData.second->groupName + "' and insufficient 'root' permission "
                "requested to save the user list!",
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        if (!relativeToExe
            && !userListPath.is_absolute())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to save user list because user list path "
                "set as not exe-relative was not absolute!", 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        path targetPath = relativeToExe 
            ? KalaDatabaseCore::GetExePath().parent_path() / userListPath
            : userListPath;

        vector<string> saveData{};

        for (const UserData& ud : allUsers)
        {
            saveData.push_back(ud.username + " " + ud.hashedPassword + " " + ud.hashSalt);
        }

        err = WriteLinesToFile(
            targetPath,
            saveData,
            false,
            override);

        if (!err.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to save user list! Reason: " + err,
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        KalaDatabaseCore::LogPrint(
            "User '" + userData.first->username + "' with ID '" + to_string(callerID) 
            + "' saved user list to path '" + userListPath.string() + "'!",
            "KDB_DATABASE",
            LogType::LOG_SUCCESS);
    }
    void Database::LoadUserList(
        u32 callerID,
        const path& userListPath,
        bool relativeToExe)
    {
        pair<User*, Group*> userData{};

        if (callerID == 0
            && !loggedInUsers.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to load user list because caller ID was set to 0 but root is already logged in!", 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        if (callerID != 0)
        {
            string err = GetUserAndGroup(
                callerID,
                "load user list",
                userData);
            if (!err.empty())
            {
                KalaDatabaseCore::LogPrint(
                    "Failed to load user list! Reason: " + err, 
                    "KDB_DATABASE",
                    LogType::LOG_WARNING);

                return;
            }

            bool isRoot = userData.first->username == USER_ROOT;

            if (!isRoot)
            {
                KalaDatabaseCore::LogPrint(
                    "User '" + userData.first->username + "' with ID '" + to_string(callerID) 
                    + "' in group '" + userData.second->groupName + "' and insufficient 'root' permission "
                    "requested to load the user list!",
                    "KDB_DATABASE",
                    LogType::LOG_WARNING);

                return;
            }
        }

        if (!relativeToExe
            && !userListPath.is_absolute())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to load user list because user list path "
                "set as not exe-relative was not absolute!", 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        path targetPath = relativeToExe 
            ? KalaDatabaseCore::GetExePath().parent_path() / userListPath
            : userListPath;

        vector<string> loadData{};

        string err = ReadLinesFromFile(
            targetPath,
            loadData);

        if (!err.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to load user list! Reason: " + err,
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        vector<UserData> verifiedData{};

        auto is_loaded_data_valid = [
            &loadData,
            &verifiedData,
            &userListPath]() -> bool
            {
                if (loadData.empty())
                {
                    KalaDatabaseCore::LogPrint(
                        "Failed to load user list from path '" + userListPath.string() + "' because it had no data!",
                        "KDB_DATABASE",
                        LogType::LOG_WARNING);

                    return false;
                }

                vector<string> userDuplicateCheck{};

                for (size_t i = 0; i < loadData.size(); i++)
                {
                    const string& line = loadData[i];

                    if (!ContainsSpace(line))
                    {
                        KalaDatabaseCore::LogPrint(
                            "Failed to load user list from path '" + userListPath.string() + "' "
                            "because line '" + to_string(i) + "' had no spaces!",
                            "KDB_DATABASE",
                            LogType::LOG_WARNING);

                        return false;
                    }

                    vector<string> splitString{};
                    string err = SplitString(line, " ", splitString);
                    if (!err.empty())
                    {
                        KalaDatabaseCore::LogPrint(
                            "Failed to load user list from path '" + userListPath.string() + "' "
                            "because line '" + to_string(i) + "' was malformed! Reason: " + err,
                            "KDB_DATABASE",
                            LogType::LOG_WARNING);

                        return false;
                    }

                    if (splitString.size() < 3)
                    {
                        KalaDatabaseCore::LogPrint(
                            "Failed to load user list from path '" + userListPath.string() + "' "
                            "because line '" + to_string(i) + "' has too few spaces!",
                            "KDB_DATABASE",
                            LogType::LOG_WARNING);

                        return false;
                    }

                    if (splitString.size() > 3)
                    {
                        KalaDatabaseCore::LogPrint(
                            "Failed to load user list from path '" + userListPath.string() + "' "
                            "because line '" + to_string(i) + "' has too many spaces!",
                            "KDB_DATABASE",
                            LogType::LOG_WARNING);

                        return false;
                    }

                    userDuplicateCheck.push_back(splitString[0]);

                    verifiedData.push_back(
                    {
                        .username = splitString[0],
                        .hashedPassword = splitString[1],
                        .hashSalt = splitString[2] 
                    });
                }

                if (HasDuplicates(userDuplicateCheck))
                {
                    KalaDatabaseCore::LogPrint(
                        "Failed to load user list from path '" + userListPath.string() + "' "
                        "because duplicate usernames were found!",
                        "KDB_DATABASE",
                        LogType::LOG_WARNING);

                    return false;
                }

                if (!ContainsValue(userDuplicateCheck, USER_ROOT))
                {
                    KalaDatabaseCore::LogPrint(
                        "Failed to load user list from path '" + userListPath.string() + "' "
                        "because no root username was found!",
                        "KDB_DATABASE",
                        LogType::LOG_WARNING);

                    return false;
                }

                return true;
            };

        if (!is_loaded_data_valid()) return;

        vector<u32> loggedInUsersCopy = loggedInUsers;
        for (const u32 uid : loggedInUsersCopy)
        {
            User* u{};
            string err = User::GetRegistry().GetContent(uid, u);
            if (!err.empty())
            {
                KalaDatabaseCore::ForceClose(
                    "KalaDatabase database error",
                    "Failed to get load user list from path '" 
                    + userListPath.string() + "' because user ID was invalid! Reason: " + err);
            }

            if (u->username != USER_ROOT)
            {
                Logout(
                    User::GetRootUserPersistentID(),
                    u->username);
            }
        }

        allUsers = std::move(verifiedData);

        if (callerID != 0)
        {
            KalaDatabaseCore::LogPrint(
                "User '" + userData.first->username + "' with ID '" + to_string(callerID) 
                + "' loaded user list from path '" + userListPath.string() + "'!",
                "KDB_DATABASE",
                LogType::LOG_SUCCESS);
        }
        else
        {
            KalaDatabaseCore::LogPrint(
                "Loaded user list from path '" + userListPath.string() + "'!",
                "KDB_DATABASE",
                LogType::LOG_SUCCESS);
        }
    }

    const path& Database::GetLoadedDatabasePath() { return loadedDatabasePath; }

    void Database::SaveDatabase(
        u32 callerID,
        const path& databasePath,
        bool relativeToExe,
        bool override)
    {

    }

    void Database::LoadDatabase(
        u32 callerID,
        const path& databasePath,
        bool relativeToExe)
    {

    }

    bool Database::IsUserLoggedIn(
        u32 callerID,
        string_view username)
    {
        if (loggedInUsers.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to get logged in state for user '" + string(username) + "' because no users are online!",
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return false;
        }

        pair<User*, Group*> userData{};

        string err = GetUserAndGroup(
            callerID,
            "get user logged in state",
            userData);
        if (!err.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to get user logged in state for user '" + string(username) + "'! Reason: " + err, 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return false;
        }

        bool isRoot = userData.first->username == USER_ROOT;

        string inGroupMsg = isRoot 
            ? ""
            : "in group '" + userData.second->groupName + "'";

        if (isRoot
            || userData.second->HasUserPermission(User::GetRootUserPersistentID(), UserPermissions::U_GET_ACTIVE_USERS))
        {
            KalaDatabaseCore::LogPrint(
                "User '" + userData.first->username + "' with ID '" + to_string(callerID) 
                + "' " + inGroupMsg + " requested login state for user '" + string(username) + "'",
                "KDB_DATABASE",
                LogType::LOG_INFO);

            for (const u32 uid : loggedInUsers)
            {
                User* u{};
                string err = User::GetRegistry().GetContent(uid, u);
                if (!err.empty())
                {
                    KalaDatabaseCore::ForceClose(
                        "KalaDatabase database error",
                        "Failed to get user logged in state because user ID was invalid! Reason: " + err);
                }

                if (u->username == username) return true;
            }

            return false;
        }

        KalaDatabaseCore::LogPrint(
            "User '" + userData.first->username + "' with ID '" + to_string(callerID) 
            + "' " + inGroupMsg + " and insufficient 'U_GET_ACTIVE_USERS' permission "
            "requested login state for user '" + string(username) + "'!",
            "KDB_DATABASE",
            LogType::LOG_WARNING);

        return false;
    }

    void Database::Login(
        string_view username,
        string_view password)
    {
        if (username.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to log in user because username was empty!", 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        if (password.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to log in user '" + string(username) + "' because password was empty!", 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        if (allUsers.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to log in user '" + string(username) + "' because user list was empty! "
                "Make sure to load user list before logging in.", 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        UserData* userData{};

        for (UserData& ud : allUsers)
        {
            if (ud.username == username)
            {
                userData = &ud;
                break;
            }
        }

        if (!userData)
        {
            KalaDatabaseCore::LogPrint(
                "Failed to log in user '" + string(username) + "' because it was not found! "
                "Did you forget to initialize the user?", 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        string err = VerifyPassword(
            password, 
            { userData->hashedPassword, userData->hashSalt });

        if (!err.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to log in user '" + string(username) + "'! Reason: " + err, 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        if (!ContainsValue(loggedInUsers, User::GetRootUserRegistryID()))
        {
            KalaDatabaseCore::LogPrint(
                "Failed to log in user '" + string(username) + "' because root has not yet logged in!", 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        for (const u32 uid : loggedInUsers)
        {
            User* u{};
            string err = User::GetRegistry().GetContent(uid, u);
            if (!err.empty())
            {
                KalaDatabaseCore::ForceClose(
                    "KalaDatabase database error",
                    "Failed to log in because user ID was invalid! Reason: " + err);
            }

            if (u->username == username)
            {
                KalaDatabaseCore::LogPrint(
                    "Failed to log in user '" + string(username) + "' because it is already logged in!", 
                    "KDB_DATABASE",
                    LogType::LOG_WARNING);

                return;
            }
        }

        User* user{};
        for (User* u : User::GetRegistry().GetAllContent())
        {
            if (u->username == username)
            {
                user = u;
                break;
            }
        }
        
        if (!user)
        {
            KalaDatabaseCore::ForceClose(
                "KalaDatabase database error",
                "Failed to log in because user was invalid!"); 
        }

        loggedInUsers.push_back(user->ID);

        if (isVerboseLoggingEnabled)
        {
            KalaDatabaseCore::LogPrint(
                "User '" + string(username) + "' has logged in!",
                "KDB_DATABASE",
                LogType::LOG_VERBOSE);
        }
    }

    void Database::Logout(
        u32 callerID,
        string_view username)
    {
        if (username.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to log out user because username was empty!", 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        if (allUsers.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to log out user '" + string(username) + "' because user list was empty! "
                "Make sure to load user list before logging in.", 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        UserData* targetUserData{};

        for (UserData& ud : allUsers)
        {
            if (ud.username == username)
            {
                targetUserData = &ud;
                break;
            }
        }

        if (!targetUserData)
        {
            KalaDatabaseCore::LogPrint(
                "Failed to log out user '" + string(username) + "' because it was not found! "
                "Did you forget to initialize the user?", 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        pair<User*, Group*> callerUserData{};

        string err = GetUserAndGroup(
            callerID,
            "log out user",
            callerUserData);
        if (!err.empty())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to log out user '" + string(username) + "'! Reason: " + err, 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }
        
        User* user{};
        for (const u32 uid : loggedInUsers)
        {
            User* u{};
            string err = User::GetRegistry().GetContent(uid, u);
            if (!err.empty())
            {
                KalaDatabaseCore::ForceClose(
                    "KalaDatabase database error",
                    "Failed to log out because user ID was invalid! Reason: " + err);
            }

            if (u->username == username)
            {
                user = u;
                break;
            }
        }

        if (!user)
        {
            KalaDatabaseCore::LogPrint(
                "Failed to log out user '" + string(username) + "' because it is not logged in!", 
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        bool canLogOut = 
            callerUserData.first->username == USER_ROOT
            || callerID == user->persistentID
            || callerUserData.second->HasUserPermission(User::GetRootUserPersistentID(), UserPermissions::U_LOG_OUT_USERS);

        if (!canLogOut)
        {
            KalaDatabaseCore::LogPrint(
                "User '" + callerUserData.first->username + "' with ID '" + to_string(callerID) 
                + "' in group '" + callerUserData.second->groupName + "' and insufficient permissions "
                "requested to log out user '" + string(username) + "'!",
                "KDB_DATABASE",
                LogType::LOG_WARNING);

            return;
        }

        //root logoff call does full shutdown of database and all users
        if (callerUserData.first->username == USER_ROOT)
        {
            if (!loadedDatabasePath.empty())
            {
                UnloadDatabase(callerID);
            }

            vector<u32> loggedInUsersCopy = loggedInUsers;
            for (const u32 uid : loggedInUsersCopy)
            {
                if (user->ID != uid)
                {
                    User* u{};
                    string err = User::GetRegistry().GetContent(uid, u);
                    if (!err.empty())
                    {
                        KalaDatabaseCore::ForceClose(
                            "KalaDatabase database error",
                            "Failed to log out user because it was invalid! Reason: " + err);
                    }

                    Logout(u->persistentID, u->username);
                }
            }

            erase(loggedInUsers, user->ID);

            if (isVerboseLoggingEnabled)
            {
                KalaDatabaseCore::LogPrint(
                    "User '" + string(username) + "' has logged out!",
                    "KDB_DATABASE",
                    LogType::LOG_VERBOSE);
            }

            if (!loggedInUsers.empty())
            {
                KalaDatabaseCore::LogPrint(
                    "User lists were not cleared out after root logged off!",
                    "KDB_DATABASE",
                    LogType::LOG_ERROR);

                loggedInUsers.clear();
            }

            allUsers.clear();
            loadedUserListPath.clear();

            return;
        }

        erase(loggedInUsers, user->ID);

        if (isVerboseLoggingEnabled)
        {
            KalaDatabaseCore::LogPrint(
                "User '" + string(username) + "' has logged out!",
                "KDB_DATABASE",
                LogType::LOG_VERBOSE);
        }
    }
}