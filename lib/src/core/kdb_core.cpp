//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include "core/kdb_core.hpp"
#include "users/kdb_user.hpp"
#include "users/kdb_group.hpp"

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::TimeFormat;
using KalaHeaders::KalaLog::DateFormat;

using KalaDatabase::Users::USER_ROOT;
using KalaDatabase::Users::User;
using KalaDatabase::Users::Group;

using std::string;
using std::vector;
using std::function;

static bool isVerboseLoggingEnabled{};

static u32 globalID{};

static vector<string> activityLogs{};

static function<void(string, string)> externalHandler{};

namespace KalaDatabase::Core
{
    bool KalaDatabaseCore::IsVerboseLoggingEnabled() { return isVerboseLoggingEnabled; }
    void KalaDatabaseCore::SetVerboseLoggingState(bool newValue) { isVerboseLoggingEnabled = newValue; }

    u32 KalaDatabaseCore::GetGlobalID() { return globalID; }
	void KalaDatabaseCore::SetGlobalID(u32 newID) { globalID = newID; }

    const vector<string>& KalaDatabaseCore::GetActivityLogs(u32 callerID)
    {
        static vector<string> empty{};

        User* u{};
        string err = User::GetRegistry().GetContent(callerID, u);
        if (!err.empty())
        {
            LogPrint(
                "Failed to get activity logs because user ID was invalid! Reason: " + err,
                "KDB_CORE",
                LogType::LOG_WARNING);

            return empty;
        }

        Group* g{};
        err = Group::GetRegistry().GetContent(u->groupID, g);
        if (!err.empty())
        {
            ForceClose(
                "KalaDatabase core error",
                "Failed to get activity logs because user '" 
                + to_string(callerID) + "' had an invalid group! Reason: " + err);
        }

        if (u->username == USER_ROOT)
        {
            if (isVerboseLoggingEnabled)
            {
                LogPrint(
                    "User '" + u->username + "' with ID '" + to_string(callerID) 
                    + "' in group '" + g->groupName + "' accessed activity logs.",
                    "KDB_CORE",
                    LogType::LOG_VERBOSE);
            }

            return activityLogs;
        }

        LogPrint(
            "User '" + u->username + "' with ID '" + to_string(callerID) 
            + "' in group '" + g->groupName + "' attempted to access activity logs!",
            "KDB_CORE",
            LogType::LOG_WARNING);

        return empty;
    }

	void KalaDatabaseCore::SetExternalHandler(function<void (string, string)>&& newExternalHandler)
	{ 
		externalHandler = std::move(newExternalHandler);
	}

    void KalaDatabaseCore::ForceClose(
		string_view target,
		string_view reason)
	{
		if (externalHandler) externalHandler(string(target), string(reason));
		else
		{
			Log::Print(
				"\n================"
				"\nFORCE CLOSE"
				"\n================\n",
				true);

			Log::Print(
				reason,
				target,
				LogType::LOG_ERROR,
				2,
				true,
				TimeFormat::TIME_NONE,
				DateFormat::DATE_NONE);

#if defined(KWIN_ANY)
			__debugbreak();
#else
			raise(SIGTRAP);
#endif
		}

		_Exit(1);
	}

    void KalaDatabaseCore::LogPrint(
        string_view message,
        string_view target,
        LogType logType,
        bool printToConsole,
        bool printToActivityLogs)
    {
        if (!printToConsole
            && !printToActivityLogs)
        {
            Log::Print(
                "Failed to print log message because neither "
                "printToConsole or printToActivityLogs was true!",
                "KDB_CORE",
                LogType::LOG_ERROR,
                2);

            return;
        }

        if (!isVerboseLoggingEnabled
            && logType == LogType::LOG_VERBOSE)
        {
            return;
        }

        if (logType == LogType::LOG_DEBUG)
        {
#if !defined (KDEBUG)
            return;
#endif
        }

        if (printToConsole) Log::Print(message, target, logType);
        if (printToActivityLogs)
        {
            string time = "[ " + Log::GetTime() + " ] ";
            string type{};

            switch (logType)
            {
            default:
            case LogType::LOG_INFO:
                break;
            case LogType::LOG_DEBUG:
                type = "[ DEBUG ] ";
                break;
            case LogType::LOG_SUCCESS:
                type = "[ SUCCESS ] ";
                break;
            case LogType::LOG_WARNING:
                type = "[ WARNING ] ";
                break;
            case LogType::LOG_ERROR:
                type = "[ ERROR ] ";
                break;
            }

            string targetStr = "[ " + string(target) + " ] ";

            activityLogs.push_back(time + type + targetStr + string(message));
        }
    }
}