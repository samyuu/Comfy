#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "IDecoder.h"
#include "Audio/SampleProvider/ISampleProvider.h"

namespace Comfy::Audio
{
	class DecoderFactory : NonCopyable
	{
	public:
		DecoderFactory();
		~DecoderFactory() = default;

		std::unique_ptr<ISampleProvider> DecodeFile(std::string_view filePath);

	public:
		static DecoderFactory& GetInstance();

	private:
		std::vector<std::unique_ptr<IDecoder>> availableDecoders;
	
		template <typename T>
		void RegisterDecoder()
		{
			static_assert(std::is_base_of_v<IDecoder, T>, "T must inherit from IAudioDecoder");
			availableDecoders.push_back(std::make_unique<T>());
		}

		void RegisterAllDecoders();
	};
}
