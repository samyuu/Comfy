#include "TestComponent.h"
#include "../Application.h"

TestComponent::TestComponent(Application* parent) : BaseWindow(parent)
{
}

TestComponent::~TestComponent()
{
}

const char* TestComponent::GetGuiName() const
{
	return u8"Test Component";
}

ImGuiWindowFlags TestComponent::GetWindowFlags() const
{
	return ImGuiWindowFlags_NoBackground;
}