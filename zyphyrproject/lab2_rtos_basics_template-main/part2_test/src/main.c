#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include <zephyr/sys/printk.h>


#define DELAY_400       200
#define STACK_SIZE      500 


#define PRIO_Button 4
#define PRIO_Blinky 5


#define BUTTON0_NODE DT_ALIAS(button0)




struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(BUTTON0_NODE, gpios);


volatile int current_led = 0;
K_SEM_DEFINE(led_sem, 0, 1);
K_MUTEX_DEFINE(led_mutex);

static struct gpio_callback btn_cb_data;
void button_isr()
{
    k_sem_give(&led_sem);
}
void init() {
    /* Configures LED and button */
    gpio_pin_configure_dt(&button0, GPIO_INPUT);

    /* Registers interrupt on edge to active level */
    gpio_pin_interrupt_configure_dt(&button0, GPIO_INT_EDGE_TO_ACTIVE);
    /* Initializes callback struct with ISR and the pins on which ISR should trigger  */
    gpio_init_callback(&btn_cb_data, button_isr, BIT(button0.pin));
    /* Adds ISR callback to the device */
    gpio_add_callback_dt(&button0, &btn_cb_data);

    struct k_mutex led_mutex;
    k_mutex_init(&led_mutex);
}




void blinky_task()
{
    init();
    int local_led = 0;

    printk("Hello, Zephyr!\n");
    while (1) {
        k_sleep(K_SECONDS(1));
        printk("1 second passed\n");
    }

	for(;;)
    {
        // lock mutex
        k_mutex_lock(&led_mutex, K_FOREVER);
        local_led = current_led;
        k_mutex_unlock(&led_mutex);  // unlock mutex

        
        
        switch(local_led)
        {
            case 0:
                
              
                k_msleep(DELAY_400);
                printk("LED0\n");
                break;
            case 1:


                k_msleep(DELAY_400);
                printk("LED1\n");
                break;
                
            case 2:
                
                k_msleep(DELAY_400);
                printk("LED2\n");
                break;
                
            case 3:
    
            
                k_msleep(DELAY_400);
                printk("LED3\n");
                break;
                
        }
    }
}

void button_task()
{   
    int local_led = 0;

    for (;;) {
        /* Wait until semaphore is given by ISR (meaning button is pressed), 
        take semaphore and toggle LED*/
        k_sem_take(&led_sem, K_FOREVER);

        // lock mutex

        local_led = current_led;
        // unlock mutex

        if (local_led == 3)
        {
            local_led = 0;
        }
        else
        {
            local_led++;
        }

        k_mutex_lock(&led_mutex, K_FOREVER);
        //printf("Button pressed, changing LED to %d\n", local_led);
        current_led = local_led;
        k_mutex_unlock(&led_mutex);
    }
}

K_THREAD_DEFINE(blinky, STACK_SIZE, blinky_task, NULL, NULL, NULL, PRIO_Blinky, 0, 0);
K_THREAD_DEFINE(button, STACK_SIZE, button_task, NULL, NULL, NULL, PRIO_Button, 0, 0);


