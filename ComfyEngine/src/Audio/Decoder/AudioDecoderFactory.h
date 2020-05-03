#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "IAudioDecoder.h"
#include "Audio/SampleProvider/MemorySampleProvider.h"

namespace Comfy::Audio
{
	class AudioDecoderFactory : NonCopyable
	{
		static std::unique_ptr<AudioDecoderFactory> instance;

	public:
		~AudioDecoderFactory();

		std::shared_ptr<MemorySampleProvider> DecodeFile(std::string_view filePath);
		static AudioDecoderFactory* GetInstance();

	private:
		AudioDecoderFactory();

	protected:
		std::vector<std::unique_ptr<IAudioDecoder>> availableDecoders;
	
		template <typename T>
		void RegisterDecoder()
		{
			static_assert(std::is_base_of<IAudioDecoder, T>::value, "T must inherit from IAudioDecoder");
			availableDecoders.push_back(std::make_unique<T>());
		}

		void RegisterAllDecoders();
	};
}
