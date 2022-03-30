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
#include "majorturtle/battery.h"
#include "turtlebot/PizzaOrder.h"

class CMotion
{
	public:
    CMotion(int argc, char** argv);
	  ~CMotion();

		void PublishVelocity( double lin_vel, double ang_vel);
		void Controlmotion();
		void Publish(ros::NodeHandle node ); // publish node
		void Run();
		void Subscribe( ros::NodeHandle node );
		void MotionCallback( const std_msgs::String::ConstPtr &msg );
		void StatusCallback( const std_msgs::String::ConstPtr &msg );
		

		void BatteryStatusCallback( const majorturtle::battery::ConstPtr &msg );
		void PublishLineStatue();
		void line_switch();
		void line_follow(std::string direction);
		
	private:
		int current_line = 0; // 0 for inner 1 for outer

		float current_distance = 0;
		double lin_vel; // linear velocity
		double ang_vel; // angular velocity
		ros::Subscriber sub_dir_;
		ros::Subscriber sub_bot_status;
		ros::Subscriber sub_bot_battery_status;
		

		ros::Publisher pub_vel_; // variable of publisher for published vel
		ros::Publisher pub_line_statue_;
		int dir;  /// Direction message to read published directions
		int bot_status;
		std::string TurtlebotName;
		float PI = 3.1415926535897;
		float angle = 90*2*PI/360;
		float distance = 4.4;
		float current_angle = 0;
		float angular_speed = 30*2*PI/360;
		double T0;
		double T1;


		int battery_flag; // 0 charging 1 for working

};

int main( int argc, char** argv )
{
   	// Create instance of Turtlebot
    CMotion MyMotion(argc,argv);

    return 0;
}

// Run the turtlebox
void CMotion::Run()
{
    ros::Rate loop_rate( 30 ); // Set frequency as 30Hz
		// TurtlebotName = GetTurtlebotName();
    // Infinite loop
    while( ros::ok() )
    {
		Controlmotion();
    	ros::spinOnce(); // operate ros
		loop_rate.sleep();
    }
}

// Constructor: init everything
CMotion::CMotion(int argc, char** argv)
{
    // init ROS
    ros::init( argc, argv, "MotionNode" );
    ros::NodeHandle nh; // create a ROS node

		Subscribe(nh); // subscribe node from bottom carmer
		Publish(nh); // publish node in movement

    Run();
}

// Destructor of Turtlebot
CMotion::~CMotion()
{
    // stop the machine
    ros::shutdown();
}

//--------------------------------------------------------------------------------------------------------
// Subscribe node
void CMotion::Subscribe( ros::NodeHandle node )
{
  	sub_dir_ = node.subscribe( "line_detect", 1, &CMotion::MotionCallback, this);
	sub_bot_status = node.subscribe( "busy_status", 1, &CMotion::StatusCallback, this);
	sub_bot_battery_status = node.subscribe( "battery", 1, &CMotion::BatteryStatusCallback, this);
}

//------------------------------------------------------------------------------------------------------
// Publish output velocity
void CMotion::Publish(ros::NodeHandle node)
{
  	pub_vel_   = node.advertise<geometry_msgs::Twist>( "cmd_vel", 10 );
	pub_line_statue_   = node.advertise<std_msgs::String>( "line_state", 10 );
}

// Pizza amount callback
void CMotion::StatusCallback( const std_msgs::String::ConstPtr &msg )
{
	std::string recieved_string;
	recieved_string = msg->data.c_str();
	bot_status = stoi(recieved_string);


}

void CMotion::BatteryStatusCallback( const majorturtle::battery::ConstPtr &msg )
{
	battery_flag = msg->Battery_Charge_flag.data;
}

//--------------------------------------------------------------------------------------------------------
// Image callback
void CMotion::MotionCallback( const std_msgs::String::ConstPtr &msg )
{
	// std::cout << " flage 1" <<std::endl;
	std::string recieved_string;
	recieved_string = msg->data.c_str();
	dir = stoi(recieved_string);
}

//------------------------------------------------------------------------------------------------------
// move the turtlebot
void CMotion::Controlmotion()
{

	// T0 = ros::Time::now().toSec();
	PublishLineStatue();
	if ((current_line ==0 && ((bot_status == 0) || (bot_status == 2))))  // inner line
	{
		std::cout<<"(inner loop)line follow clockwise"<<std::endl;
		line_follow("clockwise");
	}
	else if ((current_line == 1 && bot_status == 1 && battery_flag == 1)|| (current_line ==0 && bot_status == 1 && battery_flag == 0 ) )
	{
		std::cout<<"(outer loop)line follow counter"<<std::endl;
		line_follow("counter");					// outer line
	}
	else if (current_line == 0 && bot_status == 1 && battery_flag == 1)
	{
		current_line = 3;						// inter to outer
		line_switch();
		current_line = 1;
		PublishLineStatue();
	}

	else if ((current_line == 1 && ((bot_status == 0) || (bot_status == 2))) || (current_line == 1 && battery_flag == 0 && bot_status == 1) )//&& TurtlebotName == "Turtlebot/tb3_0")
	{
		line_switch();							// outer to inter
		current_line = 0;
		PublishLineStatue();
	}
}
void CMotion::line_switch()
{
		T0 = ros::Time::now().toSec();
		// current_line = 3;
		PublishLineStatue();
		while(current_angle < angle)
		{
			lin_vel = 0.0;
			ang_vel = -abs(angular_speed);
			PublishVelocity( lin_vel, ang_vel ); // publish velocity
			T1 = ros::Time::now().toSec();
			current_angle = angular_speed*(T1-T0);

		}
		while(current_distance < distance)
		{
			lin_vel = 0.2;
			ang_vel = 0;
			PublishVelocity( lin_vel, ang_vel ); // publish velocity
			T1 = ros::Time::now().toSec();
			current_distance = lin_vel*(T1-T0);
			std::cout<<TurtlebotName<<": Distance is: "<<distance<<"cm, Current Distance is: "<<current_distance<<"cm"<<std::endl;

		}
		std::cout<<"anything------------------------------------------------------"<<std::endl;

		current_angle = 0;
		current_distance = 0;
}

void CMotion::line_follow(std::string direction)
{

			if (dir == 0)
			{
				lin_vel = 0.2;		
				ang_vel = 0.15;
				PublishVelocity( lin_vel, ang_vel ); // publish velocity

			}
			else if(dir == 1)	// straight line
			{
				lin_vel = 0.2;
				ang_vel = 0;
				PublishVelocity( lin_vel, ang_vel ); // publish velocity

			}
			else if(dir == 2)
			{
				lin_vel = 0.2;
				ang_vel = -0.15;
				PublishVelocity( lin_vel, ang_vel ); // publish velocity
			}

			else if(dir == 3 && direction == "clockwise")
			{
				lin_vel = 0.0;
				ang_vel = 0.25;
				PublishVelocity( lin_vel, ang_vel ); // publish velocity
			}
			else if(dir == 3 && direction == "counter")
			{
				lin_vel = 0.0;
				ang_vel = -0.25;
				PublishVelocity( lin_vel, ang_vel ); // publish velocity
			}
}
//------------------------------------------------------------------------------------------------------
// Publish velocity for the next move
void CMotion::PublishVelocity( double lin_vel, double ang_vel)
{
    geometry_msgs::Twist msg; // create geometry msg

    msg.linear.x = lin_vel; // store linear vel into msg
    msg.angular.z = ang_vel; // store angular vel into msg

    pub_vel_.publish(msg); // publish velocity
}

void CMotion::PublishLineStatue()
{
    std_msgs::String msg;
		std::stringstream ss;

    ss 	<< current_line;
    msg.data = ss.str();

    pub_line_statue_.publish(msg);
}



