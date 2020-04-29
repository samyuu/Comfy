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

		u32 ReadColor(StreamReader& reader)
		{
			u32 value = 0;
			*(reinterpret_cast<u8*>(&value) + 0) = reader.ReadU8();
			*(reinterpret_cast<u8*>(&value) + 1) = reader.ReadU8();
			*(reinterpret_cast<u8*>(&value) + 2) = reader.ReadU8();
			const auto padding = reader.ReadU8();
			return value;
		}

		void ReadProperty1DPointer(Property1D& property, StreamReader& reader)
		{
			size_t keyFrameCount = reader.ReadSize();
			FileAddr keyFramesPointer = reader.ReadPtr();

			if (keyFrameCount > 0 && keyFramesPointer != FileAddr::NullPtr)
			{
				reader.ReadAt(keyFramesPointer, [keyFrameCount, &property](StreamReader& reader)
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

		void ReadProperty2DPointer(Property2D& property, StreamReader& reader)
		{
			ReadProperty1DPointer(property.X, reader);
			ReadProperty1DPointer(property.Y, reader);
		}

		void ReadProperty3DPointer(Property3D& property, StreamReader& reader)
		{
			ReadProperty1DPointer(property.X, reader);
			ReadProperty1DPointer(property.Y, reader);
			ReadProperty1DPointer(property.Z, reader);
		}

		void ReadLayerVideo2D(LayerVideo2D& transform, StreamReader& reader)
		{
			ReadProperty2DPointer(transform.Origin, reader);
			ReadProperty2DPointer(transform.Position, reader);
			ReadProperty1DPointer(transform.Rotation, reader);
			ReadProperty2DPointer(transform.Scale, reader);
			ReadProperty1DPointer(transform.Opacity, reader);
		}

		void ReadLayerVideo3D(LayerVideo3D& transform, StreamReader& reader)
		{
			ReadProperty1DPointer(transform.OriginZ, reader);
			ReadProperty1DPointer(transform.PositionZ, reader);
			ReadProperty3DPointer(transform.Direction, reader);
			ReadProperty2DPointer(transform.Rotation, reader);
			ReadProperty1DPointer(transform.ScaleZ, reader);
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
				writer.WriteU32(0x00000000);		// key frames count
				writer.WritePtr(FileAddr::NullPtr); // key frames offset
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
			WriteProperty3DPointer(transform.Direction, writer);
			WriteProperty2DPointer(transform.Rotation, writer);
			WriteProperty1DPointer(transform.ScaleZ, writer);
		}

		void ReadLayerVideo(RefPtr<LayerVideo>& layerVideo, StreamReader& reader)
		{
			layerVideo = MakeRef<LayerVideo>();
			layerVideo->TransferMode.BlendMode = static_cast<AetBlendMode>(reader.ReadU8());
			layerVideo->TransferMode.Flags = ReadFlagsStruct<TransferFlags>(reader);
			layerVideo->TransferMode.TrackMatte = static_cast<TrackMatte>(reader.ReadU8());
			reader.ReadU8();

			if (reader.GetPointerMode() == PtrMode::Mode64Bit)
				reader.ReadU32();

			ReadLayerVideo2D(layerVideo->Transform, reader);

			FileAddr perspectivePropertiesPointer = reader.ReadPtr();
			if (perspectivePropertiesPointer != FileAddr::NullPtr)
			{
				reader.ReadAt(perspectivePropertiesPointer, [layerVideo](StreamReader& reader)
				{
					layerVideo->Transform3D = MakeRef<LayerVideo3D>();
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
			SetProperty1DStartFrame(transform.Direction.X, startFrame);
			SetProperty1DStartFrame(transform.Direction.Y, startFrame);
			SetProperty1DStartFrame(transform.Direction.Z, startFrame);
			SetProperty1DStartFrame(transform.Rotation.X, startFrame);
			SetProperty1DStartFrame(transform.Rotation.Y, startFrame);
			SetProperty1DStartFrame(transform.ScaleZ, startFrame);
		}
	}

	void Layer::Read(StreamReader& reader)
	{
		filePosition = reader.GetPosition();
		name = reader.ReadStrPtr();
		StartFrame = reader.ReadF32();
		EndFrame = reader.ReadF32();
		StartOffset = reader.ReadF32();
		TimeScale = reader.ReadF32();

		Flags = ReadFlagsStruct<LayerFlags>(reader);
		Quality = static_cast<LayerQuality>(reader.ReadU8());
		ItemType = static_cast<Aet::ItemType>(reader.ReadU8());

		if (reader.GetPointerMode() == PtrMode::Mode64Bit)
			reader.ReadU32();

		itemFilePtr = reader.ReadPtr();
		parentFilePtr = reader.ReadPtr();

		size_t markerCount = reader.ReadSize();
		FileAddr markersPointer = reader.ReadPtr();

		if (markerCount > 0 && markersPointer != FileAddr::NullPtr)
		{
			Markers.reserve(markerCount);
			for (size_t i = 0; i < markerCount; i++)
				Markers.push_back(MakeRef<Marker>());

			reader.ReadAt(markersPointer, [this](StreamReader& reader)
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

		FileAddr layerVideoPointer = reader.ReadPtr();
		if (layerVideoPointer != FileAddr::NullPtr)
		{
			reader.ReadAt(layerVideoPointer, [this](StreamReader& reader)
			{
				ReadLayerVideo(this->LayerVideo, reader);
				SetLayerVideoStartFrame(this->LayerVideo->Transform, StartFrame);
				if (this->LayerVideo->Transform3D != nullptr)
					SetLayerVideoStartFrame(*this->LayerVideo->Transform3D, StartFrame);
			});
		}

		audioDataFilePtr = reader.ReadPtr();
	}

	void Scene::Read(StreamReader& reader)
	{
		Name = reader.ReadStrPtr();
		StartFrame = reader.ReadF32();
		EndFrame = reader.ReadF32();
		FrameRate = reader.ReadF32();
		BackgroundColor = ReadColor(reader);
		Resolution = reader.ReadIV2();

		FileAddr cameraOffsetPtr = reader.ReadPtr();
		if (cameraOffsetPtr != FileAddr::NullPtr)
		{
			Camera = MakeRef<Aet::Camera>();
			reader.ReadAt(cameraOffsetPtr, [this](StreamReader& reader)
			{
				ReadProperty3DPointer(Camera->Eye, reader);
				ReadProperty3DPointer(Camera->Position, reader);
				ReadProperty3DPointer(Camera->Direction, reader);
				ReadProperty3DPointer(Camera->Rotation, reader);
				ReadProperty1DPointer(Camera->Zoom, reader);
			});
		}

		size_t compCount = reader.ReadSize();
		FileAddr compsPtr = reader.ReadPtr();
		if (compCount > 0 && compsPtr != FileAddr::NullPtr)
		{
			Compositions.resize(compCount - 1);
			reader.ReadAt(compsPtr, [this](StreamReader& reader)
			{
				const auto readCompFunction = [](StreamReader& reader, RefPtr<Composition>& comp)
				{
					comp = MakeRef<Composition>();
					comp->filePosition = reader.GetPosition();

					size_t layerCount = reader.ReadSize();
					FileAddr layersPointer = reader.ReadPtr();

					if (layerCount > 0 && layersPointer != FileAddr::NullPtr)
					{
						comp->GetLayers().resize(layerCount);
						reader.ReadAt(layersPointer, [&comp](StreamReader& reader)
						{
							for (auto& layer : comp->GetLayers())
							{
								layer = MakeRef<Layer>();
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

		size_t videoCount = reader.ReadSize();
		FileAddr videosPtr = reader.ReadPtr();
		if (videoCount > 0 && videosPtr != FileAddr::NullPtr)
		{
			Videos.resize(videoCount);
			reader.ReadAt(videosPtr, [this](StreamReader& reader)
			{
				for (auto& video : Videos)
				{
					video = MakeRef<Video>();
					video->filePosition = reader.GetPosition();
					video->Color = ReadColor(reader);
					video->Size.x = reader.ReadU16();
					video->Size.y = reader.ReadU16();
					video->FilesPerFrame = reader.ReadF32();

					u32 spriteCount = reader.ReadU32();
					FileAddr spritesPointer = reader.ReadPtr();

					if (spriteCount > 0 && spritesPointer != FileAddr::NullPtr)
					{
						video->Sources.resize(spriteCount);
						reader.ReadAt(spritesPointer, [&video](StreamReader& reader)
						{
							for (auto& source : video->Sources)
							{
								source.Name = reader.ReadStrPtr();
								source.ID = SprID(reader.ReadU32());
							}
						});
					}
				}
			});
		}

		size_t audioCount = reader.ReadSize();
		FileAddr audiosPtr = reader.ReadPtr();
		if (audioCount > 0 && audiosPtr != FileAddr::NullPtr)
		{
			Audios.resize(audioCount);
			reader.ReadAt(audiosPtr, [this](StreamReader& reader)
			{
				for (auto& audio : Audios)
				{
					audio = MakeRef<Audio>();
					audio->filePosition = reader.GetPosition();
					audio->SoundID = reader.ReadU32();
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
				writer.WritePtr(FileAddr::NullPtr); // camera offset
			}

			assert(RootComposition != nullptr);
			writer.WriteU32(static_cast<u32>(Compositions.size()) + 1);
			writer.WriteFuncPtr([this](StreamWriter& writer)
			{
				const auto writeCompFunction = [](StreamWriter& writer, const RefPtr<Composition>& comp)
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

								FileAddr itemFilePosition = FileAddr::NullPtr;
								if (layer->ItemType == ItemType::Video && layer->GetVideoItem() != nullptr)
									itemFilePosition = layer->GetVideoItem()->filePosition;
								else if (layer->ItemType == ItemType::Audio && layer->GetAudioItem() != nullptr)
									itemFilePosition = layer->GetAudioItem()->filePosition;
								else if (layer->ItemType == ItemType::Composition && layer->GetCompItem() != nullptr)
									itemFilePosition = layer->GetCompItem()->filePosition;

								if (itemFilePosition != FileAddr::NullPtr)
								{
									writer.WriteDelayedPtr([itemFilePosition](StreamWriter& writer)
									{
										writer.WritePtr(itemFilePosition);
									});
								}
								else
								{
									writer.WritePtr(FileAddr::NullPtr); // item offset
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
									writer.WritePtr(FileAddr::NullPtr); // parent offset
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
									writer.WriteU32(0x00000000);		// markers size
									writer.WritePtr(FileAddr::NullPtr); // markers offset
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
											writer.WritePtr(FileAddr::NullPtr); // LayerVideo3D offset
										}

										writer.WriteAlignmentPadding(16);
									});
								}
								else
								{
									writer.WritePtr(FileAddr::NullPtr); // LayerVideo offset
								}

								// TODO: audioDataFilePtr
								writer.WritePtr(FileAddr::NullPtr); // LayerAudio offset
							}

							writer.WriteAlignmentPadding(16);
						});
					}
					else
					{
						writer.WriteU32(0x00000000);		// compositions size
						writer.WritePtr(FileAddr::NullPtr); // compositions offset
					}
				};

				for (auto& comp : Compositions)
					writeCompFunction(writer, comp);
				writeCompFunction(writer, RootComposition);

				writer.WriteAlignmentPadding(16);
			});

			if (Videos.size() > 0)
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
							writer.WriteU32(0x00000000);		// sources size
							writer.WritePtr(FileAddr::NullPtr); // sources offset
						}
					}
					writer.WriteAlignmentPadding(16);
				});
			}
			else
			{
				writer.WriteU32(0x00000000);		// videos size
				writer.WritePtr(FileAddr::NullPtr); // videos offset
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
				writer.WriteU32(0x00000000);		// audios size
				writer.WritePtr(FileAddr::NullPtr);	// audios offset
			}

			writer.WriteAlignmentPadding(16);
		});
	}

	void AetSet::Read(StreamReader& reader)
	{
		u32 signature = reader.ReadU32();
		if (signature == 'AETC' || signature == 'CTEA')
		{
			reader.SetEndianness(Endianness::Little);
			u32 dataSize = reader.ReadU32();
			u32 dataOffset = reader.ReadU32();
			u32 endianSignaure = reader.ReadU32();

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

		size_t sceneCount = 0;
		while (reader.ReadPtr() != FileAddr::NullPtr)
			sceneCount++;
		scenes.reserve(sceneCount);

		reader.ReadAt(startAddress, [this, sceneCount](StreamReader& reader)
		{
			for (size_t i = 0; i < sceneCount; i++)
			{
				scenes.push_back(MakeRef<Scene>());
				auto& scene = scenes.back();

				reader.ReadAt(reader.ReadPtr(), [&scene](StreamReader& reader)
				{
					scene->Read(reader);
				});

				scene->UpdateParentPointers();
				scene->LinkPostRead();
				scene->UpdateCompNamesAfterLayerItems();
			}
		});
	}

	void AetSet::Write(StreamWriter& writer)
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
	}
}
