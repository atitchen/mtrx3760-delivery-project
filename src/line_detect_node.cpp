#include <cv_bridge/cv_bridge.h>
#include <cstdlib>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/opencv.hpp>
#include <ros/ros.h>
#include "ros/console.h"
#include <iostream>
#include <vector>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/aruco.hpp>
#include <vector>
#include "opencv2/aruco/dictionary.hpp"
#include <geometry_msgs/Twist.h>
#include <std_msgs/String.h>

//Open CV Test
class CLineDetect
{
	public:
		CLineDetect(int argc, char** argv);
		~CLineDetect();
		void Run();
		void Subscribe( ros::NodeHandle node );
		void LineImageCallback(const sensor_msgs::Image::ConstPtr &msg);
		void Publish( ros::NodeHandle node );
		void PublishDir( int dir );

	private:
		ros::Subscriber sub_line_image_;
		ros::Publisher pub_dir_;
    	cv_bridge::CvImagePtr bridge;
		cv::Scalar LowerYellow;
		cv::Scalar UpperYellow;
		cv::Mat img_hsv;
		cv::Mat img_mask;

		cv::Mat img;  /// Input image in opencv matrix format
    	cv::Mat img_filt;  /// Filtered image in opencv matrix format
    	int dir;  /// Direction message to be publishe
		cv::Mat Gauss(cv::Mat input);
		int colorthresh(cv::Mat input);
};

int main( int argc, char** argv )
{
   	// Create instance of LineDetect
    CLineDetect MyLineDetect(argc,argv);

    return 0;
}

// Run the turtlebox
void CLineDetect::Run()
{
    ros::Rate loop_rate( 30 ); // Set frequency as 30Hz

    // Infinite loop
    while( ros::ok() )
    {
		if (!img.empty()) {
			int dir; // create a var to store dir

			// Perform image processing
			img_filt = Gauss(img);
			dir = colorthresh(img_filt);
			PublishDir(dir);

		}
        ros::spinOnce(); // operate ros
		loop_rate.sleep();
    }
		// Closing image viewer
		cv::destroyWindow("Turtlebot View");
}

// Constructor: init everything
CLineDetect::CLineDetect(int argc, char** argv)
{
    // init ROS
    ros::init( argc, argv, "LineDetectNode" );
    ros::NodeHandle nh; // create a ROS node

		Subscribe(nh); // subscribe node from bottom carmer
		Publish(nh); // publish node for dir

    Run();
}

// Destructor of Turtlebot
CLineDetect::~CLineDetect()
{
    // stop the machine
    ros::shutdown();
}

//--------------------------------------------------------------------------------------------------------
// Subscribe node
void CLineDetect::Subscribe( ros::NodeHandle node )
{
  	sub_line_image_ = node.subscribe( "downwards_camera/downwards_rgb/image_raw", 1000, &CLineDetect::LineImageCallback, this);
}

//--------------------------------------------------------------------------------------------------------
// Subscribe node
void CLineDetect::Publish( ros::NodeHandle node )
{
  	pub_dir_ = node.advertise<std_msgs::String> ("line_detect", 10);
}

//--------------------------------------------------------------------------------------------------------
// Image callback
void CLineDetect::LineImageCallback(const sensor_msgs::Image::ConstPtr &msg)
		{
			cv_bridge::CvImagePtr cv_ptr;
			try {
		    cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
		    img = cv_ptr->image;
		    cv::waitKey(30);
		  }
		  catch (cv_bridge::Exception& e) {
		    ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
		  }
}

//--------------------------------------------------------------------------------------------------------
// Applying Gaussian Filter
cv::Mat CLineDetect::Gauss(cv::Mat input) {
			  cv::Mat output;
			  cv::GaussianBlur(input, output, cv::Size(3, 3), 0.1, 0.1);
			  return output;
}

//--------------------------------------------------------------------------------------------------------
// Applying Gaussian Filter
int CLineDetect::colorthresh(cv::Mat input) {
			// Initializaing variables
			cv::Size s = input.size();
			std::vector<std::vector<cv::Point> > v;
			auto w = s.width;
			auto h = s.height;
			auto c_x = 0.0;
			// Detect all objects within the HSV range
			cv::cvtColor(input, CLineDetect::img_hsv, CV_BGR2HSV);
			CLineDetect::LowerYellow = {20, 43, 46};
			CLineDetect::UpperYellow = {30, 255, 255};
			cv::inRange(CLineDetect::img_hsv, LowerYellow,
			UpperYellow, CLineDetect::img_mask);
			img_mask(cv::Rect(0, 0, w, 0.8*h)) = 0;
			// Find contours for better visualization
			cv::findContours(CLineDetect::img_mask, v, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
			// If contours exist add a bounding
			// Choosing contours with maximum area
			if (v.size() != 0) {
			auto area = 0;
			auto idx = 0;
			auto count = 0;
			while (count < v.size()) {
			if (area < v[count].size()) {
				idx = count;
				area = v[count].size();
			}
			count++;
			}
			cv::Rect rect = boundingRect(v[idx]);
			cv::Point pt1, pt2, pt3;
			pt1.x = rect.x;
			pt1.y = rect.y;
			pt2.x = rect.x + rect.width;
			pt2.y = rect.y + rect.height;
			pt3.x = pt1.x+5;
			pt3.y = pt1.y-5;
			// Drawing the rectangle using points obtained
			rectangle(input, pt1, pt2, CV_RGB(255, 0, 0), 2);
			// Inserting text box
			cv::putText(input, "Line Detected", pt3,
			CV_FONT_HERSHEY_COMPLEX, 1, CV_RGB(255, 0, 0));
			}
			// Mask image to limit the future turns affecting the output
			img_mask(cv::Rect(0.7*w, 0, 0.3*w, h)) = 0;
			img_mask(cv::Rect(0, 0, 0.3*w, h)) = 0;
			// Perform centroid detection of line
			cv::Moments M = cv::moments(CLineDetect::img_mask);
			if (M.m00 > 0) {
			cv::Point p1(M.m10/M.m00, M.m01/M.m00);
			cv::circle(CLineDetect::img_mask, p1, 5, cv::Scalar(155, 200, 0), -1);
			}
			c_x = M.m10/M.m00;
			// std::cout << c_x << std::endl;
			// Tolerance to chooise directions
			auto tol = 15;
			auto count = cv::countNonZero(img_mask);

			// Turn left if centroid is to the left of the image center minus tolerance
			// Turn right if centroid is to the right of the image center plus tolerance
			// Go straight if centroid is near image center
			if (c_x > w/2+tol) {
			dir = 2;
			}

			else if (c_x < w/2-tol) {
			dir = 0;
			}

			else {
			dir = 1;
			}

			// Search if no line detected
			if (count == 0) {
			dir = 3;
			}
			// Output images viewed by the turtlebot
			// cv::namedWindow("Turtlebot View");
			// imshow("Turtlebot View", input);
			return dir;
}

//--------------------------------------------------------------------------------------------------------
// Applying Gaussian Filter
void CLineDetect::PublishDir( int dir ) {
	std_msgs::String msg;
	std::stringstream ss;

	ss << dir;
	msg.data = ss.str();

	// std::cout << "Dir of line detect: " << dir<<std::endl;
	pub_dir_.publish(msg);
}
