/* Program to run a pair of servo motor using QNX */
/* 18th Nov 2015*/

// include headers
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include <sys/netmgr.h>
#include <math.h>
#include <stdint.h>
#include <hw/inout.h>
#include <pthread.h>
#include <time.h>

#define High 0xFF
#define Low 0x00
#define CTL_Initialize 0x80

#define EXIT_SUCCESS 0

/* Define and Declare Input output port configuration */
/* define the control address for Port A and Port B:
 #define IO_PORT_SIZE 1
 Calculations show below:
 CTRL_ADDRESS_A = 0x280 + 0x08 = 288
 CTRL_ADDRESS_B = 0x280 + 0x09 = 0x289
 CTRL_ADDRESS_Control_Reg = 0x280 + 0x0B */
#define CTRL_ADDRESS_A 0x288
#define CTRL_ADDRESS_B 0x289
#define CTRL_ADDRESS_Control_Reg 0x28B
#define IO_PORT_SIZE 1
#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL

struct timespec timer_val1;
struct timespec timer_val2;

pthread_mutex_t lock1;

uintptr_t ctrlHandle_A;
uintptr_t ctrlHandle_B;
uintptr_t ctrlHandle_Control_Reg;

typedef union {
	struct _pulse pulse;
} my_message_t; //This union is for timer module.

// use struct _clockperiod to vary the realtime clock resolution ...
struct _clockperiod clock_resolution;

// define variables ...
int user_input_servo1;
int user_input_servo2;
int duty_cycle;
int select_servo_recipie;
int loop;
int current_position_servo1;
int current_position_servo2;
// servo positions duty cycle values to be used to run servo through all five required positions ...
enum servo_positions {
	position0 = 390,
	position1 = 700,
	position2 = 1010,
	position3 = 1280,
	position4 = 1640,
	position5 = 2140,

};

// define functions used ...
void get_store_user_command();
void *Thread_Servo1();
void *Thread_Servo2();
void run_servo1(int duty_cycle);
void run_servo2(int duty_cycle);
void start_recipie_execution_servo1();
void start_recipie_execution_servo2();
void Mov_right_Servo1();
void Mov_left_Servo1();
void Mov_right_Servo2();
void Mov_left_Servo2();
void Loop_start_1(int loop);
void Loop_start_2(int loop);

// this function is used to run loop with servo1...
void Loop_start_1(int loop) {
	int j;
	for (j = 0; j < loop; j++) {
		run_servo1(position4);
		run_servo1(position5);
		run_servo1(position0);
	}
}

// this function is used to run loop with servo2...
void Loop_start_2(int loop) {
	int i;

	for (i = 0; i < loop; i++) {
		run_servo2(position4);
		run_servo2(position5);
		run_servo2(position0);
	}
}

// This function stores the user input into the variables which are used to check input and run accordingly...
void get_store_user_command() {
	int User_com_store[2] = { 0 };
	int index = 0;
	{

		User_com_store[index] = getchar();
		index++;
		User_com_store[index] = getchar();
		index = 0;
		user_input_servo1 = User_com_store[0];
		user_input_servo2 = User_com_store[1];
		printf("\n\r --- user_input_servo1=%d user_input_servo2 =%d\n\r ",
				user_input_servo1, user_input_servo1);
	}
}

// Thread which runs te servo1 according to the user input ...
void *Thread_Servo1() {
	printf("Servo 1 running\n\r");// just a check
	int servo1_input; // common input to servo 1 and servo2 ...
	pthread_mutex_lock(&lock1); // mutex to protect the variable which is read by this thread and written by the main function...
	servo1_input = user_input_servo1;
	pthread_mutex_unlock(&lock1);
	if (select_servo_recipie == 66) // checks if the input is B...

	{
		start_recipie_execution_servo1(); // runs the recipe
	} else if (select_servo_recipie == 82) // checks if the user input is R...
	{
		Mov_right_Servo1(); // Moves to the right by one position...
	} else if (select_servo_recipie == 76) // checks if the user input is L...
	{
		Mov_left_Servo1(); // Moves to the left by one position ...
	} else {
		printf("user_input_servo1 is %d", user_input_servo1);// just a check
	}
}

void *Thread_Servo2() {
	printf("Servo2 running\n\r");// just a check
	int servo2_input;
	pthread_mutex_lock(&lock1); // mutex used to protect the variable from double access from te main as well  as this thread ...
	servo2_input = user_input_servo2;
	pthread_mutex_unlock(&lock1);
	if (select_servo_recipie == 66) // checks if the user input is B
	{

		start_recipie_execution_servo2(); // runs the recipe
	}

	else if (select_servo_recipie == 82) // if the user input is R
	{
		Mov_right_Servo2(); // moves the servo to one position right
	} else if (select_servo_recipie == 76)// if the user input is L
	{
		Mov_left_Servo2();// moves the servo to one position left .. .
	} else {
	}
}

// This function runs the servo motor it takes dutycycle value as input and moves to the respective position and also tracks the current position...
void run_servo1(int duty_cycle) {
	int i;
	for (i = 0; i < 26; i++) {
		out8(ctrlHandle_A, High);
		usleep(duty_cycle);
		out8(ctrlHandle_A, Low);
		usleep(20000 - duty_cycle);
	}
	current_position_servo1 = duty_cycle; // updates the current position...
}

// This function runs the servo motor it takes dutycycle value as input and moves to the respective position and also tracks the current position...
void run_servo2(int duty_cycle) {
	int i;
	for (i = 0; i < 26; i++) {
		out8(ctrlHandle_B, High);
		usleep(duty_cycle);
		out8(ctrlHandle_B, Low);
		usleep(20000 - duty_cycle);
	}
	current_position_servo2 = duty_cycle; // updates the current position...
}

// This function runs the recipe for the servo1 ...
// some recipe for demo
void start_recipie_execution_servo1() {
	printf("start_recipie_execution running");
	run_servo1(position0);
	run_servo1(position5);
	run_servo1(position0);
	delay(2000);
	run_servo1(position1);
	run_servo1(position2);
	run_servo1(position3);
	delay(1000);
	run_servo1(position4);
	delay(1000);
	run_servo1(position0);
	run_servo1(position5);
	run_servo1(position0);
	delay(1000);
	Loop_start_1(5);
	delay(1000);

}

// This function runs the recipe for the servo2 ...
// some recipe for demo
void start_recipie_execution_servo2() {
	printf("start_recipie_execution running");
	run_servo2(position0);
	run_servo2(position5);
	run_servo2(position0);
	delay(2000);
	run_servo2(position1);
	run_servo2(position2);
	run_servo2(position3);
	delay(1000);
	run_servo2(position0);
	run_servo2(position5);
	run_servo2(position0);
	run_servo2(position3);
	delay(1000);
	Loop_start_2(5);
	delay(1000);

}

// This function checks the current position and moves a position right if it is within the limit i.e. not in position 0
void Mov_right_Servo1() {
	if (current_position_servo1 == position5) {
		run_servo1(position4);
	} else if (current_position_servo1 == position4) {
		run_servo1(position3);
	} else if (current_position_servo1 == position3) {
		run_servo1(position2);
	} else if (current_position_servo1 == position2) {
		run_servo1(position1);
	} else if (current_position_servo1 == position1) {
		run_servo1(position0);
	} else {
	}

}

// This function checks the current position and moves a position right if it is within the limit i.e. not in position 0
void Mov_right_Servo2() {
	if (current_position_servo2 == position5) {
		run_servo2(position4);
	} else if (current_position_servo2 == position4) {
		run_servo2(position3);
	} else if (current_position_servo2 == position3) {
		run_servo2(position2);
	} else if (current_position_servo2 == position2) {
		run_servo2(position1);
	} else if (current_position_servo2 == position1) {
		run_servo2(position0);
	} else {
	}
}

// This function checks the current position and moves a position left if it is within the limit i.e. not in position 5
void Mov_left_Servo1() {
	if (current_position_servo2 == position0) {
		run_servo1(position1);
	} else if (current_position_servo2 == position1) {
		run_servo1(position2);
	} else if (current_position_servo2 == position2) {
		run_servo1(position3);
	} else if (current_position_servo2 == position3) {
		run_servo1(position4);
	} else if (current_position_servo2 == position4) {
		run_servo1(position5);
	} else {
	}
}

// This function checks the current position and moves a position right if it is within the limit i.e. not in position 5
void Mov_left_Servo2() {
	if (current_position_servo2 == position0) {
		run_servo2(position1);
	} else if (current_position_servo2 == position1) {
		run_servo2(position2);
	} else if (current_position_servo2 == position2) {
		run_servo2(position3);
	} else if (current_position_servo2 == position3) {
		run_servo2(position4);
	} else if (current_position_servo2 == position4) {
		run_servo2(position5);
	} else {
	}
}

int main() {
	printf("Main in");

	if (ThreadCtl(_NTO_TCTL_IO, NULL) == -1) {
		printf("Failed to get I/O access permission");
		return 1;
	}
	// create control handle for the ports ...
	ctrlHandle_A = mmap_device_io(IO_PORT_SIZE, CTRL_ADDRESS_A);
	ctrlHandle_B = mmap_device_io(IO_PORT_SIZE, CTRL_ADDRESS_B);
	ctrlHandle_Control_Reg = mmap_device_io(IO_PORT_SIZE,
			CTRL_ADDRESS_Control_Reg);
	// set the clock resolution to 0.1 mili secs
	clock_resolution.nsec = 100000;
	clock_resolution.fract = 0;
	if (ClockPeriod(CLOCK_REALTIME, &clock_resolution, NULL, 0) < 0) {
		printf("Failed to set clock");
	}

	pthread_t thread0, thread1;// initialize threads

	// Initialize the I/O ports ...
	out8(ctrlHandle_Control_Reg, CTL_Initialize ); // Initialize the control register ...
	out8(ctrlHandle_A, Low); // Initialize I/O port A as output port for servo1...
	out8(ctrlHandle_B, Low); // Initialize I/O port B as output port servo2...
	printf(" Thread created");
	printf("enter 1 to run servo1 and servo2 recipies simultaneously \n\r");
	//	printf("enter 2 to run servo2 recipies alone \n\r");
	//	printf("enter 3 to run servo1 and servo2 recipies simultaneously \n\r");
	select_servo_recipie = getchar();

	// create threads to run Servo1 and Servo2
	pthread_create(&thread0, NULL, Thread_Servo1, NULL);
	pthread_create(&thread1, NULL, Thread_Servo2, NULL);

	//    printf("\n\r Press keys to give Command For servos....\n\r");            // prompts user for input...
	//    printf("\n\r 1st character For Servo1 And 2nd character For Servo2\n\r"); // prompts user for input...
	// This loop checks for the user input everytime ...
	for (;;) {
		printf("Please enter the command in the format>> '*' '*' ");
		printf("\n\r");
		int input[5];
		gets(input);
		get_store_user_command(); // waits for user to give commands for both the servos...
		printf("Please enter the command in the format>> '*' '*' ");
		printf("\n\r");
	}

	//	printf("Exiting thread");b
	pthread_join(thread0, NULL);
	pthread_join(thread1, NULL);
	return EXIT_SUCCESS;
}

