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
/*--------------------------------------------------------------------*/
/*------------------- LOCAL FUNCTIONS PROTOTYPES ---------------------*/
/*--------------------------------------------------------------------*/


/*--------------------------------------------------------------------*/
/*--------------------------- DEFINITIONS ----------------------------*/
/*--------------------------------------------------------------------*/


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
