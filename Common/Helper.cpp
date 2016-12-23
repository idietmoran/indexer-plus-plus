// This file is the part of the Indexer++ project.
// Copyright (C) 2016 Anna Krykora <krykoraanna@gmail.com>. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.


#include "Helper.h"
#include "WindowsWrapper.h"

#include <string.h>
#include <iostream>
#include <sstream>
#include <thread>

namespace indexer_common {

    using namespace std;


#ifdef WIN32
    const char16_t* Empty16String = reinterpret_cast<char16_t*>(L"");
#else
    const char16_t* Empty16String = u"";
#endif


    char16_t* Helper::CopyU16StringToChar16(const u16string& s) {
        auto res = new char16_t[s.size() + 1];

        memcpy(res, s.c_str(), (s.size() + 1) * sizeof(char16_t));

        return res;
    }

    wstring Helper::StringToWstring(const string& str) {
        wstring wstr(str.size(), L'\0');

        for (size_t i = 0; i < str.size(); ++i) {
            wstr[i] = str[i];
        }

        return wstr;
    }

    string Helper::WStringToString(const wstring& w_str) {
        string str(w_str.size(), '\0');

        for (size_t i = 0; i < w_str.size(); ++i) {
            str[i] = static_cast<char>(w_str[i]);
        }

        return str;
    }

    wstring Helper::U16stringToWstring(const u16string& s) {
        wstring wstr(s.size(), L'\0');

        for (size_t i = 0; i < s.size(); ++i) {
            wstr[i] = s[i];
        }

        return wstr;
    }

    u16string Helper::WstringToU16string(const wstring& s) {
        u16string res(s.size(), L'\0');

        for (size_t i = 0; i < s.size(); ++i) {
            res[i] = s[i];
        }

        return res;
    }

    const wstring Helper::Char16ToWstring(const char16_t* s) {
        return wstring(reinterpret_cast<const wchar_t*>(s));
    }

    bool Helper::Utf16ToUtf8(const u16string& source_utf_16, char* dest_utf_8_buffer, int buffer_size) {

        int source_size = static_cast<int>(source_utf_16.size());
        if (source_size == 0) {
            dest_utf_8_buffer[0] = '\0';  // add a null-terminator.
            return true;
        }

        auto source = reinterpret_cast<const wchar_t*>(source_utf_16.data());

        int target_size = WideCharToMultiByte(CP_UTF8, 0, source, source_size, NULL, 0, NULL, NULL);
        target_size = min(target_size, buffer_size);
        int result = WideCharToMultiByte(CP_UTF8, 0, source, source_size, dest_utf_8_buffer, target_size, NULL, NULL);

        if (result == 0)  // convention error
            return false;

        dest_utf_8_buffer[target_size] = '\0';  // add a null-terminator.
        return true;
    }

    int Helper::Str16Len(const char16_t* s) {
        auto e = s;

        while (*e != '\0')
            ++e;

        return e - s;
    }

    uint64 Helper::PairDwordToInt64(DWORD high, DWORD low) {
        return (static_cast<uint64>(high) << 32) + low;
    }

    uint64 Helper::LargeIntegerToInt64(LARGE_INTEGER li) {
        return (static_cast<uint64>(li.HighPart) << 32) + li.LowPart;
    }

    wstring Helper::IntToHex(DWORD x) {
        wstringstream sstream;
        sstream << hex << x;
        wstring result = sstream.str();
        return result;
    }

    u16string Helper::ToUpper(u16string& s) {
        wcout.imbue(locale());
        auto& f = use_facet<ctype<char16_t>>(locale());
        f.toupper(&s[0], &s[0] + s.size());
        return s;
    }

    wstring Helper::ToUpper(wstring& s) {
        wcout.imbue(locale());
        auto& f = use_facet<ctype<wchar_t>>(locale());
        f.toupper(&s[0], &s[0] + s.size());
        return s;
    }

    wstring Helper::GetLastErrorString() {
#ifdef WIN32
        DWORD error = GetLastError();

        LPVOID lp_msg_buff;

        DWORD buff_len =
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                          error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lp_msg_buff, 0, NULL);

        if (buff_len) {
            LPCSTR lp_msg_str = (LPCSTR)lp_msg_buff;
            string result(lp_msg_str, lp_msg_str + buff_len * 2);

            LocalFree(lp_msg_buff);

            return StringToWstring(result);
        }
#endif
        return wstring();
    }

    bool Helper::DirExist(const wstring& path) {
#ifdef WIN32  // TODO is it needed to implement on Linux?
        DWORD attr = GetFileAttributes(path.c_str());

        return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
#else
        return true;
#endif
    }

    char16_t* Helper::GetFilename(const USN_RECORD& record, ushort* name_length) {
        *name_length = (ushort)(record.FileNameLength / 2);

        char16_t* filename = new char16_t[*name_length + 1];
        char16_t* filenameInRecord = (char16_t*)((unsigned char*)&record + record.FileNameOffset);
        memcpy(filename, filenameInRecord, record.FileNameLength);
        *(filename + record.FileNameLength / 2) = L'\0';

        return filename;
    }

    const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)

    typedef struct tagTHREADNAME_INFO {
        DWORD dwType;      // Must be 0x1000.
        LPCSTR szName;     // Pointer to name (in user addr space).
        DWORD dwThreadID;  // Thread ID (-1=caller thread).
        DWORD dwFlags;     // Reserved for future use, must be zero.
    } THREADNAME_INFO;

#pragma pack(pop)

    void Helper::SetThreadName(thread* thread, const char* thread_name) {
#ifdef WIN32
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = thread_name;
        info.dwThreadID = GetThreadId(thread->native_handle());
        info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable : 6320 6322)
        __try {
            RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
#pragma warning(pop)
#endif
    }

    bool Helper::IsAsciiString(const wchar_t* s) {
        for (; *s != '\0'; ++s)
            if (*s > 127) return false;

        return true;
    }

    char16_t* Helper::GetDriveName(char16_t drive_letter) {
        auto drive_name = new char16_t[3];
        drive_name[0] = drive_letter;
        drive_name[1] = ':';
        drive_name[2] = '\0';
        return drive_name;
    }

    uint Helper::GetNumberOfProcessors() {
#ifdef WIN32
        // May return 0 when not able to detect.
        auto num = thread::hardware_concurrency();

        if (!num)  // Try WinAPI.
        {
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);

            num = sysinfo.dwNumberOfProcessors;
        }

        if (!num) num = 2;  // Fallback value;

        return num;
#else
        return 4;
#endif
    }

    int Helper::SizeFromBytesToKiloBytes(uint64 size_in_bytes) {
        return static_cast<int>((size_in_bytes + 1023) >> 10);
    }

    void WriteToOutput(const string& s) {
        cerr << "  LogMessage threadid=" << this_thread::get_id() << " " << s << "\n";
    }

    void WriteToOutput(const wstring& s) {
        wcerr << L"  LogMessage threadid=" << this_thread::get_id() << L" " << s << L"\n";
    }

} // namespace indexer_common