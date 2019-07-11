#include "AetSet.h"
#include "FileSystem/BinaryReader.h"
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

	static void ReadKeyFrameProperties(KeyFrameProperties* properties, BinaryReader& reader)
	{
		for (auto& keyFrames : *properties)
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

	}

	static void ReadAnimationData(AnimationData* animationData, BinaryReader& reader)
	{
		animationData->BlendMode = reader.Read<AetBlendMode>();
		reader.ReadByte();
		animationData->UseTextureMask = reader.ReadBool();
		reader.ReadByte();
		animationData->Properties = std::make_shared<KeyFrameProperties>();

		ReadKeyFrameProperties(animationData->Properties.get(), reader);

		void* propertiesExtraDataPointer = reader.ReadPtr();
		if (propertiesExtraDataPointer != nullptr)
		{
			reader.ReadAt(propertiesExtraDataPointer, [animationData](BinaryReader& reader)
			{
				animationData->PerspectiveProperties = std::make_shared<KeyFrameProperties>();
				ReadKeyFrameProperties(animationData->PerspectiveProperties.get(), reader);
			});
		}
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

	AetRegion* AetObj::GetRegion()
	{
		assert(parentAet != nullptr);

		int32_t regionIndex = references.RegionIndex;

		if (regionIndex >= 0 && regionIndex < parentAet->AetRegions.size())
			return &parentAet->AetRegions[regionIndex];

		return nullptr;
	}

	AetLayer* AetObj::GetLayer()
	{
		assert(parentAet != nullptr);

		int32_t layerIndex = references.LayerIndex;

		if (layerIndex >= 0 && layerIndex < parentAet->AetLayers.size())
			return &parentAet->AetLayers[layerIndex];

		return nullptr;
	}

	AetObj* AetObj::GetParent()
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

		TypeFlag = reader.Read<AetTypeFlags>();
		UnknownTypeByte = reader.ReadByte();
		Type = reader.Read<AetObjType>();

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
				ReadAnimationData(&this->AnimationData, reader);
			});
		}

		unknownFilePtr = reader.ReadPtr();
	}

	void Aet::Read(BinaryReader& reader)
	{
		Name = reader.ReadStrPtr();
		unknownValue = reader.ReadUInt32();
		FrameDuration = reader.ReadFloat();
		FrameRate = reader.ReadFloat();
		BackgroundColor = reader.ReadUInt32();
		Width = reader.ReadInt32();
		Height = reader.ReadInt32();
		unknownFilePtr0 = reader.ReadPtr();

		uint32_t layersCount = reader.ReadUInt32();
		AetLayers.resize(layersCount);

		reader.ReadAt(reader.ReadPtr(), [this](BinaryReader& reader)
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

		uint32_t regionCount = reader.ReadUInt32();
		AetRegions.resize(regionCount);

		reader.ReadAt(reader.ReadPtr(), [this](BinaryReader& reader)
		{
			for (auto& region : AetRegions)
			{
				region.filePosition = reader.GetPositionPtr();
				region.Color = reader.ReadUInt32();
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

		unknownFilePtr1 = reader.ReadPtr();
		unknownFilePtr2 = reader.ReadPtr();
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
				if (aetObj.Type == AetObjType::Pic)
					FindObjReferencedRegion(&aetObj);
				else if (aetObj.Type == AetObjType::Eff)
					FindObjReferencedLayer(&aetObj);

				FindObjReferencedParent(&aetObj);
			}
		}
	}

	void Aet::FindObjReferencedRegion(AetObj* aetObj)
	{
		if (aetObj->dataFilePtr != nullptr)
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
		}

		aetObj->references.RegionIndex = -1;
	}

	void Aet::FindObjReferencedLayer(AetObj* aetObj)
	{
		if (aetObj->dataFilePtr != nullptr)
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
}