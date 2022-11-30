#ifndef __AVATAR__
#define __AVATAR__

#include "engine/utils/types_3d.h"
#include "engine/render/camera.h"
#include "engine/utils/timer.h"
#include "world.h"

class MAvatar
{
public:
	YVec3f Position;
	YVec3f Velocity;
	YVec3f Direction;

	float baseSpeed;
	float fallAcceleration;
	float jumpStrenght;

	bool Move;
	bool Jump;
	float Height;
	float CurrentHeight;
	float Width;
	bool avance;
	bool recule;
	bool gauche;
	bool droite;
	bool Standing;
	bool InWater;
	bool Crouch;
	bool Run;
	bool IsOnFloor;

	YCamera *Cam;
	MWorld *World;

	YTimer _TimerStanding;

	MAvatar(YCamera *cam, MWorld *world)
	{
		Position = YVec3f((MWorld::MAT_SIZE_METERS) / 2, (MWorld::MAT_SIZE_METERS) / 2, (MWorld::MAT_HEIGHT_METERS));
		Velocity = YVec3f();
		Height = 1.8f * MCube::CUBE_SIZE;
		CurrentHeight = Height;
		Width = 0.4f * MCube::CUBE_SIZE;
		Cam = cam;
		avance = false;
		recule = false;
		gauche = false;
		droite = false;
		Standing = false;
		Jump = false;
		World = world;
		InWater = false;
		Crouch = false;
		Run = false;
		IsOnFloor = false;

		baseSpeed = 10;
		fallAcceleration = 20;
		jumpStrenght = 20;
	}

	void update(float elapsed)
	{
		if (elapsed > 1.0f / 60.0f)
			elapsed = 1.0f / 60.0f;

		updatePlacement(elapsed);
	}

private:
	void updatePlacement(float elapsed)
	{
		YVec3f vecRigth = Direction.cross(YVec3f(0, 0, 1));
		if (Jump)
		{
			Velocity.Z = jumpStrenght;
		}
		if (!IsOnFloor)
		{
			if (Velocity.Z < 0) {
				Velocity.Z = max(Velocity.Z - fallAcceleration * elapsed, -fallAcceleration * 5);
			}
			else {
				Velocity.Z = max(Velocity.Z - fallAcceleration * 2 * elapsed, -fallAcceleration * 5);
			}
		}

		YVec3f horizontalVel = YVec3f();
		if (avance)
		{
			horizontalVel.X = Direction.X;
			horizontalVel.Y = Direction.Y;
		}
		else if (recule)
		{
			horizontalVel.X = -Direction.X;
			horizontalVel.Y = -Direction.Y;
		}

		if (droite)
		{
			horizontalVel += Cam->RightVec;
		}
		else if (gauche)
		{
			horizontalVel -= Cam->RightVec;
		}

		horizontalVel = horizontalVel.normalize() * baseSpeed;
		Velocity.X = horizontalVel.X;
		Velocity.Y = horizontalVel.Y;

		move(Velocity * elapsed);

		float colMin;
		MWorld::MAxis axis;
		IsOnFloor = false;
		for (size_t i = 0; i < 3; i++)
		{
			printf("pos : %.2f %.2f %.2f\n", Position.X, Position.Y, Position.Z);
			axis = World->getMinCol(Position, YVec3f(), Width, Height, colMin, true);
			if (axis != 0) {
				printf("axis : %s -> %f\n", axis == 1 ? "X" : axis == 2 ? "Y" : "Z", colMin);
			}
			switch (axis)
			{
			case MWorld::AXIS_X:
				Position.X += colMin;
				Velocity.X = 0;
				break;
	  		case MWorld::AXIS_Y:
				Position.Y += colMin;
				Velocity.Y = 0;
				break;
			case MWorld::AXIS_Z:
				IsOnFloor = true;
				Position.Z += colMin;
				Velocity.Z = 0;
				break;
			}
		}
		printf("Velocity : X=%f,Y=%f,Z=%f\n", Velocity.X, Velocity.Y, Velocity.Z);
	}

	void move(YVec3f delta)
	{
		Position += delta;
	}
};

#endif