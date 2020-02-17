#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include <optional>

namespace Editor
{
	struct ProcessData
	{
		uint32_t ID;
		void* Handle;
	};

	struct ProcessSettings
	{
		std::string ProcessName;
		struct
		{
			uintptr_t ViewPoint;
			uintptr_t Interest;
			uintptr_t Rotation;
			uintptr_t FieldOfView;
		} Addresses;
	};

	struct ProcessCameraData
	{
		vec3 ViewPoint, Interest;
		float Rotation, FieldOfView;
	};

	class ExternalProcess
	{
	public:
		void ParseConfig(const uint8_t* buffer, size_t size);
		
		bool IsAttached() const;
		void Attach();
		void Detach();

		ProcessCameraData ReadCamera();
		void WriteCamera(const ProcessCameraData& cameraData);

		ProcessData GetProcess() const;
		const ProcessSettings& GetSettings() const;

	private:
		std::optional<ProcessData> process;
		ProcessSettings settings;
	};
}
