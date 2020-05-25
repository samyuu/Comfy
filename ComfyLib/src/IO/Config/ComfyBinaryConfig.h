#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "IO/Stream/FileInterfaces.h"
#include <optional>

namespace Comfy::IO
{
	class ComfyBinaryConfig : public IStreamReadable, public IStreamWritable, NonCopyable
	{
	public:
		ComfyBinaryConfig();
		~ComfyBinaryConfig();

	public:
		void Read(StreamReader& reader) override;
		void Write(StreamWriter& writer) override;

	public:
		std::optional<bool> GetBool(std::string_view id) const;
		void SetBool(std::string_view id, bool value);

		std::optional<i32> GetI32(std::string_view id) const;
		void SetI32(std::string_view id, i32 value);

		std::optional<ivec2> GetIVec2(std::string_view id) const;
		void SetIVec2(std::string_view id, ivec2 value);

		std::optional<ivec4> GetIVec4(std::string_view id) const;
		void SetIVec4(std::string_view id, ivec4 value);

		std::optional<f32> GetF32(std::string_view id) const;
		void SetF32(std::string_view id, f32 value);

		std::optional<std::string> GetStr(std::string_view id) const;
		void SetStr(std::string_view id, std::string_view value);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
