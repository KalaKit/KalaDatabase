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

        //Get the user list containing all usernames and hashed passwords,
        //can only be called by root
        KNODISCARD
		static const vector<UserData>& GetUserList(u32 callerID);
        //Saves the current in-memory user list to 'users.txt' relative to the executable,
        //can only be called by root
        static void SaveUserList(u32 callerID);
        //Loads the on-disk user list from 'users.txt' to memory,
        //can only be called by root
        static void LoadUserList(u32 callerID);
        
        KNODISCARD
        static bool IsUserLoggedIn(
            u32 callerID,
            string_view username);

        //Use userID 0 if logging in as self,
        //use real user ID if logging in as someone else,
        //for example a server for this database accepts
        //the login request of one of the server users
        static void Login(
            u32 callerID,
            string_view username,
            string_view password);

        //Pass own ID if logging off as self,
        //user real user ID if logging out as someone else,
        //for example the real user really did log off
        //and a server for this database no longer detects this user as active,
        //if root is logging off then currently loaded database will be saved and unloaded
        //and all other active users will also be logged off
        static void Logout(
            u32 callerID,
            string_view username);

        static const path& GetLoadedDatabasePath();

        //Save the current in-memory database on disk,
        //can only be called by root
        static void Save(
            u32 callerID,
            const path& targetPath);

        //Load an existing database into memory,
        //can only be called by root
        static void Load(
            u32 callerID,
            const path& targetPath);

        //Unload the current database off of memory, always saves to disk,
        //can only be called by root
        static void Unload(u32 callerID);
    };
}