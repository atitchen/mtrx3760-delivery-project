#include "std_msgs/String.h"
#include <cv_bridge/cv_bridge.h>
#include "pizza_shop/PizzaOrder.h"
#include <cstdlib>
#include <ros/ros.h>
#include "ros/console.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <list>
#include <utility>
#include <iostream>
#include <sstream>
#include <string>
#include "std_msgs/Int8.h"


class CPizzaShop
{

	public:
		CPizzaShop( int argc, char** argv );
		~CPizzaShop();

		void Publish( ros::NodeHandle node );
		void Subscribe( ros::NodeHandle node );
		void StatusCallback( const std_msgs::String::ConstPtr &msg );
		void StatusCallback2( const std_msgs::String::ConstPtr &msg ); 
		void OrderCallback(const pizza_shop::PizzaOrder::ConstPtr &msg );

		void Run();
		void PublishPizzaOrder();
		std::string GetStatus(int turtlebot3_status);

	private:
		ros::Publisher pub_pizza_order1;
		ros::Publisher pub_pizza_order2;
		
		ros::Subscriber sub_bot_status;
		ros::Subscriber sub_bot_status2;
		ros::Subscriber sub_pizza_order;

		std::list<std::pair<int,int>> pizza_order_list;

		int bot_status1;
		int bot_status2;
		bool listen_flag;
		bool publish_success_flag;
};



int main( int argc, char** argv )
{

	CPizzaShop MyPizzaShop(argc,argv);
    return 0;
}

CPizzaShop::CPizzaShop( int argc, char** argv )
{
	bot_status1 = 0;		// Both bots are free initially
	bot_status2 = 0;

	ros::init( argc, argv, "PizzaShop_Node" );
	ros::NodeHandle nh;

	Subscribe(nh);
	Publish(nh);

	Run();
}

CPizzaShop::~CPizzaShop()
{
	ros::shutdown();
}

void CPizzaShop::Publish( ros::NodeHandle node )
{
	pub_pizza_order1 = node.advertise<pizza_shop::PizzaOrder> ("tb3_0/PizzaOrder", 10);
	pub_pizza_order2 = node.advertise<pizza_shop::PizzaOrder> ("tb3_1/PizzaOrder", 10);	
}

// Subscribe node
void CPizzaShop::Subscribe( ros::NodeHandle node )
{
  	sub_bot_status = node.subscribe( "tb3_0/busy_status", 1000, &CPizzaShop::StatusCallback, this);
	sub_bot_status2 = node.subscribe( "tb3_1/busy_status", 1000, &CPizzaShop::StatusCallback2, this);
	
	sub_pizza_order = node.subscribe( "UserInput", 1000, &CPizzaShop::OrderCallback, this);
}

void CPizzaShop::PublishPizzaOrder()
{
	pizza_shop::PizzaOrder msg;
	msg.house_ID.data = pizza_order_list.front().first;
	msg.num_of_pizza.data = pizza_order_list.front().second;
	std::cout<<"Bot 0 status: = "<<GetStatus(bot_status1)<< std::endl;
	std::cout<<"Bot 1 status: = "<<GetStatus(bot_status2)<< std::endl;

	if(!bot_status1)  // bot 1 is free
	{
		pub_pizza_order1.publish(msg);
		std::cout<<"Job assigned to tb3_0!"<<std::endl;
	}
	else if(!bot_status2)		    // bot 2 is free
	{
		pub_pizza_order2.publish(msg);
		std::cout<<"Job assigned to tb3_1!"<<std::endl;
	}
	
	pizza_order_list.erase(pizza_order_list.begin());

}

// Pizza amount callback
void CPizzaShop::StatusCallback( const std_msgs::String::ConstPtr &msg )
{
	std::string recieved_string;
	recieved_string = msg->data.c_str();
	bot_status1 = stoi(recieved_string);
	
			
	
}

void CPizzaShop::StatusCallback2( const std_msgs::String::ConstPtr &msg )
{
	std::string recieved_string;
	recieved_string = msg->data.c_str();
	bot_status2 = stoi(recieved_string);


}

void CPizzaShop::OrderCallback( const pizza_shop::PizzaOrder::ConstPtr &msg )
{	
	int house_number = msg->house_ID.data;
	int number_of_pizzas = msg->num_of_pizza.data;

	pizza_order_list.push_back( std::make_pair(house_number, number_of_pizzas));

}


void CPizzaShop::Run()
{

	ros::Rate rate(30);	
	while (ros::ok()){

		if((!bot_status1 || !bot_status2 ) && (pizza_order_list.size() > 0))
		{
			PublishPizzaOrder();
		}	
		ros::spinOnce();
    	rate.sleep();
	}
}

std::string CPizzaShop::GetStatus(int turtlebot3_status)
{
	std::string status;
	if(turtlebot3_status == 0)
	{
	status = "FREE";
	}
	else
	{
	status = "NOT FREE";
	}
	return status;
}