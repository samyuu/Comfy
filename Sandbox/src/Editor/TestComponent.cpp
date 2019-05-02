#include "TestComponent.h"
#include "../Application.h"

TestComponent::TestComponent(Application* parent) : BaseWindow(parent)
{
}

TestComponent::~TestComponent()
{
}

const char* TestComponent::GetGuiName()
{
	return u8"Test Component";
}

ImGuiWindowFlags TestComponent::GetWindowFlags()
{
	return ImGuiWindowFlags_NoBackground;
}