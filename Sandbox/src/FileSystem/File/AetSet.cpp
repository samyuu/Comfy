#include "AetSet.h"
#include "../BinaryReader.h"
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
		"Opcatiy",
	};

	static void ReadKeyFrameProperties(KeyFrameProperties* properties, BinaryReader& reader)
	{
		for (auto keyFrames = &properties->OriginX; keyFrames <= &properties->Opacity; keyFrames++)
		{
			uint32_t keyFramesCount = reader.ReadUInt32();
			void* keyFramesPointer = reader.ReadPtr();

			if (keyFramesCount > 0 && keyFramesPointer != nullptr)
			{
				reader.ReadAt(keyFramesPointer, [keyFramesCount, keyFrames](BinaryReader& reader)
				{
					if (keyFramesCount == 1)
					{
						keyFrames->push_back({ 0.0f, reader.ReadFloat(), 0.0f });
					}
					else
					{
						for (uint32_t i = 0; i < keyFramesCount; i++)
							keyFrames->push_back({ reader.ReadFloat(), 0.0f, 0.0f });

						for (uint32_t i = 0; i < keyFramesCount; i++)
						{
							keyFrames->at(i).Value = reader.ReadFloat();
							keyFrames->at(i).Interpolation = reader.ReadFloat();
						}
					}
				});
			}
		}

	}

	void AetSet::UpdateLayerNames()
	{
		for (auto& aetLyo : AetLyos)
		{
			for (auto& aetLayer : aetLyo.AetLayers)
				aetLayer.Names.clear();
		}

		for (auto& aetLyo : AetLyos)
		{
			for (auto& aetLayer : aetLyo.AetLayers)
			{
				for (auto& aetObj : aetLayer.Objects)
				{
					if (aetObj.Type == AetObjType_Eff && aetObj.ReferencedLayer != nullptr)
					{
						bool nameExists = false;
						for (auto& layerNames : aetObj.ReferencedLayer->Names)
						{
							if (layerNames == aetObj.Name)
							{
								nameExists = true;
								break;
							}
						}

						if (!nameExists)
							aetObj.ReferencedLayer->Names.emplace_back(aetObj.Name);
					}
				}
			}
		}

		for (auto& aetLyo : AetLyos)
		{
			for (auto& aetLayer : aetLyo.AetLayers)
			{
				aetLayer.CommaSeperatedNames = "";
				for (size_t i = 0; i < aetLayer.Names.size(); i++)
				{
					aetLayer.CommaSeperatedNames.append(aetLayer.Names[i]);
					if (i < aetLayer.Names.size() - 1)
						aetLayer.CommaSeperatedNames.append(", ");
				}
			}
		}
	}

	void AetSet::Read(BinaryReader& reader)
	{
		AetSet* aetSet = this;

		void* startAddress = reader.GetPositionPtr();
		reader.ReadAt(startAddress, [&aetSet](BinaryReader& reader)
		{
			void* lyoPointer;
			while ((lyoPointer = reader.ReadPtr()) != nullptr)
			{
				aetSet->AetLyos.emplace_back();
				AetLyo* aetLyo = &aetSet->AetLyos.back();

				reader.ReadAt(lyoPointer, [&aetLyo](BinaryReader& reader)
				{
					aetLyo->Name = reader.ReadStrPtr();
					aetLyo->Unknown = reader.ReadUInt32();
					aetLyo->FrameDuration = reader.ReadFloat();
					aetLyo->FrameRate = reader.ReadFloat();
					aetLyo->MaybeColor = reader.ReadUInt32();
					aetLyo->Width = reader.ReadInt32();
					aetLyo->Height = reader.ReadInt32();
					aetLyo->DontChangeMe = reader.ReadUInt32();

					uint32_t layersCount = reader.ReadUInt32();
					reader.ReadAt(reader.ReadPtr(), [layersCount, &aetLyo](BinaryReader& reader)
					{
						for (size_t i = 0; i < layersCount; i++)
						{
							aetLyo->AetLayers.emplace_back();
							AetLayer* aetLayer = &aetLyo->AetLayers.back();
							aetLayer->FilePtr = reader.GetPositionPtr();

							uint32_t objectCount = reader.ReadUInt32();
							reader.ReadAt(reader.ReadPtr(), [objectCount, &aetLayer](BinaryReader& reader)
							{
								for (uint32_t i = 0; i < objectCount; i++)
								{
									aetLayer->Objects.emplace_back();
									AetObj* aetObj = &aetLayer->Objects.back();

									aetObj->FilePtr = reader.GetPositionPtr();
									aetObj->Name = reader.ReadStrPtr();
									aetObj->LoopStart = reader.ReadFloat();
									aetObj->LoopEnd = reader.ReadFloat();
									aetObj->StartFrame = reader.ReadFloat();
									aetObj->PlaybackSpeed = reader.ReadFloat();
									for (size_t i = 0; i < sizeof(aetObj->UnknownBytes); i++)
										aetObj->UnknownBytes[i] = reader.ReadByte();
									aetObj->Type = reader.Read<AetObjType>();

									if (aetObj->Type != AetObjType_Nop)
									{
										aetObj->DataFilePtr = reader.ReadPtr();
										aetObj->ParentFilePtr = reader.ReadPtr();

										uint32_t markerCount = reader.ReadUInt32();
										void* markersPointer = reader.ReadPtr();

										if (markerCount > 0 && markersPointer != nullptr)
										{
											reader.ReadAt(markersPointer, [markerCount, &aetObj](BinaryReader& reader)
											{
												aetObj->Markers.reserve(markerCount);
												for (uint32_t i = 0; i < markerCount; i++)
													aetObj->Markers.push_back({ reader.ReadFloat(), reader.ReadStrPtr() });
											});
										}

										void* animationDataPointer = reader.ReadPtr();
										if (animationDataPointer != nullptr)
										{
											AnimationData* animationData = &aetObj->AnimationData;
											reader.ReadAt(animationDataPointer, [&animationData](BinaryReader& reader)
											{
												animationData->BlendMode = reader.Read<AetBlendMode>();
												animationData->UnknownFlag0 = reader.Read<unk8_t>();
												animationData->UseTextureMask = reader.ReadBoolean();
												animationData->UnknownFlag2 = reader.Read<unk8_t>();
												animationData->Properties = std::make_unique<KeyFrameProperties>();
												ReadKeyFrameProperties(animationData->Properties.get(), reader);

												void* propertiesExtraDataPointer = reader.ReadPtr();
												if (propertiesExtraDataPointer != nullptr)
												{
													reader.ReadAt(propertiesExtraDataPointer, [&animationData](BinaryReader& reader)
													{
														animationData->PropertiesExtraData = std::make_unique<KeyFrameProperties>();
														ReadKeyFrameProperties(animationData->PropertiesExtraData.get(), reader);
													});
												}
											});
										}

										aetObj->UnknownFilePtr = reader.ReadPtr();
									}
								}
							});
						}
					});

					uint32_t spriteEntriesCount = reader.ReadUInt32();
					reader.ReadAt(reader.ReadPtr(), [spriteEntriesCount, &aetLyo](BinaryReader& reader)
					{
						for (size_t i = 0; i < spriteEntriesCount; i++)
						{
							aetLyo->SpriteEntries.emplace_back();
							SpriteEntry* spriteEntry = &aetLyo->SpriteEntries.back();

							spriteEntry->FilePtr = reader.GetPositionPtr();
							spriteEntry->Unknown = reader.ReadUInt32();
							spriteEntry->Width = reader.ReadUInt16();
							spriteEntry->Height = reader.ReadUInt16();
							spriteEntry->Frames = reader.ReadFloat();

							uint32_t spritesCount = reader.ReadUInt32();
							void* spritesPointer = reader.ReadPtr();

							spriteEntry->Sprites.reserve(spritesCount);
							reader.ReadAt(spritesPointer, [spritesCount, &spriteEntry](BinaryReader& reader)
							{
								for (uint32_t i = 0; i < spritesCount; i++)
									spriteEntry->Sprites.push_back({ reader.ReadStrPtr(), reader.ReadUInt32() });
							});
						}
					});
				});
			}
		});

		LinkPostRead(aetSet);
		UpdateLayerNames();
	}

	void AetSet::LinkPostRead(AetSet* aetSet)
	{
		for (auto &aetLyo : aetSet->AetLyos)
		{
			for (auto &aetLayer : aetLyo.AetLayers)
			{
				for (auto &aetObj : aetLayer.Objects)
				{
					aetObj.ReferencedSprite = nullptr;
					aetObj.ReferencedLayer = nullptr;
					if (aetObj.DataFilePtr != nullptr)
					{
						if (aetObj.Type == AetObjType_Pic)
						{
							for (auto &spriteEntry : aetLyo.SpriteEntries)
								if (spriteEntry.FilePtr == aetObj.DataFilePtr)
									aetObj.ReferencedSprite = &spriteEntry;
						}
						else if (aetObj.Type == AetObjType_Eff)
						{
							for (auto &layer : aetLyo.AetLayers)
								if (layer.FilePtr == aetObj.DataFilePtr)
									aetObj.ReferencedLayer = &layer;
						}
					}

					aetObj.ReferencedObjParent = nullptr;
					if (aetObj.ParentFilePtr != nullptr)
					{
						for (auto &otherObj : aetLayer.Objects)
							if (otherObj.FilePtr == aetObj.ParentFilePtr)
								aetObj.ReferencedObjParent = &otherObj;
					}
				}
			}
		}
	}
}