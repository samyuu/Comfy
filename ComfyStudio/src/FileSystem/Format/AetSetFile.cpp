#include "AetSet.h"
#include "FileSystem/BinaryReader.h"
#include "FileSystem/BinaryWriter.h"

namespace FileSystem
{
	static uint32_t ReadColor(BinaryReader& reader)
	{
		uint32_t value = 0;
		*((uint8_t*)&value + 0) = reader.ReadUInt8();
		*((uint8_t*)&value + 1) = reader.ReadUInt8();
		*((uint8_t*)&value + 2) = reader.ReadUInt8();
		reader.SetPosition(reader.GetPosition() + 1);
		return value;
	}

	static void ReadKeyFramesPointer(KeyFrameCollection& keyFrames, BinaryReader& reader)
	{
		uint32_t keyFrameCount = reader.ReadUInt32();
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
					for (uint32_t i = 0; i < keyFrameCount; i++)
						keyFrames[i].Frame = reader.ReadFloat();

					for (uint32_t i = 0; i < keyFrameCount; i++)
					{
						keyFrames[i].Value = reader.ReadFloat();
						keyFrames[i].Interpolation = reader.ReadFloat();
					}
				}
			});
		}
	}

	static void ReadKeyFrameProperties(KeyFrameProperties* properties, BinaryReader& reader)
	{
		for (KeyFrameCollection& keyFrames : *properties)
			ReadKeyFramesPointer(keyFrames, reader);
	}

	static void WriteKeyFramesPointer(KeyFrameCollection& keyFrames, BinaryWriter& writer)
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
						writer.WriteFloat(keyFrame.Interpolation);
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

	static void WriteKeyFrameProperties(KeyFrameProperties* properties, BinaryWriter& writer)
	{
		for (KeyFrameCollection& keyFrames : *properties)
			WriteKeyFramesPointer(keyFrames, writer);
	}

	static void ReadAnimationData(RefPtr<AnimationData>& animationData, BinaryReader& reader)
	{
		animationData = MakeRef<FileSystem::AnimationData>();
		animationData->BlendMode = static_cast<AetBlendMode>(reader.ReadUInt8());
		reader.ReadUInt8();
		animationData->UseTextureMask = reader.ReadBool();
		reader.ReadUInt8();

		ReadKeyFrameProperties(&animationData->Properties, reader);

		void* perspectivePropertiesPointer = reader.ReadPtr();
		if (perspectivePropertiesPointer != nullptr)
		{
			reader.ReadAt(perspectivePropertiesPointer, [animationData](BinaryReader& reader)
			{
				animationData->PerspectiveProperties = MakeRef<KeyFrameProperties>();
				ReadKeyFrameProperties(animationData->PerspectiveProperties.get(), reader);
			});
		}
	}

	void AetObj::Read(BinaryReader& reader)
	{
		filePosition = reader.GetPositionPtr();
		name = reader.ReadStrPtr();
		LoopStart = reader.ReadFloat();
		LoopEnd = reader.ReadFloat();
		StartOffset = reader.ReadFloat();
		PlaybackSpeed = reader.ReadFloat();

		Flags.AllBits = reader.ReadUInt16();
		TypePaddingByte = reader.ReadUInt8();
		Type = static_cast<AetObjType>(reader.ReadUInt8());

		dataFilePtr = reader.ReadPtr();
		parentFilePtr = reader.ReadPtr();

		uint32_t markerCount = reader.ReadUInt32();
		void* markersPointer = reader.ReadPtr();

		if (markerCount > 0 && markersPointer != nullptr)
		{
			Markers.reserve(markerCount);
			for (size_t i = 0; i < markerCount; i++)
				Markers.push_back(MakeRef<AetMarker>());

			reader.ReadAt(markersPointer, [this](BinaryReader& reader)
			{
				for (RefPtr<AetMarker>& marker : Markers)
				{
					marker->Frame = reader.ReadFloat();
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
						keyFrames.front().Frame = LoopStart;
				}
			});
		}

		// TODO:
		/*unknownFilePtr =*/ reader.ReadPtr();
	}

	void Aet::Read(BinaryReader& reader)
	{
		Name = reader.ReadStrPtr();
		FrameStart = reader.ReadFloat();
		FrameDuration = reader.ReadFloat();
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

		uint32_t layersCount = reader.ReadUInt32();
		void* layersPtr = reader.ReadPtr();
		if (layersCount > 0 && layersPtr != nullptr)
		{
			Layers.resize(layersCount - 1);
			reader.ReadAt(layersPtr, [this](BinaryReader& reader)
			{
				const auto readLayerFunction = [](BinaryReader& reader, RefPtr<AetLayer>& layer)
				{
					layer = MakeRef<AetLayer>();
					layer->filePosition = reader.GetPositionPtr();

					uint32_t objectCount = reader.ReadUInt32();
					void* objectsPointer = reader.ReadPtr();

					if (objectCount > 0 && objectsPointer != nullptr)
					{
						layer->resize(objectCount);
						reader.ReadAt(objectsPointer, [&layer](BinaryReader& reader)
						{
							for (RefPtr<AetObj>& object : *layer)
							{
								object = MakeRef<AetObj>();
								object->Read(reader);
							}
						});
					}
				};

				for (RefPtr<AetLayer>& layer : Layers)
					readLayerFunction(reader, layer);

				readLayerFunction(reader, RootLayer);
			});
		}

		uint32_t regionCount = reader.ReadUInt32();
		void* regionsPtr = reader.ReadPtr();
		if (regionCount > 0 && regionsPtr != nullptr)
		{
			Regions.resize(regionCount);
			reader.ReadAt(regionsPtr, [this](BinaryReader& reader)
			{
				for (RefPtr<AetRegion>& region : Regions)
				{
					region = MakeRef<AetRegion>();
					region->filePosition = reader.GetPositionPtr();
					region->Color = ReadColor(reader);
					region->Width = reader.ReadUInt16();
					region->Height = reader.ReadUInt16();
					region->Frames = reader.ReadFloat();

					uint32_t spriteCount = reader.ReadUInt32();
					void* spritesPointer = reader.ReadPtr();

					if (spriteCount > 0 && spritesPointer != nullptr)
					{
						region->sprites.resize(spriteCount);
						reader.ReadAt(spritesPointer, [&region](BinaryReader& reader)
						{
							for (AetSprite& sprite : region->sprites)
							{
								sprite.Name = reader.ReadStrPtr();
								sprite.ID = reader.ReadUInt32();
							}
						});
					}
				}
			});
		}

		uint32_t soundEffectCount = reader.ReadUInt32();
		void* soundEffectsPtr = reader.ReadPtr();
		if (soundEffectCount > 0 && soundEffectsPtr != nullptr)
		{
			SoundEffects.resize(soundEffectCount);
			reader.ReadAt(soundEffectsPtr, [this](BinaryReader& reader)
			{
				for (RefPtr<AetSoundEffect>& soundEffect : SoundEffects)
				{
					soundEffect = MakeRef<AetSoundEffect>();
					soundEffect->filePosition = reader.GetPositionPtr();
					soundEffect->Data[0] = reader.ReadUInt32();
					soundEffect->Data[1] = reader.ReadUInt32();
					soundEffect->Data[2] = reader.ReadUInt32();
					soundEffect->Data[3] = reader.ReadUInt32();
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
			writer.WriteFloat(FrameStart);
			writer.WriteFloat(FrameDuration);
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

			assert(RootLayer != nullptr);
			if (Layers.size() > 0)
			{
				writer.WriteUInt32(static_cast<uint32_t>(Layers.size()) + 1);
				writer.WritePtr([this](BinaryWriter& writer)
				{
					const auto writeLayerFunction = [](BinaryWriter& writer, const RefPtr<AetLayer>& layer) 
					{
						layer->filePosition = writer.GetPositionPtr();
						if (layer->size() > 0)
						{
							writer.WriteUInt32(static_cast<uint32_t>(layer->size()));
							writer.WritePtr([&layer](BinaryWriter& writer)
							{
								for (RefPtr<AetObj>& obj : *layer)
								{
									obj->filePosition = writer.GetPositionPtr();
									writer.WriteStrPtr(&obj->name);
									writer.WriteFloat(obj->LoopStart);
									writer.WriteFloat(obj->LoopEnd);
									writer.WriteFloat(obj->StartOffset);
									writer.WriteFloat(obj->PlaybackSpeed);
									writer.Write<AetObjFlags>(obj->Flags);
									writer.WriteUInt8(obj->TypePaddingByte);
									writer.Write<AetObjType>(obj->Type);

									bool hasData =
										(obj->Type == AetObjType::Pic && obj->GetReferencedRegion() != nullptr) ||
										(obj->Type == AetObjType::Aif && obj->GetReferencedSoundEffect() != nullptr) ||
										(obj->Type == AetObjType::Eff && obj->GetReferencedLayer() != nullptr);

									if (hasData)
									{
										writer.WriteDelayedPtr([&obj](BinaryWriter& writer)
										{
											void* filePosition =
												(obj->Type == AetObjType::Pic) ? obj->GetReferencedRegion()->filePosition :
												(obj->Type == AetObjType::Aif) ? obj->GetReferencedSoundEffect()->filePosition :
												obj->GetReferencedLayer()->filePosition;

											writer.WritePtr(filePosition);
										});
									}
									else
									{
										writer.WritePtr(nullptr); // Data offset
									}

									if (obj->GetReferencedParentObj() != nullptr)
									{
										writer.WriteDelayedPtr([&obj](BinaryWriter& writer)
										{
											writer.WritePtr(obj->GetReferencedParentObj()->filePosition);
										});
									}
									else
									{
										writer.WritePtr(nullptr); // Parent offset
									}

									if (obj->Markers.size() > 0)
									{
										writer.WriteUInt32(static_cast<uint32_t>(obj->Markers.size()));
										writer.WritePtr([&obj](BinaryWriter& writer)
										{
											for (RefPtr<AetMarker>& marker : obj->Markers)
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

									if (obj->AnimationData != nullptr)
									{
										AnimationData& animationData = *obj->AnimationData.get();
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

									// TODO: unknownFilePtr
									writer.WritePtr(nullptr); // extra data offset
								}

								writer.WriteAlignmentPadding(16);
							});
						}
						else
						{
							writer.WriteUInt32(0x00000000); // AetLayer size
							writer.WritePtr(nullptr);		// AetLayer offset
						}
					};

					for (RefPtr<AetLayer>& layer : Layers)
						writeLayerFunction(writer, layer);
					writeLayerFunction(writer, RootLayer);

					writer.WriteAlignmentPadding(16);
				});
			}
			else
			{
				writer.WriteUInt32(0x00000000); // AetLayers size
				writer.WritePtr(nullptr);		// AetLayers offset
			}

			if (Regions.size() > 0)
			{
				writer.WriteUInt32(static_cast<uint32_t>(Regions.size()));
				writer.WritePtr([this](BinaryWriter& writer)
				{
					for (RefPtr<AetRegion>& region : Regions)
					{
						region->filePosition = writer.GetPositionPtr();
						writer.WriteUInt32(region->Color);
						writer.WriteInt16(region->Width);
						writer.WriteInt16(region->Height);
						writer.WriteFloat(region->Frames);
						if (region->SpriteCount() > 0)
						{
							writer.WriteUInt32(static_cast<uint32_t>(region->SpriteCount()));
							writer.WritePtr([&region](BinaryWriter& writer)
							{
								for (AetSprite& sprite : region->GetSprites())
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
				writer.WriteUInt32(0x00000000); // AetRegions size
				writer.WritePtr(nullptr);		// AetRegions offset
			}

			if (SoundEffects.size() > 0)
			{
				writer.WriteUInt32(static_cast<uint32_t>(SoundEffects.size()));
				writer.WritePtr([this](BinaryWriter& writer)
				{
					for (RefPtr<AetSoundEffect>& soundEffect : SoundEffects)
					{
						soundEffect->filePosition = writer.GetPositionPtr();
						writer.WriteUInt32(soundEffect->Data[0]);
						writer.WriteUInt32(soundEffect->Data[1]);
						writer.WriteUInt32(soundEffect->Data[2]);
						writer.WriteUInt32(soundEffect->Data[3]);
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
			reader.ReadUInt32();
			reader.SetPosition(reader.ReadPtr());
			reader.SetEndianness(Endianness::Big);
		}
		else
		{
			reader.SetPosition(reader.GetPosition() - sizeof(uint32_t));
		}

		void* startAddress = reader.GetPositionPtr();

		int32_t aetCount = 0;
		while (reader.ReadPtr() != nullptr)
			aetCount++;
		aets.reserve(aetCount);

		reader.ReadAt(startAddress, [this, aetCount](BinaryReader& reader)
		{
			for (int i = 0; i < aetCount; i++)
			{
				aets.push_back(MakeRef<Aet>());
				const RefPtr<Aet>& aet = aets.back();

				reader.ReadAt(reader.ReadPtr(), [&aet](BinaryReader& reader)
				{
					aet->Read(reader);
				});

				aet->UpdateParentPointers();
				aet->InternalLinkPostRead();
				aet->InternalUpdateLayerNames();
			}
		});
	}

	void AetSet::Write(BinaryWriter& writer)
	{
		for (RefPtr<Aet>& aet : aets)
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