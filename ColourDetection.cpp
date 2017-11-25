////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "raspicam/raspicam_cv.h"

using namespace cv;
using namespace std;
using namespace raspicam;

//Red 	0,145,128	- 10, 255, 255
//Green 	51,127,62	- 75,255,255
//Blue  	75,127,127	- 107,255,255
//Yellow 	20,85,150	- 35,255,255

int lowH[4] = { 0, 51, 75, 20 };
int highH[4] = { 10, 75, 107, 35 };

int lowS[4] = { 158, 127, 127, 85 };
int highS[4] = { 255, 255, 255, 255 };

int lowV[4] = { 158, 127, 127, 150 };
int highV[4] = { 255, 255, 255, 255 };

enum Colours { Red,	Green, Blue, Yellow };

int main( int argc, char** argv )
 {
    //VideoCapture cap(0); //capture the video from webcam

    //if ( !cap.isOpened() )  // if not success, exit program
    //{
         //cout << "Cannot open the web cam" << endl;
         //return -1;
    //}

	RaspiCam_Cv Camera;
    
    Camera.set (CV_CAP_PROP_FORMAT, CV_8UC3 );  
    Camera.set (CV_CAP_PROP_FRAME_WIDTH, 320 );
    Camera.set (CV_CAP_PROP_FRAME_HEIGHT, 240 );
    Camera.set (CV_CAP_PROP_BRIGHTNESS, 50 );
	//Camera.set (CV_CAP_PROP_CONTRAST, 80 );	 
	//Camera.set (CV_CAP_PROP_GAIN, 90 );
	//Camera.set (CV_CAP_PROP_WHITE_BALANCE_RED_V, 1);
    //Camera.set (CV_CAP_PROP_WHITE_BALANCE_BLUE_U, 1);
    
    if ( !Camera.open() ) {
        fprintf(stderr, "Failed to init open camera\n");   
		exit(-1);
    }

    while (true)
    {
        Mat imgOriginal;

		bool bSuccess = Camera.grab();
		Camera.retrieve(imgOriginal);
		
        //bool bSuccess = cap.read(imgOriginal); // read a new frame from video

        if (!bSuccess) //if not success, break loop
        {
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }

		Mat imgHSV;
				
		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
		
		
		for(int colour = Red; colour <= Yellow; colour++ )
		{	 
			Mat imgThresholded;
			inRange(imgHSV, Scalar(lowH[colour], lowS[colour], lowV[colour]), Scalar(highH[colour], highS[colour], highV[colour]), imgThresholded); //Threshold the image
		  
			//morphological opening (removes small objects from the foreground)
			erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
			dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

			//morphological closing (removes small holes from the foreground)
			dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
			erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
			
			//imshow("Thresholded Image", imgThresholded); //show the thresholded image

			//Calculate the moments of the thresholded image
			Moments oMoments = moments(imgThresholded);

			//double dM01 = oMoments.m01;
			//double dM10 = oMoments.m10;
			double dArea = oMoments.m00;
			
			// if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero 
			if (dArea > 100000)
			{
				//printf("Area = %f\n",dArea);
				if(colour == Red)
				{
					rectangle(imgOriginal,Point(0,0),Point(30,30),Scalar(0,0,255),CV_FILLED);
					printf("Red\n");
				}
				else if(colour == Green)
				{
					rectangle(imgOriginal,Point(0,0),Point(30,30),Scalar(0,255,0),CV_FILLED);
					printf("Green\n");
				}
				else if(colour == Blue)
				{
					rectangle(imgOriginal,Point(0,0),Point(30,30),Scalar(255,0,0),CV_FILLED);
					printf("Blue\n");
				}			
				else if(colour == Yellow)
				{
					rectangle(imgOriginal,Point(0,0),Point(30,30),Scalar(0,255,255),CV_FILLED);
					printf("Yellow\n");
				}					
				else
					rectangle(imgOriginal,Point(0,0),Point(30,30),Scalar(0,0,0),CV_FILLED);
				
			}	
			waitKey(5);
		}

		//imgOriginal = imgOriginal + imgLines;	
		//rectangle(imgOriginal,Point(0,0),Point(30,30),Scalar(0,0,0),CV_FILLED);	
		imshow("Original", imgOriginal); //show the original image

        if (waitKey(50) == 27) //wait for 'esc' key press for 50ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed by user" << endl;
            break; 
        }
   }
   return 0;
}
