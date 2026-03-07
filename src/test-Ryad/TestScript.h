#pragma once
#include <Script.h>
#include <iostream> 

START_SCRIPT(TestScript)

int count = 0;
int collideCount = 0;
void OnStart()
{
	std::cout << "TestScript Start" << std::endl;
}

void OnUpdate()
{
	//count++;
	//std::cout << "TestScript Update : " << count << std::endl;
}

void OnDestroy()
{
	std::cout << "TestScript Destroy" << std::endl;
}

void OnCollisionEnter() {
	std::cout << "TestScript Started Colliding" << std::endl;
}

void OnCollision() {
	collideCount++;
	std::cout << "TestScript Collide : " << collideCount << std::endl;
}

void OnCollisionExit() {
	std::cout << "TestScript exited Colliding" << std::endl;
}
END_SCRIPT(TestScript)