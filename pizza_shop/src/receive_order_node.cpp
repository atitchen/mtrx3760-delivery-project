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


class CRecieveOrder
{

	public:
		CRecieveOrder( int argc, char** argv );
		~CRecieveOrder();

		void Publish( ros::NodeHandle node );
		void Run();
		void PublishPizzaOrder();
		void GetPizzaOrder(int count);

	private:
		ros::Publisher pub_pizza_order;
		std::list<std::pair<int,int>> pizza_order_list;

		int bot_status;
		int bot_status2;
		bool listen_flag;
		bool publish_success_flag;
};



int main( int argc, char** argv )
{
	// Create instance of LineDetect
	CRecieveOrder MyRecieveOrder(argc,argv);
    return 0;
}

void CRecieveOrder::Publish( ros::NodeHandle node )
{
	pub_pizza_order = node.advertise<pizza_shop::PizzaOrder> ("UserInput", 1000);

}

void CRecieveOrder::PublishPizzaOrder()
{
	pizza_shop::PizzaOrder msg;
	msg.house_ID.data = pizza_order_list.front().first;
	msg.num_of_pizza.data = pizza_order_list.front().second;

	
	pub_pizza_order.publish(msg);
	
	//std::cout << pizza_order_list.front().second << " pizzas sent to house " << pizza_order_list.front().first << "\n";

	pizza_order_list.erase(pizza_order_list.begin());

}


CRecieveOrder::CRecieveOrder( int argc, char** argv )
{

	ros::init( argc, argv, "recieve_order_node" );
	ros::NodeHandle nh;

	Publish(nh);

	Run();
}

CRecieveOrder::~CRecieveOrder()
{
	ros::shutdown();
}


void CRecieveOrder::Run()
{
	int count = 0;
	int order_num = 0;
	while (ros::ok())
	{
		ros::Rate rate(10);

		if(count%2==0)
		{
			GetPizzaOrder(order_num );
			PublishPizzaOrder();
			order_num++;
		}



		ros::spinOnce();
    	rate.sleep();
			
		count++;
	}
}

void CRecieveOrder::GetPizzaOrder(int count)
{	
	int house_number;
	int number_of_pizzas;
	std::cout<< std::endl << "------- Order Number #" << count <<" -------"<< std::endl;
	std::cout << "\nEnter House Number: ";
	std::cin >> house_number;
	std::cout << "\nEnter Number of Pizza's: ";
	std::cin >> number_of_pizzas;
	std::cout << "\nNew Order Created.\n\n";

	pizza_order_list.push_back( std::make_pair(house_number, number_of_pizzas));
}