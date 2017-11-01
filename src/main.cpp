#include "MyTypes.h"
#include "NodeClient.h"
#include "BGSDetector.h"
#include<chrono>



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
            auto start = std::chrono::system_clock::now();
            backgroundSubstraction(frame0,frame1,frame2,bgModel,mask,15.0);

            std::vector<cv::Rect> detections,found;
        
            // cv::Mat structuringElement3x3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
            cv::Mat structuringElement5x5 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
            // cv::Mat structuringElement7x7 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
            // cv::Mat structuringElement9x9 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9, 9));
        
            /*
            cv::dilate(imgThresh, imgThresh, structuringElement7x7);
            cv::erode(imgThresh, imgThresh, structuringElement3x3);
            */
        
            cv::dilate(mask, mask, structuringElement5x5);
            cv::dilate(mask, mask, structuringElement5x5);
            cv::erode(mask, mask, structuringElement5x5);
        
        
        
            std::vector<std::vector<cv::Point> > contours;
        
            

            // contour detection
            cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
            std::vector<std::vector<cv::Point> > convexHulls(contours.size());
        
            for (unsigned int i = 0; i < contours.size(); i++)
            {
                cv::convexHull(contours[i], convexHulls[i]);
            }
        
            // convex hulls
            for (auto &convexHull : convexHulls) {
                Blob possibleBlob(convexHull);
        
                if (possibleBlob.currentBoundingRect.area() > 100 &&
                    possibleBlob.dblCurrentAspectRatio >= 0.2 &&
                    possibleBlob.dblCurrentAspectRatio <= 1.25 &&
                    possibleBlob.currentBoundingRect.width > 20 &&
                    possibleBlob.currentBoundingRect.height > 20 &&
                    possibleBlob.dblCurrentDiagonalSize > 30.0 &&
                    (cv::contourArea(possibleBlob.currentContour) /
                     (double)possibleBlob.currentBoundingRect.area()) > 0.40)
                {
                    found.push_back(possibleBlob.currentBoundingRect);
                }
            }
        
            size_t i, j;
        
            for (i=0; i<found.size(); i++)
            {
                cv::Rect r = found[i];
                for (j=0; j<found.size(); j++)
                    if (j!=i && (r & found[j])==r)
                        break;
                if (j==found.size())
                {
                    r.x += cvRound(r.width*0.1);
                    r.width = cvRound(r.width*0.8);
                    r.y += cvRound(r.height*0.07);
                    r.height = cvRound(r.height*0.8);
                    detections.push_back(r);
                }
        
            }

            

            client.sendBinMask(mask);


          //  imshow(VIDEO_CAPTURE,frame0);
         //   imshow(BINARY_MASK,mask);

            frame2 = frame1.clone();
            frame1 = frame0.clone();
            videoCap.read(frame0);

            auto end = std::chrono::system_clock::now();

            auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            std::cout << "total time " << elapsed.count() << " ms\n";
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
