// This file is the part of the Indexer++ project.
// Copyright (C) 2016 Anna Krykora <krykoraanna@gmail.com>. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.

#pragma once

#include <string>
#include <vector>

#include "Macros.h"

// Parses and stores commandline arguments. All of them are used for testing purposes.
namespace indexer_common {

    class CommandlineArguments {
       public:
        NO_COPY(CommandlineArguments)

        static CommandlineArguments& Instance() {
            static CommandlineArguments instance_;
            return instance_;
        }

        static void Parse(const std::vector<std::wstring>& command_line_args);

        // If set to true, the test framework serializes all received USN Journal records to file.
        bool SaveUSNJournalRecords;

        // If set to true, the test framework serializes created FileInfo objects from MFT records to file.
        bool SaveFileInfos;

        // If set to true, the test framework serializes raw MFT to a file.
        // The file path is specified in RawMFTSerializer ctor.
        bool SaveRawMFT;

        // Serialized USN Journal records file path for the test framework.
        std::wstring ReplayUSNRecPath;

        // Serialized FileInfo objects file path for the test framework.
        std::wstring ReplayFileInfosPath;

        // Serialized NTFS volume raw MFT file path for the test framework.
        std::wstring RawMFTPath;

       private:
        CommandlineArguments();

		static void SetPath(const std::wstring& arg, const wchar_t* path_arg_name, std::wstring* path);

		static void SetBoolFlag(const std::wstring& arg, const wchar_t* flag_name, bool* flag);

		constexpr static wchar_t* kSaveParsedInFileInfoRecordArgName{ L"saveFileInfos" };

		constexpr static wchar_t* kReplayFileInfosPathArgName{ L"replayFileInfosPath" };

		constexpr static wchar_t* kSaveUSNJournalRecordsArgName{ L"saveUSNJournalRecords" };

		constexpr static wchar_t* kReplayUSNRecPathArgName{ L"replayUSNRecPath" };

		constexpr static wchar_t* kSaveRawMFTArgName{ L"saveRawMFT" };

		constexpr static wchar_t* kRawMFTPathArgName{ L"rawMFTPath" };
    };

} // namespace indexer_common