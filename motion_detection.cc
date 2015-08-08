/*
	Title 		:	Motion Detection by OpenCV
	Version 	:	1.0
	Date		:	2015-08-07
	Author		: 	Ruby
	Features	:
	-This program can detect motion by a normol camera.
	-Motion detection was only triggered one time in one second.
 */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <cv.h>
#include <highgui.h>
using namespace std;

int main(int argc, char **argv) {
	//you can set this value by manual
	//used to make sure of the accuracy of detection
	int detectThreshold = 19;
	if (argc >= 2) {
		detectThreshold = atoi(argv[1]);
		printf("=== Motion detection threshold has been set to: [%d] ===\n", detectThreshold);
	}

	//Create a single capture
	CvCapture *pCapture =  cvCreateCameraCapture(-1);
	//set properties
	cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH, 1920);
	cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT, 1080);
	cvSetCaptureProperty(pCapture, CV_CAP_PROP_BRIGHTNESS, 20);
	cvSetCaptureProperty(pCapture, CV_CAP_PROP_CONTRAST, 10);

	if (pCapture == NULL) {	//init capture failed
		fprintf(stderr, "Can't initialize webcam!\n");
		return 1;
	}

	//Preparing images
	IplImage *pImageA = cvQueryFrame(pCapture);
	IplImage *pImageB = cvCreateImage(CvSize(pImageA->width, pImageA->height), pImageA->depth, pImageA->nChannels);
	IplImage *pDiff = cvCloneImage(pImageB);
	IplImage *pGrayscaleImage = NULL;

	//Create histogram
	int nDims = 256;
	float hRangeArr[] = { 0,256 };
	float *hRanges = hRangeArr;
	CvHistogram* pHist = cvCreateHist(1, &nDims, CV_HIST_ARRAY, &hRanges, 1);

	//set a timestamp,preventing triggerin the monition detection for multi times in a second
	time_t TimeStamp = 0;
	float fMaxValue = 0.0;  //for hist calc

//---------------- Update --------------------
//
// Main Loop
//--------------------------------------------
	while (true)
	{	
		//Get current frame
		pImageA = cvQueryFrame(pCapture);
		if (!pImageA)
		{
			fprintf(stdout, "Can't grab images!\n");
			break;
		}
		//Find diff between two img
		cvAbsDiff(pImageA, pImageB, pDiff);
		//replacing old img
		cvCopy(pImageA, pImageB);  //2nd is dest

		pGrayscaleImage = cvCreateImage(cvGetSize(pDiff), IPL_DEPTH_8U, 1);
		//converting to grayscale img
		cvCvtColor(pDiff, pGrayscaleImage, CV_BGR2GRAY);

		//Calc Hist
		cvCalcHist(&pGrayscaleImage, pHist, 0, 0);

		fMaxValue = 0.0; // reset

		//Get the max bins value of hist
		cvGetMinMaxHistValue(pHist, 0, &fMaxValue);
		//magnifying bins of hist
		cvConvertScale(pHist->bins, pHist->bins, (fMaxValue ? (255.0 / fMaxValue) : 0.0), 0);

		//Real1d cvt
		double dRealtimeVal = cvGetReal1D(pHist->bins, 10);

		if (dRealtimeVal > detectThreshold)
		{//TimeStamp
			time_t CurrentTimeStamp = time(NULL);
			if (CurrentTimeStamp - TimeStamp >= 1)
			{
				TimeStamp = CurrentTimeStamp;
			}
		}
		
		cvReleaseImage(&pGrayscaleImage);  // free memory
		pGrayscaleImage = NULL;

		cvWaitKey(10);  // wait for N milliseconds

	}

	cvReleaseCapture(&pCapture);  // stop capturing images & release resources
	cvReleaseHist(&pHist);
	cvReleaseImage(&pImageA);
	cvReleaseImage(&pImageB);
	cvReleaseImage(&pDiff);

	pCapture = NULL;
	pHist = NULL;
	pImageA = NULL;
	pImageB = NULL;
	pDiff = NULL;

	return 0;
}