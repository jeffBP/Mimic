#define _SCL_SECURE_NO_WARNINGS  

#include <Windows.h>
#include <Ole2.h>

//Flags for setting the record and playback features
bool playback = true;
bool record = false;

#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>

#include "RunSerial.cpp"
#include "SerialPort.h"

#include <fstream>
using namespace std;

#include <iostream>
#include <vector>
#include <string>

#define PI 3.14159265

// OpenGL Variables

HANDLE rgbStream;
HANDLE depthStream;
INuiSensor* sensor;
Vector4 skeletonPosition[NUI_SKELETON_POSITION_COUNT];

double shoulderAngle = 0;
double elbowAngle = 0;


//Find Arduino port, then change this variable to match
char *port_name = "\\\\.\\COM4";
SerialPort arduino(port_name);

bool initKinect() {
	/*
	Initializes Kinect Sensor with seated tracking
	*/
	
	int numSensors;

	if (NuiGetSensorCount(&numSensors) < 0 || numSensors < 1) return false;
	if (NuiCreateSensorByIndex(0, &sensor) < 0) return false;

	sensor->NuiInitialize(
		NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX
		| NUI_INITIALIZE_FLAG_USES_COLOR
		| NUI_INITIALIZE_FLAG_USES_SKELETON);

	sensor->NuiSkeletonTrackingEnable(
		NULL,
		NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT
	);
	return sensor;
}
void getSkeletalData() {
	/*
	Saves information of detected skeleton to global vector "skeletonPosition"
	If none detected, it does nothing
	*/
	NUI_SKELETON_FRAME skeletonFrame = { 0 };
	if (sensor->NuiSkeletonGetNextFrame(0, &skeletonFrame) >= 0) {
		sensor->NuiTransformSmooth(&skeletonFrame, NULL);
		//process frame data
	}
	for (int z = 0; z < NUI_SKELETON_COUNT; z++) {
		const NUI_SKELETON_DATA& skeleton = skeletonFrame.SkeletonData[z];

		if (skeleton.eTrackingState == NUI_SKELETON_TRACKED) {
			for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++) {
				skeletonPosition[i] = skeleton.SkeletonPositions[i];
				if (skeleton.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_NOT_TRACKED) {
					skeletonPosition[i].w = 0;
				}
			}
		}
	}
}

void processSkeletalData() {
	/*
	Gets skeleton data for right arm, transforms it to servo positions, and saves it to global variables. 
	*/

	Vector4 rHand = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT];
	Vector4 rElb = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_RIGHT];
	Vector4 rShoul = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_RIGHT];

	shoulderAngle = (atan2(-(rElb.y - rShoul.y),  (rElb.x - rShoul.x)) * 180 / PI) + 90;
	elbowAngle = (atan2(rHand.y - rElb.y,rHand.x - rElb.x) *180 / PI);
	elbowAngle = elbowAngle + shoulderAngle - 90;

	cout << shoulderAngle << ", " << elbowAngle << "\n";

}

void readDataToArduino(string angles) {
	/*
	Builds string as character vector to send to arduino. 
	Sends string to arduino
	Waits for a response from arduino

	*/

	char incomingData[MAX_DATA_LENGTH] = "";
	char output[MAX_DATA_LENGTH] = "";
	
		std::string input_string = angles;

		//Creating a c string
		char *c_string = new char[10];

		//copying the std::string to c string
		std::copy(input_string.begin(), input_string.end(), c_string);

		arduino.writeSerialPort(c_string, input_string.size() + 1);

		//Getting reply from arduino
		arduino.readSerialPort(output, MAX_DATA_LENGTH);
		cout << output << "\n";

		//freeing c_string memory
		delete[] c_string;

	
}
void saveData(string name, double ang1, double ang2) {
	/*
	Saves ang1 and ang2 to file named <name>
	*/

	ofstream file(name, std::ios::app);

	if (ang1 != 90 && ang2 != 0) {
		//Checks if data is defaulted
		file << to_string(int(ang1)) << ", " << to_string(int(ang2))<< "\n";
	}
	file.close();

}

void main() {

	//Initialize and set processes
	initKinect();
	playback = false;
	record = false;

	//Check if Connected
	if (arduino.isConnected()) cout << "Connection Established" << endl;
	else cout << "ERROR, check port name";

	//Run if not playing back from file
	if (playback == false){
		while (arduino.isConnected()) {
			//Process and send information
			getSkeletalData();
			processSkeletalData();
			//If we want to record information
			if (record) {
				saveData("wave2.txt", shoulderAngle, elbowAngle);
			}
			readDataToArduino("" + shoulderAngle + ", " + elbowAngle + "\n");

		}
	}
	//If playback from file
	else {
		//Wait for connection
		while (arduino.isConnected() != true) {}

		//Read angles from file
		string str;
		ifstream angles("wave2.txt");
		if (angles.is_open()) {
			while (getline(angles, str)) {
				readDataToArduino(str);
				Sleep(20);
			}
		}
		angles.close();
		system("pause");
		
	}
		
}

