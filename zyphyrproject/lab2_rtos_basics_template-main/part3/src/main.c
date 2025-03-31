#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>


#define DELAY_400       200
#define STACK_SIZE      1024 


#define PRIO_Button 4
#define PRIO_Blinky 5


#define BUTTON0_NODE DT_ALIAS(button0)



struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(BUTTON0_NODE, gpios);


volatile bool button_pressed = true;
volatile bool finished = false;

K_SEM_DEFINE(led_sem, 0, 1);
K_MUTEX_DEFINE(led_mutex);


volatile uint32_t start_time = 0;
volatile uint32_t end_time = 0;


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

void main_task(){
    
    bool local_fin = false;
    uint32_t local_start = 0;
    uint32_t local_end = 0;
    uint32_t time_difference = 0;
    

    init();

    k_msleep(2000); // sleep for 4s to let the system connect to terminal

    printk(" Welcome to The Button Game!\n");
    printk("Press the button to start and stop the timer\n");
    printk("try to get as close to 3 seconds as possible\n");


    while(true){
        
        k_msleep(200); 
        k_mutex_lock(&led_mutex, K_FOREVER);
            local_fin = finished;
            local_start = start_time;
            local_end = end_time;
        k_mutex_unlock(&led_mutex);


    

        if (local_fin){
            time_difference = local_end - local_start;

            if (time_difference > 3100){
                printk("You were too slow\n");
                printk("You were %d ms too slow\n", time_difference - 3000);
            }
            else if (time_difference < 2900){
                printk("You were too fast\n");
                printk("You were %d ms too fast\n", 3000 - time_difference);
            }
            else{
                printk("You were perfect\n");
            }

            k_mutex_lock(&led_mutex, K_FOREVER);
                
                finished = false;

            k_mutex_unlock(&led_mutex);

        }
            
       
    }

}



void button_task()
{   
  
    for (;;) {
        /* Wait until semaphore is given by ISR (meaning button is pressed), 
        take semaphore and toggle LED*/
        
        k_sem_take(&led_sem, K_FOREVER);

        

        if (button_pressed == true) //first button press
        {
            
            printk("Button pressed, start time\n");

            k_mutex_lock(&led_mutex, K_FOREVER);
            start_time = k_uptime_get_32();
            button_pressed = false;
            k_mutex_unlock(&led_mutex);
        }
        else    //secound button press
        {
            
            printk("Button pressed, stop time\n");

            k_mutex_lock(&led_mutex, K_FOREVER);
            end_time = k_uptime_get_32();
            finished = true;
            button_pressed = true;
            k_mutex_unlock(&led_mutex);

            
        }
    }
}




K_THREAD_DEFINE(main_handler, STACK_SIZE, main_task, NULL, NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(button, STACK_SIZE, button_task, NULL, NULL, NULL, 5, 0, 0);