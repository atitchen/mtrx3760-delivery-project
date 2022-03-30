#include "ros/ros.h"
#include <string>
#include "majorturtle/battery.h"
#include "ros/console.h"
#include "std_msgs/String.h"
#include "std_msgs/Int8.h"
#include "std_msgs/Float32.h"
#include "nav_msgs/Odometry.h"
#include <sstream>
#include <cmath>

class CBattery
{
	public:
    CBattery(int argc, char** argv);
		void Publish( ros::NodeHandle node );
		void Battery_Capacity(float lost_charge_num);
		void PublishBattery();
    void RunBatterydetection();
		void Subscribe( ros::NodeHandle node );
		void OdomCallback(const nav_msgs::Odometry::ConstPtr& msg);
		void LineStatusCallback(const std_msgs::String::ConstPtr &msg);
		float distance2BatteryCapacity();
	private:

		ros::Publisher pub;
		float Capacity1_ = 1000.0;
		int percentage;
		ros::Subscriber sub_odom_;
		ros::Subscriber sub_line_status;
		float move_times = 0 ;
		float move_times_;
		int line_state;
		float old_x = 0;
		float old_y = 0;
};


int main(int argc, char **argv)
{
  CBattery MyBattery(argc,argv);

  return 0;
}

// Run the turtlebox
void CBattery::RunBatterydetection()
{
    ros::Rate loop_rate(5);

    // Infinite loop
    while( ros::ok() )
    {
        float lost_charge_num = distance2BatteryCapacity();

      	Battery_Capacity(lost_charge_num);
        PublishBattery();
        ros::spinOnce(); // operate ros
        loop_rate.sleep();
    }
}

// Constructor: init everything
CBattery::CBattery(int argc, char** argv)
{
    // init ROS
    ros::init(argc, argv, "Battery_Node");
    // create a ROS node
    ros::NodeHandle n;

    Subscribe(n);
    Publish(n); // publish node in movement

    RunBatterydetection();
}

//------------------------------------------------------------------------------------------------------

void CBattery::Publish(ros::NodeHandle node)
{
    pub = node.advertise<majorturtle::battery>("battery", 10);
}

void CBattery::PublishBattery()
{
    majorturtle::battery msg;
   //sg.robot_name.data = "tb3_1";
    msg.Battery_Capacity.data = percentage;
    if(percentage <= 10 && line_state == 1)
    {
      msg.Battery_Charge_flag.data = 0;
      msg.robot_state.data = "Go to charge";
    }
		else if(percentage == 100 && line_state == 0)
		{
			msg.Battery_Charge_flag.data = 1;
			msg.robot_state.data = "Charge finish";
		}
		else if(percentage < 100 && line_state == 0)
		{
			msg.Battery_Charge_flag.data = 0;
			msg.robot_state.data = "Charging";
		}
    else
    {
      msg.Battery_Charge_flag.data = 1;
      msg.robot_state.data = "Working";
    }

    pub.publish(msg);
}

void CBattery::Battery_Capacity(float lost_charge_num)
{
	std::string Turtlebotname;
    Turtlebotname = ros::this_node::getNamespace(); 

	Capacity1_ = Capacity1_-lost_charge_num;
	// std::cout<<"Battery Percentage"<<Turtlebotname<<": "<<Capacity1_/10<<"%"<<std::endl;
	percentage =  Capacity1_/10;
	if (percentage > 100)
	{
		percentage = 100;
		Capacity1_ = 1000;
	}
	else if (percentage < 0)
	{
		percentage = 0;
		Capacity1_ = 0;
	}

}



void CBattery::Subscribe (ros::NodeHandle node )
{
  sub_odom_ = node.subscribe( "odom", 1, &CBattery::OdomCallback, this);
	sub_line_status = node.subscribe( "line_state", 1, &CBattery::LineStatusCallback, this);
}

void CBattery::LineStatusCallback(const std_msgs::String::ConstPtr &msg)
{
	std::string recieved_string;
	recieved_string = msg->data.c_str();
	line_state = stoi(recieved_string);
	//std::cout<<"line_statue"<<line_statue<<std::endl;
}

void CBattery::OdomCallback(const nav_msgs::Odometry::ConstPtr& msg)
{
    float x_1 = msg->pose.pose.position.x;
    float y_1 = msg->pose.pose.position.y;
    if (old_x == 0 && old_y == 0 )
    {
      move_times = 0;
    }
    else if(old_x != x_1 || old_y != y_1 )
    {
      move_times = 1;
    }
    old_x = x_1;
    old_y = y_1;

}


float CBattery::distance2BatteryCapacity()
{
	if(line_state == 0)
	{
		move_times_  = -10 * move_times;
	}
	else
	{

		move_times_ = move_times;
	}
	// std::cout<<"move_times_"<<move_times_<<std::endl;
  return move_times_;
}
