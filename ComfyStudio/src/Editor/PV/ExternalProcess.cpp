#include "ExternalProcess.h"
#include <Windows.h>
#include <TlHelp32.h>

namespace Comfy::Editor
{
	namespace
	{
		template <typename T>
		std::optional<T> ReadProcessValue(const ExternalProcess::ProcessData& process, uintptr_t address)
		{
			T valueToRead;

			SIZE_T bytesRead = 0;
			const BOOL result = ::ReadProcessMemory(static_cast<HANDLE>(process.Handle), reinterpret_cast<void*>(address), &valueToRead, sizeof(T), &bytesRead);

			return (result == 0) ? std::optional<T> {} : valueToRead;
		}

		template <typename T>
		void TryReadProcessValueElseReset(std::optional<ExternalProcess::ProcessData>& process, T& valueOut, uintptr_t address)
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
		bool WriteProcessValue(const ExternalProcess::ProcessData& process, uintptr_t address, T valueToWrite)
		{
			SIZE_T bytesWritten = 0;
			const BOOL result = ::WriteProcessMemory(static_cast<HANDLE>(process.Handle), reinterpret_cast<void*>(address), &valueToWrite, sizeof(T), &bytesWritten);

			return (result != 0);
		}

		template <typename T>
		void TryWriteProcessValueElseReset(std::optional<ExternalProcess::ProcessData>& process, T valueIn, uintptr_t address)
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

		settings.Addresses = *(reinterpret_cast<const ProcessSettings::AddressData*>(buffer));
		buffer += sizeof(ProcessSettings::AddressData);

		assert(std::strcmp(settings.Addresses.CameraIdentifier.data(), "camera") == 0);
		assert(std::strcmp(settings.Addresses.LightParamIdentifier.data(), "light_param") == 0);
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

	ExternalProcess::CameraData ExternalProcess::ReadCamera()
	{
		CameraData cameraData {};
		if (!process.has_value())
			return cameraData;

		TryReadProcessValueElseReset(process, cameraData.ViewPoint, settings.Addresses.Camera.ViewPoint);
		TryReadProcessValueElseReset(process, cameraData.Interest, settings.Addresses.Camera.Interest);
		TryReadProcessValueElseReset(process, cameraData.Rotation, settings.Addresses.Camera.Rotation);
		TryReadProcessValueElseReset(process, cameraData.FieldOfView, settings.Addresses.Camera.FieldOfView);

		return cameraData;
	}

	void ExternalProcess::WriteCamera(const CameraData& cameraData)
	{
		if (!process.has_value())
			return;

		TryWriteProcessValueElseReset(process, cameraData.ViewPoint, settings.Addresses.Camera.ViewPoint);
		TryWriteProcessValueElseReset(process, cameraData.Interest, settings.Addresses.Camera.Interest);
		TryWriteProcessValueElseReset(process, cameraData.Rotation, settings.Addresses.Camera.Rotation);
		TryWriteProcessValueElseReset(process, cameraData.FieldOfView, settings.Addresses.Camera.FieldOfView);
	}

	ExternalProcess::LightParamData ExternalProcess::ReadLightParam()
	{
		LightParamData lightData {};
		if (!process.has_value())
			return lightData;

		TryReadProcessValueElseReset(process, lightData.Character.Ambient, settings.Addresses.LightParam.Character.Ambient);
		TryReadProcessValueElseReset(process, lightData.Character.Diffuse, settings.Addresses.LightParam.Character.Diffuse);
		TryReadProcessValueElseReset(process, lightData.Character.Specular, settings.Addresses.LightParam.Character.Specular);
		TryReadProcessValueElseReset(process, lightData.Character.Position, settings.Addresses.LightParam.Character.Position);

		TryReadProcessValueElseReset(process, lightData.Stage.Ambient, settings.Addresses.LightParam.Stage.Ambient);
		TryReadProcessValueElseReset(process, lightData.Stage.Diffuse, settings.Addresses.LightParam.Stage.Diffuse);
		TryReadProcessValueElseReset(process, lightData.Stage.Specular, settings.Addresses.LightParam.Stage.Specular);
		TryReadProcessValueElseReset(process, lightData.Stage.Position, settings.Addresses.LightParam.Stage.Position);

		TryReadProcessValueElseReset(process, lightData.IBLCharacter.Color, settings.Addresses.LightParam.IBLCharacter.Color);
		// NOTE: Unused
		// TryReadProcessValueElseReset(process, lightData.IBLCharacter.Matrices, settings.Addresses.LightParam.IBLCharacter.Matrices);
		TryReadProcessValueElseReset(process, lightData.IBLStage.Color, settings.Addresses.LightParam.IBLStage.Color);
		TryReadProcessValueElseReset(process, lightData.IBLStage.Matrices, settings.Addresses.LightParam.IBLStage.Matrices);

		return lightData;
	}

	void ExternalProcess::WriteLightParam(const LightParamData& lightData)
	{
		if (!process.has_value())
			return;

		TryWriteProcessValueElseReset(process, lightData.Character.Ambient, settings.Addresses.LightParam.Character.Ambient);
		TryWriteProcessValueElseReset(process, lightData.Character.Diffuse, settings.Addresses.LightParam.Character.Diffuse);
		TryWriteProcessValueElseReset(process, lightData.Character.Specular, settings.Addresses.LightParam.Character.Specular);
		TryWriteProcessValueElseReset(process, lightData.Character.Position, settings.Addresses.LightParam.Character.Position);

		TryWriteProcessValueElseReset(process, lightData.Stage.Ambient, settings.Addresses.LightParam.Stage.Ambient);
		TryWriteProcessValueElseReset(process, lightData.Stage.Diffuse, settings.Addresses.LightParam.Stage.Diffuse);
		TryWriteProcessValueElseReset(process, lightData.Stage.Specular, settings.Addresses.LightParam.Stage.Specular);
		TryWriteProcessValueElseReset(process, lightData.Stage.Position, settings.Addresses.LightParam.Stage.Position);

		TryWriteProcessValueElseReset(process, lightData.IBLCharacter.Color, settings.Addresses.LightParam.IBLCharacter.Color);
		// NOTE: Unused
		// TryWriteProcessValueElseReset(process, lightData.IBLCharacter.Matrices, settings.Addresses.LightParam.IBLCharacter.Matrices);
		TryWriteProcessValueElseReset(process, lightData.IBLStage.Color, settings.Addresses.LightParam.IBLStage.Color);
		TryWriteProcessValueElseReset(process, lightData.IBLStage.Matrices, settings.Addresses.LightParam.IBLStage.Matrices);
	}

	ExternalProcess::ProcessData ExternalProcess::GetProcess() const
	{
		return process.value_or(ProcessData {});
	}

	const ExternalProcess::ProcessSettings& ExternalProcess::GetSettings() const
	{
		return settings;
	}
}
