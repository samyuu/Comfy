#include "TestTask.h"

namespace Comfy::Sandbox::Tests
{
	class Renderer3DTest : public ITestTask
	{
	public:
		COMFY_REGISTER_TEST_TASK(Renderer3DTest);

		Renderer3DTest()
		{
		}

		void Update() override
		{
		}

	private:
	};
}
