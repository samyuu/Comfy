#include "ExternalProcess.h"
#include <Windows.h>
#include <TlHelp32.h>

namespace Editor
{
	namespace
	{
		template <typename T>
		std::optional<T> ReadProcessValue(const ProcessData& process, uintptr_t address)
		{
			T valueToRead;

			SIZE_T bytesRead = 0;
			const BOOL result = ::ReadProcessMemory(static_cast<HANDLE>(process.Handle), reinterpret_cast<void*>(address), &valueToRead, sizeof(T), &bytesRead);

			return (result == 0) ? std::optional<T> {} : valueToRead;
		}

		template <typename T>
		void TryReadProcessValueElseReset(std::optional<ProcessData>& process, T& valueOut, uintptr_t address)
		{
			if (!process.has_value())
				return;

			auto readValue = ReadProcessValue<T>(process.value(), address);

			if (readValue.has_value())
				valueOut = readValue.value();
			else
				process.reset();
		}

		template <typename T>
		bool WriteProcessValue(const ProcessData& process, uintptr_t address, T valueToWrite)
		{
			SIZE_T bytesWritten = 0;
			const BOOL result = ::WriteProcessMemory(static_cast<HANDLE>(process.Handle), reinterpret_cast<void*>(address), &valueToWrite, sizeof(T), &bytesWritten);

			return (result != 0);
		}

		template <typename T>
		void TryWriteProcessValueElseReset(std::optional<ProcessData>& process, T valueIn, uintptr_t address)
		{
			if (!process.has_value())
				return;

			bool valueWritten = WriteProcessValue(process.value(), address, valueIn);

			if (!valueWritten)
				process.reset();
		}
	}

	void ExternalProcess::ParseConfig(const uint8_t* buffer, size_t size)
	{
		constexpr size_t processNameMaxLength = 64;

		settings.ProcessName = reinterpret_cast<const char*>(buffer);
		buffer += processNameMaxLength;

		settings.Addresses.ViewPoint = *(reinterpret_cast<const uintptr_t*>(buffer) + 0);
		settings.Addresses.Interest = *(reinterpret_cast<const uintptr_t*>(buffer) + 1);
		settings.Addresses.Rotation = *(reinterpret_cast<const uintptr_t*>(buffer) + 2);
		settings.Addresses.FieldOfView = *(reinterpret_cast<const uintptr_t*>(buffer) + 3);
	}

	bool ExternalProcess::IsAttached() const
	{
		return process.has_value();
	}

	void ExternalProcess::Attach()
	{
		process.reset();

		if (settings.ProcessName.empty())
			return;

		HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		PROCESSENTRY32 processEntry = {};
		processEntry.dwSize = sizeof(processEntry);
		
		if (::Process32First(snapshot, &processEntry))
		{
			do
			{
				if (std::string_view(processEntry.szExeFile) == settings.ProcessName)
				{
					process.emplace();
					process->ID = processEntry.th32ProcessID;
					process->Handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, process->ID);

					if (process->Handle == NULL)
						process.reset();
					else
						break;
				}
			}
			while (::Process32Next(snapshot, &processEntry));
		}

		::CloseHandle(snapshot);
	}

	void ExternalProcess::Detach()
	{
		process.reset();
	}

	ProcessCameraData ExternalProcess::ReadCamera()
	{
		ProcessCameraData cameraData {};

		if (!process.has_value())
			return cameraData;

		TryReadProcessValueElseReset(process, cameraData.ViewPoint, settings.Addresses.ViewPoint);
		TryReadProcessValueElseReset(process, cameraData.Interest, settings.Addresses.Interest);
		TryReadProcessValueElseReset(process, cameraData.Rotation, settings.Addresses.Rotation);
		TryReadProcessValueElseReset(process, cameraData.FieldOfView, settings.Addresses.FieldOfView);

		return cameraData;
	}

	void ExternalProcess::WriteCamera(const ProcessCameraData& cameraData)
	{
		if (!process.has_value())
			return;

		TryWriteProcessValueElseReset(process, cameraData.ViewPoint, settings.Addresses.ViewPoint);
		TryWriteProcessValueElseReset(process, cameraData.Interest, settings.Addresses.Interest);
		TryWriteProcessValueElseReset(process, cameraData.Rotation, settings.Addresses.Rotation);
		TryWriteProcessValueElseReset(process, cameraData.FieldOfView, settings.Addresses.FieldOfView);
	}

	ProcessData ExternalProcess::GetProcess() const
	{
		return process.value_or(ProcessData {});
	}

	const ProcessSettings& ExternalProcess::GetSettings() const
	{
		return settings;
	}
}
