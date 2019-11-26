#include "AetSet.h"
#include "FileSystem/BinaryReader.h"
#include "FileSystem/BinaryWriter.h"

using namespace FileSystem;

namespace Graphics
{
	namespace
	{
		uint32_t ReadColor(BinaryReader& reader)
		{
			uint32_t value = 0;
			*((uint8_t*)&value + 0) = reader.ReadUInt8();
			*((uint8_t*)&value + 1) = reader.ReadUInt8();
			*((uint8_t*)&value + 2) = reader.ReadUInt8();
			reader.SetPosition(reader.GetPosition() + 1);
			return value;
		}

		void ReadKeyFramesPointer(KeyFrameCollection& keyFrames, BinaryReader& reader)
		{
			size_t keyFrameCount = reader.ReadSize();
			void* keyFramesPointer = reader.ReadPtr();

			if (keyFrameCount > 0 && keyFramesPointer != nullptr)
			{
				reader.ReadAt(keyFramesPointer, [keyFrameCount, &keyFrames](BinaryReader& reader)
				{
					keyFrames.resize(keyFrameCount);

					if (keyFrameCount == 1)
					{
						keyFrames.front().Value = reader.ReadFloat();
					}
					else
					{
						for (size_t i = 0; i < keyFrameCount; i++)
							keyFrames[i].Frame = reader.ReadFloat();

						for (size_t i = 0; i < keyFrameCount; i++)
						{
							keyFrames[i].Value = reader.ReadFloat();
							keyFrames[i].Curve = reader.ReadFloat();
						}
					}
				});
			}
		}

		void ReadKeyFrameProperties(AetKeyFrameProperties* properties, BinaryReader& reader)
		{
			for (KeyFrameCollection& keyFrames : *properties)
				ReadKeyFramesPointer(keyFrames, reader);
		}

		void WriteKeyFramesPointer(KeyFrameCollection& keyFrames, BinaryWriter& writer)
		{
			if (keyFrames.size() > 0)
			{
				writer.WriteUInt32(static_cast<uint32_t>(keyFrames.size()));
				writer.WritePtr([&keyFrames](BinaryWriter& writer)
				{
					if (keyFrames.size() == 1)
					{
						writer.WriteFloat(keyFrames.front().Value);
					}
					else
					{
						for (AetKeyFrame& keyFrame : keyFrames)
							writer.WriteFloat(keyFrame.Frame);

						for (AetKeyFrame& keyFrame : keyFrames)
						{
							writer.WriteFloat(keyFrame.Value);
							writer.WriteFloat(keyFrame.Curve);
						}
					}
				});
			}
			else
			{
				writer.WriteUInt32(0x00000000); // key frames count
				writer.WritePtr(nullptr);		// key frames offset
			}
		}

		void WriteKeyFrameProperties(AetKeyFrameProperties* properties, BinaryWriter& writer)
		{
			for (KeyFrameCollection& keyFrames : *properties)
				WriteKeyFramesPointer(keyFrames, writer);
		}

		void ReadAnimationData(RefPtr<AetAnimationData>& animationData, BinaryReader& reader)
		{
			animationData = MakeRef<AetAnimationData>();
			animationData->BlendMode = static_cast<AetBlendMode>(reader.ReadUInt8());
			reader.ReadUInt8();
			animationData->UseTextureMask = reader.ReadBool();
			reader.ReadUInt8();

			if (reader.GetPointerMode() == PtrMode::Mode64Bit)
				reader.ReadUInt32();

			ReadKeyFrameProperties(&animationData->Properties, reader);

			void* perspectivePropertiesPointer = reader.ReadPtr();
			if (perspectivePropertiesPointer != nullptr)
			{
				reader.ReadAt(perspectivePropertiesPointer, [animationData](BinaryReader& reader)
				{
					animationData->PerspectiveProperties = MakeRef<AetKeyFrameProperties>();
					ReadKeyFrameProperties(animationData->PerspectiveProperties.get(), reader);
				});
			}
		}
	}

	void AetLayer::Read(BinaryReader& reader)
	{
		filePosition = reader.GetPositionPtr();
		name = reader.ReadStrPtr();
		StartFrame = reader.ReadFloat();
		EndFrame = reader.ReadFloat();
		StartOffset = reader.ReadFloat();
		PlaybackSpeed = reader.ReadFloat();

		Flags.AllBits = reader.ReadUInt16();
		TypePaddingByte = reader.ReadUInt8();
		Type = static_cast<AetLayerType>(reader.ReadUInt8());

		if (reader.GetPointerMode() == PtrMode::Mode64Bit)
			reader.ReadUInt32();

		dataFilePtr = reader.ReadPtr();
		parentFilePtr = reader.ReadPtr();

		size_t markerCount = reader.ReadSize();
		void* markersPointer = reader.ReadPtr();

		if (markerCount > 0 && markersPointer != nullptr)
		{
			Markers.reserve(markerCount);
			for (size_t i = 0; i < markerCount; i++)
				Markers.push_back(MakeRef<AetMarker>());

			reader.ReadAt(markersPointer, [this](BinaryReader& reader)
			{
				for (auto& marker : Markers)
				{
					marker->Frame = reader.ReadFloat();
					if (reader.GetPointerMode() == PtrMode::Mode64Bit)
						reader.ReadUInt32();
					marker->Name = reader.ReadStrPtr();
				}
			});
		}

		void* animationDataPointer = reader.ReadPtr();
		if (animationDataPointer != nullptr)
		{
			reader.ReadAt(animationDataPointer, [this](BinaryReader& reader)
			{
				ReadAnimationData(this->AnimationData, reader);

				for (auto& keyFrames : this->AnimationData->Properties)
				{
					if (keyFrames.size() == 1)
						keyFrames.front().Frame = StartFrame;
				}
			});
		}

		audioDataFilePtr = reader.ReadPtr();
	}

	void Aet::Read(BinaryReader& reader)
	{
		Name = reader.ReadStrPtr();
		StartFrame = reader.ReadFloat();
		EndFrame = reader.ReadFloat();
		FrameRate = reader.ReadFloat();
		BackgroundColor = ReadColor(reader);

		Resolution.x = reader.ReadInt32();
		Resolution.y = reader.ReadInt32();

		void* cameraOffsetPtr = reader.ReadPtr();
		if (cameraOffsetPtr != nullptr)
		{
			Camera = MakeRef<AetCamera>();
			reader.ReadAt(cameraOffsetPtr, [this](BinaryReader& reader)
			{
				ReadKeyFramesPointer(Camera->PositionX, reader);
				ReadKeyFramesPointer(Camera->PositionY, reader);
			});
		}

		size_t compCount = reader.ReadSize();
		void* compsPtr = reader.ReadPtr();
		if (compCount > 0 && compsPtr != nullptr)
		{
			Compositions.resize(compCount - 1);
			reader.ReadAt(compsPtr, [this](BinaryReader& reader)
			{
				const auto readCompFunction = [](BinaryReader& reader, RefPtr<AetComposition>& comp)
				{
					comp = MakeRef<AetComposition>();
					comp->filePosition = reader.GetPositionPtr();

					size_t layerCount = reader.ReadSize();
					void* layersPointer = reader.ReadPtr();

					if (layerCount > 0 && layersPointer != nullptr)
					{
						comp->resize(layerCount);
						reader.ReadAt(layersPointer, [&comp](BinaryReader& reader)
						{
							for (auto& layer : *comp)
							{
								layer = MakeRef<AetLayer>();
								layer->Read(reader);
							}
						});
					}
				};

				for (auto& comp : Compositions)
					readCompFunction(reader, comp);

				readCompFunction(reader, RootComposition);
			});
		}

		size_t surfaceCount = reader.ReadSize();
		void* surfacesPtr = reader.ReadPtr();
		if (surfaceCount > 0 && surfacesPtr != nullptr)
		{
			Surfaces.resize(surfaceCount);
			reader.ReadAt(surfacesPtr, [this](BinaryReader& reader)
			{
				for (auto& surface : Surfaces)
				{
					surface = MakeRef<AetSurface>();
					surface->filePosition = reader.GetPositionPtr();
					surface->Color = ReadColor(reader);
					surface->Size.x = reader.ReadUInt16();
					surface->Size.y = reader.ReadUInt16();
					surface->Frames = reader.ReadFloat();

					uint32_t spriteCount = reader.ReadUInt32();
					void* spritesPointer = reader.ReadPtr();

					if (spriteCount > 0 && spritesPointer != nullptr)
					{
						surface->sprites.resize(spriteCount);
						reader.ReadAt(spritesPointer, [&surface](BinaryReader& reader)
						{
							for (AetSpriteIdentifier& sprite : surface->sprites)
							{
								sprite.Name = reader.ReadStrPtr();
								sprite.ID = reader.ReadUInt32();
							}
						});
					}
				}
			});
		}

		size_t soundEffectCount = reader.ReadSize();
		void* soundEffectsPtr = reader.ReadPtr();
		if (soundEffectCount > 0 && soundEffectsPtr != nullptr)
		{
			SoundEffects.resize(soundEffectCount);
			reader.ReadAt(soundEffectsPtr, [this](BinaryReader& reader)
			{
				for (auto& soundEffect : SoundEffects)
				{
					soundEffect = MakeRef<AetSoundEffect>();
					soundEffect->filePosition = reader.GetPositionPtr();
					soundEffect->Data = reader.ReadUInt32();
				}
			});
		}
	}

	void Aet::Write(BinaryWriter& writer)
	{
		writer.WritePtr([this](BinaryWriter& writer)
		{
			void* aetFilePosition = writer.GetPositionPtr();
			writer.WriteStrPtr(&Name);
			writer.WriteFloat(StartFrame);
			writer.WriteFloat(EndFrame);
			writer.WriteFloat(FrameRate);
			writer.WriteUInt32(BackgroundColor);
			writer.WriteInt32(Resolution.x);
			writer.WriteInt32(Resolution.y);

			if (Camera != nullptr)
			{
				writer.WritePtr([this](BinaryWriter& writer)
				{
					WriteKeyFramesPointer(this->Camera->PositionX, writer);
					WriteKeyFramesPointer(this->Camera->PositionY, writer);
				});
			}
			else
			{
				writer.WritePtr(nullptr); // AetCamera offset
			}

			assert(RootComposition != nullptr);
			writer.WriteUInt32(static_cast<uint32_t>(Compositions.size()) + 1);
			writer.WritePtr([this](BinaryWriter& writer)
			{
				const auto writeCompFunction = [](BinaryWriter& writer, const RefPtr<AetComposition>& comp)
				{
					comp->filePosition = writer.GetPositionPtr();
					if (comp->size() > 0)
					{
						writer.WriteUInt32(static_cast<uint32_t>(comp->size()));
						writer.WritePtr([&comp](BinaryWriter& writer)
						{
							for (auto& layer : *comp)
							{
								layer->filePosition = writer.GetPositionPtr();
								writer.WriteStrPtr(&layer->name);
								writer.WriteFloat(layer->StartFrame);
								writer.WriteFloat(layer->EndFrame);
								writer.WriteFloat(layer->StartOffset);
								writer.WriteFloat(layer->PlaybackSpeed);
								writer.Write<AetLayerFlags>(layer->Flags);
								writer.WriteUInt8(layer->TypePaddingByte);
								writer.Write<AetLayerType>(layer->Type);

								bool hasData =
									(layer->Type == AetLayerType::Pic && layer->GetReferencedSurface() != nullptr) ||
									(layer->Type == AetLayerType::Aif && layer->GetReferencedSoundEffect() != nullptr) ||
									(layer->Type == AetLayerType::Eff && layer->GetReferencedComposition() != nullptr);

								if (hasData)
								{
									writer.WriteDelayedPtr([&layer](BinaryWriter& writer)
									{
										void* filePosition =
											(layer->Type == AetLayerType::Pic) ? layer->GetReferencedSurface()->filePosition :
											(layer->Type == AetLayerType::Aif) ? layer->GetReferencedSoundEffect()->filePosition :
											layer->GetReferencedComposition()->filePosition;

										writer.WritePtr(filePosition);
									});
								}
								else
								{
									writer.WritePtr(nullptr); // Data offset
								}

								if (layer->GetReferencedParentLayer() != nullptr)
								{
									writer.WriteDelayedPtr([&layer](BinaryWriter& writer)
									{
										writer.WritePtr(layer->GetReferencedParentLayer()->filePosition);
									});
								}
								else
								{
									writer.WritePtr(nullptr); // Parent offset
								}

								if (layer->Markers.size() > 0)
								{
									writer.WriteUInt32(static_cast<uint32_t>(layer->Markers.size()));
									writer.WritePtr([&layer](BinaryWriter& writer)
									{
										for (auto& marker : layer->Markers)
										{
											writer.WriteFloat(marker->Frame);
											writer.WriteStrPtr(&marker->Name);
										}
									});
								}
								else
								{
									writer.WriteUInt32(0x00000000); // Markers size
									writer.WritePtr(nullptr);		// Markers offset
								}

								if (layer->AnimationData != nullptr)
								{
									AetAnimationData& animationData = *layer->AnimationData.get();
									writer.WritePtr([&animationData](BinaryWriter& writer)
									{
										writer.Write<AetBlendMode>(animationData.BlendMode);
										writer.WriteUInt8(0x00);
										writer.WriteBool(animationData.UseTextureMask);
										writer.WriteUInt8(0x00);

										WriteKeyFrameProperties(&animationData.Properties, writer);

										if (animationData.PerspectiveProperties != nullptr)
										{
											writer.WritePtr([&animationData](BinaryWriter& writer)
											{
												WriteKeyFrameProperties(animationData.PerspectiveProperties.get(), writer);
												writer.WriteAlignmentPadding(16);
											});
										}
										else
										{
											writer.WritePtr(nullptr); // PerspectiveProperties offset
										}

										writer.WriteAlignmentPadding(16);
									});
								}
								else
								{
									writer.WritePtr(nullptr); // AnimationData offset
								}

								// TODO: audioDataFilePtr
								writer.WritePtr(nullptr); // AudioData offset
							}

							writer.WriteAlignmentPadding(16);
						});
					}
					else
					{
						writer.WriteUInt32(0x00000000); // AetComposition size
						writer.WritePtr(nullptr);		// AetComposition offset
					}
				};

				for (auto& comp : Compositions)
					writeCompFunction(writer, comp);
				writeCompFunction(writer, RootComposition);

				writer.WriteAlignmentPadding(16);
			});

			if (Surfaces.size() > 0)
			{
				writer.WriteUInt32(static_cast<uint32_t>(Surfaces.size()));
				writer.WritePtr([this](BinaryWriter& writer)
				{
					for (auto& surface : Surfaces)
					{
						surface->filePosition = writer.GetPositionPtr();
						writer.WriteUInt32(surface->Color);
						writer.WriteInt16(static_cast<int16_t>(surface->Size.x));
						writer.WriteInt16(static_cast<int16_t>(surface->Size.y));
						writer.WriteFloat(surface->Frames);
						if (surface->SpriteCount() > 0)
						{
							writer.WriteUInt32(static_cast<uint32_t>(surface->SpriteCount()));
							writer.WritePtr([&surface](BinaryWriter& writer)
							{
								for (AetSpriteIdentifier& sprite : surface->GetSprites())
								{
									writer.WriteStrPtr(&sprite.Name);
									writer.WriteUInt32(sprite.ID);
								}
							});
						}
						else
						{
							writer.WriteUInt32(0x00000000); // AetSprites size
							writer.WritePtr(nullptr);		// AetSprites offset
						}
					}
					writer.WriteAlignmentPadding(16);
				});
			}
			else
			{
				writer.WriteUInt32(0x00000000); // AetSurfaces size
				writer.WritePtr(nullptr);		// AetSurfaces offset
			}

			if (SoundEffects.size() > 0)
			{
				writer.WriteUInt32(static_cast<uint32_t>(SoundEffects.size()));
				writer.WritePtr([this](BinaryWriter& writer)
				{
					for (auto& soundEffect : SoundEffects)
					{
						soundEffect->filePosition = writer.GetPositionPtr();
						writer.WriteUInt32(soundEffect->Data);
					}
					writer.WriteAlignmentPadding(16);
				});
			}
			else
			{
				writer.WriteUInt32(0x00000000); // AetSoundEffects size
				writer.WritePtr(nullptr);		// AetSoundEffects offset
			}

			writer.WriteAlignmentPadding(16);
		});
	}

	void AetSet::Read(BinaryReader& reader)
	{
		uint32_t signature = reader.ReadUInt32();
		if (signature == 'AETC' || signature == 'CTEA')
		{
			reader.SetEndianness(Endianness::Little);
			uint32_t dataSize = reader.ReadUInt32();
			uint32_t dataOffset = reader.ReadUInt32();
			uint32_t endianSignaure = reader.ReadUInt32();

			enum { LittleEndian = 0x10000000, BigEndian = 0x18000000 };

			reader.SetPosition(dataOffset);
			reader.SetEndianness(endianSignaure == LittleEndian ? Endianness::Little : Endianness::Big);

			if (dataOffset < 0x40)
			{
				reader.SetPointerMode(PtrMode::Mode64Bit);
				reader.SetStreamOffset(dataOffset);
			}
		}
		else
		{
			reader.SetPosition(reader.GetPosition() - sizeof(signature));
		}

		void* startAddress = reader.GetPositionPtr();

		size_t aetCount = 0;
		while (reader.ReadPtr() != nullptr)
			aetCount++;
		aets.reserve(aetCount);

		reader.ReadAt(startAddress, [this, aetCount](BinaryReader& reader)
		{
			for (size_t i = 0; i < aetCount; i++)
			{
				aets.push_back(MakeRef<Aet>());
				auto& aet = aets.back();

				reader.ReadAt(reader.ReadPtr(), [&aet](BinaryReader& reader)
				{
					aet->Read(reader);
				});

				aet->UpdateParentPointers();
				aet->InternalLinkPostRead();
				aet->InternalUpdateCompositionNamesAfterLayerReferences();
			}
		});
	}

	void AetSet::Write(BinaryWriter& writer)
	{
		for (auto& aet : aets)
		{
			assert(aet != nullptr);
			aet->Write(writer);
		}

		writer.WritePtr(nullptr);
		writer.WriteAlignmentPadding(16);

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushStringPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushDelayedWritePool();
	}
}