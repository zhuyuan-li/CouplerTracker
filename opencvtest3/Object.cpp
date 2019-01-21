#include "Object.h"



Object::Object()
{
	//set values for default constructor
	setType("Undefined Object");
	setColour(Scalar(0, 0, 0));

}

Object::Object(string name) {

	setType(name);

	if (name == "apple") {

		//TODO: use "calibration mode" to find HSV min
		//and HSV max values

		setHSVmin(Scalar(0, 0, 0));
		setHSVmax(Scalar(255, 255, 255));

		//BGR value for Green:
		setColour(Scalar(171, 250, 125));

	}
	if (name == "Corner") {

		//TODO: use "calibration mode" to find HSV min
		//and HSV max values

		setHSVmin(Scalar(0, 240, 200));
		setHSVmax(Scalar(255, 255, 255));

		//BGR value for Yellow:
		setColour(Scalar(225, 250, 0));

	}
	if (name == "Coupler") {

		//TODO: use "calibration mode" to find HSV min
		//and HSV max values

		setHSVmin(Scalar(140, 0, 200));
		setHSVmax(Scalar(255, 255, 255));

		//BGR value for Red:
		setColour(Scalar(255, 0, 255));

	}



}

Object::~Object(void)
{
}

int Object::getXPos() {

	return Object::xPos;

}

void Object::setXPos(int x) {

	Object::xPos = x;

}

int Object::getYPos() {

	return Object::yPos;

}

void Object::setYPos(int y) {

	Object::yPos = y;

}

Scalar Object::getHSVmin() {

	return Object::HSVmin;

}
Scalar Object::getHSVmax() {

	return Object::HSVmax;
}

void Object::setHSVmin(Scalar min) {

	Object::HSVmin = min;
}


void Object::setHSVmax(Scalar max) {

	Object::HSVmax = max;
}