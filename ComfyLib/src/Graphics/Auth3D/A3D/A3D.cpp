#include "A3D.h"
#include "Misc/StringUtil.h"
#include "Misc/StringParseHelper.h"
#include "Misc/TextDatabaseParser.h"

namespace Comfy::Graphics
{
	using namespace Util;

	namespace
	{
		struct A3DParser final : public StringParsing::TextDatabaseParser
		{
		private:
			// NOTE: Use a template so the A3DKeyFrameType check can be done outside the keyFrameCount loop as a constexpr if without duplicating code
			template <typename TValue, A3DKeyFrameType TType>
			void ParseProperty1DRawDataValueList(A3DProperty1D& output)
			{
				constexpr std::array<size_t, EnumCount<A3DKeyFrameType>()> valuesPerKeyFramePerType = { 1, 2, 3, 4 };
				constexpr size_t valuesPerKeyFrame = valuesPerKeyFramePerType[static_cast<size_t>(TType)];

				const size_t keyFrameCount = output.RawData.ValueListSize / valuesPerKeyFrame;
				output.Keys.resize(keyFrameCount);

				for (size_t i = 0; i < keyFrameCount; i++)
				{
					auto& key = output.Keys[i];
					key.Type = output.RawData.KeyType;

					if constexpr (TType == A3DKeyFrameType::Frame)
					{
						key.Frame = ParseAdvanceCommaSeparatedValueString<TValue>();
						key.Value = 0.0f;
						key.StartCurve = 0.0f;
						key.EndCurve = 0.0f;
					}
					else if constexpr (TType == A3DKeyFrameType::FrameValue)
					{
						key.Frame = ParseAdvanceCommaSeparatedValueString<TValue>();
						key.Value = ParseAdvanceCommaSeparatedValueString<TValue>();
						key.StartCurve = 0.0f;
						key.EndCurve = 0.0f;
					}
					else if constexpr (TType == A3DKeyFrameType::FrameValueCurveStart)
					{
						key.Frame = ParseAdvanceCommaSeparatedValueString<TValue>();
						key.Value = ParseAdvanceCommaSeparatedValueString<TValue>();
						key.StartCurve = ParseAdvanceCommaSeparatedValueString<TValue>();
						key.EndCurve = 0.0f;
					}
					else if constexpr (TType == A3DKeyFrameType::FrameValueCurveStartEnd)
					{
						key.Frame = ParseAdvanceCommaSeparatedValueString<TValue>();
						key.Value = ParseAdvanceCommaSeparatedValueString<TValue>();
						key.StartCurve = ParseAdvanceCommaSeparatedValueString<TValue>();
						key.EndCurve = ParseAdvanceCommaSeparatedValueString<TValue>();
					}
					else
					{
						static_assert(false);
					}
				}
			}

			bool TryParseProperty1D(A3DProperty1D& output)
			{
				if (CompareProperty("value"))
				{
					output.StaticValue = ParseValueString<float>();
				}
				else if (CompareProperty("type"))
				{
					output.Type = ParseEnumValueString<A3DInterpolationType>();
				}
				else if (CompareProperty("raw_data_key_type"))
				{
					output.RawData.KeyType = ParseEnumValueString<A3DKeyFrameType>();
				}
				else if (CompareProperty("raw_data"))
				{
					if (CompareProperty("value_type"))
					{
						if (ParseValueString() == "float")
							output.RawData.ValueType = A3DValueType::Float;
						else
							output.RawData.ValueType = A3DValueType::Unknown;
					}
					else if (CompareProperty("value_list_size"))
					{
						output.RawData.ValueListSize = ParseValueString<u32>();
					}
					else if (CompareProperty("value_list"))
					{
						if (output.RawData.ValueType == A3DValueType::Float)
						{
							switch (output.RawData.KeyType)
							{
							case A3DKeyFrameType::Frame:
								ParseProperty1DRawDataValueList<float, A3DKeyFrameType::Frame>(output);
								break;

							case A3DKeyFrameType::FrameValue:
								ParseProperty1DRawDataValueList<float, A3DKeyFrameType::FrameValue>(output);
								break;

							case A3DKeyFrameType::FrameValueCurveStart:
								ParseProperty1DRawDataValueList<float, A3DKeyFrameType::FrameValueCurveStart>(output);
								break;

							case A3DKeyFrameType::FrameValueCurveStartEnd:
								ParseProperty1DRawDataValueList<float, A3DKeyFrameType::FrameValueCurveStartEnd>(output);
								break;
							}
						}
					}
					else
					{
						return false;
					}
				}
				else if (CompareProperty("max"))
				{
					output.Max = ParseValueString<float>();
				}
				else if (CompareProperty("key"))
				{
					if (!TryParseLength(output.Keys))
					{
						auto& key = output.Keys[ParseAdvanceIndexProperty()];

						if (CompareProperty("type"))
							key.Type = ParseEnumValueString<A3DKeyFrameType>();
						else if (CompareProperty("data"))
						{
							switch (key.Type)
							{
							case A3DKeyFrameType::Frame:
							{
								key.Frame = ParseValueString<float>();
								key.Value = 0.0f;
								key.StartCurve = 0.0f;
								key.EndCurve = 0.0f;
							}
							break;

							case A3DKeyFrameType::FrameValue:
							{
								auto[frame, value] = ParseCommaSeparatedArray<float, 2>();
								key.Frame = frame;
								key.Value = value;
								key.StartCurve = 0.0f;
								key.EndCurve = 0.0f;
							}
							break;

							case A3DKeyFrameType::FrameValueCurveStart:
							{
								auto[frame, value, startCurve] = ParseCommaSeparatedArray<float, 3>();
								key.Frame = frame;
								key.Value = value;
								key.StartCurve = startCurve;
								key.EndCurve = 0.0f;
							}
							break;

							case A3DKeyFrameType::FrameValueCurveStartEnd:
							{
								auto[frame, value, startCurve, endCurve] = ParseCommaSeparatedArray<float, 4>();
								key.Frame = frame;
								key.Value = value;
								key.StartCurve = startCurve;
								key.EndCurve = endCurve;
							}
							break;
							}
						}
					}
				}
				else if (CompareProperty("ep_type_pre"))
				{
					output.EPTypePre = ParseEnumValueString<EPType>();
				}
				else if (CompareProperty("ep_type_post"))
				{
					output.EPTypePost = ParseEnumValueString<EPType>();
				}
				else if (IsLastProperty())
				{
					output.Enabled = ParseValueString<bool>();
				}
				else
				{
					return false;
				}

				return true;
			}

			bool TryParseProperty3D(A3DProperty3D& output)
			{
				if (CompareProperty("z"))
					return TryParseProperty1D(output.Z);
				else if (CompareProperty("y"))
					return TryParseProperty1D(output.Y);
				else if (CompareProperty("x"))
					return TryParseProperty1D(output.X);

				return false;
			}

			bool TryParsePropertyRGB(A3DPropertyRGB& output)
			{
				if (CompareProperty("b"))
					return TryParseProperty1D(output.B);
				else if (CompareProperty("g"))
					return TryParseProperty1D(output.G);
				else if (CompareProperty("r"))
					return TryParseProperty1D(output.R);

				return false;
			}

			bool TryParseLightColor(A3DLightColor& output)
			{
				if (CompareProperty("Specular"))
					return TryParsePropertyRGB(output.Specular);
				else if (CompareProperty("Incandescence"))
					return TryParsePropertyRGB(output.Incandescence);
				else if (CompareProperty("Diffuse"))
					return TryParsePropertyRGB(output.Diffuse);
				else if (CompareProperty("Ambient"))
					return TryParsePropertyRGB(output.Ambient);

				return false;
			}

			bool TryParseTransformProperties(A3DTransform& output)
			{
				if (CompareProperty("visibility"))
					return TryParseProperty1D(output.Visibility);
				else if (CompareProperty("trans"))
					return TryParseProperty3D(output.Translation);
				else if (CompareProperty("scale"))
					return TryParseProperty3D(output.Scale);
				else if (CompareProperty("rot"))
					return TryParseProperty3D(output.Rotation);

				return false;
			}

			bool TryParseObjectHRCs(std::vector<A3DObjectHRC>& output)
			{
				if (!TryParseLength(output))
				{
					auto& objectHRC = output[ParseAdvanceIndexProperty()];

					if (CompareProperty("uid_name"))
						objectHRC.UIDName = ParseValueString();
					else if (CompareProperty("shadow"))
						objectHRC.Shadow = ParseValueString<int>();
					else if (CompareProperty("node"))
					{
						if (!TryParseLength(objectHRC.Nodes))
						{
							auto& node = objectHRC.Nodes[ParseAdvanceIndexProperty()];

							if (!TryParseTransformProperties(node.Transform))
							{
								if (CompareProperty("parent"))
									node.Parent = ParseValueString<u32>();
								else if (CompareProperty("name"))
									node.Name = ParseValueString();
							}
						}
					}
					else if (CompareProperty("name"))
					{
						objectHRC.Name = ParseValueString();
					}
					else
					{
						return false;
					}

					return true;
				}

				return false;
			}

			void ParseA3DProperties(A3D& a3d)
			{
				if (CompareProperty("_"))
				{
					if (CompareProperty("property"))
					{
						if (CompareProperty("version"))
							a3d.Metadata.Property.Version = ParseValueString<u32>();
					}
					else if (CompareProperty("file_name"))
					{
						a3d.Metadata.FileName = ParseValueString();
					}
					else if (CompareProperty("converter"))
					{
						if (CompareProperty("version"))
							a3d.Metadata.Converter.Version = ParseValueString<u32>();
					}
				}
				else if (CompareProperty("play_control"))
				{
					if (CompareProperty("size"))
						a3d.PlayControl.Duration = ParseValueString<frame_t>();
					else if (CompareProperty("fps"))
						a3d.PlayControl.FrameRate = ParseValueString<frame_t>();
					else if (CompareProperty("begin"))
						a3d.PlayControl.Begin = ParseValueString<frame_t>();
				}
				else if (CompareProperty("point"))
				{
					if (!TryParseLength(a3d.Points))
					{
						auto& point = a3d.Points[ParseAdvanceIndexProperty()];

						if (!TryParseTransformProperties(point.Transform))
						{
							if (CompareProperty("name"))
								point.Name = ParseValueString();
						}
					}
				}
				else if (CompareProperty("curve"))
				{
					if (!TryParseLength(a3d.Curves))
					{
						auto& curve = a3d.Curves[ParseAdvanceIndexProperty()];

						if (CompareProperty("name"))
							curve.Name = ParseValueString();
						else if (CompareProperty("cv"))
							TryParseProperty1D(curve.CV);
					}
				}
				else if (CompareProperty("camera_root"))
				{
					if (!TryParseLength(a3d.CameraRoot))
					{
						auto& camera = a3d.CameraRoot[ParseAdvanceIndexProperty()];

						if (!TryParseTransformProperties(camera.Transform))
						{
							if (CompareProperty("view_point"))
							{
								if (!TryParseTransformProperties(camera.ViewPoint.Transform))
								{
									if (CompareProperty("roll"))
										TryParseProperty3D(camera.ViewPoint.Roll);
									else if (CompareProperty("fov_is_horizontal"))
										camera.ViewPoint.HorizontalFieldOfView = ParseValueString<int>();
									else if (CompareProperty("fov"))
										TryParseProperty1D(camera.ViewPoint.FieldOfView);
									else if (CompareProperty("aspect"))
										camera.ViewPoint.AspectRatio = ParseValueString<float>();
								}
							}
							else if (CompareProperty("interest"))
								TryParseTransformProperties(camera.Interest);
						}
					}
				}
				else if (CompareProperty("camera_auxiliary"))
				{
					if (CompareProperty("exposure"))
						TryParseProperty1D(a3d.CameraAuxiliary.Exposure);
					else if (CompareProperty("gamma"))
						TryParseProperty1D(a3d.CameraAuxiliary.Gamma);
					else if (CompareProperty("saturate"))
						TryParseProperty1D(a3d.CameraAuxiliary.Saturate);
					else if (CompareProperty("auto_exposure"))
						TryParseProperty1D(a3d.CameraAuxiliary.AutoExposure);
				}
				else if (CompareProperty("light"))
				{
					if (!TryParseLength(a3d.Lights))
					{
						auto& light = a3d.Lights[ParseAdvanceIndexProperty()];

						if (!TryParseLightColor(light.Color))
						{
							if (CompareProperty("type"))
								light.Type = ParseValueString();
							else if (CompareProperty("spot_direction"))
								TryParseTransformProperties(light.SpotDirection);
							else if (CompareProperty("position"))
								TryParseTransformProperties(light.Position);
							else if (CompareProperty("name"))
								light.Name = ParseValueString();
							else if (CompareProperty("id"))
								light.ID = ParseValueString<u32>();
						}
					}
				}
				else if (CompareProperty("fog"))
				{
					if (!TryParseLength(a3d.Fog))
					{
						auto& fog = a3d.Fog[ParseAdvanceIndexProperty()];

						if (CompareProperty("type"))
							fog.ID = ParseValueString<u32>();
						else if (CompareProperty("density"))
							TryParseProperty1D(fog.Density);
						else if (CompareProperty("start"))
							TryParseProperty1D(fog.Start);
						else if (CompareProperty("end"))
							TryParseProperty1D(fog.End);
						else if (CompareProperty("Diffuse"))
							TryParsePropertyRGB(fog.Diffuse);
					}
				}
				else if (CompareProperty("post_process"))
				{
					if (!TryParseLightColor(a3d.PostProcess.LightColor))
					{
						if (CompareProperty("lens_flare"))
							TryParseProperty1D(a3d.PostProcess.LensFlare);
						else if (CompareProperty("lens_ghost"))
							TryParseProperty1D(a3d.PostProcess.LensGhost);
						else if (CompareProperty("lens_shaft"))
							TryParseProperty1D(a3d.PostProcess.LensShaft);
					}
				}
				else if (CompareProperty("dof"))
				{
					if (!TryParseTransformProperties(a3d.DepthOfField.Transform))
					{
						if (CompareProperty("name"))
							a3d.DepthOfField.Name = ParseValueString();
					}
				}
				else if (CompareProperty("chara"))
				{
					if (!TryParseLength(a3d.Characters))
					{
						auto& character = a3d.Characters[ParseAdvanceIndexProperty()];

						if (!TryParseTransformProperties(character.Transform))
						{
							if (CompareProperty("name"))
								character.Name = ParseValueString();
						}
					}
				}
				else if (CompareProperty("motion"))
				{
					if (!TryParseLength(a3d.Motions))
						a3d.Motions[ParseAdvanceIndexProperty()] = ParseValueString();
				}
				else if (CompareProperty("auth_2d"))
				{
					if (!TryParseLength(a3d.Auth2D))
					{
						auto& auth2D = a3d.Auth2D[ParseAdvanceIndexProperty()];

						if (CompareProperty("name"))
							auth2D.Name = ParseValueString();
					}
				}
				else if (CompareProperty("object"))
				{
					if (!TryParseLength(a3d.Objects))
					{
						auto& object = a3d.Objects[ParseAdvanceIndexProperty()];

						if (!TryParseTransformProperties(object.Transform))
						{
							if (CompareProperty("name"))
								object.Name = ParseValueString();
							else if (CompareProperty("uid_name"))
								object.UIDName = ParseValueString();
							else if (CompareProperty("pat"))
								object.Pat = ParseValueString();
							else if (CompareProperty("pat_offset"))
								object.PatOffset = ParseValueString<u32>();
							else if (CompareProperty("morph"))
								object.MorphName = ParseValueString();
							else if (CompareProperty("morph_offset"))
								object.MorphOffset = ParseValueString<u32>();
							else if (CompareProperty("parent_name"))
								object.ParentName = ParseValueString();
							else if (CompareProperty("tex_pat"))
							{
								if (!TryParseLength(object.TexturePatterns))
								{
									auto& texturePat = object.TexturePatterns[ParseAdvanceIndexProperty()];

									if (CompareProperty("name"))
										texturePat.Name = ParseValueString();
									else if (CompareProperty("pat"))
										texturePat.PatternName = ParseValueString();
									else if (CompareProperty("pat_offset"))
										texturePat.PatternOffset = ParseValueString<u32>();
								}
							}
							else if (CompareProperty("tex_transform"))
							{
								if (!TryParseLength(object.TextureTransforms))
								{
									auto& textureTransform = object.TextureTransforms[ParseAdvanceIndexProperty()];

									if (CompareProperty("name"))
										textureTransform.Name = ParseValueString();
									else if (CompareProperty("coverageU"))
										TryParseProperty1D(textureTransform.CoverageU);
									else if (CompareProperty("coverageV"))
										TryParseProperty1D(textureTransform.CoverageV);
									else if (CompareProperty("repeatU"))
										TryParseProperty1D(textureTransform.RepeatU);
									else if (CompareProperty("repeatV"))
										TryParseProperty1D(textureTransform.RepeatV);
									else if (CompareProperty("rotate"))
										TryParseProperty1D(textureTransform.Rotate);
									else if (CompareProperty("rotateFrame"))
										TryParseProperty1D(textureTransform.RotateFrame);
									else if (CompareProperty("offsetU"))
										TryParseProperty1D(textureTransform.OffsetU);
									else if (CompareProperty("offsetV"))
										TryParseProperty1D(textureTransform.OffsetV);
									else if (CompareProperty("translateFrameU"))
										TryParseProperty1D(textureTransform.TranslateFrameU);
									else if (CompareProperty("translateFrameV"))
										TryParseProperty1D(textureTransform.TranslateFrameV);
								}
							}
						}
					}
				}
				else if (CompareProperty("object_list"))
				{
					if (!TryParseLength(a3d.ObjectList))
						a3d.ObjectList[ParseAdvanceIndexProperty()] = ParseValueString();
				}
				else if (CompareProperty("objhrc"))
				{
					TryParseObjectHRCs(a3d.ObjectsHRC);
				}
				else if (CompareProperty("objhrc_list"))
				{
					if (!TryParseLength(a3d.ObjectHRCList))
						a3d.ObjectHRCList[ParseAdvanceIndexProperty()] = ParseValueString();
				}
				else if (CompareProperty("m_objhrc"))
				{
					TryParseObjectHRCs(a3d.MObjectsHRC);
				}
				else if (CompareProperty("m_objhrc_list"))
				{
					if (!TryParseLength(a3d.MObjectHRCList))
						a3d.MObjectHRCList[ParseAdvanceIndexProperty()] = ParseValueString();
				}
				else if (CompareProperty("event"))
				{
					if (!TryParseLength(a3d.Events))
					{
						auto& event = a3d.Events[ParseAdvanceIndexProperty()];

						if (CompareProperty("type"))
							event.Type = ParseEnumValueString<A3DEventType>();
						else if (CompareProperty("name"))
							event.Name = ParseValueString();
						else if (CompareProperty("begin"))
							event.Begin = ParseValueString<frame_t>();
						else if (CompareProperty("end"))
							event.End = ParseValueString<frame_t>();
						else if (CompareProperty("param1"))
							event.Parameters[0] = ParseValueString();
						else if (CompareProperty("ref"))
							event.Reference = ParseValueString();
						else if (CompareProperty("time_ref_scale"))
							event.TimeReferenceScale = ParseValueString<float>();
					}
				}
			}

		public:
			bool Parse(A3D& a3d, const char* startOfTextBuffer, const char* endOfTextBuffer)
			{
				const char* textBuffer = reinterpret_cast<const char*>(startOfTextBuffer);

				auto formatLine = StringParsing::GetLineAdvanceToNextLine(textBuffer);
				auto formatIdentifier = formatLine.substr(1, 4);

				if (StartsWith(formatIdentifier, "A3DA"))
					a3d.Metadata.Format = A3DFormat::Text;
				else if (StartsWith(formatIdentifier, "A3DC"))
					a3d.Metadata.Format = A3DFormat::Binary;
				else if (StartsWith(formatIdentifier, "A3DJ"))
					a3d.Metadata.Format = A3DFormat::Json;
				else if (StartsWith(formatIdentifier, "A3DM"))
					a3d.Metadata.Format = A3DFormat::MessagePack;

				if (a3d.Metadata.Format != A3DFormat::Text)
					return false;

				// NOTE: Update text buffer start beacuse the format line has already been processed
				startOfTextBuffer = textBuffer;

				// NOTE: Parse lines backwards to parse length properties before their array data
				textBuffer = endOfTextBuffer;

				while (textBuffer >= startOfTextBuffer)
				{
					const std::string_view currentLine = StringParsing::AdvanceToStartOfPreviousLineGetNonCommentLine(textBuffer, startOfTextBuffer);
					if (textBuffer <= startOfTextBuffer)
						break;

					StateParseNewLinePropertiesAndValue(currentLine);
					ParseA3DProperties(a3d);
				}

				return true;
			}
		};
	}

	A3D::A3D()
	{
	}

	void A3D::Parse(const u8* buffer, size_t bufferSize)
	{
		const char* startOfTextBuffer = reinterpret_cast<const char*>(buffer);
		const char* endOfTextBuffer = reinterpret_cast<const char*>(buffer + bufferSize);

		A3DParser parser;
		parser.Parse(*this, startOfTextBuffer, endOfTextBuffer);

		UpdateReferencePointers();
	}

	void A3D::UpdateReferencePointers()
	{
		auto findReferenceByName = [](const auto& refName, auto& collectionToSearch)
		{
			return refName.empty() ? nullptr : FindIfOrNull(collectionToSearch, [&](const auto& item) { return MatchesInsensitive(item.Name, refName); });
		};

		std::for_each(Objects.begin(), Objects.end(), [&](A3DObject& object) 
		{ 
			object.Parent = findReferenceByName(object.ParentName, Objects); 
			object.Morph = findReferenceByName(object.MorphName, Curves);

			std::for_each(object.TexturePatterns.begin(), object.TexturePatterns.end(), [&](A3DTexturePattern& texturePattern) 
			{ 
				texturePattern.Pattern = findReferenceByName(texturePattern.PatternName, Curves); 
			});
		});
	}
}
