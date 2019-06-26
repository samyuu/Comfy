#pragma once

class IInitializable
{
public:
	virtual void InitializeID() = 0;
};

class IBindable
{
public:
	virtual void Bind() = 0;
	virtual void UnBind() = 0;
};

class IGraphicsObject : public IInitializable, IBindable
{
public:
};
