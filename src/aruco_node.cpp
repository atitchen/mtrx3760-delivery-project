

#include <ros/ros.h>
#include <iostream>
#include <string>
#include <vector>
#include <cv_bridge/cv_bridge.h> // cv_bridge
#include <sensor_msgs/Image.h>  // ?
#include <sensor_msgs/CameraInfo.h> // ?
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include "opencv2/aruco/dictionary.hpp"
#include <std_msgs/String.h>

//Open CV Test

class CArucoDetect
{
	public:
		CArucoDetect(int argc, char** argv); // Constructor
		~CArucoDetect(); // Destructor
		void Run(); // Run Infinitely
        void Subscribe( ros::NodeHandle node ); // Subscribe the camera
        void Publish( ros::NodeHandle node ); // Publish the house ID
		void ImageCallback(const sensor_msgs::Image::ConstPtr &msg); // Callback func
        void PublishID();
	private:
		ros::Subscriber sub_image_; // subscriber node
        ros::Publisher pub_aruco_; // publisher node
        cv_bridge::CvImagePtr bridge; // pointer of cv_bridge
        // A pointer that stores specific aruco
		cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

        std::vector<int> markerIds; // arco ID
};

int main( int argc, char** argv )
{
    CArucoDetect MyAruco(argc,argv);
    return 0;
}

// Run the turtlebox
void CArucoDetect::Run()
{
    ros::Rate loop_rate( 30 ); // Set frequency as 30Hz

    // Infinite loop
    while( ros::ok() )
    {

        ros::spinOnce(); // operate ros
        loop_rate.sleep();

        PublishID();
    }
}

// Constructor: init everything
CArucoDetect::CArucoDetect(int argc, char** argv)
{
    // init ROS
    ros::init( argc, argv, "ArucoNode" );
    ros::NodeHandle nh; // create a ROS node

    Publish(nh);
    Subscribe(nh); // subscribe node in sensor
    Run(); // run infinitely
}

// Destructor of Turtlebot
CArucoDetect::~CArucoDetect()
{
    // stop the machine
    ros::shutdown();
}

//--------------------------------------------------------------------------------------------------------
// Subscribe node
void CArucoDetect::Subscribe( ros::NodeHandle node )
{
  	sub_image_ = node.subscribe( "camera/rgb/image_raw", 1000, &CArucoDetect::ImageCallback, this);
}

void CArucoDetect::Publish( ros::NodeHandle node )
{
    pub_aruco_ = node.advertise<std_msgs::String> ("aruco_id", 10);
}

// Image callback
void CArucoDetect::ImageCallback(const sensor_msgs::Image::ConstPtr &msg)
{
    cv_bridge::CvImagePtr cv_ptr;

    // Convert ros camera image into cv image
    try
    {
        cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    }

    // Print errors if cv_bridge doesn't work
    catch (cv_bridge::Exception& e)
    {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }

    // read image from cv pointer
    cv::Mat img = cv_ptr->image;
    cv::waitKey(30);

    std::vector<std::vector<cv::Point2f>> markerCorners;

    // detect markers
    cv::aruco::detectMarkers(img ,dictionary, markerCorners, markerIds);
}

void CArucoDetect::PublishID()
{
    std_msgs::String msg;
	std::stringstream ss;

    for(auto i: markerIds)
    {
        ss << i;
        msg.data = ss.str();

        // std::cout << "Current Aruco ID: " << i <<std::endl;

        break;
    }

	pub_aruco_.publish(msg);

    if(markerIds.size() > 0)
    {
        markerIds.erase(markerIds.begin());
    }

}
