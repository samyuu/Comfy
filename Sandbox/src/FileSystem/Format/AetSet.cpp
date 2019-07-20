#include "AetSet.h"
#include "FileSystem/BinaryReader.h"
#include "FileSystem/BinaryWriter.h"
#include <assert.h>

namespace FileSystem
{
	std::array<const char*, 8> KeyFrameProperties::PropertyNames =
	{
		"Origin X",
		"Origin Y",
		"Position X",
		"Position Y",
		"Rotation",
		"Scale X",
		"Scale Y",
		"Opactiy",
	};

	std::array<const char*, 4> AetObj::TypeNames =
	{
		"nop",
		"pic",
		"aif",
		"eff",
	};

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
					keyFrames[0].Value = reader.ReadFloat();
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
		for (auto& keyFrames : *properties)
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
					for (auto& keyFrame : keyFrames)
						writer.WriteFloat(keyFrame.Frame);
					for (auto& keyFrame : keyFrames)
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
		for (auto& keyFrames : *properties)
			WriteKeyFramesPointer(keyFrames, writer);
	}

	static void ReadAnimationData(std::shared_ptr<AnimationData>& animationData, BinaryReader& reader)
	{
		animationData = std::make_shared<FileSystem::AnimationData>();
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
				animationData->PerspectiveProperties = std::make_shared<KeyFrameProperties>();
				ReadKeyFrameProperties(animationData->PerspectiveProperties.get(), reader);
			});
		}
	}

	AetObj::AetObj()
	{
	}

	AetObj::AetObj(AetObjType type, const char* name, Aet* parent)
	{
		LoopStart = 0.0f;
		LoopEnd = 60.0f;
		StartFrame = 0.0f;
		PlaybackSpeed = 1.0f;
		Flags = AetObjFlags_Visible | AetObjFlags_Audible;
		TypePaddingByte = 0x3;
		Type = type;

		if (type != AetObjType::Aif)
		{
			AnimationData = std::make_shared<FileSystem::AnimationData>();
			AnimationData->BlendMode = AetBlendMode::Alpha;
			AnimationData->UseTextureMask = false;
		}

		parentAet = parent;
		SetName(name);
	}

	AetObj::~AetObj()
	{
	}

	const char* AetObj::GetName()
	{
		return name.c_str();
	}

	void AetObj::SetName(const char* value)
	{
		name = value;
		parentAet->UpdateLayerNames();
	}

	AetRegion* AetObj::GetRegion() const
	{
		assert(parentAet != nullptr);

		int32_t regionIndex = references.RegionIndex;
		if (regionIndex >= 0 && regionIndex < parentAet->AetRegions.size())
			return &parentAet->AetRegions[regionIndex];

		return nullptr;
	}

	AetSoundEffect* AetObj::GetSoundEffect() const
	{
		assert(parentAet != nullptr);

		int32_t soundEffectIndex = references.SoundEffectIndex;
		if (soundEffectIndex >= 0 && soundEffectIndex < parentAet->AetSoundEffects.size())
			return &parentAet->AetSoundEffects[soundEffectIndex];

		return nullptr;
	}

	AetLayer* AetObj::GetLayer() const
	{
		assert(parentAet != nullptr);

		int32_t layerIndex = references.LayerIndex;
		if (layerIndex >= 0 && layerIndex < parentAet->AetLayers.size())
			return &parentAet->AetLayers[layerIndex];

		return nullptr;
	}

	AetObj* AetObj::GetParent() const
	{
		assert(parentAet != nullptr);

		int32_t layerIndex = references.ParentLayerIndex;
		int32_t objIndex = references.ParentObjIndex;

		if (layerIndex >= 0 && layerIndex < parentAet->AetLayers.size() && objIndex >= 0 && objIndex < parentAet->AetLayers[layerIndex].size())
			return &parentAet->AetLayers[layerIndex][objIndex];

		return nullptr;
	}

	void AetObj::Read(BinaryReader& reader)
	{
		filePosition = reader.GetPositionPtr();
		name = reader.ReadStrPtr();
		LoopStart = reader.ReadFloat();
		LoopEnd = reader.ReadFloat();
		StartFrame = reader.ReadFloat();
		PlaybackSpeed = reader.ReadFloat();

		Flags = static_cast<AetObjFlags>(reader.ReadUInt16());
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
				for (auto& marker : Markers)
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
			this->PositionOffset = std::make_shared<FileSystem::PositionOffset>();
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
				for (auto& layer : AetLayers)
				{
					layer.filePosition = reader.GetPositionPtr();

					uint32_t objectCount = reader.ReadUInt32();
					void* objectsPointer = reader.ReadPtr();

					if (objectCount > 0 && objectsPointer != nullptr)
					{
						layer.resize(objectCount);
						reader.ReadAt(objectsPointer, [this, &layer](BinaryReader& reader)
						{
							for (auto& object : layer)
							{
								object.parentAet = this;
								object.Read(reader);
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
				for (auto& region : AetRegions)
				{
					region.filePosition = reader.GetPositionPtr();
					region.Color = ReadColor(reader);
					region.Width = reader.ReadUInt16();
					region.Height = reader.ReadUInt16();
					region.Frames = reader.ReadFloat();

					uint32_t spriteCount = reader.ReadUInt32();
					void* spritesPointer = reader.ReadPtr();

					if (spriteCount > 0 && spritesPointer != nullptr)
					{
						region.Sprites.resize(spriteCount);
						reader.ReadAt(spritesPointer, [&region](BinaryReader& reader)
						{
							for (auto& sprite : region.Sprites)
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
				for (auto& soundEffect : AetSoundEffects)
				{
					soundEffect.filePosition = reader.GetPositionPtr();
					soundEffect.Data[0] = reader.ReadUInt32();
					soundEffect.Data[1] = reader.ReadUInt32();
					soundEffect.Data[2] = reader.ReadUInt32();
					soundEffect.Data[3] = reader.ReadUInt32();
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
					for (auto& layer : AetLayers)
					{
						layer.filePosition = writer.GetPositionPtr();
						if (layer.size() > 0)
						{
							writer.WriteUInt32(static_cast<uint32_t>(layer.size()));
							writer.WritePtr([&layer](BinaryWriter& writer)
							{
								for (auto& obj : layer)
								{
									obj.filePosition = writer.GetPositionPtr();
									writer.WriteStrPtr(&obj.name);
									writer.WriteFloat(obj.LoopStart);
									writer.WriteFloat(obj.LoopEnd);
									writer.WriteFloat(obj.StartFrame);
									writer.WriteFloat(obj.PlaybackSpeed);
									writer.Write<AetObjFlags>(obj.Flags);
									writer.WriteUInt8(obj.TypePaddingByte);
									writer.Write<AetObjType>(obj.Type);

									bool hasData =
										(obj.Type == AetObjType::Pic && obj.GetRegion() != nullptr) ||
										(obj.Type == AetObjType::Aif && obj.GetSoundEffect() != nullptr) ||
										(obj.Type == AetObjType::Eff && obj.GetLayer() != nullptr);

									if (hasData)
									{
										writer.WriteDelayedPtr([&obj](BinaryWriter& writer)
										{
											void* filePosition =
												(obj.Type == AetObjType::Pic) ? obj.GetRegion()->filePosition :
												(obj.Type == AetObjType::Aif) ? obj.GetSoundEffect()->filePosition :
												obj.GetLayer()->filePosition;

											writer.WritePtr(filePosition);
										});
									}
									else
									{
										writer.WritePtr(nullptr); // Data offset
									}

									if (obj.GetParent() != nullptr)
									{
										writer.WriteDelayedPtr([&obj](BinaryWriter& writer)
										{
											writer.WritePtr(obj.GetParent()->filePosition);
										});
									}
									else
									{
										writer.WritePtr(nullptr); // Parent offset
									}

									if (obj.Markers.size() > 0)
									{
										writer.WriteUInt32(static_cast<uint32_t>(obj.Markers.size()));
										writer.WritePtr([&obj](BinaryWriter& writer)
										{
											for (auto& marker : obj.Markers)
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

									if (obj.AnimationData != nullptr)
									{
										AnimationData& animationData = *obj.AnimationData.get();
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
					for (auto& region : AetRegions)
					{
						region.filePosition = writer.GetPositionPtr();
						writer.WriteUInt32(region.Color);
						writer.WriteInt16(region.Width);
						writer.WriteInt16(region.Height);
						writer.WriteFloat(region.Frames);
						if (region.Sprites.size() > 0)
						{
							writer.WriteUInt32(static_cast<uint32_t>(region.Sprites.size()));
							writer.WritePtr([&region](BinaryWriter& writer)
							{
								for (auto& sprite : region.Sprites)
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
					for (auto& soundEffect : AetSoundEffects)
					{
						soundEffect.filePosition = writer.GetPositionPtr();
						writer.WriteUInt32(soundEffect.Data[0]);
						writer.WriteUInt32(soundEffect.Data[1]);
						writer.WriteUInt32(soundEffect.Data[2]);
						writer.WriteUInt32(soundEffect.Data[3]);
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

	void Aet::UpdateLayerNames()
	{
		for (auto& aetLayer : AetLayers)
		{
			aetLayer.Names.clear();

			for (auto& aetObj : aetLayer)
			{
				AetLayer* referencedLayer;

				if (aetObj.Type == AetObjType::Eff && (referencedLayer = aetObj.GetLayer()) != nullptr)
				{
					bool nameExists = false;
					for (auto& layerNames : referencedLayer->Names)
					{
						if (layerNames == aetObj.name)
						{
							nameExists = true;
							break;
						}
					}

					if (!nameExists)
						referencedLayer->Names.emplace_back(aetObj.name);
				}
			}
		}

		for (auto& aetLayer : AetLayers)
		{
			aetLayer.CommaSeparatedNames.clear();

			for (size_t i = 0; i < aetLayer.Names.size(); i++)
			{
				aetLayer.CommaSeparatedNames.append(aetLayer.Names[i]);
				if (i < aetLayer.Names.size() - 1)
					aetLayer.CommaSeparatedNames.append(", ");
			}
		}
	}

	void Aet::LinkPostRead()
	{
		int32_t layerIndex = 0;
		for (auto &aetLayer : AetLayers)
		{
			aetLayer.thisIndex = layerIndex++;

			for (auto &aetObj : aetLayer)
			{
				if (aetObj.dataFilePtr != nullptr)
				{
					if (aetObj.Type == AetObjType::Pic)
						FindObjReferencedRegion(&aetObj);
					else if (aetObj.Type == AetObjType::Aif)
						FindObjReferencedSoundEffect(&aetObj);
					else if (aetObj.Type == AetObjType::Eff)
						FindObjReferencedLayer(&aetObj);
				}
				FindObjReferencedParent(&aetObj);
			}
		}
	}

	void Aet::FindObjReferencedRegion(AetObj* aetObj)
	{
		int32_t regionIndex = 0;
		for (auto &otherRegion : AetRegions)
		{
			if (otherRegion.filePosition == aetObj->dataFilePtr)
			{
				aetObj->references.RegionIndex = regionIndex;
				return;
			}
			regionIndex++;
		}

		aetObj->references.RegionIndex = -1;
	}

	void Aet::FindObjReferencedSoundEffect(AetObj* aetObj)
	{
		int32_t soundEffectIndex = 0;
		for (auto &otherSoundEffect : AetSoundEffects)
		{
			if (otherSoundEffect.filePosition == aetObj->dataFilePtr)
			{
				aetObj->references.SoundEffectIndex = soundEffectIndex;
				return;
			}
			soundEffectIndex++;
		}

		aetObj->references.SoundEffectIndex = -1;
	}

	void Aet::FindObjReferencedLayer(AetObj* aetObj)
	{
		int32_t layerIndex = 0;
		for (auto &layer : AetLayers)
		{
			if (layer.filePosition == aetObj->dataFilePtr)
			{
				aetObj->references.LayerIndex = layerIndex;
				return;
			}
			layerIndex++;
		}

		aetObj->references.LayerIndex = -1;
	}

	void Aet::FindObjReferencedParent(AetObj* aetObj)
	{
		if (aetObj->parentFilePtr != nullptr)
		{
			int32_t layerIndex = 0;
			for (auto& otherLayer : AetLayers)
			{
				int32_t objIndex = 0;
				for (auto& otherObj : otherLayer)
				{
					if (otherObj.filePosition == aetObj->parentFilePtr)
					{
						aetObj->references.ParentLayerIndex = layerIndex;
						aetObj->references.ParentObjIndex = objIndex;
						return;
					}

					objIndex++;
				}
				layerIndex++;
			}
		}

		aetObj->references.ParentLayerIndex = -1;
		aetObj->references.ParentObjIndex = -1;
	}

	void AetSet::ClearSpriteCache()
	{
		for (auto& aet : aets)
		{
			for (auto& region : aet.AetRegions)
			{
				for (auto& sprite : region.Sprites)
					sprite.SpriteCache = nullptr;
			}
		}
	}

	void AetSet::Read(BinaryReader& reader)
	{
		//reader.SetPosition(0x40);

		void* startAddress = reader.GetPositionPtr();

		uint32_t aetCount = 0;
		while (reader.ReadPtr() != nullptr)
			aetCount++;
		aets.resize(aetCount);

		reader.ReadAt(startAddress, [this](BinaryReader& reader)
		{
			int32_t aetIndex = 0;
			for (auto& aet : aets)
			{
				reader.ReadAt(reader.ReadPtr(), [&aet](BinaryReader& reader)
				{
					aet.Read(reader);
				});

				aet.thisIndex = aetIndex++;
				aet.LinkPostRead();

				aet.UpdateLayerNames();
			}
		});
	}

	void AetSet::Write(BinaryWriter& writer)
	{
		for (auto& aet : aets)
			aet.Write(writer);

		writer.WritePtr(nullptr);
		writer.WriteAlignmentPadding(16);

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushStringPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushDelayedWritePool();
	}
}