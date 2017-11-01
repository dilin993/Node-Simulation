#include "MyTypes.h"
#include "NodeClient.h"



void backgroundSubstraction(Mat &frame0, Mat &frame1, Mat &frame2, Mat &bgModel, Mat &mask, double TH=15)
{
    Mat frame0g,frame1g,frame2g;

    // convert frames to gray
    cvtColor(frame0,frame0g,COLOR_BGR2GRAY);
    cvtColor(frame1,frame1g,COLOR_BGR2GRAY);
    cvtColor(frame2,frame2g,COLOR_BGR2GRAY);

    bgModel = 0.5*frame0g + 0.3*frame1g + 0.2*frame2g;

    Mat diff;
    absdiff(frame0g,bgModel,diff);

    threshold(diff,mask,TH,255,THRESH_BINARY);
}


int main()
{
    try
    {
        NodeClient client("10.0.0.200",8080);
        client.connect();

        std::cout << "-------------Node Simulation-------------" << std::endl << std::endl;
        std::cout << "Connection established!" << std::endl;

        VideoCapture videoCap(0); // open web cam for input

        const int FPS = 30;

        Mat frame0,frame1,frame2,bgModel,mask;

        //const char *VIDEO_CAPTURE = "Video Capture";
        //const char *BINARY_MASK = "Binary Mask";
        //namedWindow(VIDEO_CAPTURE, CV_WINDOW_AUTOSIZE);
        //namedWindow(BINARY_MASK, CV_WINDOW_AUTOSIZE);


        videoCap.read(frame0);
        videoCap.read(frame1);
        videoCap.read(frame2);


        while(videoCap.isOpened())
        {

            backgroundSubstraction(frame0,frame1,frame2,bgModel,mask,15.0);
            client.sendBinMask(mask);


          //  imshow(VIDEO_CAPTURE,frame0);
         //   imshow(BINARY_MASK,mask);

            frame2 = frame1.clone();
            frame1 = frame0.clone();
            videoCap.read(frame0);

           // waitKey(1000/FPS);
        }


    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
