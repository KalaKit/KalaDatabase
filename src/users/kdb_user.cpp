//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <memory>

#include "string_utils.hpp"

#include "users/kdb_user.hpp"
#include "users/kdb_group.hpp"
#include "core/kdb_core.hpp"
#include "core/kdb_database.hpp"

using KalaHeaders::KalaLog::LogType;

using KalaHeaders::KalaString::IsStringInRange;

using KalaHeaders::KalaPasswordHasher::HashPassword;

using KalaDatabase::Core::KalaDatabaseCore;
using KalaDatabase::Core::Database;
using KalaDatabase::Core::UserData;
using KalaDatabase::Users::USER_ROOT;
using KalaDatabase::Users::User;
using KalaDatabase::Users::GroupPermissions;
using KalaDatabase::Users::UserPermissions;
using KalaDatabase::Users::TablePermissions;
using KalaDatabase::Users::Group;

using std::string;
using std::to_string;
using std::pair;
using std::unique_ptr;
using std::make_unique;

static bool isVerboseLoggingEnabled{};
static bool initializingRootUser{};

static constexpr u32 ROOT_USER_PERSISTENT_ID = 1;

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
        if (!Database::IsInitialized())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to get registry ID by persistent ID because KalaDatabase has not been initialized!",
                "KDB_USER",
                LogType::LOG_WARNING);

            return 0;
        }

        for (User* u : registry.GetAllContent())
        {
            if (u->persistentID == persistentID) return u->ID;
        }

        KalaDatabaseCore::LogPrint(
            "Failed to get user registry ID by persistent ID '" 
            + to_string(persistentID) + "' because it was invalid!",
            "KDB_USER",
            LogType::LOG_WARNING);

        return 0;
    }
    u32 User::GetPersistentIDByRegistryID(u32 registryID)
    {
        if (!Database::IsInitialized())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to get persistent ID by registry ID because KalaDatabase has not been initialized!",
                "KDB_USER",
                LogType::LOG_WARNING);

            return 0;
        }

        for (User* u : registry.GetAllContent())
        {
            if (u->ID == registryID) return u->persistentID;
        }

        KalaDatabaseCore::LogPrint(
            "Failed to get persistent ID by registry ID '" 
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
        if (!Database::IsInitialized())
        {
            KalaDatabaseCore::LogPrint(
                "Failed to initialize user because KalaDatabase has not been initialized!",
                "KDB_USER",
                LogType::LOG_WARNING);

            return nullptr;
        }

        if (!initializingRootUser)
        {
            if (username == USER_ROOT)
            {
                KalaDatabaseCore::LogPrint(
                    "Failed to initialize user because username '" 
                    + string(USER_ROOT) + "' is reserved for root user!",
                    "KDB_USER",
                    LogType::LOG_WARNING);

                return nullptr;
            }

            if (!IsStringInRange(
                username,
                MIN_USER_PASS_SIZE,
                MAX_USER_PASS_SIZE))
            {
                KalaDatabaseCore::LogPrint(
                    "Failed to initialize user because its username length was out of range!",
                    "KDB_USER",
                    LogType::LOG_WARNING);

                return nullptr;
            }

            if (!IsStringInRange(
                password,
                MIN_USER_PASS_SIZE,
                MAX_USER_PASS_SIZE))
            {
                KalaDatabaseCore::LogPrint(
                    "Failed to initialize user '" + string(username) + "' because its password length was out of range!",
                    "KDB_USER",
                    LogType::LOG_WARNING);

                return nullptr;
            }

            if (callerID == 0)
            {
                KalaDatabaseCore::LogPrint(
                    "Failed to initialize user '" + string(username) + "' because caller ID was empty!", 
                    "KDB_USER",
                    LogType::LOG_WARNING);

                return nullptr;
            }
            if (groupID == 0)
            {
                KalaDatabaseCore::LogPrint(
                    "Failed to initialize user '" + string(username) + "' because group ID was empty!", 
                    "KDB_USER",
                    LogType::LOG_WARNING);

                return nullptr;
            }
            
            if (persistentID == ROOT_USER_PERSISTENT_ID)
            {
                KalaDatabaseCore::LogPrint(
                    "Failed to initialize user '" + string(username) + "' because persistent ID '" 
                    + to_string(ROOT_USER_PERSISTENT_ID) + "' is reserved for root user!", 
                    "KDB_USER",
                    LogType::LOG_WARNING);

                return nullptr;
            }
        }

        if (KalaDatabaseCore::IsPersistentIDInUse(persistentID))
        {
            KalaDatabaseCore::LogPrint(
                "Failed to initialize user '" + string(username) + "' because persistent ID '" 
                + to_string(persistentID) + "' is already in use!", 
                "KDB_USER",
                LogType::LOG_WARNING);

            return nullptr;
        }

        pair<User*, Group*> userData{};

        if (callerID != 0)
        {
            string err = GetUserAndGroup(
                callerID,
                "load user list",
                userData);
            if (!err.empty())
            {
                KalaDatabaseCore::LogPrint(
                    "Failed to initialize user '" + string(username) + "'! Reason: " + err, 
                    "KDB_USER",
                    LogType::LOG_WARNING);

                return nullptr;
            }

            bool canCreateUser = 
                userData.first->username == USER_ROOT
                || userData.second->HasUserPermission(GetRootUserPersistentID(), UserPermissions::U_CREATE_USERS);

            if (!canCreateUser)
            {
                KalaDatabaseCore::LogPrint(
                    "User '" + userData.first->username + "' with ID '" + to_string(callerID) 
                    + "' in group '" + userData.second->groupName + "' and insufficient 'root' and 'U_CREATE_USERS' permission "
                    "requested to create a user!",
                    "KDB_USER",
                    LogType::LOG_WARNING);

                return nullptr;
            }
        }

        pair<array<u8, HASH_SIZE_BYTES>, array<u8, SALT_SIZE_BYTES>> result{};

        if (!initializingRootUser)
        {
            string err = HashPassword(
                password,
                result);
            if (!err.empty())
            {
                KalaDatabaseCore::LogPrint(
                    "Failed to initialize user '" + string(username) + "'! Reason: " + err,
                    "KDB_USER",
                    LogType::LOG_WARNING);

                return nullptr;
            }
        }

		u32 newID = KalaDatabaseCore::GetGlobalID() + 1;
		KalaDatabaseCore::SetGlobalID(newID);

        if (persistentID == 0)
        {
            persistentID = KalaDatabaseCore::GetPersistentID() + 1;
            KalaDatabaseCore::SetPersistentID(persistentID);
        }

		unique_ptr<User> newUser = make_unique<User>();
		User* userPtr = newUser.get();

        userPtr->ID = newID;
        userPtr->persistentID = persistentID;
        userPtr->groupID = groupID;

        userPtr->username = username;
        userPtr->hashedPassword = std::move(result);

		string err = registry.AddContent(
			newID, 
			std::move(newUser));
		if (!err.empty())
		{
			KalaDatabaseCore::ForceClose(
				"KalaDatabase user error",
				"Failed to initialize user '" + string(username) + "'! Reason: " + err);
		}

        if (isVerboseLoggingEnabled)
        {
            KalaDatabaseCore::LogPrint(
                "Initialized user '" + string(username) + "' via caller ID '" + to_string(callerID) + "'!",
                "KDB_USER",
                LogType::LOG_VERBOSE);
        }

        return userPtr;
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

    const pair<array<u8, HASH_SIZE_BYTES>, array<u8, SALT_SIZE_BYTES>>& User::GetPassword(u32 callerID) const
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

    void User::SetRootInitState(bool value) { initializingRootUser = value; }

    bool User::RootHasPassword()
    {

    }

    void User::Destroy(u32 callerID)
    {
        
    }

    User::~User()
    {

    }
}