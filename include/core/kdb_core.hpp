//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <filesystem>

#include "core_utils.hpp"
#include "log_utils.hpp"

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
    using std::string;
    using std::string_view;
    using std::vector;
    using std::function;
    using std::filesystem::path;

    using KalaHeaders::KalaLog::LogType;

    class LIB_API KalaDatabaseCore
    {
    friend class Database;
    friend class KalaDatabase::Users::Group;
    friend class KalaDatabase::Users::User;
    friend class KalaDatabase::Data::Table;
    friend class KalaDatabase::Data::Field;
    public:
        static bool IsVerboseLoggingEnabled();
        static void SetVerboseLoggingState(bool newValue);

        static u32 GetGlobalID();
		static void SetGlobalID(u32 newID);

        //Returns the current highest persistent ID
        static u32 GetPersistentID();
        static void SetPersistentID(u32 newValue);

        static bool IsPersistentIDInUse(u32 persistentID);

		KNODISCARD
		static path GetExePath();

        //Returns all activity logs for this user, cannot be read by anyone except root
        KNODISCARD
		static const vector<string>& GetActivityLogs(u32 callerID);

        //External handler for force close, overrides local version so external version can do its own action
        static void SetExternalHandler(function<void(string, string)>&& externalHandler);

        //Force-closes the application and gives a breakpoint, good for hard stops or bad user errors,
        //assigning a callback via SetExternalHandler will always use whatever is assigned there,
        //if the callback is unassigned or invalid then it falls back to the local setup
		KNORETURN
        static void ForceClose(
			string_view title,
			string_view reason);
    private:
        //Private logger for activity logs,
        //should never be used outside of KalaDatabase internal codebase
        static void LogPrint(
            string_view message,
            string_view target,
            LogType logType,
            bool printToConsole = true,
            bool printToActivityLogs = true);
    };
}