//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <string_view>
#include <vector>
#include <array>
#include <filesystem>

#include "core_utils.hpp"
#include "password_hasher.hpp"

namespace KalaDatabase::Users
{
    class Group;
    class User;
}

namespace KalaDatabase::Data
{
    class Table;
    class Field;
}

namespace KalaDatabase::Core
{
    using KalaHeaders::KalaPasswordHasher::HASH_SIZE_BYTES;
    using KalaHeaders::KalaPasswordHasher::SALT_SIZE_BYTES;

    using std::string;
    using std::string_view;
    using std::vector;
    using std::array;
    using std::filesystem::path;

    struct LIB_API UserData
    {
        string username{};
        
        array<u8, HASH_SIZE_BYTES> hashedPassword{};
        array<u8, SALT_SIZE_BYTES> passwordSalt{};
    };

    class LIB_API Database
    {
    friend class KalaDatabase::Users::Group;
    friend class KalaDatabase::Users::User;
    friend class KalaDatabase::Data::Table;
    friend class KalaDatabase::Data::Field;
    public:
        static bool IsVerboseLoggingEnabled();
        static void SetVerboseLoggingState(bool newValue);

        static bool IsInitialized();
        //Creates the root user, the 'Everyone' group and allows to load user list and database,
        //loading the user list overrides the created root user password,
        //or if no user list exists then you must set root password manually before you can save the user list
        static void Initialize();

        static const path& GetLoadedUserListPath();

        //Get the user list containing all usernames and hashed passwords,
        //can only be called by root
        KNODISCARD
		static const vector<UserData>& GetUserList(u32 callerID);
        //Saves the current in-memory user list as a '.txt' file to the target path,
        //accepts absolute path and path relative to executable,
        //can only be called by root,
        //set relativeToExe to true if you want this file to be saved relative to the exe dir,
        //otherwise it must be a full path whose parent directory exists,
        //set override to true if you want to overwrite the file at the existing path
        static void SaveUserList(
            u32 callerID,
            const path& userListPath,
            bool relativeToExe = true,
            bool override = false);
        //Loads the on-disk user list from a '.txt' file to memory,
        //accepts absolute path and path relative to executable,
        //set relativeToExe to true if you want this file to be loaded relative to the exe dir,
        //can only be called by root, logs off all logged in users except root
        static void LoadUserList(
            u32 callerID,
            const path& userListPath,
            bool relativeToExe = true);

        static const path& GetLoadedDatabasePath();

        //Saves the current in-memory database as a '.kdb' file to the target path,
        //accepts absolute path and path relative to executable,
        //can only be called by root,
        //set relativeToExe to true if you want this file to be saved relative to the exe dir,
        //otherwise it must be a full path whose parent directory exists,
        //set override to true if you want to overwrite the file at the existing path
        static void SaveDatabase(
            u32 callerID,
            const path& databasePath,
            bool relativeToExe = true,
            bool override = false);
        //Loads the on-disk database from a '.kdb' file to memory,
        //accepts absolute path and path relative to executable,
        //set relativeToExe to true if you want this file to be loaded relative to the exe dir,
        //can only be called by root, logs off all logged in users except root
        static void LoadDatabase(
            u32 callerID,
            const path& databasePath,
            bool relativeToExe = true);
        //Unloads the database from memory, saves to disk before unloading,
        //can only be called by root, logs off all logged in users except root
        static void UnloadDatabase(u32 callerID);
        
        //Returns true if user with username is online,
        //requires permission 'U_GET_ACTIVE_USERS'
        KNODISCARD
        static bool IsUserLoggedIn(
            u32 callerID,
            string_view username);

        //Log in as a new user
        static void Login(
            string_view username,
            string_view password);

        //Pass own ID if logging off as self,
        //use other user ID if logging out as someone else,
        //for example the real user really did log off
        //and a server for this database no longer detects this user as active,
        //if root is logging off then currently loaded database
        //and user list will be saved and unloaded
        //and all other active users will also be logged off
        static void Logout(
            u32 callerID,
            string_view username);

        //Unloads database and user list, clears all tables, fields, users and groups
        static void Shutdown();
    private:
        static const vector<u32>& GetLoggedInUsers();
    };
}