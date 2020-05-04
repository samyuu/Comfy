#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "IAudioDecoder.h"
#include "Audio/SampleProvider/MemorySampleProvider.h"

namespace Comfy::Audio
{
	class AudioDecoderFactory : NonCopyable
	{
	public:
		AudioDecoderFactory();
		~AudioDecoderFactory() = default;

		std::unique_ptr<ISampleProvider> DecodeFile(std::string_view filePath);

	public:
		static AudioDecoderFactory& GetInstance();

	private:
		std::vector<std::unique_ptr<IAudioDecoder>> availableDecoders;
	
		template <typename T>
		void RegisterDecoder()
		{
			static_assert(std::is_base_of_v<IAudioDecoder, T>, "T must inherit from IAudioDecoder");
			availableDecoders.push_back(std::make_unique<T>());
		}

		void RegisterAllDecoders();
	};
}
