#ifndef ACTOR_EXTENDER_INCLUDED
#define ACTOR_EXTENDER_INCLUDED

#include "SM64DS_2.h"

// Replace this struct and its constructor with your own stuff
struct ActorExtension
{
	Matrix4x3 stuff1 = {};
	Vector3   stuff2;
	unsigned  stuff3 = 3;
	Matrix4x3 stuff4;
	// Model  stuff5; // Error: all members need to be trivially destructible

	inline ActorExtension() = default;
};

ActorExtension* GetActorExtension(const Actor& actor);

#endif