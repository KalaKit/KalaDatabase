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
    using std::vector;
    using std::pair;
    using std::default_delete;

    using KalaDatabase::Core::KalaDatabaseRegistry;

    //Always exists, cannot be deleted, all new users are moved to this group.
    //Has no group, user or table permissions, its permissions cannot be modified
    static constexpr string_view GROUP_EVERYONE = "Everyone";

    static constexpr u8 MAX_GROUP_NAME_LENGTH = 16;
    static constexpr u8 MIN_GROUP_NAME_LENGTH = 4;

    //Root has full permission over all groups,
    //root has no group of its own so it cannot modify its group permissions,
    //users in the same group only have read permission over their own group,
    //maximum permission value is 31
    enum class GroupPermissions : u8
    {
        G_RENAME_GROUPS            = 1 << 0,
        G_READ_GROUP_PERMISSIONS   = 1 << 1,
        G_CHANGE_GROUP_PERMISSIONS = 1 << 2,
        G_CREATE_GROUPS            = 1 << 3,
        G_DELETE_GROUPS            = 1 << 4
    };

    //Root has full permission over all users but cannot
    //change its username or modify its permissions,
    //maximum permission value is 127
    enum class UserPermissions : u8
    {
        U_CHANGE_USERNAME  = 1 << 0,
        U_CREATE_USERS     = 1 << 1,
        U_DELETE_USERS     = 1 << 2,
        U_LOG_OUT_USERS    = 1 << 3,
        U_CHANGE_GROUP     = 1 << 4,
        U_CHANGE_PASSWORD  = 1 << 5,
        U_GET_ACTIVE_USERS = 1 << 6
    };

    //Root has full permission over all tables,
    //tables which are not listed in the tableIDs vector cannot be
    //read or edited at all unless this group has T_MODIFY_ALL_FIELDS permission,
    //using T_MODIFY_ALL_TABLES ignores tableIDs restriction entirely,
    //maximum permission value is 127
    enum class TablePermissions : u8
    {
        T_SET_TABLE_OWNER      = 1 << 0,
        T_ADD_REMOVE_SUBTABLES = 1 << 1,
        T_ADD_REMOVE_FIELDS    = 1 << 2,
        T_MODIFY_ALL_FIELDS    = 1 << 3,
        T_MODIFY_ALL_TABLES    = 1 << 4,
        T_CREATE_TABLES        = 1 << 5,
        T_DELETE_TABLES        = 1 << 6
    };

    class LIB_API Group
    {
    friend class KalaDatabase::Core::KalaDatabaseCore;
    friend class KalaDatabase::Core::Database;
    friend class User;
	friend struct default_delete<Group>;
    public:
        KNODISCARD
		static KalaDatabaseRegistry<Group>& GetRegistry();

        static bool IsVerboseLoggingEnabled();
        static void SetVerboseLoggingState(bool newValue);

        //Does not need caller ID, only persistent ID whose registry ID is needed
        static u32 GetRegistryIDByPersistentID(u32 persistentID);
        //Does not need caller ID, only registry ID whose persistent ID is needed
        static u32 GetPersistentIDByRegistryID(u32 registryID);
        
        static u32 GetEveryoneGroupRegistryID();
        static u32 GetEveryoneGroupPersistentID();

        //Create a new group, leave persistent ID as 0 if you want it to be auto-assigned,
        //each persistent ID must be unique, they cannot be shared across groups, users, tables and fields
        KNODISCARD
		static Group* Initialize(
            u32 callerID,
            u32 persistentID,
            string_view groupName,
            u8 groupPermissions = {},
            u8 userPermissions = {},
            u8 tablePermissions = {});

        u32 GetID() const;

        KNODISCARD
        u32 GetPersistentID() const;
        void SetPersistentID(
            u32 callerID,
            u32 newValue);

        //Get all user IDs in this group
        const vector<u32>& GetUserIDs() const;
        //Get all tables this group can read,
        //Root can modify all tables,
        //users with the T_FULL table permission can modify all tables
        const vector<u32>& GetTableIDs() const;

        const string& GetGroupName(u32 callerID) const;
        void SetGroupName(
            u32 callerID,
            string_view newValue);

        bool HasGroupPermission(
            u32 callerID,
            GroupPermissions permission);
        u8 GetGroupPermissions(u32 callerID) const;
        void SetGroupPermissions(
            u32 callerID,
            u8 newValue);

        bool HasUserPermission(
            u32 callerID,
            UserPermissions permission);
        u8 GetUserPermissions(u32 callerID) const;
        void SetUserPermissions(
            u32 callerID,
            u8 newValue);

        bool HasTablePermission(
            u32 callerID,
            TablePermissions permission);
        u8 GetTablePermissions(u32 callerID) const;
        void SetTablePermissions(
            u32 callerID,
            u8 newValue);

        //Destroying a group moves its users to the 'Everyone' group and removes it from the database,
        //must save database to apply changes on disk
        void Destroy(u32 callerID);
    private:
        ~Group();

        u32 ID{};
        u32 persistentID{};
        //all users in this group
        vector<u32> userIDs{};
        //all tables this group can read
        vector<u32> tableIDs{};

        string groupName{};

        u8 groupPermissions{};
        u8 userPermissions{};
        u8 tablePermissions{};
    };
}