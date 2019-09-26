#pragma once

namespace Graphics
{
	class IInitializable
	{
	public:
		virtual void InitializeID() = 0;
	};

	class IBindable
	{
	public:
		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;
	};

	class ILabeledObject
	{
	public:
		virtual void SetObjectLabel(const char* label) = 0;
	};

	class IGraphicsObject : public IInitializable, IBindable, ILabeledObject
	{
	public:
	};
}
