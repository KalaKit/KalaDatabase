//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <string_view>
#include <vector>
#include <filesystem>

#include "core_utils.hpp"

namespace KalaDatabase::Core
{
    using std::string;
    using std::string_view;
    using std::vector;
    using std::filesystem::path;

    struct LIB_API UserData
    {
        string username{};
        
        string hashedPassword{};
        string hashSalt{};
    };

    class LIB_API Database
    {
    public:
        static bool IsVerboseLoggingEnabled();
        static void SetVerboseLoggingState(bool newValue);

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
        //can only be called by root, logs off all logged in users except root,
        //can be called with caller ID 0 if root has not yet logged in
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
    };
}