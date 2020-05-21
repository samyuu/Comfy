#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include <optional>

namespace Comfy::Studio::Editor
{
	class ExternalProcess
	{
	public:
		struct ProcessData
		{
			u32 ID;
			void* Handle;
		};

		struct ProcessSettings
		{
			struct AddressData
			{
				std::array<char, 16> CameraIdentifier;
				struct Camera
				{
					uintptr_t ViewPoint;
					uintptr_t Interest;
					uintptr_t Rotation;
					uintptr_t FieldOfView;
				} Camera;
				std::array<char, 16> LightParamIdentifier;
				struct LightParam
				{
					struct Light
					{
						uintptr_t Ambient;
						uintptr_t Diffuse;
						uintptr_t Specular;
						uintptr_t Position;
					} Character, Stage;
					struct IBLLight
					{
						uintptr_t LightColor;
						uintptr_t IrradianceRGB;
					} IBL0, IBL1;
				} LightParam;
			};

			std::string ProcessName;
			AddressData Addresses;
		};

		struct CameraData
		{
			vec3 ViewPoint, Interest;
			float Rotation, FieldOfView;
		};

		struct LightParamData
		{
			struct Light
			{
				vec4 Ambient, Diffuse, Specular;
				vec3 Position;
			} Character, Stage;
			struct IBLLight
			{
				vec3 LightColor;
				std::array<mat4, 3> IrradianceRGB;
			} IBL0, IBL1;
		};

	public:
		void ParseConfig(const u8* buffer, size_t size);

		bool IsAttached() const;
		void Attach();
		void Detach();

		CameraData ReadCamera();
		void WriteCamera(const CameraData& cameraData);

		LightParamData ReadLightParam();
		void WriteLightParam(const LightParamData& lightData);

		ProcessData GetProcess() const;
		const ProcessSettings& GetSettings() const;

	private:
		std::optional<ProcessData> process;
		ProcessSettings settings;
	};
}
