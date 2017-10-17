#include <iostream>
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;


#define WIDTH 320
#define HEIGHT 240

const char *VIDEO_CAPTURE = "Video Capture";
const char *BINARY_MASK = "Binary Mask";
//const char *BINARY_MASK_R = "Binary Mask Recieved";

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


unsigned char * getBitBufferFromMat(Mat &binMask)
{
    int N = (binMask.rows * binMask.cols)/8+5;
    auto const buffer = new unsigned char[N];
    buffer[0] = (unsigned char)(binMask.rows & 255);
    buffer[1] = (unsigned char)(binMask.rows  >> 8);
    buffer[2] = (unsigned char)(binMask.cols & 255);
    buffer[3] = (unsigned char)(binMask.cols  >> 8);

    int bytePos = 4;
    unsigned char bitPos = 0;
    unsigned char temp;
    unsigned char bit;

    for(int i=0;i<binMask.rows;i++)
    {
        for(int j=0;j<binMask.cols;j++)
        {
            if(binMask.at<unsigned char>(i,j)>0)
                bit = 1;
            else
                bit = 0;

            if(bitPos==0)
                temp = bit;
            else
                temp = (temp<<1) | bit;

            bitPos++;
            if(bitPos>=8)
            {
                buffer[bytePos] = temp;
                bitPos = 0;
                bytePos++;
            }
        }
    }

    return buffer;
}


Mat getMatFromBitBuffer(unsigned char *buffer)
{
    int rows = (buffer[1] << 8) + buffer[0];
    int cols = (buffer[3] << 8) + buffer[2];

    Mat binMask(rows,cols,CV_8UC1);

    int bytePos = 4;
    unsigned char bitPos = 0;
    unsigned char temp;

    for(int i=0;i<binMask.rows;i++)
    {
        for(int j=0;j<binMask.cols;j++)
        {
            temp = buffer[bytePos];
            temp = (temp & (unsigned char)(1<<(7-bitPos)))>>(7-bitPos);
            binMask.at<unsigned char>(i,j) = (unsigned char)(temp*255);
            bitPos++;
            if(bitPos>=8)
            {
                bitPos = 0;
                bytePos++;
            }
        }
    }

    return binMask;
}

void handle_write(const boost::system::error_code& error,
                  size_t bytes_transferred)
{
    cout << bytes_transferred << " bytes written.\t code: " << boost::system::system_error(error).what();
}

int main()
{
    try
    {

        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);
        //tcp::resolver::query query(argv[1], "daytime");
        tcp::resolver::query query("192.168.1.7", "8080");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;
        tcp::socket socket(io_service);
        boost::system::error_code error = boost::asio::error::host_not_found;

        while (error && endpoint_iterator != end)
        {
            socket.close();
            socket.connect(*endpoint_iterator++, error);
        }
        if (error)
            throw boost::system::system_error(error);


        std::cout << "-------------Node Simulation-------------" << std::endl << std::endl;
        std::cout << "Connection established!" << std::endl;

        VideoCapture videoCap(0); // open web cam for input

        videoCap.set(CAP_PROP_FRAME_WIDTH,320);
        videoCap.set(CAP_PROP_FRAME_WIDTH,240);
        const int FPS = (int)videoCap.get(CAP_PROP_FPS);

        Mat frame0,frame1,frame2,bgModel,mask;


        namedWindow(VIDEO_CAPTURE, CV_WINDOW_AUTOSIZE);
        namedWindow(BINARY_MASK, CV_WINDOW_AUTOSIZE);
//        namedWindow(BINARY_MASK_R, CV_WINDOW_AUTOSIZE);

        videoCap.read(frame0);
        videoCap.read(frame1);
        videoCap.read(frame2);

//        Mat maskR;
        unsigned char *buffer= nullptr;

        while(videoCap.isOpened())
        {

            backgroundSubstraction(frame0,frame1,frame2,bgModel,mask,15.0);

            delete[] buffer;

            buffer = getBitBufferFromMat(mask);

            int N = (mask.rows * mask.cols)/8+5;
//            socket.write_some(boost::asio::buffer(buffer,N), error);
            socket.async_write_some(boost::asio::buffer(buffer,N),handle_write);
            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.


//            maskR = getMatFromBitBuffer(buffer);

            imshow(VIDEO_CAPTURE,frame0);
            imshow(BINARY_MASK,mask);
//            imshow(BINARY_MASK_R,maskR);

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