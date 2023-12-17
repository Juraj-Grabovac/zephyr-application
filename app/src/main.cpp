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
#include <zephyr/zbus/zbus.h>

using namespace std;

/*--------------------------------------------------------------------*/
/*------------------- LOCAL FUNCTIONS PROTOTYPES ---------------------*/
/*--------------------------------------------------------------------*/
static void blink_led_scheduler(struct k_work *work);
static void listener_callback(const struct zbus_channel *chan);
static void poll_gpio_input(void);


/*--------------------------------------------------------------------*/
/*--------------------------- DEFINITIONS ----------------------------*/
/*--------------------------------------------------------------------*/


/* zbus channel definition */
ZBUS_CHAN_DEFINE(zbus_chan,         /* Name */
		 int,                       /* Message type */
		 NULL,                      /* Validator */
		 NULL,                      /* User data */
		 ZBUS_OBSERVERS(listener),  /* observers */
		 0                          /* Initial value is 0 */
);

/* zbus listener definition */
ZBUS_LISTENER_DEFINE(listener, listener_callback);

/* k_work_delayable definition */
K_WORK_DELAYABLE_DEFINE(blink_work, blink_led_scheduler);

/* size of stack area used by thread */
#define STACKSIZE 1024

/* scheduling priority used by thread */
#define PRIORITY 7

/* poll_gpio_input_id thread definition */
K_THREAD_DEFINE(poll_gpio_input_id, STACKSIZE, poll_gpio_input, NULL, NULL, NULL, PRIORITY, 0, 0);

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

/**
 * @brief publish pin state
 *
 * Publishes pin_state on zbus named zbus_chan
 */
void PollClass::publish_pin_state(void)
{
    cout << "Publish pin state: " << pin_state << endl;
    zbus_chan_pub(&zbus_chan, &(pin_state), K_SECONDS(1));
}

/**
 * @brief blink led
 *
 * Toggles LED to blink
 *
 * @return ret - less than 0 when toggling is unsuccessful, otherwise greater or equal to 0
 */
int ReactClass::blink_led (void)
{
    int ret;

    ret = gpio_pin_toggle_dt(&led);

    return ret;
}

/**
 * @brief increment sleep time
 *
 * Increases the sleep time of the LED by 100ms
 */
void ReactClass::increment_sleep_time (void)
{
    sleep_time += 100;

    cout << "New sleep time: " << sleep_time << " ms" << endl;
}

/**
 * @brief retrieves sleep time
 *
 * Retrieves the sleep time of the LED
 *
 * @return sleep_time - sleep time of the LED
 */
int ReactClass::get_sleep_time(void)
{
    return sleep_time;
}

/* Created classes for blinking LEDs and GPIO polling */
PollClass poll_gpio;
ReactClass react_led;


/*--------------------------------------------------------------------*/
/*---------------------------- FUNCTIONS -----------------------------*/
/*--------------------------------------------------------------------*/

/**
 * @brief blink led scheduler
 *
 * Function schedules the blinking time of the LED. 
 * LED sleep time is obtained from the react_led class.
 * 
 * @param work - pointer to k_work structure
 */
static void blink_led_scheduler(struct k_work *work)
{
    int ret;
    
    cout << "BLINK" << endl;

    ret = react_led.blink_led();

    if (ret < 0) 
    {
        k_work_cancel_delayable(&blink_work);
    }
    else
    {
        k_work_reschedule(&blink_work, K_MSEC(react_led.get_sleep_time()));
    }
}

/**
 * @brief zbus listener callback
 *
 * Callback is called when a new message appears on zbus named zbus_chan.
 * Function reads the value from zbus and increases the sleep time of the LED.
 * 
 * @param chan - pointer to zbus_channel structure
 */
static void listener_callback(const struct zbus_channel *chan)
{
	const int *msg = (int *)zbus_chan_const_msg(chan);

    cout << "Listener callback: " << *msg << endl;

    react_led.increment_sleep_time();
}

/**
 * @brief poll gpio input pin
 *
 * Function polls the gpio input pin using the poll_gpio class.
 * If a change occurs on a pin, the new state of the pin is published to zbus_chan.
 * Function also logs pin change and new pin state
 */
static void poll_gpio_input(void)
{
    int val;

	while (1) 
    {
        val = poll_gpio.check_pin_state_change();

        if(val == 1)
        {
            poll_gpio.publish_pin_state();

            LOG_INF("Pin State Changed! New Pin State: %d", poll_gpio.get_pin_state());
        }
	}
}

/**
 * @brief main function
 *
 * Function main serves for initialization/configuration of input/output pins 
 * and also for scheduling the initial sleep time of the LED
 * 
 * @return 0 or 1 - 0 if an error occurs during pin initialization/configuration, otherwise 1
 */
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

    poll_gpio.init_pin_state(gpio_pin_get_dt(&button));

    k_work_schedule(&blink_work, K_MSEC(react_led.get_sleep_time()));

	return 1;
}
