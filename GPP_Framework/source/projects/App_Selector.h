#pragma once
// Here we determine which application is currently active
// Create the define here using the "ActiveApp_..." convention and set up the correct include and typedef in the #ifdef below.

//#define ActiveApp_Sandbox
#define ActiveApp_Steering

//---------- Registered Applications -----------
#ifdef ActiveApp_Sandbox
#include "projects/Movement/Sandbox/App_Sandbox.h"
using CurrentApp = App_Sandbox;
#endif

#ifdef ActiveApp_Steering
#include "projects/Movement/SteeringBehaviors/Steering/App_SteeringBehaviors.h"
using CurrentApp = App_SteeringBehaviors;
#endif // ActiveApp_Steering

class App_Selector {
public:
	static IApp* CreateApp() {
		IApp* myApp = new CurrentApp();
		return myApp;
	}
};