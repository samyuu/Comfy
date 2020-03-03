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
			*(reinterpret_cast<uint8_t*>(&value) + 0) = reader.ReadU8();
			*(reinterpret_cast<uint8_t*>(&value) + 1) = reader.ReadU8();
			*(reinterpret_cast<uint8_t*>(&value) + 2) = reader.ReadU8();
			const auto padding = reader.ReadU8();
			return value;
		}

		void ReadProperty1DPointer(AetProperty1D& property, BinaryReader& reader)
		{
			size_t keyFrameCount = reader.ReadSize();
			FileAddr keyFramesPointer = reader.ReadPtr();

			if (keyFrameCount > 0 && keyFramesPointer != FileAddr::NullPtr)
			{
				reader.ReadAt(keyFramesPointer, [keyFrameCount, &property](BinaryReader& reader)
				{
					property->resize(keyFrameCount);

					if (keyFrameCount == 1)
					{
						property->front().Value = reader.ReadF32();
					}
					else
					{
						for (size_t i = 0; i < keyFrameCount; i++)
							property.Keys[i].Frame = reader.ReadF32();

						for (size_t i = 0; i < keyFrameCount; i++)
						{
							property.Keys[i].Value = reader.ReadF32();
							property.Keys[i].Curve = reader.ReadF32();
						}
					}
				});
			}
		}

		void ReadProperty2DPointer(AetProperty2D& property, BinaryReader& reader)
		{
			ReadProperty1DPointer(property.X, reader);
			ReadProperty1DPointer(property.Y, reader);
		}

		void ReadTransform(AetTransform& transform, BinaryReader& reader)
		{
			ReadProperty2DPointer(transform.Origin, reader);
			ReadProperty2DPointer(transform.Position, reader);
			ReadProperty1DPointer(transform.Rotation, reader);
			ReadProperty2DPointer(transform.Scale, reader);
			ReadProperty1DPointer(transform.Opacity, reader);
		}

		void WriteProperty1DPointer(const AetProperty1D& property, BinaryWriter& writer)
		{
			if (property->size() > 0)
			{
				writer.WriteU32(static_cast<uint32_t>(property->size()));
				writer.WritePtr([&property](BinaryWriter& writer)
				{
					if (property->size() == 1)
					{
						writer.WriteF32(property->front().Value);
					}
					else
					{
						for (auto& keyFrame : property.Keys)
							writer.WriteF32(keyFrame.Frame);

						for (auto& keyFrame : property.Keys)
						{
							writer.WriteF32(keyFrame.Value);
							writer.WriteF32(keyFrame.Curve);
						}
					}
				});
			}
			else
			{
				writer.WriteU32(0x00000000);		// key frames count
				writer.WritePtr(FileAddr::NullPtr); // key frames offset
			}
		}

		void WriteProperty2DPointer(const AetProperty2D& property, BinaryWriter& writer)
		{
			WriteProperty1DPointer(property.X, writer);
			WriteProperty1DPointer(property.Y, writer);
		}

		void WriteTransform(const AetTransform& transform, BinaryWriter& writer)
		{
			WriteProperty2DPointer(transform.Origin, writer);
			WriteProperty2DPointer(transform.Position, writer);
			WriteProperty1DPointer(transform.Rotation, writer);
			WriteProperty2DPointer(transform.Scale, writer);
			WriteProperty1DPointer(transform.Opacity, writer);
		}

		void ReadAnimationData(RefPtr<AetAnimationData>& animationData, BinaryReader& reader)
		{
			animationData = MakeRef<AetAnimationData>();
			animationData->BlendMode = static_cast<AetBlendMode>(reader.ReadU8());
			reader.ReadU8();
			animationData->UseTextureMask = reader.ReadBool();
			reader.ReadU8();

			if (reader.GetPointerMode() == PtrMode::Mode64Bit)
				reader.ReadU32();

			ReadTransform(animationData->Transform, reader);

			FileAddr perspectivePropertiesPointer = reader.ReadPtr();
			if (perspectivePropertiesPointer != FileAddr::NullPtr)
			{
				reader.ReadAt(perspectivePropertiesPointer, [animationData](BinaryReader& reader)
				{
					animationData->PerspectiveTransform = MakeRef<AetTransform>();
					ReadTransform(*animationData->PerspectiveTransform, reader);
				});
			}
		}

		void SetTransformStartFrame(AetTransform& transform, frame_t startFrame)
		{
			auto setProperty1DStartFrame = [](auto& property, frame_t startFrame)
			{
				if (property->size() == 1)
					property->front().Frame = startFrame;
			};

			setProperty1DStartFrame(transform.Origin.X, startFrame);
			setProperty1DStartFrame(transform.Origin.Y, startFrame);
			setProperty1DStartFrame(transform.Position.X, startFrame);
			setProperty1DStartFrame(transform.Position.Y, startFrame);
			setProperty1DStartFrame(transform.Rotation, startFrame);
			setProperty1DStartFrame(transform.Scale.X, startFrame);
			setProperty1DStartFrame(transform.Scale.Y, startFrame);
			setProperty1DStartFrame(transform.Opacity, startFrame);
		}
	}

	void AetLayer::Read(BinaryReader& reader)
	{
		filePosition = reader.GetPosition();
		name = reader.ReadStrPtr();
		StartFrame = reader.ReadF32();
		EndFrame = reader.ReadF32();
		StartOffset = reader.ReadF32();
		PlaybackSpeed = reader.ReadF32();

		Flags.AllBits = reader.ReadU16();
		TypePaddingByte = reader.ReadU8();
		Type = static_cast<AetLayerType>(reader.ReadU8());

		if (reader.GetPointerMode() == PtrMode::Mode64Bit)
			reader.ReadU32();

		dataFilePtr = reader.ReadPtr();
		parentFilePtr = reader.ReadPtr();

		size_t markerCount = reader.ReadSize();
		FileAddr markersPointer = reader.ReadPtr();

		if (markerCount > 0 && markersPointer != FileAddr::NullPtr)
		{
			Markers.reserve(markerCount);
			for (size_t i = 0; i < markerCount; i++)
				Markers.push_back(MakeRef<AetMarker>());

			reader.ReadAt(markersPointer, [this](BinaryReader& reader)
			{
				for (auto& marker : Markers)
				{
					marker->Frame = reader.ReadF32();
					if (reader.GetPointerMode() == PtrMode::Mode64Bit)
						reader.ReadU32();
					marker->Name = reader.ReadStrPtr();
				}
			});
		}

		FileAddr animationDataPointer = reader.ReadPtr();
		if (animationDataPointer != FileAddr::NullPtr)
		{
			reader.ReadAt(animationDataPointer, [this](BinaryReader& reader)
			{
				ReadAnimationData(AnimationData, reader);

				SetTransformStartFrame(AnimationData->Transform, StartFrame);
				if (AnimationData->PerspectiveTransform != nullptr)
					SetTransformStartFrame(*AnimationData->PerspectiveTransform, StartFrame);
			});
		}

		audioDataFilePtr = reader.ReadPtr();
	}

	void Aet::Read(BinaryReader& reader)
	{
		Name = reader.ReadStrPtr();
		StartFrame = reader.ReadF32();
		EndFrame = reader.ReadF32();
		FrameRate = reader.ReadF32();
		BackgroundColor = ReadColor(reader);

		Resolution.x = reader.ReadI32();
		Resolution.y = reader.ReadI32();

		FileAddr cameraOffsetPtr = reader.ReadPtr();
		if (cameraOffsetPtr != FileAddr::NullPtr)
		{
			Camera = MakeRef<AetCamera>();
			reader.ReadAt(cameraOffsetPtr, [this](BinaryReader& reader)
			{
				ReadProperty2DPointer(Camera->Position, reader);
			});
		}

		size_t compCount = reader.ReadSize();
		FileAddr compsPtr = reader.ReadPtr();
		if (compCount > 0 && compsPtr != FileAddr::NullPtr)
		{
			Compositions.resize(compCount - 1);
			reader.ReadAt(compsPtr, [this](BinaryReader& reader)
			{
				const auto readCompFunction = [](BinaryReader& reader, RefPtr<AetComposition>& comp)
				{
					comp = MakeRef<AetComposition>();
					comp->filePosition = reader.GetPosition();

					size_t layerCount = reader.ReadSize();
					FileAddr layersPointer = reader.ReadPtr();

					if (layerCount > 0 && layersPointer != FileAddr::NullPtr)
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
		FileAddr surfacesPtr = reader.ReadPtr();
		if (surfaceCount > 0 && surfacesPtr != FileAddr::NullPtr)
		{
			Surfaces.resize(surfaceCount);
			reader.ReadAt(surfacesPtr, [this](BinaryReader& reader)
			{
				for (auto& surface : Surfaces)
				{
					surface = MakeRef<AetSurface>();
					surface->filePosition = reader.GetPosition();
					surface->Color = ReadColor(reader);
					surface->Size.x = reader.ReadU16();
					surface->Size.y = reader.ReadU16();
					surface->Frames = reader.ReadF32();

					uint32_t spriteCount = reader.ReadU32();
					FileAddr spritesPointer = reader.ReadPtr();

					if (spriteCount > 0 && spritesPointer != FileAddr::NullPtr)
					{
						surface->sprites.resize(spriteCount);
						reader.ReadAt(spritesPointer, [&surface](BinaryReader& reader)
						{
							for (AetSpriteIdentifier& sprite : surface->sprites)
							{
								sprite.Name = reader.ReadStrPtr();
								sprite.ID = SprID(reader.ReadU32());
							}
						});
					}
				}
			});
		}

		size_t soundEffectCount = reader.ReadSize();
		FileAddr soundEffectsPtr = reader.ReadPtr();
		if (soundEffectCount > 0 && soundEffectsPtr != FileAddr::NullPtr)
		{
			SoundEffects.resize(soundEffectCount);
			reader.ReadAt(soundEffectsPtr, [this](BinaryReader& reader)
			{
				for (auto& soundEffect : SoundEffects)
				{
					soundEffect = MakeRef<AetSoundEffect>();
					soundEffect->filePosition = reader.GetPosition();
					soundEffect->Data = reader.ReadU32();
				}
			});
		}
	}

	void Aet::Write(BinaryWriter& writer)
	{
		writer.WritePtr([this](BinaryWriter& writer)
		{
			FileAddr aetFilePosition = writer.GetPosition();
			writer.WriteStrPtr(Name);
			writer.WriteF32(StartFrame);
			writer.WriteF32(EndFrame);
			writer.WriteF32(FrameRate);
			writer.WriteU32(BackgroundColor);
			writer.WriteI32(Resolution.x);
			writer.WriteI32(Resolution.y);

			if (Camera != nullptr)
			{
				writer.WritePtr([this](BinaryWriter& writer)
				{
					WriteProperty2DPointer(Camera->Position, writer);
				});
			}
			else
			{
				writer.WritePtr(FileAddr::NullPtr); // AetCamera offset
			}

			assert(RootComposition != nullptr);
			writer.WriteU32(static_cast<uint32_t>(Compositions.size()) + 1);
			writer.WritePtr([this](BinaryWriter& writer)
			{
				const auto writeCompFunction = [](BinaryWriter& writer, const RefPtr<AetComposition>& comp)
				{
					comp->filePosition = writer.GetPosition();
					if (comp->size() > 0)
					{
						writer.WriteU32(static_cast<uint32_t>(comp->size()));
						writer.WritePtr([&comp](BinaryWriter& writer)
						{
							for (auto& layer : *comp)
							{
								layer->filePosition = writer.GetPosition();
								writer.WriteStrPtr(layer->name);
								writer.WriteF32(layer->StartFrame);
								writer.WriteF32(layer->EndFrame);
								writer.WriteF32(layer->StartOffset);
								writer.WriteF32(layer->PlaybackSpeed);
								writer.WriteType<AetLayerFlags>(layer->Flags);
								writer.WriteU8(layer->TypePaddingByte);
								writer.WriteType<AetLayerType>(layer->Type);

								bool hasData =
									(layer->Type == AetLayerType::Pic && layer->GetReferencedSurface() != nullptr) ||
									(layer->Type == AetLayerType::Aif && layer->GetReferencedSoundEffect() != nullptr) ||
									(layer->Type == AetLayerType::Eff && layer->GetReferencedComposition() != nullptr);

								if (hasData)
								{
									writer.WriteDelayedPtr([&layer](BinaryWriter& writer)
									{
										FileAddr filePosition =
											(layer->Type == AetLayerType::Pic) ? layer->GetReferencedSurface()->filePosition :
											(layer->Type == AetLayerType::Aif) ? layer->GetReferencedSoundEffect()->filePosition :
											layer->GetReferencedComposition()->filePosition;

										writer.WritePtr(filePosition);
									});
								}
								else
								{
									writer.WritePtr(FileAddr::NullPtr); // Data offset
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
									writer.WritePtr(FileAddr::NullPtr); // Parent offset
								}

								if (layer->Markers.size() > 0)
								{
									writer.WriteU32(static_cast<uint32_t>(layer->Markers.size()));
									writer.WritePtr([&layer](BinaryWriter& writer)
									{
										for (auto& marker : layer->Markers)
										{
											writer.WriteF32(marker->Frame);
											writer.WriteStrPtr(marker->Name);
										}
									});
								}
								else
								{
									writer.WriteU32(0x00000000);		// Markers size
									writer.WritePtr(FileAddr::NullPtr); // Markers offset
								}

								if (layer->AnimationData != nullptr)
								{
									AetAnimationData& animationData = *layer->AnimationData.get();
									writer.WritePtr([&animationData](BinaryWriter& writer)
									{
										writer.WriteType<AetBlendMode>(animationData.BlendMode);
										writer.WriteU8(0x00);
										writer.WriteBool(animationData.UseTextureMask);
										writer.WriteU8(0x00);

										WriteTransform(animationData.Transform, writer);

										if (animationData.PerspectiveTransform != nullptr)
										{
											writer.WritePtr([&animationData](BinaryWriter& writer)
											{
												WriteTransform(*animationData.PerspectiveTransform, writer);
												writer.WriteAlignmentPadding(16);
											});
										}
										else
										{
											writer.WritePtr(FileAddr::NullPtr); // PerspectiveProperties offset
										}

										writer.WriteAlignmentPadding(16);
									});
								}
								else
								{
									writer.WritePtr(FileAddr::NullPtr); // AnimationData offset
								}

								// TODO: audioDataFilePtr
								writer.WritePtr(FileAddr::NullPtr); // AudioData offset
							}

							writer.WriteAlignmentPadding(16);
						});
					}
					else
					{
						writer.WriteU32(0x00000000);		// AetComposition size
						writer.WritePtr(FileAddr::NullPtr); // AetComposition offset
					}
				};

				for (auto& comp : Compositions)
					writeCompFunction(writer, comp);
				writeCompFunction(writer, RootComposition);

				writer.WriteAlignmentPadding(16);
			});

			if (Surfaces.size() > 0)
			{
				writer.WriteU32(static_cast<uint32_t>(Surfaces.size()));
				writer.WritePtr([this](BinaryWriter& writer)
				{
					for (auto& surface : Surfaces)
					{
						surface->filePosition = writer.GetPosition();
						writer.WriteU32(surface->Color);
						writer.WriteI16(static_cast<int16_t>(surface->Size.x));
						writer.WriteI16(static_cast<int16_t>(surface->Size.y));
						writer.WriteF32(surface->Frames);
						if (surface->SpriteCount() > 0)
						{
							writer.WriteU32(static_cast<uint32_t>(surface->SpriteCount()));
							writer.WritePtr([&surface](BinaryWriter& writer)
							{
								for (AetSpriteIdentifier& sprite : surface->GetSprites())
								{
									writer.WriteStrPtr(sprite.Name);
									writer.WriteU32(static_cast<uint32_t>(sprite.ID));
								}
							});
						}
						else
						{
							writer.WriteU32(0x00000000);		// AetSprites size
							writer.WritePtr(FileAddr::NullPtr); // AetSprites offset
						}
					}
					writer.WriteAlignmentPadding(16);
				});
			}
			else
			{
				writer.WriteU32(0x00000000);		// AetSurfaces size
				writer.WritePtr(FileAddr::NullPtr); // AetSurfaces offset
			}

			if (SoundEffects.size() > 0)
			{
				writer.WriteU32(static_cast<uint32_t>(SoundEffects.size()));
				writer.WritePtr([this](BinaryWriter& writer)
				{
					for (auto& soundEffect : SoundEffects)
					{
						soundEffect->filePosition = writer.GetPosition();
						writer.WriteU32(soundEffect->Data);
					}
					writer.WriteAlignmentPadding(16);
				});
			}
			else
			{
				writer.WriteU32(0x00000000);		// AetSoundEffects size
				writer.WritePtr(FileAddr::NullPtr);	// AetSoundEffects offset
			}

			writer.WriteAlignmentPadding(16);
		});
	}

	void AetSet::Read(BinaryReader& reader)
	{
		uint32_t signature = reader.ReadU32();
		if (signature == 'AETC' || signature == 'CTEA')
		{
			reader.SetEndianness(Endianness::Little);
			uint32_t dataSize = reader.ReadU32();
			uint32_t dataOffset = reader.ReadU32();
			uint32_t endianSignaure = reader.ReadU32();

			enum { LittleEndian = 0x10000000, BigEndian = 0x18000000 };

			reader.SetPosition(static_cast<FileAddr>(dataOffset));
			reader.SetEndianness(endianSignaure == LittleEndian ? Endianness::Little : Endianness::Big);

			if (dataOffset < 0x40)
			{
				reader.SetPointerMode(PtrMode::Mode64Bit);
				reader.SetStreamSeekOffset(static_cast<FileAddr>(dataOffset));
			}
		}
		else
		{
			reader.SetPosition(reader.GetPosition() - FileAddr(sizeof(signature)));
		}

		FileAddr startAddress = reader.GetPosition();

		size_t aetCount = 0;
		while (reader.ReadPtr() != FileAddr::NullPtr)
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

		writer.WritePtr(FileAddr::NullPtr);
		writer.WriteAlignmentPadding(16);

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushStringPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushDelayedWritePool();
	}
}
