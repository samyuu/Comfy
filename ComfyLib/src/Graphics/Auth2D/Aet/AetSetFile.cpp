#include "AetSet.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/Stream/Manipulator/StreamWriter.h"

using namespace Comfy::IO;

namespace Comfy::Graphics::Aet
{
	namespace
	{
		template <typename FlagsStruct>
		FlagsStruct ReadFlagsStruct(StreamReader& reader)
		{
			if constexpr (sizeof(FlagsStruct) == sizeof(u8))
			{
				const auto uintFlags = reader.ReadU8();
				return *reinterpret_cast<const FlagsStruct*>(&uintFlags);
			}
			else if constexpr (sizeof(FlagsStruct) == sizeof(u16))
			{
				const auto uintFlags = reader.ReadU16();
				return *reinterpret_cast<const FlagsStruct*>(&uintFlags);
			}
			else if constexpr (sizeof(FlagsStruct) == sizeof(u32))
			{
				const auto uintFlags = reader.ReadU32();
				return *reinterpret_cast<const FlagsStruct*>(&uintFlags);
			}
			else
			{
				static_assert(false);
			}
		}

		u32 ReadU32ColorRGB(StreamReader& reader)
		{
			u32 value = 0;
			*(reinterpret_cast<u8*>(&value) + 0) = reader.ReadU8();
			*(reinterpret_cast<u8*>(&value) + 1) = reader.ReadU8();
			*(reinterpret_cast<u8*>(&value) + 2) = reader.ReadU8();
			const auto padding = reader.ReadU8();
			return value;
		}

		StreamResult ReadProperty1DPointer(Property1D& property, StreamReader& reader)
		{
			const auto keyFrameCount = reader.ReadSize();
			const auto keyFramesOffset = reader.ReadPtr();

			if (keyFrameCount < 1)
				return StreamResult::Success;

			if (!reader.IsValidPointer(keyFramesOffset))
				return StreamResult::BadPointer;

			reader.ReadAtOffsetAware(keyFramesOffset, [keyFrameCount, &property](StreamReader& reader)
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

			return StreamResult::Success;
		}

		StreamResult ReadProperty2DPointer(Property2D& property, StreamReader& reader)
		{
			if (auto result = ReadProperty1DPointer(property.X, reader); result != StreamResult::Success) return result;
			if (auto result = ReadProperty1DPointer(property.Y, reader); result != StreamResult::Success) return result;
			return StreamResult::Success;
		}

		StreamResult ReadProperty3DPointer(Property3D& property, StreamReader& reader)
		{
			if (auto result = ReadProperty1DPointer(property.X, reader); result != StreamResult::Success) return result;
			if (auto result = ReadProperty1DPointer(property.Y, reader); result != StreamResult::Success) return result;
			if (auto result = ReadProperty1DPointer(property.Z, reader); result != StreamResult::Success) return result;
			return StreamResult::Success;
		}

		StreamResult ReadLayerVideo2D(LayerVideo2D& transform, StreamReader& reader)
		{
			if (auto result = ReadProperty2DPointer(transform.Origin, reader); result != StreamResult::Success) return result;
			if (auto result = ReadProperty2DPointer(transform.Position, reader); result != StreamResult::Success) return result;
			if (auto result = ReadProperty1DPointer(transform.Rotation, reader); result != StreamResult::Success) return result;
			if (auto result = ReadProperty2DPointer(transform.Scale, reader); result != StreamResult::Success) return result;
			if (auto result = ReadProperty1DPointer(transform.Opacity, reader); result != StreamResult::Success) return result;
			return StreamResult::Success;
		}

		StreamResult ReadLayerVideo3D(LayerVideo3D& transform, StreamReader& reader)
		{
			if (auto result = ReadProperty1DPointer(transform.OriginZ, reader); result != StreamResult::Success) return result;
			if (auto result = ReadProperty1DPointer(transform.PositionZ, reader); result != StreamResult::Success) return result;
			if (auto result = ReadProperty3DPointer(transform.DirectionXYZ, reader); result != StreamResult::Success) return result;
			if (auto result = ReadProperty2DPointer(transform.RotationXY, reader); result != StreamResult::Success) return result;
			if (auto result = ReadProperty1DPointer(transform.ScaleZ, reader); result != StreamResult::Success) return result;
			return StreamResult::Success;
		}

		template <typename FlagsStruct>
		void WriteFlagsStruct(const FlagsStruct& flags, StreamWriter& writer)
		{
			if constexpr (sizeof(FlagsStruct) == sizeof(u8))
				writer.WriteU8(*reinterpret_cast<const u8*>(&flags));
			else if constexpr (sizeof(FlagsStruct) == sizeof(u16))
				writer.WriteU16(*reinterpret_cast<const u8*>(&flags));
			else if constexpr (sizeof(FlagsStruct) == sizeof(u32))
				writer.WriteU32(*reinterpret_cast<const u32*>(&flags));
			else
				static_assert(false);
		}

		void WriteProperty1DPointer(const Property1D& property, StreamWriter& writer)
		{
			if (property->size() > 0)
			{
				writer.WriteU32(static_cast<u32>(property->size()));
				writer.WriteFuncPtr([&property](StreamWriter& writer)
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
				writer.WriteU32(0x00000000);
				writer.WritePtr(FileAddr::NullPtr);
			}
		}

		void WriteProperty2DPointer(const Property2D& property, StreamWriter& writer)
		{
			WriteProperty1DPointer(property.X, writer);
			WriteProperty1DPointer(property.Y, writer);
		}

		void WriteProperty3DPointer(const Property3D& property, StreamWriter& writer)
		{
			WriteProperty1DPointer(property.X, writer);
			WriteProperty1DPointer(property.Y, writer);
			WriteProperty1DPointer(property.Z, writer);
		}

		void WriteTransform(const LayerVideo2D& transform, StreamWriter& writer)
		{
			WriteProperty2DPointer(transform.Origin, writer);
			WriteProperty2DPointer(transform.Position, writer);
			WriteProperty1DPointer(transform.Rotation, writer);
			WriteProperty2DPointer(transform.Scale, writer);
			WriteProperty1DPointer(transform.Opacity, writer);
		}

		void WriteTransform(const LayerVideo3D& transform, StreamWriter& writer)
		{
			WriteProperty1DPointer(transform.OriginZ, writer);
			WriteProperty1DPointer(transform.PositionZ, writer);
			WriteProperty3DPointer(transform.DirectionXYZ, writer);
			WriteProperty2DPointer(transform.RotationXY, writer);
			WriteProperty1DPointer(transform.ScaleZ, writer);
		}

		void ReadLayerVideo(std::shared_ptr<LayerVideo>& layerVideo, StreamReader& reader)
		{
			layerVideo = std::make_shared<LayerVideo>();
			layerVideo->TransferMode.BlendMode = static_cast<AetBlendMode>(reader.ReadU8());
			layerVideo->TransferMode.Flags = ReadFlagsStruct<TransferFlags>(reader);
			layerVideo->TransferMode.TrackMatte = static_cast<TrackMatte>(reader.ReadU8());
			reader.Skip(static_cast<FileAddr>(sizeof(u8)));

			if (reader.GetPointerMode() == PtrMode::Mode64Bit)
				reader.ReadU32();

			ReadLayerVideo2D(layerVideo->Transform, reader);

			const auto perspectivePropertiesOffset = reader.ReadPtr();
			if (perspectivePropertiesOffset != FileAddr::NullPtr)
			{
				reader.ReadAtOffsetAware(perspectivePropertiesOffset, [layerVideo](StreamReader& reader)
				{
					layerVideo->Transform3D = std::make_shared<LayerVideo3D>();
					ReadLayerVideo3D(*layerVideo->Transform3D, reader);
				});
			}
		}

		void SetProperty1DStartFrame(Property1D& property, frame_t startFrame)
		{
			if (property->size() == 1)
				property->front().Frame = startFrame;
		}

		void SetLayerVideoStartFrame(LayerVideo2D& transform, frame_t startFrame)
		{
			SetProperty1DStartFrame(transform.Origin.X, startFrame);
			SetProperty1DStartFrame(transform.Origin.Y, startFrame);
			SetProperty1DStartFrame(transform.Position.X, startFrame);
			SetProperty1DStartFrame(transform.Position.Y, startFrame);
			SetProperty1DStartFrame(transform.Rotation, startFrame);
			SetProperty1DStartFrame(transform.Scale.X, startFrame);
			SetProperty1DStartFrame(transform.Scale.Y, startFrame);
			SetProperty1DStartFrame(transform.Opacity, startFrame);
		}

		void SetLayerVideoStartFrame(LayerVideo3D& transform, frame_t startFrame)
		{
			SetProperty1DStartFrame(transform.OriginZ, startFrame);
			SetProperty1DStartFrame(transform.PositionZ, startFrame);
			SetProperty1DStartFrame(transform.DirectionXYZ.X, startFrame);
			SetProperty1DStartFrame(transform.DirectionXYZ.Y, startFrame);
			SetProperty1DStartFrame(transform.DirectionXYZ.Z, startFrame);
			SetProperty1DStartFrame(transform.RotationXY.X, startFrame);
			SetProperty1DStartFrame(transform.RotationXY.Y, startFrame);
			SetProperty1DStartFrame(transform.ScaleZ, startFrame);
		}
	}

	void Layer::Read(StreamReader& reader)
	{
		filePosition = reader.GetPosition();
		name = reader.ReadStrPtrOffsetAware();
		StartFrame = reader.ReadF32();
		EndFrame = reader.ReadF32();
		StartOffset = reader.ReadF32();
		TimeScale = reader.ReadF32();

		Flags = ReadFlagsStruct<LayerFlags>(reader);
		Quality = static_cast<LayerQuality>(reader.ReadU8());
		ItemType = static_cast<Aet::ItemType>(reader.ReadU8());

		if (reader.GetPointerMode() == PtrMode::Mode64Bit)
			reader.ReadU32();

		itemFileOffset = reader.ReadPtr();
		parentFileOffset = reader.ReadPtr();

		const auto markerCount = reader.ReadSize();
		const auto markersOffset = reader.ReadPtr();

		if (markerCount > 0 && markersOffset != FileAddr::NullPtr)
		{
			Markers.reserve(markerCount);
			for (size_t i = 0; i < markerCount; i++)
				Markers.push_back(std::make_shared<Marker>());

			reader.ReadAtOffsetAware(markersOffset, [this](StreamReader& reader)
			{
				for (auto& marker : Markers)
				{
					marker->Frame = reader.ReadF32();
					if (reader.GetPointerMode() == PtrMode::Mode64Bit)
						reader.ReadU32();
					marker->Name = reader.ReadStrPtrOffsetAware();
				}
			});
		}

		const auto layerVideoOffset = reader.ReadPtr();
		if (layerVideoOffset != FileAddr::NullPtr)
		{
			reader.ReadAtOffsetAware(layerVideoOffset, [&](StreamReader& reader)
			{
				ReadLayerVideo(this->LayerVideo, reader);
				SetLayerVideoStartFrame(this->LayerVideo->Transform, StartFrame);

				if (this->LayerVideo->Transform3D != nullptr)
					SetLayerVideoStartFrame(*this->LayerVideo->Transform3D, StartFrame);
			});
		}

		audioDataFileOffset = reader.ReadPtr();
	}

	void Scene::Read(StreamReader& reader)
	{
		Name = reader.ReadStrPtrOffsetAware();
		StartFrame = reader.ReadF32();
		EndFrame = reader.ReadF32();
		FrameRate = reader.ReadF32();
		BackgroundColor = ReadU32ColorRGB(reader);
		Resolution = reader.ReadIV2();

		const auto cameraOffset = reader.ReadPtr();
		if (cameraOffset != FileAddr::NullPtr)
		{
			Camera = std::make_shared<Aet::Camera>();
			reader.ReadAtOffsetAware(cameraOffset, [this](StreamReader& reader)
			{
				ReadProperty3DPointer(Camera->Eye, reader);
				ReadProperty3DPointer(Camera->Position, reader);
				ReadProperty3DPointer(Camera->Direction, reader);
				ReadProperty3DPointer(Camera->Rotation, reader);
				ReadProperty1DPointer(Camera->Zoom, reader);
			});
		}

		const auto compCount = reader.ReadSize();
		const auto compsOffset = reader.ReadPtr();
		if (compCount > 0 && compsOffset != FileAddr::NullPtr)
		{
			Compositions.resize(compCount - 1);
			reader.ReadAtOffsetAware(compsOffset, [this](StreamReader& reader)
			{
				const auto readMakeComp = [](StreamReader& reader, std::shared_ptr<Composition>& comp)
				{
					comp = std::make_shared<Composition>();
					comp->filePosition = reader.GetPosition();

					const auto layerCount = reader.ReadSize();
					const auto layersOffset = reader.ReadPtr();

					if (layerCount > 0 && layersOffset != FileAddr::NullPtr)
					{
						comp->GetLayers().resize(layerCount);
						reader.ReadAtOffsetAware(layersOffset, [&comp](StreamReader& reader)
						{
							for (auto& layer : comp->GetLayers())
							{
								layer = std::make_shared<Layer>();
								layer->Read(reader);
							}
						});
					}
				};

				for (auto& comp : Compositions)
					readMakeComp(reader, comp);

				readMakeComp(reader, RootComposition);
			});
		}

		const auto videoCount = reader.ReadSize();
		const auto videosOffset = reader.ReadPtr();
		if (videoCount > 0 && videosOffset != FileAddr::NullPtr)
		{
			Videos.reserve(videoCount);
			reader.ReadAtOffsetAware(videosOffset, [&](StreamReader& reader)
			{
				for (size_t i = 0; i < videoCount; i++)
				{
					auto& video = *Videos.emplace_back(std::make_shared<Video>());
					video.filePosition = reader.GetPosition();
					video.Color = ReadU32ColorRGB(reader);
					video.Size.x = reader.ReadU16();
					video.Size.y = reader.ReadU16();
					video.FilesPerFrame = reader.ReadF32();

					const auto spriteCount = reader.ReadU32();
					const auto spritesOffset = reader.ReadPtr();

					if (spriteCount > 0 && spritesOffset != FileAddr::NullPtr)
					{
						video.Sources.resize(spriteCount);
						reader.ReadAtOffsetAware(spritesOffset, [&video](StreamReader& reader)
						{
							for (auto& source : video.Sources)
							{
								source.Name = reader.ReadStrPtrOffsetAware();
								source.ID = SprID(reader.ReadU32());
							}
						});
					}
				}
			});
		}

		const auto audioCount = reader.ReadSize();
		const auto audiosOffset = reader.ReadPtr();
		if (audioCount > 0 && audiosOffset != FileAddr::NullPtr)
		{
			Audios.reserve(audioCount);
			reader.ReadAtOffsetAware(audiosOffset, [&](StreamReader& reader)
			{
				for (size_t i = 0; i < audioCount; i++)
				{
					auto& audio = *Audios.emplace_back(std::make_shared<Audio>());
					audio.filePosition = reader.GetPosition();
					audio.SoundID = reader.ReadU32();
				}
			});
		}
	}

	void Scene::Write(StreamWriter& writer)
	{
		writer.WriteFuncPtr([this](StreamWriter& writer)
		{
			FileAddr sceneFilePosition = writer.GetPosition();
			writer.WriteStrPtr(Name);
			writer.WriteF32(StartFrame);
			writer.WriteF32(EndFrame);
			writer.WriteF32(FrameRate);
			writer.WriteU32(BackgroundColor);
			writer.WriteI32(Resolution.x);
			writer.WriteI32(Resolution.y);

			if (this->Camera != nullptr)
			{
				writer.WriteFuncPtr([this](StreamWriter& writer)
				{
					WriteProperty3DPointer(Camera->Eye, writer);
					WriteProperty3DPointer(Camera->Position, writer);
					WriteProperty3DPointer(Camera->Position, writer);
					WriteProperty3DPointer(Camera->Direction, writer);
					WriteProperty3DPointer(Camera->Rotation, writer);
					WriteProperty1DPointer(Camera->Zoom, writer);
				});
			}
			else
			{
				writer.WritePtr(FileAddr::NullPtr);
			}

			assert(RootComposition != nullptr);
			writer.WriteU32(static_cast<u32>(Compositions.size()) + 1);
			writer.WriteFuncPtr([this](StreamWriter& writer)
			{
				const auto writeComp = [](StreamWriter& writer, const std::shared_ptr<Composition>& comp)
				{
					comp->filePosition = writer.GetPosition();
					if (comp->GetLayers().size() > 0)
					{
						writer.WriteU32(static_cast<u32>(comp->GetLayers().size()));
						writer.WriteFuncPtr([&comp](StreamWriter& writer)
						{
							for (auto& layer : comp->GetLayers())
							{
								layer->filePosition = writer.GetPosition();
								writer.WriteStrPtr(layer->name);
								writer.WriteF32(layer->StartFrame);
								writer.WriteF32(layer->EndFrame);
								writer.WriteF32(layer->StartOffset);
								writer.WriteF32(layer->TimeScale);
								WriteFlagsStruct<LayerFlags>(layer->Flags, writer);
								writer.WriteU8(static_cast<u8>(layer->Quality));
								writer.WriteU8(static_cast<u8>(layer->ItemType));

								FileAddr itemFileOffset = FileAddr::NullPtr;
								if (layer->ItemType == ItemType::Video && layer->GetVideoItem() != nullptr)
									itemFileOffset = layer->GetVideoItem()->filePosition;
								else if (layer->ItemType == ItemType::Audio && layer->GetAudioItem() != nullptr)
									itemFileOffset = layer->GetAudioItem()->filePosition;
								else if (layer->ItemType == ItemType::Composition && layer->GetCompItem() != nullptr)
									itemFileOffset = layer->GetCompItem()->filePosition;

								if (itemFileOffset != FileAddr::NullPtr)
								{
									writer.WriteDelayedPtr([itemFileOffset](StreamWriter& writer)
									{
										writer.WritePtr(itemFileOffset);
									});
								}
								else
								{
									writer.WritePtr(FileAddr::NullPtr);
								}

								if (layer->GetRefParentLayer() != nullptr)
								{
									writer.WriteDelayedPtr([&layer](StreamWriter& writer)
									{
										writer.WritePtr(layer->GetRefParentLayer()->filePosition);
									});
								}
								else
								{
									writer.WritePtr(FileAddr::NullPtr);
								}

								if (layer->Markers.size() > 0)
								{
									writer.WriteU32(static_cast<u32>(layer->Markers.size()));
									writer.WriteFuncPtr([&layer](StreamWriter& writer)
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
									writer.WriteU32(0x00000000);
									writer.WritePtr(FileAddr::NullPtr);
								}

								if (layer->LayerVideo != nullptr)
								{
									LayerVideo& layerVideo = *layer->LayerVideo;
									writer.WriteFuncPtr([&layerVideo](StreamWriter& writer)
									{
										writer.WriteU8(static_cast<u8>(layerVideo.TransferMode.BlendMode));
										WriteFlagsStruct<TransferFlags>(layerVideo.TransferMode.Flags, writer);
										writer.WriteU8(static_cast<u8>(layerVideo.TransferMode.TrackMatte));
										writer.WriteU8(0xCC);

										WriteTransform(layerVideo.Transform, writer);

										if (layerVideo.Transform3D != nullptr)
										{
											writer.WriteFuncPtr([&layerVideo](StreamWriter& writer)
											{
												WriteTransform(*layerVideo.Transform3D, writer);
												writer.WriteAlignmentPadding(16);
											});
										}
										else
										{
											writer.WritePtr(FileAddr::NullPtr);
										}

										writer.WriteAlignmentPadding(16);
									});
								}
								else
								{
									writer.WritePtr(FileAddr::NullPtr);
								}

								// TODO: audioDataFilePtr
								writer.WritePtr(FileAddr::NullPtr);
							}

							writer.WriteAlignmentPadding(16);
						});
					}
					else
					{
						writer.WriteU32(0x00000000);
						writer.WritePtr(FileAddr::NullPtr);
					}
				};

				for (auto& comp : Compositions)
					writeComp(writer, comp);
				writeComp(writer, RootComposition);

				writer.WriteAlignmentPadding(16);
			});

			if (!Videos.empty())
			{
				writer.WriteU32(static_cast<u32>(Videos.size()));
				writer.WriteFuncPtr([this](StreamWriter& writer)
				{
					for (auto& video : Videos)
					{
						video->filePosition = writer.GetPosition();
						writer.WriteU32(video->Color);
						writer.WriteI16(static_cast<i16>(video->Size.x));
						writer.WriteI16(static_cast<i16>(video->Size.y));
						writer.WriteF32(video->FilesPerFrame);
						if (!video->Sources.empty())
						{
							writer.WriteU32(static_cast<u32>(video->Sources.size()));
							writer.WriteFuncPtr([&video](StreamWriter& writer)
							{
								for (auto& source : video->Sources)
								{
									writer.WriteStrPtr(source.Name);
									writer.WriteU32(static_cast<u32>(source.ID));
								}
							});
						}
						else
						{
							writer.WriteU32(0x00000000);
							writer.WritePtr(FileAddr::NullPtr);
						}
					}
					writer.WriteAlignmentPadding(16);
				});
			}
			else
			{
				writer.WriteU32(0x00000000);
				writer.WritePtr(FileAddr::NullPtr);
			}

			if (Audios.size() > 0)
			{
				writer.WriteU32(static_cast<u32>(Audios.size()));
				writer.WriteFuncPtr([this](StreamWriter& writer)
				{
					for (auto& audio : Audios)
					{
						audio->filePosition = writer.GetPosition();
						writer.WriteU32(audio->SoundID);
					}
					writer.WriteAlignmentPadding(16);
				});
			}
			else
			{
				writer.WriteU32(0x00000000);
				writer.WritePtr(FileAddr::NullPtr);
			}

			writer.WriteAlignmentPadding(16);
		});
	}

	StreamResult AetSet::Read(StreamReader& reader)
	{
		const auto baseHeader = SectionHeader::TryRead(reader, SectionSignature::AETC);
		SectionHeader::ScanPOFSectionsSetPointerMode(reader);

		if (baseHeader.has_value())
		{
			reader.SetEndianness(baseHeader->Endianness);
			reader.Seek(baseHeader->StartOfSubSectionAddress());

			if (reader.GetPointerMode() == PtrMode::Mode64Bit)
				reader.PushBaseOffset();
		}

		size_t sceneCount = 0;
		reader.ReadAt(reader.GetPosition(), [&](StreamReader& reader)
		{
			while (reader.ReadPtr() != FileAddr::NullPtr)
				sceneCount++;
		});

		scenes.reserve(sceneCount);
		for (size_t i = 0; i < sceneCount; i++)
		{
			auto& scene = *scenes.emplace_back(std::make_shared<Scene>());
			reader.ReadAtOffsetAware(reader.ReadPtr(), [&](StreamReader& reader)
			{
				scene.Read(reader);
			});

			scene.UpdateParentPointers();
			scene.LinkPostRead();
			scene.UpdateCompNamesAfterLayerItems();
		}

		if (baseHeader.has_value() && reader.GetPointerMode() == PtrMode::Mode64Bit)
			reader.PopBaseOffset();

		return StreamResult::Success;
	}

	StreamResult AetSet::Write(StreamWriter& writer)
	{
		for (auto& scene : scenes)
		{
			assert(scene != nullptr);
			scene->Write(writer);
		}

		writer.WritePtr(FileAddr::NullPtr);
		writer.WriteAlignmentPadding(16);

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushStringPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushDelayedWritePool();

		return StreamResult::Success;
	}
}
