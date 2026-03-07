#pragma once

class Entity;

namespace Camera
{
	Entity* Camera(float viewX = 0.f, float viewY = 0.f, float viewW = 1.f, float viewH = 1.f,int RenderOrder = 0);
}