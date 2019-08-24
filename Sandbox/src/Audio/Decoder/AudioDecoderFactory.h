#pragma once
#include "Types.h"
#include "IAudioDecoder.h"
#include "Audio/SampleProvider/MemorySampleProvider.h"
#include <vector>

namespace Audio
{
	class AudioDecoderFactory
	{
		static UniquePtr<AudioDecoderFactory> instance;

	public:
		AudioDecoderFactory(const AudioDecoderFactory& other) = delete;
		~AudioDecoderFactory();

		RefPtr<MemorySampleProvider> DecodeFile(const std::string& filePath);
		static AudioDecoderFactory* GetInstance();

	private:
		AudioDecoderFactory();

	protected:
		std::vector<UniquePtr<IAudioDecoder>> availableDecoders;
	
		template <class T>
		inline void RegisterDecoder()
		{
			static_assert(std::is_base_of<IAudioDecoder, T>::value, "T must inherit from IAudioDecoder");
			availableDecoders.push_back(MakeUnique<T>());
		}

		void RegisterAllDecoders();
	};
}