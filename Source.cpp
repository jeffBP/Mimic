#define _SCL_SECURE_NO_WARNINGS  

#include <Windows.h>
#include <Ole2.h>

#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glut.h>

#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>

#include "RunSerial.cpp"
#include "SerialPort.h"
#include <iostream>
#include <fstream>
using namespace std;

#include <iostream>
#include <vector>
#include <string>

#define width 640
#define height 480
#define PI 3.14159265
#define GL_BGRA 0x80E1
// OpenGL Variables
GLuint textureId;              // ID of the texture to contain Kinect RGB Data
GLubyte textData[width*height * 4];  // BGRA array containing the texture data

HANDLE rgbStream;
HANDLE depthStream;
INuiSensor* sensor;
Vector4 skeletonPosition[NUI_SKELETON_POSITION_COUNT];

int vIndex = 0;
vector<double> kinectShoulder = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
vector<double> kinectElbow = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


double shoulderAngle = 0;
double elbowAngle = 0;



int lastAvg = 0;

char *port_name = "\\\\.\\COM4";
SerialPort arduino(port_name);

bool initKinect() {
	
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
void drawKinectData() {
	glBindTexture(GL_TEXTURE_2D, textureId);
	getSkeletalData();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)textData);
	const Vector4& lh = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT];
	const Vector4& le = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_LEFT];
	const Vector4& ls = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_LEFT];
	const Vector4& rh = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT];
	const Vector4& re = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_RIGHT];
	const Vector4& rs = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_RIGHT];
	glBegin(GL_LINES);
	glColor3f(1.f, 0.f, 0.f);
	if (lh.w > 0 && le.w > 0 && ls.w > 0) {
		// lower left arm
		glVertex3f(lh.x, lh.y, lh.z);
		glVertex3f(le.x, le.y, le.z);
		// upper left arm
		glVertex3f(le.x, le.y, le.z);
		glVertex3f(ls.x, ls.y, ls.z);
	}
	if (rh.w > 0 && re.w > 0 && rs.w > 0) {
		// lower right arm
		glVertex3f(rh.x, rh.y, rh.z);
		glVertex3f(re.x, re.y, re.z);
		// upper right arm
		glVertex3f(re.x, re.y, re.z);
		glVertex3f(rs.x, rs.y, rs.z);
	}
	glEnd();
}

void sendSkeletalData(int rShoulAng, int rElbAng) {

	if (rShoulAng > 160) {
		rShoulAng = 160;
	}
	else if (rShoulAng < 30) {
		rShoulAng = 30;
	}
	

	

}

bool withinAngThreshold() {
	return true;
}

void processSkeletalData() {
	Vector4 rHand = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT];
	Vector4 rElb = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_RIGHT];
	Vector4 rShoul = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_RIGHT];

	shoulderAngle = (atan2(-(rElb.y - rShoul.y),  (rElb.x - rShoul.x)) * 180 / PI) + 90;
	elbowAngle = (atan2(rHand.y - rElb.y,rHand.x - rElb.x) *180 / PI);
	elbowAngle = elbowAngle + shoulderAngle - 90;

	cout << shoulderAngle << ", " << elbowAngle << "\n";
	/*
	kinectShoulder[vIndex] = shoulderAngle;
	kinectElbow[vIndex] = elbowAngle;
	if (shoulderAngle != 0) {
		if (kinectShoulder[9] != 0) {
			double sumShoulder = 0;
			double sumElbow = 0;
			for (int i = 0; i < kinectShoulder.size(); i++) {
				sumShoulder += kinectShoulder[i];
				sumElbow += kinectElbow[i];
			}
			int avgShoulderAngle = sumShoulder / kinectShoulder.size();
			int avgElbowAngle = sumElbow / kinectElbow.size();
			
			
				sendSkeletalData(avgShoulderAngle, avgElbowAngle);
				lastAvg = avgShoulderAngle;
			reinitVectors();

		}
		else {
			vIndex++;
		}
	}
	*/
	
}
void sendDataToArduino(double ang1, double ang2) {
	char incomingData[MAX_DATA_LENGTH] = "";
	char output[MAX_DATA_LENGTH] = "";

	if (ang1 != 90 && ang2 != 0) {
		std::string input_string;
		//Getting input
		input_string = "" + to_string(int(ang1)) + ", " + to_string(int(ang2));
		//Creating a c string
		char *c_string = new char[10];
		//copying the std::string to c string
		cout << input_string << "\n";
		std::copy(input_string.begin(), input_string.end(), c_string);
		//Adding the delimiter
		cout << input_string.size() + 1 << "\n";
		//c_string[input_string.size()] = '\n';
		/*
		//Writing string to arduino
		for (int i = 0; i < input_string.size(); i++) {
		char* c = reinterpret_cast<char*>(input_string[i]);
		cout << c << "\n";
		arduino.writeSerialPort(c, 1);
		}
		*/
		arduino.writeSerialPort(c_string, input_string.size() + 1);
		//Getting reply from arduino
		Sleep(35);
		arduino.readSerialPort(output, MAX_DATA_LENGTH);
		cout << output << "\n";

		//freeing c_string memory
		delete[] c_string;

	}
}
void readDataToArduino(string angles) {
	char incomingData[MAX_DATA_LENGTH] = "";
	char output[MAX_DATA_LENGTH] = "";
	
		std::string input_string;
		//Getting input
		input_string = angles;
		//Creating a c string
		char *c_string = new char[10];
		//copying the std::string to c string
		cout << input_string << "\n";
		std::copy(input_string.begin(), input_string.end(), c_string);
		//Adding the delimiter
		cout << input_string.size() + 1 << "\n";
		//c_string[input_string.size()] = '\n';
		arduino.writeSerialPort(c_string, input_string.size() + 1);
		//Getting reply from arduino
		arduino.readSerialPort(output, MAX_DATA_LENGTH);
		cout << output << "\n";

		//freeing c_string memory
		delete[] c_string;

	
}
void saveData(string name, double ang1, double ang2) {
	ofstream file(name, std::ios::app);
	if (ang1 != 90 && ang2 != 0) {
		file << to_string(int(ang1)) << ", " << to_string(int(ang2))<< "\n";
	}
	file.close();

}

void main() {
	initKinect();	

		//This code snippet will help you to read data from arduino

		/*Portname must contain these backslashes, and remember to
		replace the following com port*/
		
	//String for incoming data
	
	
		if (arduino.isConnected()) cout << "Connection Established" << endl;
		else cout << "ERROR, check port name";
		/*
		while (arduino.isConnected()) {
			getSkeletalData();
			processSkeletalData();
			//saveData("wave.txt", shoulderAngle, elbowAngle);
			//sendDataToArduino(shoulderAngle, elbowAngle);
			
		
		}
		*/
		while (arduino.isConnected() != true) {}

		string str;
		ifstream angles("wave.txt");
		if (angles.is_open()) {
			while (getline(angles, str)) {
				readDataToArduino(str);
				Sleep(35);
			}
		}
		angles.close();
		system("pause");
		
}

