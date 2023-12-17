/**
 ***********************************************************************
 * @file   main.cpp 
 * @author Juraj Grabovac (jgrabovac2@gmail.com)
 * @date   17/12/2023
 * @brief  This file is implementation of Task 1
 ***********************************************************************
*/

/*--------------------------------------------------------------------*/
/*---------------------------- INCLUDES ------------------------------*/
/*--------------------------------------------------------------------*/
#include <iostream>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

using namespace std;

/*--------------------------------------------------------------------*/
/*------------------- LOCAL FUNCTIONS PROTOTYPES ---------------------*/
/*--------------------------------------------------------------------*/


/*--------------------------------------------------------------------*/
/*--------------------------- DEFINITIONS ----------------------------*/
/*--------------------------------------------------------------------*/


/* sleep time = 100 msec */
#define INIT_SLEEP_TIME_MS 100

/* Get button configuration from the devicetree sw0 alias. */
#define SW0_NODE DT_ALIAS(sw0)
/* A build error on this line means your board is unsupported. */
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,{0});

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
/* A build error on this line means your board is unsupported. */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);


/*--------------------------------------------------------------------*/
/*----------------------------- CLASSES ------------------------------*/
/*--------------------------------------------------------------------*/

/**
 * @class PollClass - poll the gpio input pin
 */
class PollClass {
private:
    //attributes
    int pin_state; 

public:
    //methods
    void init_pin_state(int);
    int get_pin_state(void);
    int check_pin_state_change(void);
    void publish_pin_state(void);
};

/**
 * @class ReactClass - controls blink led
 */
class ReactClass {
private:
    //attributes
    int sleep_time {INIT_SLEEP_TIME_MS}; 

public:
    //methods
    int blink_led(void);
    void increment_sleep_time(void);
    int get_sleep_time(void);
};

/**
 * @brief initializes pin state
 *
 * Initializes the pin_state attribute of the PollClass to the state of the input gpio pin
 * 
 * @param state - gpio input pin state
 */
void PollClass::init_pin_state(int state)
{
    cout << "Pin state init: " << state << endl;
    pin_state = state;
}

/**
 * @brief retrieves pin state
 *
 * Retrieves the pin state of the gpio input pin
 *
 * @return pin_state
 */
int PollClass::get_pin_state(void)
{
    return pin_state;
}

/**
 * @brief check pin state change
 *
 * Checks if there is a change on the input gpio pin
 *
 * @return ret_val - 0 if there is no change on input gpio pin, otherwise 1
 */
int PollClass::check_pin_state_change(void)
{
    int new_pin_state;
    int ret_val = 0;

    new_pin_state = gpio_pin_get_dt(&button);

    if(new_pin_state != pin_state)
    {
        cout << "New pin state: " << new_pin_state << endl;
        pin_state = new_pin_state;
        ret_val = 1;
    }

    return ret_val;
}



/*--------------------------------------------------------------------*/
/*---------------------------- FUNCTIONS -----------------------------*/
/*--------------------------------------------------------------------*/

int main(void)
{
	int ret;

    if (!gpio_is_ready_dt(&button)) 
    {
        printk("Error: button device %s is not ready\n",
                button.port->name);
        return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) 
    {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button.port->name, button.pin);
		return 0;
	}

	if (!gpio_is_ready_dt(&led)) 
    {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
	if (ret < 0) 
    {
		return 0;
	}


	return 1;
}
