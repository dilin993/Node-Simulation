#include <iostream>
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "NodeClient.h"

using boost::asio::ip::tcp;


#define WIDTH 320
#define HEIGHT 240

const char *VIDEO_CAPTURE = "Video Capture";
const char *BINARY_MASK = "Binary Mask";


void backgroundSubstraction(Mat &frame0, Mat &frame1, Mat &frame2, Mat &bgModel, Mat &mask, double TH=15)
{
    Mat frame0g,frame1g,frame2g;

    // resize
    if(frame0.cols!=HEIGHT || frame0.rows!=HEIGHT)
    {
        resize(frame0,frame0g,Size(WIDTH,HEIGHT));
        resize(frame1,frame1g,Size(WIDTH,HEIGHT));
        resize(frame2,frame2g,Size(WIDTH,HEIGHT));
    }
    else
    {
        frame0g = frame0.clone();
        frame1g = frame1.clone();
        frame2g = frame2.clone();
    }

    // convert frames to gray
    cvtColor(frame0g,frame0g,COLOR_BGR2GRAY);
    cvtColor(frame1g,frame1g,COLOR_BGR2GRAY);
    cvtColor(frame2g,frame2g,COLOR_BGR2GRAY);

    bgModel = 0.5*frame0g + 0.3*frame1g + 0.2*frame2g;

    Mat diff;
    absdiff(frame0g,bgModel,diff);

    threshold(diff,mask,TH,255,THRESH_BINARY);
}



int main()
{
    try
    {
        NodeClient client(1);
        client.connect("localhost",8080);

        std::cout << "-------------Node Simulation-------------" << std::endl << std::endl;
        std::cout << "Connection established!" << std::endl;

        VideoCapture videoCap(0); // open web cam for input

        videoCap.set(CAP_PROP_FRAME_WIDTH,320);
        videoCap.set(CAP_PROP_FRAME_WIDTH,240);
        const int FPS = (int)videoCap.get(CAP_PROP_FPS);

        Mat frame0,frame1,frame2,bgModel,mask;


        namedWindow(VIDEO_CAPTURE, CV_WINDOW_AUTOSIZE);
        namedWindow(BINARY_MASK, CV_WINDOW_AUTOSIZE);

        videoCap.read(frame0);
        videoCap.read(frame1);
        videoCap.read(frame2);

        while(videoCap.isOpened())
        {

            backgroundSubstraction(frame0,frame1,frame2,bgModel,mask,15.0);

            client.publish(mask);

            imshow(VIDEO_CAPTURE,frame0);
            imshow(BINARY_MASK,mask);


            frame2 = frame1.clone();
            frame1 = frame0.clone();
            videoCap.read(frame0);

            waitKey(1000/FPS);
        }


    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}