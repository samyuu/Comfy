#include "ComfyBinaryConfig.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/Stream/Manipulator/StreamWriter.h"

namespace Comfy::IO
{
	struct ComfyBinaryConfig::Impl
	{
		struct DataTypeDescription
		{
			struct FlagsData
			{
				u32 Integer : 1;
				u32 Signed : 1;
				u32 FloatingPoint : 1;
				u32 Array : 1;
				u32 String : 1;
				u32 Reserved : 27;
			} Flags;

			u32 Reserved;
			size_t Size;

			DataTypeDescription()
			{
				std::memset(this, 0, sizeof(this));
				Flags.Reserved = StreamManipulator::PaddingValue;
				Reserved = StreamManipulator::PaddingValue;

				static_assert(sizeof(*this) == 16);
			}
		};

		struct ComfyHeader
		{
			std::array<u8, 4> Magic = { 0xCF, 0xCC, 0xAC, 0x90 };

			struct ComfyVersion
			{
				u8 Major = 0x02;
				u8 Minor = 0x00;
				u16 Reserved = 0xCCCC;
			} Version;

			std::array<u8, 4> CreatorID = { 'c', 'm', 'f', 'y' };
			std::array<u8, 4> ReservedID = { 0x90, 0x90, 0x90, 0x90 };

		} Header;

		struct DataTypeData
		{
			template <typename T>
			struct IDValue
			{
				std::string ID;
				T Value;

				IDValue(std::string id, T value) : ID(std::move(id)), Value(std::move(value)) {}
			};

			std::vector<IDValue<i32>> I32;
			std::vector<IDValue<f32>> F32;
			std::vector<IDValue<std::string>> Str;

			template <typename T>
			std::optional<T> Get(const std::vector<IDValue<T>>& toSerach, std::string_view id) const
			{
				auto found = std::find_if(toSerach.begin(), toSerach.end(), [&](const auto& idValue) { return (idValue.ID == id); });
				return (found != toSerach.end()) ? found->Value : std::optional<T> {};
			}

			template <typename T>
			void Set(std::vector<IDValue<T>>& toSerach, std::string_view id, T value)
			{
				auto existing = std::find_if(toSerach.begin(), toSerach.end(), [&](const auto& idValue) { return (idValue.ID == id); });
				if (existing != toSerach.end())
					existing->Value = std::move(value);
				else
					toSerach.emplace_back(std::string(id), std::move(value));
			}

			static constexpr std::string_view VectorComponents = "xyzw";

			template <typename Vec, typename Getter>
			std::optional<Vec> GetVec(std::string_view id, Getter getter)
			{
				Vec resultValue;

				auto updatedID = std::string(id) + "._";
				for (auto i = 0; i < Vec::length(); i++)
				{
					updatedID.back() = VectorComponents[i];

					if (auto value = getter(updatedID); !value.has_value())
						return std::optional<Vec> {};
					else
						resultValue[i] = value.value();
				}

				return resultValue;
			}

			template <typename Vec, typename Setter>
			void SetVec(std::string_view id, Vec value, Setter setter)
			{
				char stackIDBuffer[260];
				if (id.size() < (std::size(stackIDBuffer) + 3) && !id.empty())
				{
					const size_t bufferStrLen = (id.size() + 2);

					std::memcpy(stackIDBuffer, id.data(), id.size());
					stackIDBuffer[id.size() + 0] = '.';
					stackIDBuffer[id.size() + 2] = '\0';
					char& componentChar = stackIDBuffer[id.size() + 1];

					for (auto i = 0; i < Vec::length(); i++)
					{
						componentChar = VectorComponents[i];
						setter(std::string_view(stackIDBuffer, bufferStrLen), value[i]);
					}
				}
				else
				{
					auto stringIDBuffer = std::string(id) + "._";
					for (auto i = 0; i < Vec::length(); i++)
					{
						stringIDBuffer.back() = VectorComponents[i];
						setter(stringIDBuffer, value[i]);
					}
				}
			}

		} Data;

		StreamResult Read(StreamReader& reader)
		{
			reader.ReadBuffer(&Header, sizeof(Header));
			if (Header.Version.Major < 2)
				return StreamResult::BadFormat;

			const size_t entryCount = reader.ReadSize();
			const size_t i32Count = reader.ReadSize();
			const size_t f32Count = reader.ReadSize();
			const size_t strCount = reader.ReadSize();

			Data.I32.reserve(i32Count);
			Data.F32.reserve(f32Count);
			Data.Str.reserve(strCount);

			for (size_t i = 0; i < entryCount; i++)
			{
				DataTypeDescription typeDescription;
				reader.ReadBuffer(&typeDescription, sizeof(typeDescription));

				auto id = reader.ReadStr();

				if (typeDescription.Flags.Integer && typeDescription.Flags.Signed && typeDescription.Size == sizeof(i32))
					Data.I32.emplace_back(std::move(id), reader.ReadI32());
				else if (typeDescription.Flags.FloatingPoint && typeDescription.Size == sizeof(f32))
					Data.F32.emplace_back(std::move(id), reader.ReadF32());
				else if (typeDescription.Flags.Array && typeDescription.Flags.String)
					Data.Str.emplace_back(std::move(id), reader.ReadStrPtrOffsetAware());
				else // NOTE: Assume additional data is pointed to
					reader.ReadPtr();
			}

			return StreamResult::Success;
		}

		StreamResult Write(StreamWriter& writer)
		{
			writer.WriteBuffer(&Header, sizeof(Header));

			const size_t entryCount = (Data.I32.size() + Data.F32.size() + Data.Str.size());
			writer.WriteSize(entryCount);
			writer.WriteSize(Data.I32.size());
			writer.WriteSize(Data.F32.size());
			writer.WriteSize(Data.Str.size());

			{
				DataTypeDescription i32TypeDescription;
				i32TypeDescription.Flags.Integer = true;
				i32TypeDescription.Flags.Signed = true;
				i32TypeDescription.Size = sizeof(i32);

				for (const auto& entry : Data.I32)
				{
					writer.WriteBuffer(&i32TypeDescription, sizeof(i32TypeDescription));
					writer.WriteStr(entry.ID);
					writer.WriteI32(entry.Value);
				}
			}

			{
				DataTypeDescription f32TypeDescription;
				f32TypeDescription.Flags.FloatingPoint = true;
				f32TypeDescription.Size = sizeof(f32);

				for (const auto& entry : Data.F32)
				{
					writer.WriteBuffer(&f32TypeDescription, sizeof(f32TypeDescription));
					writer.WriteStr(entry.ID);
					writer.WriteF32(entry.Value);
				}
			}

			{
				DataTypeDescription strTypeDescription;
				strTypeDescription.Flags.Array = true;
				strTypeDescription.Flags.String = true;

				for (const auto& entry : Data.Str)
				{
					strTypeDescription.Size = entry.Value.size();

					writer.WriteBuffer(&strTypeDescription, sizeof(strTypeDescription));
					writer.WriteStr(entry.ID);
					writer.WriteStrPtr(entry.Value);
				}
			}

			writer.WriteAlignmentPadding(0x10);
			writer.FlushStringPointerPool();
			writer.WriteAlignmentPadding(0x10);

			return StreamResult::Success;
		}
	};

	ComfyBinaryConfig::ComfyBinaryConfig() : impl(std::make_unique<Impl>())
	{
	}

	ComfyBinaryConfig::~ComfyBinaryConfig()
	{
	}

	StreamResult ComfyBinaryConfig::Read(StreamReader& reader)
	{
		return impl->Read(reader);
	}

	StreamResult ComfyBinaryConfig::Write(StreamWriter& writer)
	{
		return impl->Write(writer);
	}

	std::optional<bool> ComfyBinaryConfig::GetBool(std::string_view id) const
	{
		auto result = GetI32(id);
		return result.has_value() ? static_cast<bool>(result.value()) : std::optional<bool> {};
	}

	void ComfyBinaryConfig::SetBool(std::string_view id, bool value)
	{
		SetI32(id, static_cast<i32>(value));
	}

	std::optional<i32> ComfyBinaryConfig::GetI32(std::string_view id) const
	{
		return impl->Data.Get<i32>(impl->Data.I32, id);
	}

	void ComfyBinaryConfig::SetI32(std::string_view id, i32 value)
	{
		return impl->Data.Set<i32>(impl->Data.I32, id, value);
	}

	std::optional<ivec2> ComfyBinaryConfig::GetIVec2(std::string_view id) const
	{
		return impl->Data.GetVec<ivec2>(id, [&](std::string_view id) { return GetI32(id); });
	}

	void ComfyBinaryConfig::SetIVec2(std::string_view id, ivec2 value)
	{
		impl->Data.SetVec<ivec2>(id, value, [&](std::string_view id, i32 value) { SetI32(id, value); });
	}

	std::optional<ivec4> ComfyBinaryConfig::GetIVec4(std::string_view id) const
	{
		return impl->Data.GetVec<ivec4>(id, [&](std::string_view id) { return GetI32(id); });
	}

	void ComfyBinaryConfig::SetIVec4(std::string_view id, ivec4 value)
	{
		impl->Data.SetVec<ivec4>(id, value, [&](std::string_view id, i32 value) { SetI32(id, value); });
	}

	std::optional<f32> ComfyBinaryConfig::GetF32(std::string_view id) const
	{
		return impl->Data.Get<f32>(impl->Data.F32, id);
	}

	void ComfyBinaryConfig::SetF32(std::string_view id, f32 value)
	{
		return impl->Data.Set<f32>(impl->Data.F32, id, value);
	}

	std::optional<std::string> ComfyBinaryConfig::GetStr(std::string_view id) const
	{
		return impl->Data.Get<std::string>(impl->Data.Str, id);
	}

	void ComfyBinaryConfig::SetStr(std::string_view id, std::string_view value)
	{
		return impl->Data.Set<std::string>(impl->Data.Str, id, std::string(value));
	}
}
