#pragma once

struct camera
{
	v3 Position;
	v3 Rotation;
	m4x4 Projection;
	m4x4 View;
	float Speed;
	bool NeedsUpdate;
};