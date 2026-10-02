//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <string>
#include <vector>
#include <array>

#include "core_utils.hpp"
#include "password_hasher.hpp"

#include "core/kdb_registry.hpp"

namespace KalaDatabase::Core
{
    class KalaDatabaseCore;
    class Database;
}

namespace KalaDatabase::Users
{
    using KalaHeaders::KalaPasswordHasher::HASH_SIZE_BYTES;
    using KalaHeaders::KalaPasswordHasher::SALT_SIZE_BYTES;

    using KalaDatabase::Core::KalaDatabaseRegistry;

    using std::string;
    using std::string_view;
    using std::pair;
    using std::vector;
    using std::array;
    using std::default_delete;

    //Can modify all groups, users, tables and fields, can only read its own user data,
    //cannot be given a group, does not have a group that can be modified,
    //cannot be deleted from users registry
    static constexpr string_view USER_ROOT = "root";

    static constexpr u8 MIN_USER_PASS_SIZE = 8;
    static constexpr u8 MAX_USER_PASS_SIZE = 32;

    struct LIB_API PasswordData
    {
        array<u8, HASH_SIZE_BYTES> hashedPasswordBytes{};
        array<u8, SALT_SIZE_BYTES> passwordSaltBytes{};
    };

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
        //each persistent ID must be unique, they cannot be shared across groups, users, tables and fields,
        //if no users exist then root user is automatically created first,
        //persistent ID 1 and username root are reserved only for root user
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
		const pair<array<u8, HASH_SIZE_BYTES>, array<u8, SALT_SIZE_BYTES>>& GetPassword(u32 callerID) const;
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

        static void SetRootInitState(bool value);
        static bool RootHasPassword();

        u32 ID{};
        u32 persistentID{};
        u32 groupID{};
        vector<u32> ownedTableIDs{}; //which tables does this user own

        string username{};
        pair<array<u8, HASH_SIZE_BYTES>, array<u8, SALT_SIZE_BYTES>> hashedPassword{};
    };
}