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
		animationData = MakeRefPtr<FileSystem::AnimationData>();
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
				animationData->PerspectiveProperties = MakeRefPtr<KeyFrameProperties>();
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
		StartFrame = reader.ReadFloat();
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
			Markers.resize(markerCount);
			reader.ReadAt(markersPointer, [this](BinaryReader& reader)
			{
				for (AetMarker& marker : Markers)
				{
					marker.Frame = reader.ReadFloat();
					marker.Name = reader.ReadStrPtr();
				}
			});
		}

		void* animationDataPointer = reader.ReadPtr();
		if (animationDataPointer != nullptr)
		{
			reader.ReadAt(animationDataPointer, [this](BinaryReader& reader)
			{
				ReadAnimationData(this->AnimationData, reader);
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

		Width = reader.ReadInt32();
		Height = reader.ReadInt32();

		void* positionOffsetPtr = reader.ReadPtr();
		if (positionOffsetPtr != nullptr)
		{
			this->PositionOffset = MakeRefPtr<FileSystem::PositionOffset>();
			reader.ReadAt(positionOffsetPtr, [this](BinaryReader& reader)
			{
				ReadKeyFramesPointer(this->PositionOffset->PositionX, reader);
				ReadKeyFramesPointer(this->PositionOffset->PositionY, reader);
			});
		}

		uint32_t layersCount = reader.ReadUInt32();
		void* layersPtr = reader.ReadPtr();
		if (layersCount > 0 && layersPtr != nullptr)
		{
			AetLayers.resize(layersCount);
			reader.ReadAt(layersPtr, [this](BinaryReader& reader)
			{
				for (RefPtr<AetLayer>& layer : AetLayers)
				{
					layer = MakeRefPtr<AetLayer>();
					layer->filePosition = reader.GetPositionPtr();

					uint32_t objectCount = reader.ReadUInt32();
					void* objectsPointer = reader.ReadPtr();

					if (objectCount > 0 && objectsPointer != nullptr)
					{
						layer->resize(objectCount);
						reader.ReadAt(objectsPointer, [this, &layer](BinaryReader& reader)
						{
							for (RefPtr<AetObj>& object : *layer)
							{
								object = MakeRefPtr<AetObj>();
								object->Read(reader);
							}
						});
					}
				}
			});
		}

		uint32_t regionCount = reader.ReadUInt32();
		void* regionsPtr = reader.ReadPtr();
		if (regionCount > 0 && regionsPtr != nullptr)
		{
			AetRegions.resize(regionCount);
			reader.ReadAt(regionsPtr, [this](BinaryReader& reader)
			{
				for (RefPtr<AetRegion>& region : AetRegions)
				{
					region = MakeRefPtr<AetRegion>();
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
			AetSoundEffects.resize(soundEffectCount);
			reader.ReadAt(soundEffectsPtr, [this](BinaryReader& reader)
			{
				for (RefPtr<AetSoundEffect>& soundEffect : AetSoundEffects)
				{
					soundEffect = MakeRefPtr<AetSoundEffect>();
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
			writer.WriteInt32(Width);
			writer.WriteInt32(Height);

			if (this->PositionOffset != nullptr)
			{
				writer.WritePtr([this](BinaryWriter& writer)
				{
					WriteKeyFramesPointer(this->PositionOffset->PositionX, writer);
					WriteKeyFramesPointer(this->PositionOffset->PositionY, writer);
				});
			}
			else
			{
				writer.WritePtr(nullptr); // PositionOffset offset
			}

			if (AetLayers.size() > 0)
			{
				writer.WriteUInt32(static_cast<uint32_t>(AetLayers.size()));
				writer.WritePtr([this](BinaryWriter& writer)
				{
					for (RefPtr<AetLayer>& layer : AetLayers)
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
									writer.WriteFloat(obj->StartFrame);
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
											for (AetMarker& marker : obj->Markers)
											{
												writer.WriteFloat(marker.Frame);
												writer.WriteStrPtr(&marker.Name);
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
					}
					writer.WriteAlignmentPadding(16);
				});
			}
			else
			{
				writer.WriteUInt32(0x00000000); // AetLayers size
				writer.WritePtr(nullptr);		// AetLayers offset
			}

			if (AetRegions.size() > 0)
			{
				writer.WriteUInt32(static_cast<uint32_t>(AetRegions.size()));
				writer.WritePtr([this](BinaryWriter& writer)
				{
					for (RefPtr<AetRegion>& region : AetRegions)
					{
						region->filePosition = writer.GetPositionPtr();
						writer.WriteUInt32(region->Color);
						writer.WriteInt16(region->Width);
						writer.WriteInt16(region->Height);
						writer.WriteFloat(region->Frames);
						if (region->SpriteSize() > 0)
						{
							writer.WriteUInt32(static_cast<uint32_t>(region->SpriteSize()));
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

			if (AetSoundEffects.size() > 0)
			{
				writer.WriteUInt32(static_cast<uint32_t>(AetSoundEffects.size()));
				writer.WritePtr([this](BinaryWriter& writer)
				{
					for (RefPtr<AetSoundEffect>& soundEffect : AetSoundEffects)
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
				aets.push_back(MakeRefPtr<Aet>());
				Aet* aet = aets.back().get();

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