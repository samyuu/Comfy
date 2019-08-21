#pragma once
#include "AetCommand.h"
#include "FileSystem/Format/AetSet.h"

namespace Editor::Command
{
	using namespace FileSystem;

	namespace AetSet
	{

	}

	namespace Aet
	{
		class ChangeName : public AetCommand
		{
			RefPtr<FileSystem::Aet> aet;
			std::string newName, oldName;

		public:
			ChangeName(const RefPtr<FileSystem::Aet>& aet, const std::string& name) : aet(aet), newName(name) {};

			void Do() override { oldName = aet->Name; aet->Name = newName; };
			void Undo() override { aet->Name = oldName; };
			void Redo() override { aet->Name = newName; };
			const char* GetName() override { return "Change Aet Name"; };
		};

		class ChangeStartFrame : public AetCommand
		{
			RefPtr<FileSystem::Aet> aet;
			float newFrame, oldFrame;

		public:
			ChangeStartFrame(const RefPtr<FileSystem::Aet>& aet, float startFrame) : aet(aet), newFrame(startFrame) {};

			void Do() override { oldFrame = aet->FrameStart; aet->FrameStart = newFrame; };
			void Undo() override { aet->FrameStart = oldFrame; };
			void Redo() override { aet->FrameStart = newFrame; };
			const char* GetName() override { return "Change Aet Start Frame"; };
		};

		class ChangeFrameDuration : public AetCommand
		{
			RefPtr<FileSystem::Aet> aet;
			float newDuration, oldDuration;

		public:
			ChangeFrameDuration(const RefPtr<FileSystem::Aet>& aet, float frameDuration) : aet(aet), newDuration(frameDuration) {};

			void Do() override { oldDuration = aet->FrameDuration; aet->FrameDuration = newDuration; };
			void Undo() override { aet->FrameDuration = oldDuration; };
			void Redo() override { aet->FrameDuration = newDuration; };
			const char* GetName() override { return "Change Aet Frame Duration"; };
		};

		class ChangeFrameRate : public AetCommand
		{
			RefPtr<FileSystem::Aet> aet;
			float newFrameRate, oldFrameRate;

		public:
			ChangeFrameRate(const RefPtr<FileSystem::Aet>& aet, float frameRate) : aet(aet), newFrameRate(frameRate) {};

			void Do() override { oldFrameRate = aet->FrameRate; aet->FrameRate = newFrameRate; };
			void Undo() override { aet->FrameRate = oldFrameRate; };
			void Redo() override { aet->FrameRate = newFrameRate; };
			const char* GetName() override { return "Change Aet Frame Rate"; };
		};

		class ChangeResolution : public AetCommand
		{
			RefPtr<FileSystem::Aet> aet;
			int32_t newWidth, newHeight, oldWidth, oldHeight;

		public:
			ChangeResolution(const RefPtr<FileSystem::Aet>& aet, int width, int height) : aet(aet), newWidth(width), newHeight(height) {};

			void Do() override { oldWidth = aet->Width; oldHeight = aet->Height; aet->Width = newWidth; aet->Height = newHeight; };
			void Undo() override { aet->Width = oldWidth; aet->Height = oldHeight; };
			void Redo() override { aet->Width = newWidth; aet->Height = newHeight; };
			const char* GetName() override { return "Change Resolution"; };
		};

		class ChangeBackgroundColor : public AetCommand
		{
			RefPtr<FileSystem::Aet> aet;
			uint32_t newColor, oldColor;

		public:
			ChangeBackgroundColor(const RefPtr<FileSystem::Aet>& aet, uint32_t color) : aet(aet), newColor(color) {};

			void Do() override { oldColor = aet->BackgroundColor; aet->BackgroundColor = newColor; };
			void Undo() override { aet->BackgroundColor = oldColor; };
			void Redo() override { aet->BackgroundColor = newColor; };
			const char* GetName() override { return "Change Aet Background Color"; };
		};
	}

	namespace AetLayer
	{

	}

	namespace AetObj
	{
		class ChangeName : public AetCommand
		{
			RefPtr<FileSystem::AetObj> aetObj;
			std::string newName, oldName;

		public:
			ChangeName(const RefPtr<FileSystem::AetObj>& aetObj, const std::string& name) : aetObj(aetObj), newName(name) {};

			void Do() override { oldName = aetObj->GetName(); aetObj->SetName(newName); };
			void Undo() override { aetObj->SetName(oldName); };
			void Redo() override { aetObj->SetName(newName); };
			const char* GetName() override { return "Change Object Name"; };
		};

		class ChangeLoopStart : public AetCommand
		{
			RefPtr<FileSystem::AetObj> aetObj;
			float newStart, oldStart;

		public:
			ChangeLoopStart(const RefPtr<FileSystem::AetObj>& aetObj, float loopStart) : aetObj(aetObj), newStart(loopStart) {};

			void Do() override { oldStart = aetObj->LoopStart; aetObj->LoopStart = newStart; };
			void Undo() override { aetObj->LoopStart = oldStart; };
			void Redo() override { aetObj->LoopStart = newStart; };
			const char* GetName() override { return "Change Object Loop Start"; };
		};

		class ChangeLoopEnd : public AetCommand
		{
			RefPtr<FileSystem::AetObj> aetObj;
			float newEnd, oldEnd;

		public:
			ChangeLoopEnd(const RefPtr<FileSystem::AetObj>& aetObj, float loopEnd) : aetObj(aetObj), newEnd(loopEnd) {};

			void Do() override { oldEnd = aetObj->LoopEnd; aetObj->LoopEnd = newEnd; };
			void Undo() override { aetObj->LoopEnd = oldEnd; };
			void Redo() override { aetObj->LoopEnd = newEnd; };
			const char* GetName() override { return "Change Object Loop End"; };
		};

		class ChangeStartFrame : public AetCommand
		{
			RefPtr<FileSystem::AetObj> aetObj;
			float newStart, oldStart;

		public:
			ChangeStartFrame(const RefPtr<FileSystem::AetObj>& aetObj, float startFrame) : aetObj(aetObj), newStart(startFrame) {};

			void Do() override { oldStart = aetObj->StartFrame; aetObj->StartFrame = newStart; };
			void Undo() override { aetObj->StartFrame = oldStart; };
			void Redo() override { aetObj->StartFrame = newStart; };
			const char* GetName() override { return "Change Object Start Frame"; };
		};

		class ChangePlaybackSpeed : public AetCommand
		{
			RefPtr<FileSystem::AetObj> aetObj;
			float newSpeed, oldSpeed;

		public:
			ChangePlaybackSpeed(const RefPtr<FileSystem::AetObj>& aetObj, float speed) : aetObj(aetObj), newSpeed(speed) {};

			void Do() override { oldSpeed = aetObj->PlaybackSpeed; aetObj->PlaybackSpeed = newSpeed; };
			void Undo() override { aetObj->PlaybackSpeed = oldSpeed; };
			void Redo() override { aetObj->PlaybackSpeed = newSpeed; };
			const char* GetName() override { return "Change Object Playback Speed"; };
		};
	}
}