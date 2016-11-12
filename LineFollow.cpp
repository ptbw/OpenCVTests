#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <math.h>
#include <termios.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/legacy/legacy.hpp>
#include "opencv2/video/tracking.hpp"

using namespace cv;
using namespace std;

#define PI 3.14159265

string intToString(int number) {

  std::stringstream ss;
  ss << number;
  return ss.str();
}

class Symbol {

public:
  Mat img;
  string name;

};

int lowThreshold = 90;
unsigned int highSpeed = 250;
unsigned int lowSpeed = 200;
int imageWidth = 320;
int minWidth = 30;

// Get a key press from a raw terminal stream
char getKey() 
{
	fd_set set;
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = 10;

	FD_ZERO( &set );
	FD_SET( fileno( stdin ), &set );

	int res = select( fileno( stdin )+1, &set, NULL, NULL, &tv );

	if( res > 0 )
	{
		char c;
		printf( "Input seen\n" );
		c = getchar();			// PC
		// read( fileno( stdin ), &c, 1 );  		//Pi
		return c;
	}
	else if( res < 0 )
	{
		fprintf(stderr,"Select error\n" );
		return 0;
	}
	return 0;
}


void CannyThreshold(int, void*) {
}

unsigned int CalcSpeed( double diff ) {
	double lowLimit = lowSpeed;
	double highLimit = highSpeed - lowSpeed;
	
	diff = abs(diff);
	
	// ( diff / width == speed / highLimit ) + lowLimit    
    int speed = int( (highLimit * ( diff / imageWidth )) + lowLimit );
    printf("Speed %d \n",speed);
    return speed;    
}


//TBDL - Take notice of return values
unsigned int TurnRight(double diff) {
    unsigned int ret = 0;
     
    printf("Right: ");
    // int speed = 
		CalcSpeed( diff ); 
    return ret;  
}

unsigned int TurnLeft(double diff) {
    unsigned int ret = 0;
    
    printf("Left: ");
    // int speed = 
		CalcSpeed( diff );     
    return ret; 
}

unsigned int Straight(){
    unsigned int ret = 0;
    printf("Straight: ");
    return ret; 
}

unsigned int Stop() {
    unsigned int ret = 0;
    printf("Stop: ");
    return ret; 
}


double GetPosition(CvCapture  * capture, bool viewing)
{
    Mat outImg;
    Mat roiImg;
    Mat roiImgColor;
  
    IplImage* image = cvQueryFrame(capture);
    Mat camera(image); 
        
    //Extract a region of interest from the grey scale frame
    Rect roi(0,190,640,100);
    
    camera(roi).copyTo(roiImgColor);
    cvtColor(roiImgColor, roiImg, CV_RGB2GRAY );
    
    threshold(roiImg, roiImg, lowThreshold , 255, 0);
    
    // negative image
    bitwise_not(roiImg, roiImg);    
    Mat erodeElmt = getStructuringElement(MORPH_RECT, Size(3, 3));
    Mat dilateElmt = getStructuringElement(MORPH_RECT, Size(5, 5));
    erode(roiImg, roiImg, erodeElmt);
    dilate(roiImg, roiImg, dilateElmt);
   
    roiImg.copyTo(outImg);
        
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(roiImg, contours, hierarchy, CV_RETR_TREE,CV_CHAIN_APPROX_SIMPLE, Point(0,0));
  
    for (size_t i = 0; i < contours.size(); i++) {
       float area = contourArea(contours[i]);       
       if( area > 60000 ) {
           // Stop motors and wait for positive hit
           Stop();
           return 9999.0;
       }
       else if (area > 2000) {
          Moments mu;
          mu = moments(contours[i], false);
          
          Point2f center(mu.m10 / mu.m00, 50);
          circle(outImg, center, 5, Scalar(128, 128, 128), -1, 8, 0);
          
          // Show the viewers at home what we can see
		  if( viewing )
			cv::imshow("C", outImg);  
		  
          double position = mu.m10/mu.m00;
          double diff = imageWidth - position;
          
          printf("Centre = %f Diff = %f Area %f\n", position, diff, area);
          
          return diff;
      }
    } 
    return 0;
}



int main(int argc, char** argv) {  
  bool running = true;  
  
  //VideoCapture  * capture = VideoCapture(0); 
  
  CvCapture * capture = 0; 
  capture = cvCaptureFromCAM(0);
  if(!capture)  // check if we succeeded
    return -1;
 
  // To allow us to simply press a key without Enter we set our terminal to RAW
  // and we save the original settings to restore them on exit
  struct termios oldSettings;
  struct termios  newSettings;
  
  tcgetattr( fileno( stdin ), &oldSettings );
  newSettings = oldSettings;
  newSettings.c_lflag &= (~ICANON & ~ECHO);
  tcsetattr( fileno( stdin ), TCSANOW, &newSettings );    

  
  cvNamedWindow("C", CV_WINDOW_AUTOSIZE);
  
  double t = (double) getTickCount();
  
  bool started = false;
  bool viewing = true;
  
  while (running) {

    t = (double) getTickCount() - t;
    
    double diff = GetPosition(capture, viewing);
        
    if( started ) {
		if(diff == 9999.0)
		  Stop();

		else if(diff > minWidth)
		  TurnLeft(diff);

		else if(diff < -minWidth)
		  TurnRight(diff);

		else
		  Straight();	
    }
        
    if(waitKey(1) == 'q' || getKey() == 'x')
    {   
      Stop();    
      running = false;
    }

  }
  // Put back the terminal settings as they were
  tcsetattr( fileno( stdin ), TCSANOW, &oldSettings );
  return 0;
}

