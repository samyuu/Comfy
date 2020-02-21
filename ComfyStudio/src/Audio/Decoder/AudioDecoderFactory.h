#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "IAudioDecoder.h"
#include "Audio/SampleProvider/MemorySampleProvider.h"

namespace Audio
{
	class AudioDecoderFactory : NonCopyable
	{
		static UniquePtr<AudioDecoderFactory> instance;

	public:
		~AudioDecoderFactory();

		RefPtr<MemorySampleProvider> DecodeFile(std::string_view filePath);
		static AudioDecoderFactory* GetInstance();

	private:
		AudioDecoderFactory();

	protected:
		std::vector<UniquePtr<IAudioDecoder>> availableDecoders;
	
		template <typename T>
		inline void RegisterDecoder()
		{
			static_assert(std::is_base_of<IAudioDecoder, T>::value, "T must inherit from IAudioDecoder");
			availableDecoders.push_back(MakeUnique<T>());
		}

		void RegisterAllDecoders();
	};
}