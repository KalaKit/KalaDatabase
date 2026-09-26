//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <string>

#include "core_utils.hpp"

#include "core/kdb_registry.hpp"

namespace KalaDatabase::Core
{
    class KalaDatabaseCore;
    class Database;
}

namespace KalaDatabase::Data
{
    using std::string;
    using std::string_view;
    using std::default_delete;

    using KalaDatabase::Core::KalaDatabaseRegistry;

    enum class FieldEditType : u8
    {
        //Owning user can read and write to this field
        E_PUBLIC = 0,
        //Owning user can only read this field
        E_READONLY = 1,
        //Owning user cannot read or write to this field
        E_HIDDEN = 2
    };

    enum class FieldValueType : u8
    {
        //Accepts letters, numbers and symbols
        V_ANY = 0,
        //Only accepts letters and numbers
        V_TEXT_ONLY = 1,
        //Only accepts numbers
        V_NUMBER_ONLY = 2,
        //Only accepts non-decimal numbers
        V_INTEGER_ONLY = 3,
        //Only accepts decimal numbers
        V_FLOAT_DOUBLE_ONLY = 4
    };

    class LIB_API Field
    {
    friend class KalaDatabase::Core::KalaDatabaseCore;
    friend class KalaDatabase::Core::Database;
	friend struct default_delete<Field>;
    public:
        KNODISCARD
		static KalaDatabaseRegistry<Field>& GetRegistry();

        static bool IsVerboseLoggingEnabled();
        static void SetVerboseLoggingState(bool newValue);

        //Does not need caller ID, only persistent ID whose registry ID is needed
        static u32 GetRegistryIDByPersistentID(u32 persistentID);
        //Does not need caller ID, only registry ID whose persistent ID is needed
        static u32 GetPersistentIDByRegistryID(u32 registryID);

        //Create a new field, leave persistent ID as 0 if you want it to be auto-assigned,
        //each persistent ID must be unique, they cannot be shared across groups, users, tables and fields
        KNODISCARD
		static Field* Initialize(
            u32 callerID,
            u32 persistentID,
            string_view fieldName,
            string_view fieldValue,
            u32 tableID);

        u32 GetID() const;

        KNODISCARD
        u32 GetPersistentID() const;
        void SetPersistentID(
            u32 callerID,
            u32 newValue);

        u32 GetTableID() const;
        void SetTableID(
            u32 callerID,
            u32 newValue);

        const string& GetFieldName(u32 callerID) const;
        void SetFieldName(
            u32 callerID,
            string_view newValue);

        const string& GetFieldValue(u32 callerID) const;
        void SetFieldValue(
            u32 callerID,
            string_view newValue);

        u64 GetFieldRange(u32 callerID) const;
        void SetFieldRange(
            u32 callerID,
            u64 newValue);

        FieldEditType GetFieldEditType(u32 callerID) const;
        void SetFieldEditType(
            u32 callerID,
            FieldEditType newValue);

        FieldValueType GetFieldValueType(u32 callerID) const;
        void SetFieldValueType(
            u32 callerID,
            FieldValueType newValue);

        //Returns true if this field is a numerical type and it only accepts unsigned numbers
        bool IsUnsignedOnly(u32 callerID) const;
        //Assign new unsigned state, can only be applied to fields whose value type is a numerical type
        void SetUnsignedOnly(
            u32 callerID,
            bool newValue);

        //Destroying a field removes it from its table and from the database,
        //must save database to apply changes on disk
        void Destroy(u32 callerID);
    private:
        ~Field();

        u32 ID{};
        u32 persistentID{};
        u32 tableID{};

        string fieldName{};
        string fieldValue{};
        u64 fieldRange = 10;

        FieldEditType editType{};
        FieldValueType valueType{};
        bool isUnsignedOnly{};
    };
}