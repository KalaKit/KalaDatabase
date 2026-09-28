//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <string>

#include "log_utils.hpp"

#include "core/kdb_core.hpp"
#include "core/kdb_database.hpp"
#include "users/kdb_group.hpp"
#include "users/kdb_user.hpp"
#include "data/kdb_table.hpp"
#include "data/kdb_field.hpp"

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::LogType;

using KalaDatabase::Core::KalaDatabaseCore;
using KalaDatabase::Core::Database;
using KalaDatabase::Users::GROUP_EVERYONE;
using KalaDatabase::Users::Group;
using KalaDatabase::Users::USER_ROOT;
using KalaDatabase::Users::User;
using KalaDatabase::Data::Table;
using KalaDatabase::Data::Field;

using std::string;

static User* rootUser{};
static User* regularUser{};

static Group* everyoneGroup{};

static void TestUserListLoad()
{
    Log::Print(
        "\n"
        "--------------------\n"
        "USER LIST LOAD TEST\n"
        "--------------------\n");

    //no space test
    Database::LoadUserList(
        rootUser->GetPersistentID(),
        "test_files/user_list_junk_no_spaces.txt");
        
    //too many spaces test
    Database::LoadUserList(
        rootUser->GetPersistentID(),
        "test_files/user_list_junk_too_many_spaces.txt");

    //bad byte string size test
    Database::LoadUserList(
        rootUser->GetPersistentID(),
        "test_files/user_list_junk_bad_byte_string_size.txt");

    //no root test
    Database::LoadUserList(
        rootUser->GetPersistentID(),
        "test_files/user_list_junk_no_root.txt");

    //valid test
    Database::LoadUserList(
        rootUser->GetPersistentID(),
        "test_files/user_list_valid.txt");
}

static void TestUserCreation()
{
    Log::Print(
        "\n"
        "--------------------\n"
        "USER CREATE TEST\n"
        "--------------------\n");

    //valid user create test
    regularUser = User::Initialize(
        User::GetRootUserPersistentID(),
        0,
        0,
        "username",
        "testpass");
}

static void TestLogin()
{
    Log::Print(
        "\n"
        "--------------------\n"
        "USER LOGIN TEST\n"
        "--------------------\n");

    //too short username test
    Database::Login(
        "usernam",
        "password");

    //username not found test
    Database::Login(
        "ruut",
        "password");

    //invalid password test
    Database::Login(
        "root",
        "wrongpass");

    //valid root user login text
    Database::Login(
        USER_ROOT,
        "correctpass");
}

static void TestLogout()
{
    /*
    Log::Print(
        "\n"
        "--------------------\n"
        "USER LOGOUT TEST\n"
        "--------------------\n");
    */

    Log::Print(" ");
}

int main()
{
    Log::Print(
        "Start of tests.",
        "USER_TESTS",
        LogType::LOG_INFO);

    Log::Print(" ");

    Database::Initialize();

    string err = User::GetRegistry().GetContent(User::GetRootUserRegistryID(), rootUser);
    if (!err.empty())
    {
        KalaDatabaseCore::ForceClose(
            "KalaDatabase user test error",
            "Failed to get root user after KalaDatabase initialization!");
    }
    
    TestUserCreation();

    TestUserListLoad();

    TestLogin();

    TestLogout();

    Database::Shutdown();

    Log::Print(" ");

    Log::Print(
        "End of tests.",
        "USER_TESTS",
        LogType::LOG_INFO);

    return 0;
}