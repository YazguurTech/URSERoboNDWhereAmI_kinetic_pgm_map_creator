#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
	ROS_INFO("Sending DriveToTarget Reqest - linear_x:%1.2f, angular_z:%1.2f", (float)lin_x, (float)ang_z);
	
	ball_chaser::DriveToTarget srv;
	srv.request.linear_x = lin_x;
	srv.request.angular_z = ang_z;

	// request a service and pass the velocities to it to drive the robot
	if (!client.call(srv))
		ROS_ERROR("Failed to call service /ball_chaser/command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
	ROS_INFO_STREAM("Processing image to identify a white ball.");

	int white_pixel = 255;
	int rows = img.height;
	int cols = img.step;
	int white_pixel_col = -1;
	int boundary_left_to_middle = (int)cols / 3;
	int boundary_middle_to_right = (int)cols * 2 / 3;
	int red, green, blue;
	
	// Loop through each pixel in the image and check if there's a bright white one
	// Then, identify if this pixel falls in the left, mid, or right side of the image
	// Depending on the white ball position, call the drive_bot function and pass velocities to it
	// Request a stop when there's no white ball seen by the camera

	for(int i = 0; white_pixel_col == -1 && i < rows; i++)
	{
		for(int j = 0; j < cols; j+=3)
		{
			red = img.data[i * img.step + j];
			green = img.data[i * img.step + j + 1];
			blue = img.data[i * img.step + j + 2];
			
			if(red == white_pixel && green == white_pixel && blue == white_pixel)
			{
				ROS_INFO_STREAM("Found the White Ball!");
				white_pixel_col = j;
				break;
			}
		}
	}
	
	if(white_pixel_col == -1)
	{
		// request to stop
		ROS_INFO_STREAM("Request to stop.");
		drive_robot(0.0, 0.0);
	} 
	else if(white_pixel_col < boundary_left_to_middle)
	{
		// drive left
		ROS_INFO_STREAM("Move left.");
		drive_robot(0.0, 1);
	}
	else if(white_pixel_col < boundary_middle_to_right)
	{
		// drive forward
		ROS_INFO_STREAM("Move forward.");
		drive_robot(4.0, 0.0);
	}
	else
	{
		// drive right
		ROS_INFO_STREAM("Move right.");
		drive_robot(0.0, -1);
	}
}

int main(int argc, char** argv)
{
	// Initialize the process_image node and create a handle to it
	ros::init(argc, argv, "process_image");
	ros::NodeHandle n;

	// Define a client service capable of requesting services from command_robot
	client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

	// Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
	ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

	// Handle ROS communication events
	ros::spin();

	return 0;
}








































