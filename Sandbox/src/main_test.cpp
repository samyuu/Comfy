#include "pch.h"
#include "Application.h"
#include "TimeSpan.h"
#include "FileSystem/FileHelper.h"
#include "FileSystem/FileStream.h"
#include "FileSystem/MemoryStream.h"
#include "FileSystem/BinaryReader.h"
#include "FileSystem/Format/AetSet.h"
#include "FileSystem/Format/SprSet.h"
#include "FileSystem/Format/TxpSet.h"
#include "Graphics/Auth2D/AetMgr.h"
// #include "Graphics/Utilities/TextureUtilities.h"
// #include "Graphics/Utilities/s3tc.h"
// #include "Graphics/Utilities/decompress.h"
#include <stb/stb_image_write.h>
#include <string>
#include <filesystem>
#include <FontIcons.h>

using namespace std::filesystem;
using namespace FileSystem;

// DUMMY FUNCTIONS
// ---------------
void BlockDecompressImageBC1(int width, int height, uint8_t*, uint8_t*) {};
void BlockDecompressImageBC3(int width, int height, uint8_t*, uint8_t*) {};
void BlockDecompressImageBC5(int width, int height, uint8_t*, uint8_t*) {};
// ---------------

void SaveRgbaToFile(const char* filePath, int width, int height, const void* data)
{
	stbi_write_png(filePath,
		width,
		height,
		4,
		data,
		4 * width);
}

void SaveAsPng(FileSystem::Texture* texture)
{
	char filePath[MAX_PATH];
	sprintf_s<MAX_PATH>(filePath, "%s/%s.png", "dev_ram/2d", texture->Name.c_str());

	MipMap* mipMap = texture->MipMaps.front().get();

	int width = mipMap->Width;
	int height = mipMap->Height;

	{
		uint32_t *pixelData = new uint32_t[width * height];
		{
			//TextureUtilities::DecodeTexture(texture, pixelData);
			SaveRgbaToFile(filePath, width, height, pixelData);
		}
		delete[] pixelData;
		return;
	}

	switch (mipMap->Format)
	{
	case TextureFormat::DXT1:
	{
		uint32_t *pixelData = new uint32_t[width * height];
		BlockDecompressImageBC1(width, height, (uint8_t*)mipMap->Data.data(), (unsigned char*)pixelData);

		//for (size_t i = 0; i < width * height; i++)
		//{
		//	uint32_t pixel = pixelData[i];
		//	pixelData[i] = ((pixel >> 24) & 0xff) | ((pixel << 8) & 0xff0000) | ((pixel >> 8) & 0xff00) | ((pixel << 24) & 0xff000000);
		//}

		SaveRgbaToFile(filePath, width, height, pixelData);
		delete[] pixelData;
		break;
	}

	case TextureFormat::DXT5:
	{
		uint32_t *pixelData = new uint32_t[width * height];
		BlockDecompressImageBC3(width, height, (uint8_t*)mipMap->Data.data(), (unsigned char*)pixelData);

		//for (size_t i = 0; i < width * height; i++)
		//{
		//	uint32_t pixel = pixelData[i];
		//	pixelData[i] = ((pixel >> 24) & 0xff) | ((pixel << 8) & 0xff0000) | ((pixel >> 8) & 0xff00) | ((pixel << 24) & 0xff000000);
		//}

		SaveRgbaToFile(filePath, width, height, pixelData);
		delete[] pixelData;
		break;
	}

	case TextureFormat::RGTC2:
	{
		// first texture:
		// the red channel should store the grayscale
		// the green channel should store the alpha
		uint32_t *pixelData = new uint32_t[width * height];
		BlockDecompressImageBC5(width, height, (uint8_t*)mipMap->Data.data(), (unsigned char*)pixelData);
		SaveRgbaToFile(filePath, width, height, pixelData);
		delete[] pixelData;
		break;
	}
	break;

	case TextureFormat::RGBA8:
		mipMap->Data.resize(width * height * 4);
		SaveRgbaToFile(filePath, width, height, mipMap->Data.data());
		break;

	default:
		const char* formatNames[] = { "", "RGB", "RGBA", "", "", "RGBA4", "DXT1", "DXT3", "DXT4", "DXT5", "ATI1", "ATI2" };
		Logger::LogLine("[%s]: Unsupported format: %s", texture->Name.c_str(), formatNames[(int)mipMap->Format]);
		break;
	}
}

void SaveAsDDS(FileSystem::Texture* texture)
{
	MipMap* mipMap = texture->MipMaps.front().get();

	switch (mipMap->Format)
	{
	case TextureFormat::DXT1:
	case TextureFormat::DXT5:
	case TextureFormat::RGTC2:
	{
		char filePath[MAX_PATH];
		sprintf_s<MAX_PATH>(filePath, "%s/%s.dds", "dev_ram/2d", texture->Name.c_str());
		std::string stringFilePath = std::string(filePath);
		std::wstring wFilePath(stringFilePath.begin(), stringFilePath.end());

		FileStream file;
		file.CreateReadWrite(wFilePath.c_str());
		{
			char ddsMagic[4] = "DDS"; ddsMagic[3] = 0x20;
			file.Write(ddsMagic, sizeof(ddsMagic));

			uint32_t size = 124; file.Write(&size, sizeof(size));
			uint32_t flags = 4103; file.Write(&flags, sizeof(flags));
			file.Write(&mipMap->Height, sizeof(mipMap->Height));
			file.Write(&mipMap->Width, sizeof(mipMap->Width));
			uint32_t pitchOrLinearSize = mipMap->Data.size(); file.Write(&pitchOrLinearSize, sizeof(pitchOrLinearSize));
			uint32_t depth = 0; file.Write(&depth, sizeof(depth));
			uint32_t mipMapCount = 0; file.Write(&mipMapCount, sizeof(mipMapCount));
			uint32_t reserved0[11] = {}; file.Write(&reserved0, sizeof(reserved0));
			struct Func
			{
				static void SetFourCC(char* fourCC, TextureFormat format)
				{
					const char* name = "NONE";
					switch (format)
					{
					case TextureFormat::RGBA8: name = "RGBA"; break;
					case TextureFormat::DXT1: name = "DXT1"; break;
					case TextureFormat::DXT3: name = "DXT3"; break;
					case TextureFormat::DXT5: name = "DXT5"; break;
					case TextureFormat::RGTC2: name = "ATI2"; break;
					}
					strcpy_s(fourCC, strlen(name) + 1, name);
				}
			};
			struct DDS_PIXEL_FORMAT
			{
				uint32_t size = 32;
				uint32_t flags = 4;
				uint32_t fourCC;
				uint32_t rgbBitCount = 0;
				uint32_t rBitMask = 0, gBitMask = 0, bBitMask = 0, aBitMask = 0;
			} ddsprf;
			Func::SetFourCC((char*)&ddsprf.fourCC, mipMap->Format);
			file.Write(&ddsprf, sizeof(ddsprf));

			uint32_t caps = 4096; file.Write(&caps, sizeof(caps));
			uint32_t caps2[3] = {}; file.Write(&caps2, sizeof(caps2));
			uint32_t reserved1 = 0; file.Write(&reserved1, sizeof(reserved1));

			file.Write(mipMap->Data.data(), mipMap->Data.size());
		}
		file.Close();

		//BlockDecompressImageBC1(width, height, (uint8_t*)mipMap->Data->data(), (unsigned char*)pixelData);
		//SaveRgbaToFile(filePath, width, height, pixelData);
		break;
	}

	default:
		break;
	}
}

void AetTest()
{
	std::vector<KeyFrame> keyFrameVector =
	{
		// { 0.0f, 0.2f, 0.03f},
		// { 19.0f,0.77f, 0.03f },
		// { 20.0f, 0.8f, 0.015f },
		// { 21.0f, 0.8f, 0.0f },

		{ 5.00f,  0.00f, 0.15f },
		{ 8.00f,  0.30f, 0.15f },
		{ 9.00f,  0.40f, 0.09f },
		{ 10.00f, 0.42f, 0.04f },
		{ 25.00f, 0.78f, 0.04f },
		{ 26.00f, 0.80f, 0.02f },
		{ 27.00f, 0.80f, 0.00f },
	};

	std::unique_ptr<float[]> rawValues = std::make_unique<float[]>(keyFrameVector.size() * 3);
	for (size_t i = 0; i < keyFrameVector.size(); i++)
		rawValues[i] = keyFrameVector[i].Frame;

	float* valuesPtr = &rawValues[keyFrameVector.size()];
	for (size_t i = 0; i < keyFrameVector.size(); i++)
	{
		*valuesPtr = keyFrameVector[i].Value; valuesPtr++;
		*valuesPtr = keyFrameVector[i].Interpolation; valuesPtr++;
	}

	for (float i = -1.0f; i <= keyFrameVector.back().Frame; i++)
	{
		//float result = Auth2D::AetInterpolateAccurate(keyFrameVector.size(), rawValues.get(), i);
		float recreation = Auth2D::AetMgr::Interpolate(keyFrameVector, i);

		//Logger::LogLine("ORIG FRAME [%02.0f] = %.2f", i, result);
		Logger::LogLine("EDIT FRAME [%02.0f] = %.2f", i, recreation);

		//if (result != recreation)
		//	Logger::LogLine("--- NOT SAME ---");
		//Logger::NewLine();
	}

	// target:			- // output:
	// 00 frame: 0.20	- // FRAME [00] = 0.20
	// 01 frame: 0.23	- // FRAME [01] = 0.23
	// 02 frame: 0.26	- // FRAME [02] = 0.26
	// 03 frame: 0.29	- // FRAME [03] = 0.29
	// 04 frame: 0.32	- // FRAME [04] = 0.32
	// 05 frame: 0.35	- // FRAME [05] = 0.35
	// 06 frame: 0.38	- // FRAME [06] = 0.38
	// 07 frame: 0.41	- // FRAME [07] = 0.41
	// 08 frame: 0.44	- // FRAME [08] = 0.44
	// 09 frame: 0.47	- // FRAME [09] = 0.47
	// 10 frame: 0.50	- // FRAME [10] = 0.50
	// 11 frame: 0.53	- // FRAME [11] = 0.53
	// 12 frame: 0.56	- // FRAME [12] = 0.56
	// 13 frame: 0.59	- // FRAME [13] = 0.59
	// 14 frame: 0.62	- // FRAME [14] = 0.62
	// 15 frame: 0.65	- // FRAME [15] = 0.65
	// 16 frame: 0.68	- // FRAME [16] = 0.68
	// 17 frame: 0.71	- // FRAME [17] = 0.71
	// 18 frame: 0.74	- // FRAME [18] = 0.74
	// 19 frame: 0.77	- // FRAME [19] = 0.77
	// 20 frame: 0.80	- // FRAME [20] = 0.80
}

std::vector<std::string> GenerateIconListFromSource(const std::string& sourceFilePath)
{
	std::vector<std::string> sourceLines;
	FileSystem::ReadAllLines(sourceFilePath, &sourceLines);

	constexpr size_t defineStartLine = 10;
	constexpr size_t defineStartCharacter = 8; // strlen("#define ")

	std::vector<std::string> results;
	results.reserve(sourceLines.size());

	for (size_t i = defineStartLine; i < sourceLines.size(); i++)
	{
		std::string& sourceLine = sourceLines[i];
		if (sourceLine.size() < defineStartCharacter)
			continue;

		size_t spacePosition;
		for (spacePosition = defineStartCharacter; spacePosition < sourceLine.size(); spacePosition++)
		{
			if (sourceLine[spacePosition] == ' ')
				break;
		}

		std::string defineName = sourceLine.substr(defineStartCharacter, spacePosition - defineStartCharacter);

		results.emplace_back();
		std::string& result = results.back();

		result.reserve(defineName.size() + 3 + 2 + 2);
		result += "{ \"";
		result += defineName;
		result += "\", ";
		result += defineName;
		result += " },";
	}

	return results;
}

void AetWriteTest()
{
	if (false)
	{
		AetSet aetSet;
		aetSet.Load("Y:/Games/SBZV/7.10.00/mdata/M910/rom/2d/aet_gam_eff000.bin");
		aetSet.Save("Y:/Games/SBZV/7.10.00/mdata/M910/rom/2d/aet_gam_eff000_write_test.bin");
		return;
	}

	return;

	//AetSet aetSet;
	//aetSet.Load("dev_ram/aetset/aet_tst000.bin");

	//if (false)
	//{
	//	AetSet aetSet;
	//	aetSet.Load("Y:/Games/SBZV/7.10.00/rom/2d/aet_gam_cmn.bin");
	//	aetSet.Save("Y:/Games/SBZV/7.10.00/mdata/M969/rom/2d/aet_gam_cmn.bin");
	//}

	//for (auto& layer : aetSet[0].AetLayers)
	//{
	//	for (auto& obj : layer)
	//	{
	//		if (obj.AnimationData != nullptr)
	//		{
	//			// obj.AnimationData->PerspectiveProperties = nullptr;

	//			for (auto& keyFrame : obj.AnimationData->Properties.ScaleX())
	//				keyFrame.Value *= 4.0f;

	//			for (auto& keyFrame : obj.AnimationData->Properties.ScaleY())
	//				keyFrame.Value *= 4.0f;
	//		}
	//	}
	//}

	if (false)
	{
		DEBUG_STOPWATCH(__FUNCTION__);
		{
			AetSet aftGamCmn; aftGamCmn.Load("Y:/Games/SBZV/7.10.00/rom/2d/aet_gam_cmn.bin");

			AetSet aetSet;
			aetSet.Load("Y:/Games/SBZV/7.10.00/mdata/M912/rom/2d/gam_cmn_manual/aet_gam_cmn.bin");
			{
				Aet& aet = aetSet[0];
				for (size_t i = 79; i <= 103; i++)
				{
					for (size_t j = 0; j < aet.AetLayers[i].size(); j++)
					{
						auto& obj = aet.AetLayers[i][j];

						//const char* name = obj.GetName();
						//if ((name[0] == 'p' || name[1] == '_'))
						//	continue;

						AetObj* aftObj = &aftGamCmn[0].AetLayers[i + 18][j];

						float factor = (1280.0f / 1920.0f);

						obj.AnimationData->Properties = aftObj->AnimationData->Properties;
						//obj.AnimationData->Properties.PositionX() = aftObj->AnimationData->Properties.PositionX();
						//obj.AnimationData->Properties.PositionY() = aftObj->AnimationData->Properties.PositionY();

						//if ((name[0] == 'p' || name[1] == '_'))
						//{
						//	for (auto& keyFrame : obj.AnimationData->Properties.ScaleX()) keyFrame.Value *= factor;
						//	for (auto& keyFrame : obj.AnimationData->Properties.ScaleY()) keyFrame.Value *= factor;
						//}

						// for (auto& keyFrame : obj.AnimationData->Properties.OriginX()) keyFrame.Value *= factor;
						// for (auto& keyFrame : obj.AnimationData->Properties.OriginY()) keyFrame.Value *= factor;
						// 
						// for (auto& keyFrame : obj.AnimationData->Properties.ScaleX()) keyFrame.Value *= factor;
						// for (auto& keyFrame : obj.AnimationData->Properties.ScaleY()) keyFrame.Value *= factor;

						//for (auto& keyFrame : obj.AnimationData->Properties.ScaleX()) keyFrame.Value *= factor;
						//for (auto& keyFrame : obj.AnimationData->Properties.ScaleY()) keyFrame.Value *= factor;
						//for (auto& keyFrame : obj.AnimationData->Properties.PositionX()) keyFrame.Value -= 34;
						//for (auto& keyFrame : obj.AnimationData->Properties.PositionY()) keyFrame.Value += 40;
					}
				}

				for (size_t i = 0; i < 24; i++)
				{
					AetObj& obj = aet.AetLayers.back()[i + 83];
					AetObj& aftObj = aftGamCmn[0].AetLayers.back()[i + 86];

					obj.AnimationData = aftObj.AnimationData;
				}
			}
			aetSet.Save("Y:/Games/SBZV/7.10.00/mdata/M912/rom/2d/aet_gam_cmn.bin");
		}
	}

	//aetSet.Save("dev_ram/aetset/write_test/aet_write_test.bin");
	//aetSet.Save("Y:/Games/SBZV/7.10.00/mdata/M969/rom/2d/aet_gam_eff000.bin");

	if (true)
	{
		auto files = FileSystem::GetFiles("Y:/Games/SBZV/7.10.00/rom/2d");
		for (auto& file : files)
		{
			auto fileName = FileSystem::GetFileName(file);
			if (StartsWith(fileName, "aet_") && EndsWith(fileName, ".bin") && fileName != "aet_db.bin")
			{
				AetSet aetSet;
				aetSet.Load(file);

				// for (auto& aet : aetSet)
				// {
				// 	//for (auto& layer : aet.AetLayers)
				// 	auto& layer = aet.AetLayers.back();
				// 	for (auto& obj : layer)
				// 		if (obj.AnimationData != nullptr)
				// 		{
				// 			for (auto& keyFrame : obj.AnimationData->Properties.ScaleX())
				// 				keyFrame.Value *= .75f;
				// 			for (auto& keyFrame : obj.AnimationData->Properties.ScaleY())
				// 				keyFrame.Value *= .75f;
				// 		}
				// }

				aetSet.Save("Y:/Games/SBZV/7.10.00/mdata/M969/rom/2d/" + fileName);
			}
		}
	}
}

void AetConvertTest()
{
	const auto directory = "Y:/Games/Project Diva/BLJM61079/PS3_GAME/USRDIR/rom/data/2d/";

	auto files = GetFiles(directory + std::string("input"));
	for (auto& file : files)
	{
		if (!EndsWith(file, ".aec"))
			continue;

		AetSet aetSet;
		aetSet.Load(file);

		auto fileName = GetFileName(file, false);
		aetSet.Save(directory + std::string("output/") + fileName + ".bin");
	}

	//AetSet aetSet;
	//aetSet.Load("Y:/Games/Project Diva/BLJM61079/PS3_GAME/USRDIR/rom/data/2d/aet_gam_win.aec");

	int test = 0;
}

void MainTest()
{
	glfwInit();

	if (false)
	{
		AetConvertTest();
	}

	if (false)
	{
		AetWriteTest();
	}

	if (false)
	{
		//auto results = GenerateFromSource("lib/iconfont/include/IconsFontAwesome5.h");
		//for (auto& line : results)
		//	Logger::LogLine(line.c_str());
	}

	if (false)
	{
		AetTest();
		int __debuggerbreak__ = 0;
	}

	if (false)
	{
		DEBUG_STOPWATCH("Read All glad.txt Lines");

		std::vector<std::string> fileLines;
		ReadAllLines("license/glad.txt", &fileLines);

		for (auto& line : fileLines)
			std::cout << line << std::endl;
	}

	if (false)
	{
		DEBUG_STOPWATCH("Read All glad.txt Bytes");
		std::vector<uint8_t> fileBuffer;
		ReadAllBytes("license/glad.txt", &fileBuffer);
	}

	// COLOR SWIZZLE TEST:
	if (false)
	{
		constexpr int HEADER_SIZE = 0x80;

		std::vector<uint8_t> fileBuffer;
		ReadAllBytes("rom/spr/btn_icns.dds", &fileBuffer);
		{
			uint32_t* pixelBuffer = (uint32_t*)(fileBuffer.data() + HEADER_SIZE);
			uint32_t pixelCount = (fileBuffer.size() - HEADER_SIZE) / sizeof(uint32_t);

			for (uint32_t i = 0; i < pixelCount; i++)
			{
				uint32_t pixel = pixelBuffer[i];
				auto color = ImColor(pixel), copy = color;
				color.Value.x = copy.Value.z;
				color.Value.y = copy.Value.y;
				color.Value.z = copy.Value.x;
				pixelBuffer[i] = ImU32(color);
			}
		}
		WriteAllBytes("rom/spr/btn_icns_processed.dds", fileBuffer);
	}

	// TXP STGPV623 TEST:
	if (false)
	{
		auto filePath = "Y:/Debug/FileTest/objset/stgpv623/stgpv623_tex.bin";

		TxpSet slowTxpSet, fastTxpSet;
		std::vector<uint8_t> fastBuffer;

		if (true)
		{
			// 160.54 MS with copying
			// 149.28 MS without copying
			// 146.79 MS raw non vector array
			// 88.46 MS without copying & without allocations
			// 95.83 MS without copying & new uint8_t[dataSize];

			// 147.02 MS
			DEBUG_STOPWATCH("--- Slow Parse stgpv623_tex ---");
			slowTxpSet.Load(filePath);
		}

		if (true)
		{
			// 76.84 MS
			DEBUG_STOPWATCH("--- Quick Parse stgpv623_tex ---");

			{
				// 76.29 MS
				DEBUG_STOPWATCH("ReadAllBytes stgpv623_tex");
				ReadAllBytes(filePath, &fastBuffer);
			}

			{
				// 0.38 MS
				DEBUG_STOPWATCH("TxpSet::Parse stgpv623_tex");
				fastTxpSet.Parse(fastBuffer.data());
			}
		}

		if (true)
		{
			for (size_t i = 0; i < slowTxpSet.Textures.size(); i++)
			{
				for (size_t m = 0; m < slowTxpSet.Textures[i]->MipMaps.size(); m++)
				{
					MipMap* slowMipMap = slowTxpSet.Textures[i]->MipMaps[m].get();
					MipMap* fastMipMap = fastTxpSet.Textures[i]->MipMaps[m].get();

					bool same = memcmp(slowMipMap->Data.data(), fastMipMap->DataPointer, fastMipMap->DataPointerSize) == 0;
					printf("texture[%d] mipmap[%d] = %s\n", i, m, same ? "same" : "NOT THE SAME");
				}
			}
		}
	}

	// SPR TEST:
	if (false)
	{
		auto filePath = "Y:/Dev/Comfy/Sandbox/dev_ram/sprset/spr_ps4_custom.bin";

		SprSet slowSprSet, fastSprSet;
		std::vector<uint8_t> fastBuffer;

		if (true)
		{
			// 356.71 MS
			// 115.25 MS
			DEBUG_STOPWATCH("--- Slow Parse spr_ps4_custom ---");
			slowSprSet.Load(filePath);
		}

		if (true)
		{
			// 63.44 MS
			DEBUG_STOPWATCH("--- Quick Parse spr_ps4_custom ---");

			{
				// 63.15 MS
				DEBUG_STOPWATCH("ReadAllBytes spr_ps4_custom");
				ReadAllBytes(filePath, &fastBuffer);
			}

			{
				// 0.07 MS
				DEBUG_STOPWATCH("SprSet::Parse spr_ps4_custom");
				fastSprSet.Parse(fastBuffer.data());
			}
		}
	}

	// SPR/TXP TEST:
	if (false)
	{
		{
			const wchar_t* txpSetPath = L"Y:/Debug/FileTest/objset/stgtst007/stgtst007_tex.bin";
			MemoryStream stream(txpSetPath); BinaryReader reader(&stream);

			std::unique_ptr<TxpSet> txpSet = std::make_unique<TxpSet>();
			{
				DEBUG_STOPWATCH("Parse stgtst007_tex");
				txpSet->Read(reader);
			}

			int __breakpoint = 0;
		}
		{
			const wchar_t* sprSetPath = L"Y:/Debug/FileTest/sprset/spr_gam_cmn.bin";
			MemoryStream stream(sprSetPath); BinaryReader reader(&stream);

			std::unique_ptr<SprSet> sprSet = std::make_unique<SprSet>();
			{
				DEBUG_STOPWATCH("Parse spr_gam_cmn");
				sprSet->Read(reader);
			}
			{
				DEBUG_STOPWATCH("Export PNG File Test");
				for (auto &texture : sprSet->TxpSet->Textures)
					SaveAsPng(texture.get());
			}
			{
				DEBUG_STOPWATCH("Export DDS File Test");
				for (auto &texture : sprSet->TxpSet->Textures)
					SaveAsDDS(texture.get());
			}

			int __breakpoint = 0;
		}
	}

	// AET TEST:
	if (false)
	{
		std::vector<std::wstring> aetPaths;
		std::vector<std::shared_ptr<MemoryStream>> streams;
		std::vector<std::shared_ptr<BinaryReader>> readers;
		std::vector<std::shared_ptr<AetSet>> aetSets;

		auto directory = directory_iterator(L"Y:/Debug/FileTest/AetSet");

		{
			DEBUG_STOPWATCH("Store all directory files in vector");
			for (auto &entry : directory)
				aetPaths.emplace_back(entry.path().wstring());
		}

		{
			DEBUG_STOPWATCH("Reserve vector memory");
			streams.reserve(aetPaths.size());
			readers.reserve(aetPaths.size());
			aetSets.reserve(aetPaths.size());
		}

		{
			DEBUG_STOPWATCH("Read AetSets into MemoryStreams");
			for (size_t i = 0; i < aetPaths.size(); i++)
			{
				streams.push_back(std::make_shared<MemoryStream>(aetPaths[i].c_str()));
				readers.push_back(std::make_shared<BinaryReader>(streams[i].get()));
			}
		}

		{
			DEBUG_STOPWATCH("Allocate AetSet instances");
			for (size_t i = 0; i < aetPaths.size(); i++)
				aetSets.push_back(std::make_shared<AetSet>());
		}

		{
			DEBUG_STOPWATCH("Parse AetSets");
			for (size_t i = 0; i < readers.size(); i++)
				aetSets[i]->Read(*readers[i].get());
		}
	}
}