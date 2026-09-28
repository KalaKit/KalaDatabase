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

namespace KalaDatabase::Users
{
    using std::string;
    using std::string_view;
    using std::pair;
    using std::vector;
    using std::default_delete;

    using KalaDatabase::Core::KalaDatabaseRegistry;

    //Can modify all groups, users, tables and fields, can only read its own user data,
    //cannot be given a group, does not have a group that can be modified,
    //cannot be deleted from users registry
    static constexpr string_view USER_ROOT = "root";

    static constexpr u8 MAX_USER_PASS_LENGTH = 32;
    static constexpr u8 MIN_USER_PASS_LENGTH = 8;

    class LIB_API User
    {
    friend class KalaDatabase::Core::KalaDatabaseCore;
    friend class KalaDatabase::Core::Database;
    friend class Group;
	friend struct default_delete<User>;
    public:
        KNODISCARD
		static KalaDatabaseRegistry<User>& GetRegistry();

        static bool IsVerboseLoggingEnabled();
        static void SetVerboseLoggingState(bool newValue);

        //Does not need caller ID, only persistent ID whose registry ID is needed
        static u32 GetRegistryIDByPersistentID(u32 persistentID);
        //Does not need caller ID, only registry ID whose persistent ID is needed
        static u32 GetPersistentIDByRegistryID(u32 registryID);

        static u32 GetRootUserRegistryID();
        static u32 GetRootUserPersistentID();

        //Create a new user and assign to selected group,
        //leave persistent ID as 0 if you want it to be auto-assigned,
        //each persistent ID must be unique, they cannot be shared across groups, users, tables and fields
        KNODISCARD
		static User* Initialize(
            u32 callerID,
            u32 persistentID,
            u32 groupID,
            string_view username,
            string_view password);

        KNODISCARD
		u32 GetID() const;

        KNODISCARD
        u32 GetPersistentID() const;
        void SetPersistentID(
            u32 callerID,
            u32 newValue);

        KNODISCARD
		u32 GetGroupID() const;
        void SetGroupID(
            u32 callerID,
            u32 newValue);

        KNODISCARD
		const vector<u32>& GetOwnedTableIDs() const;

        KNODISCARD
		const string& GetUsername() const;
        //Sets new username, must save user list to apply changes on disk
        void SetUsername(
            u32 callerID,
            string_view newValue);

        //Returns hashed password and hash salt
        KNODISCARD
		const pair<string, string>& GetPassword(u32 callerID) const;
        //Pass real password, gets stored as hashed,
        //must save user list to apply changes on disk
        void SetPassword(
            u32 callerID,
            string_view newValue);
        //Returns true if real password matches hashed password
        KNODISCARD
		bool IsValidPassword(
            u32 callerID,
            string_view password);

        //Destroying a user removes it from the database,
        //must save database to apply changes on disk
        void Destroy(u32 callerID);
    private:
        ~User();

        u32 ID{};
        u32 persistentID{};
        u32 groupID{};
        vector<u32> ownedTableIDs{}; //which tables does this user own

        string username{};
        pair<string, string> hashedPassword{};

        vector<string> activityLogs{};
    };
}