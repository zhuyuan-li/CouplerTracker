//The majority of this program was written by Kyle Hounslow in 2013.
//Further work with respect to object tracking was done by Julian Li in 2018.
//This program is a proof of concept only.

//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software")
//, to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
//and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//IN THE SOFTWARE.

#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "Object.h"


//a simple coordinate class to track items on the screen
class Coord {
public:
	int x = 0;
	int y = 0;
	Coord() {}
	Coord(int x, int y) : x{x}, y{y} {}
	Coord(const Coord &other) {
		x = other.x;
		y = other.y;
	}
	Coord &operator=(const Coord &other) {
		x = other.x;
		y = other.y;
		return *this;
	}
};

class TwoCoord {
public:
	Coord a;
	Coord b;
	TwoCoord(int w, int x, int y, int z) {
		a = Coord(0, 0);
		b = Coord(0, 0);
	}
	bool verticalCheck(int xValue) {
		if (abs(xValue - a.x) < 15 && abs(xValue - b.x) < 15) {
			return true;
		}
		else {
			return false;
		}
	}

	bool avgInRange(Coord ref) {
		int xAvg = (a.x + b.x) / 2;
		int yAvg = (a.y + b.y) / 2;
		if (abs(ref.x - xAvg) < 15 && abs(ref.y - yAvg) < 15) {
			return true;
		}
		else {
			return false;
		}
	}
};

//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 8;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT * FRAME_WIDTH / 1.5;
//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";
void on_trackbar(int, void*)
{//This function gets called whenever a
 // trackbar position is changed





}
string intToString(int number) {


	std::stringstream ss;
	ss << number;
	return ss.str();
}
void createTrackbars() {
	//create window for trackbars


	namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf(TrackbarName, "H_MIN", H_MIN);
	sprintf(TrackbarName, "H_MAX", H_MAX);
	sprintf(TrackbarName, "S_MIN", S_MIN);
	sprintf(TrackbarName, "S_MAX", S_MAX);
	sprintf(TrackbarName, "V_MIN", V_MIN);
	sprintf(TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);


}
void drawObject(vector<Object> theObjects, Mat &frame) {

	for (int i = 0; i<theObjects.size(); i++) {

		cv::circle(frame, cv::Point(theObjects.at(i).getXPos(), theObjects.at(i).getYPos()), 10, cv::Scalar(0, 0, 255));
		cv::putText(frame, intToString(theObjects.at(i).getXPos()) + " , " + intToString(theObjects.at(i).getYPos()), cv::Point(theObjects.at(i).getXPos(), theObjects.at(i).getYPos() + 20), 1, 1, Scalar(0, 255, 0));
		cv::putText(frame, theObjects.at(i).getType(), cv::Point(theObjects.at(i).getXPos(), theObjects.at(i).getYPos() - 30), 1, 2, theObjects.at(i).getColour());
	}
}
void morphOps(Mat &thresh) {

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);



}
void trackFilteredObject(Mat threshold, Mat HSV, Mat &cameraFeed) {

	vector <Object> apples;

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects<MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area>MIN_OBJECT_AREA) {

					Object apple;

					apple.setXPos(moment.m10 / area);
					apple.setYPos(moment.m01 / area);


					apples.push_back(apple);

					objectFound = true;

				}
				else objectFound = false;


			}
			//let user know you found an object
			if (objectFound == true) {
				//draw object location on screen
				drawObject(apples, cameraFeed);
			}

		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}
Coord trackFilteredObject(Object theObject, Mat threshold, Mat HSV, Mat &cameraFeed) {


	vector <Object> objs;
	Coord centroid(0, 0);

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects<MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area>MIN_OBJECT_AREA) {

					Object apple;

					apple.setXPos(moment.m10 / area);
					apple.setYPos(moment.m01 / area);
					apple.setType(theObject.getType());
					apple.setColour(theObject.getColour());

					objs.push_back(apple);

					objectFound = true;

				}
				else objectFound = false;


			}
			//let user know you found an object
			if (objectFound == true && theObject.getType() == "Corner") {
				int newX = 0;
				int newY = 0;

				for (int i = 0; i < objs.size(); i++) {
					newX += objs[i].getXPos();
					newY += objs[i].getYPos();
				}

				newX = newX / objs.size();
				newY = newY / objs.size();

				cv::circle(cameraFeed, cv::Point(newX, newY), 10, cv::Scalar(0, 200, 0));

				centroid.x = newX;
				centroid.y = newY;

				//draw object location on screen
				drawObject(objs, cameraFeed);
			}
			else {
				//draw object location on screen
				drawObject(objs, cameraFeed);
			}

		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
	return centroid;
}
TwoCoord trackFilteredObject(Object theObject, Mat threshold, Mat HSV, Mat &cameraFeed, Coord centroid) {
	vector <Object> objs;
	TwoCoord CCunit(0, 0, 0, 0);

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects<MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area>MIN_OBJECT_AREA) {

					Object apple;

					apple.setXPos(moment.m10 / area);
					apple.setYPos(moment.m01 / area);
					apple.setType(theObject.getType());
					apple.setColour(theObject.getColour());

					objs.push_back(apple);

					objectFound = true;

				}
				else objectFound = false;


			}
			//let user know you found an object
			if (objectFound == true && theObject.getType() == "Coupler") {

				if (objs.size() == 2) {
					CCunit.a.x = objs[0].getXPos();
					CCunit.a.y = objs[0].getYPos();

					CCunit.b.x = objs[1].getXPos();
					CCunit.b.y = objs[1].getYPos();
				}

				int newX = (CCunit.a.x + CCunit.b.x) / 2;
				int newY = (CCunit.a.y + CCunit.b.y) / 2;
				cv::circle(cameraFeed, cv::Point(newX, newY), 10, cv::Scalar(0, 0, 0));

				if (CCunit.verticalCheck(centroid.x) && CCunit.avgInRange(centroid)) {
					putText(cameraFeed, "MATCH", Point(0, 50), 1, 2, Scalar(0, 250, 0), 2);
					cv::line(cameraFeed, cv::Point(CCunit.a.x, CCunit.a.y), cv::Point(CCunit.b.x, CCunit.b.y), Scalar(0, 250, 0), 3);
				}
				else {
					cv::line(cameraFeed, cv::Point(CCunit.a.x, CCunit.a.y), cv::Point(CCunit.b.x, CCunit.b.y), Scalar(0, 0, 250), 3);
				}

				//draw object location on screen
				drawObject(objs, cameraFeed);
			}
			else {
				//draw object location on screen
				drawObject(objs, cameraFeed);
			}

		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
	return CCunit;
}
int main(int argc, char* argv[])
{
	//if we would like to calibrate our filter values, set to true.
	bool calibrationMode = false;

	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	Mat threshold;
	Mat HSV;

	if (calibrationMode) {
		//create slider bars for HSV filtering
		createTrackbars();
	}
	//video capture object to acquire webcam feed
	VideoCapture capture;
	//open capture object at location zero (default location for webcam)
	capture.open(1);
	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	while (1) {
		//store image to matrix
		capture.read(cameraFeed);
		//convert frame from BGR to HSV colorspace
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);

		if (calibrationMode == true) {
			//if in calibration mode, we track objects based on the HSV slider values.
			cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
			inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
			morphOps(threshold);
			imshow(windowName2, threshold);
			trackFilteredObject(threshold, HSV, cameraFeed);
		}
		else {
			//create some temp objects so that
			//we can use their member functions/information
			Object corner("Corner"), coupler("Coupler");

			cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
			inRange(HSV, corner.getHSVmin(), corner.getHSVmax(), threshold);
			morphOps(threshold);
			Coord a = trackFilteredObject(corner, threshold, HSV, cameraFeed);
			//
			cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
			inRange(HSV, coupler.getHSVmin(), coupler.getHSVmax(), threshold);
			morphOps(threshold);
			TwoCoord b = trackFilteredObject(coupler, threshold, HSV, cameraFeed, a);
		}

		//show frames 
		//imshow(windowName2,threshold);

		imshow(windowName, cameraFeed);
		//imshow(windowName1,HSV);


		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		waitKey(30);
	}






	return 0;
}