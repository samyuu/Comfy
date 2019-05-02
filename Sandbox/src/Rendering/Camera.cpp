#include "Camera.h"

void Camera::Update()
{
	// Direction = glm::normalize(Position - Target);
	// Right = glm::cross(UP_DIRECTION, Direction);
	// Up = glm::cross(Direction, Right);

	//viewMatrix = glm::lookAt(Position, Target, UP_DIRECTION);

	//viewMatrix = glm::lookAt(Position, Position + Front, UP_DIRECTION);

	viewMatrix = glm::lookAt(Position, Target, UpDirection);
	
	//if (Rotation != 0.0f)
	//	viewMatrix = glm::rotate(viewMatrix, glm::radians(Rotation), (Position + Target));
	//viewMatrix = glm::rotate(viewMatrix, glm::radians(Rotation), (Position + Target));
	//viewMatrix = glm::rotate(viewMatrix, glm::radians(Rotation), vec3(0.0f, 0.0f, 1.0f));

	projectionMatrix = glm::perspective(glm::radians(FieldOfView), AspectRatio, NearPlane, FarPlane);
}