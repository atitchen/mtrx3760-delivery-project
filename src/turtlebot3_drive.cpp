#include <cv_bridge/cv_bridge.h>
#include <cstdlib>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/opencv.hpp>
#include <ros/ros.h>
#include "ros/console.h"
#include "ros/this_node.h"
#include "ros/names.h"
#include "turtlebot/PizzaOrder.h"
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
#include <sstream>
#include <cmath>


class CTurtleBot
{
	  public:
		    CTurtleBot(int argc, char** argv);
		    ~CTurtleBot();
		    void RunTurtle();
        void Subscribe( ros::NodeHandle node );
        void PizzaOrderCallback(  const turtlebot::PizzaOrder::ConstPtr &msg );
        void ArucoIDCallback( const std_msgs::String::ConstPtr &msg );
        void Publish( ros::NodeHandle node );
        void PublishOrder();
        void DefaultStatus();
        std::string GetTurtlebotName();
        void CheckBusyStatus();
        void CheckHomeAdress();
        void CheckPizzaShopAdress();

        
        

    private:
        ros::Subscriber sub_pizza_order;
        ros::Subscriber sub_aruco_tag;

		    ros::Publisher pub_busy_status;
        
        int ID;
        int num;
        int bot_status;
		    bool out_flag;
        bool is_busy;
      	int aruco_tag;

        int pizza_shop_tag;
        int num_of_pizza_picked_up;

        std::string TurtlebotName;
};

int main( int argc, char** argv )
{
   	// Create instance of Turtlebot
    CTurtleBot Myturtle(argc,argv);
    return 0;
}

//--------------------------------------------------------------------------------------------------------
// Subscribe node
void CTurtleBot::Subscribe( ros::NodeHandle node )
{
    sub_pizza_order = node.subscribe( "PizzaOrder", 1000, &CTurtleBot::PizzaOrderCallback, this);
    sub_aruco_tag = node.subscribe( "aruco_id", 1000, &CTurtleBot::ArucoIDCallback, this);
}

//--------------------------------------------------------------------------------------------------------
// Order available callback
void CTurtleBot::PizzaOrderCallback( const turtlebot::PizzaOrder::ConstPtr &msg )
{
    // go to pizza shop and scan the tag of 11 (this can only excute once)

    ID = msg->house_ID.data;
    num = msg->num_of_pizza.data;
    std::cout<<"Id is "<<ID<<" num is "<<num<<std::endl;
    out_flag = true;
}



//--------------------------------------------------------------------------------------------------------
// Run the turtlebox
void CTurtleBot::RunTurtle()
{
    // int Order_status;
    ros::Rate loop_rate( 30 ); // Set frequency as 30Hz
    TurtlebotName = GetTurtlebotName();
    int count = 0;

    // Infinite loop
    while( ros::ok() )
    {
        CheckBusyStatus();
        PublishOrder();

        if(out_flag)
        {
            std::cout<< std::endl << "------- "<< TurtlebotName <<":Operation #" << count <<" -------"<< std::endl;
            PublishOrder();
            std::cout << "Currently going to House ID: " << ID << std::endl;
            std::cout << "Current no of pizza ordered: " << num<< std::endl;
            out_flag = false;

            if(!is_busy)
            {
                std::cout<<"Turtlebot"<<TurtlebotName<<" is free !"<<std::endl;
            }
            else if(is_busy)
            {
                std::cout<<"Turtlebot"<<TurtlebotName<<" is busy !"<<std::endl;
            }
            count++;
        }

        ros::spinOnce(); // operate ros
        loop_rate.sleep();       
    }
}

// Constructor: init everything
CTurtleBot::CTurtleBot(int argc, char** argv)
{
    DefaultStatus();

    // init ROS
    ros::init( argc, argv, "SampleTurtlebotNode" );
    ros::NodeHandle nh; // create a ROS node

    Subscribe(nh); // subscribe node
    Publish(nh);

    RunTurtle();
}

// Destructor of Turtlebot
CTurtleBot::~CTurtleBot()
{
    // stop the machine
    ros::shutdown();
}

void CTurtleBot::Publish( ros::NodeHandle node )
{
	  pub_busy_status = node.advertise<std_msgs::String> ("busy_status", 10);
}

std::string CTurtleBot::GetTurtlebotName()
{
    std::string Turtlebotname;
    Turtlebotname = ros::this_node::getNamespace();     // Name of the turtlebot
    return Turtlebotname;
}

void CTurtleBot::PublishOrder()
{
    std_msgs::String msg;
    std::stringstream ss;
    ss 	<< bot_status; 
    msg.data = ss.str();

    pub_busy_status.publish(msg);

}

void CTurtleBot::DefaultStatus()
{
    ID = 0;                 // No house ID or pizza amount by default
    num =0;   
    pizza_shop_tag = 11;    // The aruco tag for pizza shop is 11
    num_of_pizza_picked_up = 0;  // no pizza at the beginning
    out_flag = false;
}

void CTurtleBot::CheckBusyStatus()
{
    if( ID == 0 )            // If there are no pizzas on the turtlebot
    {
        is_busy = false;    // Turtlebot is not busy
        bot_status = 0; 
    }
    else if(num_of_pizza_picked_up != 0)
    {
        is_busy = true;
        bot_status = 1;
    }
    else 
    {
        is_busy = true;     // Turtlebot is busy
        bot_status = 2; 
    }

}


void CTurtleBot::ArucoIDCallback( const std_msgs::String::ConstPtr &msg )
{
	std::string recieved_string;
	recieved_string = msg->data.c_str();


  if(!recieved_string.empty())
  {
     aruco_tag = stoi(recieved_string);
      std::cout<<TurtlebotName<< ": QR code scanned is: "<< aruco_tag<<std::endl;
	  CheckHomeAdress();
      CheckPizzaShopAdress();
  }
}

void CTurtleBot::CheckHomeAdress()
{
	if(ID == aruco_tag)
	{
    std::cout<<"Turtlebot"<<TurtlebotName<<"has unloaded "<< num_of_pizza_picked_up <<"of pizzas"<<std::endl;
    
    num_of_pizza_picked_up = 0;
		std::cout<<"WE HAVE ARRIVED!! >>>>>>>> Rerouting back to pizza hut!"<<std::endl;
    ID = 0;
    num = 0;
	}
}

void CTurtleBot::CheckPizzaShopAdress()
{
    if(aruco_tag >= 10)
    {
        std::cout<<"picking up order ..........."<<std::endl;
        num_of_pizza_picked_up = num;
        std::cout<<"Turtlebot"<<TurtlebotName<<"has picked up "<< num_of_pizza_picked_up <<" of pizzas"<<std::endl;
        
    }    
}