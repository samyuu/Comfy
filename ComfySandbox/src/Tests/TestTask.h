#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"

#include "Window/RenderWindow.h"
#include "ImGui/Gui.h"
#include "Audio/Audio.h"
#include "Input/Input.h"
#include "Render/Render.h"
#include "Graphics/Auth2D/SprSet.h"

#include "IO/Path.h"
#include "IO/File.h"

#include "ImGui/Extensions/PropertyEditor.h"
#include "ImGui/Widgets/FileViewer.h"

#include <functional>

//#define COMFY_REGISTER_TEST_TASK(derivedClass) static inline struct StaticInit { StaticInit() { TestTaskInitializer::Register<derivedClass>(__FUNCTION__ "::" #derivedClass); } } InitInstance;
#define COMFY_REGISTER_TEST_TASK(derivedClass) static inline struct StaticInit { StaticInit() { TestTaskInitializer::Register<derivedClass>("Comfy::Sandbox::Tests::" #derivedClass); } } InitInstance;
// #define COMFY_REGISTER_TEST_TASK(derivedClass) COMFY_CALL_ON_STARTUP([] { TestTaskInitializer::Register<derivedClass>("Comfy::Sandbox::Tests::" #derivedClass); });

namespace Comfy::Sandbox::Tests
{
	class ITestTask
	{
	public:
		virtual ~ITestTask() = default;
		virtual void Update() = 0;
	};

	class TestTaskInitializer
	{
	public:
		const char* DerivedName;
		std::function<std::unique_ptr<ITestTask>()> Function;

		template <typename Func>
		static void IterateRegistered(Func func) { std::for_each(registered.begin(), registered.end(), func); }

		template <typename TestTask>
		static void Register(const char* derivedName) { registered.push_back({ derivedName, [] { return std::make_unique<TestTask>(); } }); }

	private:
		static inline std::vector<TestTaskInitializer> registered;
	};
}
