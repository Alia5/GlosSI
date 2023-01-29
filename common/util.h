/*
Copyright 2021-2023 Peter Repukat - FlatspotSoftware

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <locale>
#include <codecvt>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <tlhelp32.h>
#include <ShlObj.h>
#include <KnownFolders.h>
#endif

#include <filesystem>

#ifdef SPDLOG_H
#include <spdlog/spdlog.h>
#endif

namespace util {
	namespace string
	{
		template <typename T>
		inline std::wstring to_wstring(const T& t)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			return converter.from_bytes(t);
		}

		template <typename T>
		inline std::string to_string(const T& t)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			return converter.to_bytes(t);
		}
	}

	namespace path
	{
		inline std::filesystem::path getDataDirPath()
		{
			wchar_t* localAppDataFolder;
			std::filesystem::path path;
			if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &localAppDataFolder) != S_OK) {
				path = std::filesystem::temp_directory_path().parent_path().parent_path().parent_path();
			}
			else {
				path = std::filesystem::path(localAppDataFolder).parent_path();
			}

			path /= "Roaming";
			path /= "GlosSI";
			if (!std::filesystem::exists(path))
				std::filesystem::create_directories(path);
			return path;
		}
	}

#ifdef _WIN32
	namespace win
	{

		typedef LONG NTSTATUS, * PNTSTATUS;
#define STATUS_SUCCESS (0x00000000)

		typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

		inline RTL_OSVERSIONINFOW GetRealOSVersion()
		{
			HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
			if (hMod) {
				RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
				if (fxPtr != nullptr) {
					RTL_OSVERSIONINFOW rovi = { 0 };
					rovi.dwOSVersionInfoSize = sizeof(rovi);
					if (STATUS_SUCCESS == fxPtr(&rovi)) {
						return rovi;
					}
				}
			}
			RTL_OSVERSIONINFOW rovi = { 0 };
			return rovi;
		}
		namespace process
		{
			inline DWORD PidByName(const std::wstring& name)
			{
				PROCESSENTRY32 entry;
				entry.dwSize = sizeof(PROCESSENTRY32);
				HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
				if (Process32First(snapshot, &entry) == TRUE) {
					while (Process32Next(snapshot, &entry) == TRUE) {
						if (std::wstring(entry.szExeFile).find(name) != std::string::npos) {
							return entry.th32ProcessID;
						}
					}
				}
				CloseHandle(snapshot);
				return 0;
			}

			inline std::wstring GetProcName(DWORD pid)
			{
				PROCESSENTRY32 processInfo;
				processInfo.dwSize = sizeof(processInfo);
				const HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
				if (processesSnapshot == INVALID_HANDLE_VALUE) {
#ifdef SPDLOG_H
					spdlog::trace("util::GetProcName: can't get a process snapshot");
#endif
					return L"";
				}

				for (BOOL bok = Process32First(processesSnapshot, &processInfo);
					bok;
					bok = Process32Next(processesSnapshot, &processInfo)) {
					if (pid == processInfo.th32ProcessID) {
						CloseHandle(processesSnapshot);
						return processInfo.szExeFile;
					}
				}
				CloseHandle(processesSnapshot);
				return L"";
			}

			inline bool KillProcess(DWORD pid)
			{
				auto res = true;
				if (const auto proc = OpenProcess(PROCESS_TERMINATE, FALSE, pid)) {
#ifdef SPDLOG_H
					spdlog::debug("Terminating process: {}", pid);
#endif
					res = TerminateProcess(proc, 0);
					if (!res) {
#ifdef SPDLOG_H
						spdlog::error("Failed to terminate process: {}", pid);
#endif
					}
					CloseHandle(proc);
				}
				return res;
			}
		}
				}
#endif
			}