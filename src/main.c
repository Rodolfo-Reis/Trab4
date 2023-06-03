/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/types.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/timing/timing.h>   /* for timing services */


#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>

/* definir o "node identifier" do sensor*/
#define I2C0_NODE DT_NODELABEL(tempsensor)

/* definir  os endereços relevantes do sensor de temp */
//#define TC74_TEMP_READ           0x00
//#define TC74_CONF_RW             0x01

/* Numero de elementos de dados para guradar */
#define mem_size 10

/*definir o tamanho da stcak e prioridade das threads*/
#define Stacksize 1024
#define thread_tempread_prio 3
#define thread_print_prio 3
#define thread_but_prio 3
#define thread_led_prio 3

// array com id dos butoes 
const uint8_t buttons_pins[] = { 11,12,24,25};
//array com id dos leds
const uint8_t leds_pins[] = { 13,14,15,16};

/* Get node ID for GPIO0, which has leds and buttons */ 
#define GPIO0_NODE DT_NODELABEL(gpio0)

/* Now get the device pointer for GPIO0 */
static const struct device * gpio0_dev = DEVICE_DT_GET(GPIO0_NODE);

/*definir periocidade das threads (em ms)*/
#define thread_tempread_period 5000
#define thread_print_period 1000
#define thread_but_period 10
#define thread_led_period 1000


/* criar espaço de memoria para cada thread*/
K_THREAD_STACK_DEFINE(thread_tempread_stack, Stacksize);
K_THREAD_STACK_DEFINE(thread_print_stack, Stacksize);
K_THREAD_STACK_DEFINE(thread_but_stack, Stacksize);
K_THREAD_STACK_DEFINE(thread_led_stack, Stacksize);

/*criar variaveis para a inf de cada thread */
struct k_thread thread_tempread_data;
struct k_thread thread_print_data;
struct k_thread thread_but_data;
struct k_thread thread_led_data;

/* criar id de tarefas*/
k_tid_t thread_tempread_tid;
k_tid_t thread_print_tid;
k_tid_t thread_but_tid;
k_tid_t thread_led_tid;


/* variaveis globais (shared memory) para comunicar entre tarefas*/
int temp = 0; /* buffer para guardar valores de temperatura*/
int but1 =0;
int but2 =0;
int but3 =0;
int but4 =0; 
int led0_state =1;
int led1_state =1;
int led2_state =0;
int led3_state =0;

/* obter a configuraçao dos butoes da devicetree com alias*/
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios,
							      {0});
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw1), gpios,
							      {0});
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw2), gpios,
							      {0});
static const struct gpio_dt_spec button4 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw3), gpios,
							      {0});

/* prototipos de cada thread */
void thread_tempread_code(void *argA , void *argB, void *argC);
void thread_print_code(void *argA , void *argB, void *argC);
void thread_but_code(void *argA , void *argB, void *argC);
void thread_led_code(void *argA , void *argB, void *argC);

/*prototipos de funcoes */
void in_out_puts_config(void);


int ret;
/* defifnir o registo config do sensor -> 0000 0000 */
uint8_t config = 0x00;

/* obter o device struct e verificar se esta operacional */
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NODE);


void main(void)
{
	in_out_puts_config(); // config input and output pins

	if (!device_is_ready(dev_i2c.bus)) {
	printk("I2C bus %s is not ready!\n\r",dev_i2c.bus->name);
	return;
	}
	else{
		printk("I2C bus %s is ready!\n\r",dev_i2c.bus->name);
	}

	ret = i2c_write_dt(&dev_i2c, config, sizeof(config));
	if(ret != 0){
		printk("Failed to write to I2C device address %x at Reg. %x \n", dev_i2c.addr,config);
		return;
	}

	/*iniciar funçoe de tempo*/
	timing_init();
	timing_start();

	thread_tempread_tid = k_thread_create(&thread_tempread_data, thread_tempread_stack,
        K_THREAD_STACK_SIZEOF(thread_tempread_stack), thread_tempread_code,
        NULL, NULL, NULL, thread_tempread_prio, 0, K_NO_WAIT);

	thread_print_tid = k_thread_create(&thread_print_data, thread_print_stack,
        K_THREAD_STACK_SIZEOF(thread_print_stack), thread_print_code,
        NULL, NULL, NULL, thread_print_prio, 0, K_NO_WAIT);

	thread_but_tid = k_thread_create(&thread_but_data, thread_but_stack,
        K_THREAD_STACK_SIZEOF(thread_but_stack), thread_but_code,
        NULL, NULL, NULL, thread_but_prio, 0, K_NO_WAIT);

	thread_led_tid = k_thread_create(&thread_led_data, thread_led_stack,
        K_THREAD_STACK_SIZEOF(thread_led_stack), thread_led_code,
        NULL, NULL, NULL, thread_led_prio, 0, K_NO_WAIT);
	return;

}

/* implementaçao da thread tempread*/
void thread_tempread_code(void *argA , void *argB, void *argC){

	/* Timing variables to control task periodicity */
    int64_t fin_time=0, release_time=0;

	printk("Thread tempread init (periodic)\n");

	/* Compute next release instant */
    release_time = k_uptime_get() + thread_tempread_period;

	while(1){

		//int temp_sinal =0; /* sinal da temperatura lida, 0- positivo, 1-negativo*/
		uint8_t temp_reading;
		ret = i2c_read_dt(&dev_i2c, &temp_reading, sizeof(temp_reading));
		if(ret != 0){
			printk("Failed to write/read I2C device address %x at Reg. %x \r\n", dev_i2c.addr,config);
		}

		if(temp_reading >= 128){
			// temperatura negativa
			temp = -256 + temp_reading; // guardar o valor da temperatura no buffer de dados
		}
		else{
			//temperatura positiva
			temp = temp_reading; // guardar o valor da temperatura no buffer de dados
		}

		/* Wait for next release instant */ 
        fin_time = k_uptime_get();
        if( fin_time < release_time) {
            k_msleep(release_time - fin_time); /* There are other variants, k_sleep(), k_usleep(), ... */
            release_time += thread_tempread_period;
		}
	}
	timing_stop();
}

/* implementaçao da thread print */
void thread_print_code(void *argA , void *argB, void *argC){
	/* Timing variables to control task periodicity */
    int64_t fin_time=0, release_time=0;

	printk("Thread print init (periodic)\n");

	/* Compute next release instant */
    release_time = k_uptime_get() + thread_print_period;

	while(1){
		
		printk("%d  ",temp);
		printk("\n");
		
		printk("%d ",but1);
		printk("%d ",but2);
		printk("%d ",but3);
		printk("%d ",but4);

		printk("\n");
		printk("%d ",led0_state);
		printk("%d ",led1_state);
		printk("%d ",led2_state);
		printk("%d ",led3_state);

		printk("\n");
		printk("\n");		
		/* Wait for next release instant */ 
    	fin_time = k_uptime_get();
    	if( fin_time < release_time) {
        	k_msleep(release_time - fin_time); /* There are other variants, k_sleep(), k_usleep(), ... */
        	release_time += thread_print_period;
		}
	}
	timing_stop();
}

/* implementaçao da thread para os butoes */
void thread_but_code(void *argA , void *argB, void *argC){

	/* Timing variables to control task periodicity */
    int64_t fin_time=0, release_time=0;

	printk("Thread tempread init (periodic)\n");

	/* Compute next release instant */
    release_time = k_uptime_get() + thread_but_period;

	while(1){

		but1 = gpio_pin_get_dt(&button1);
		but2 = gpio_pin_get_dt(&button2);
		but3 = gpio_pin_get_dt(&button3);
		but4 = gpio_pin_get_dt(&button4);

		/* Wait for next release instant */ 
        fin_time = k_uptime_get();
        if( fin_time < release_time) {
            k_msleep(release_time - fin_time); /* There are other variants, k_sleep(), k_usleep(), ... */
            release_time += thread_but_period;
		}

	}
	timing_stop();
}

void thread_led_code(void *argA , void *argB, void *argC){

	/* Timing variables to control task periodicity */
    int64_t fin_time=0, release_time=0;

	printk("Thread tempread init (periodic)\n");

	/* Compute next release instant */
    release_time = k_uptime_get() + thread_led_period;

	while(1){
		gpio_pin_set(gpio0_dev,leds_pins[0], led0_state);
		gpio_pin_set(gpio0_dev,leds_pins[1], led1_state);
		gpio_pin_set(gpio0_dev,leds_pins[2], led2_state);
		gpio_pin_set(gpio0_dev,leds_pins[3], led3_state);

		/* Wait for next release instant */ 
        fin_time = k_uptime_get();
        if( fin_time < release_time) {
            k_msleep(release_time - fin_time); /* There are other variants, k_sleep(), k_usleep(), ... */
            release_time += thread_led_period;
		}
	}
	timing_stop();
}



void in_out_puts_config(void){
	
	/* configurar butoes da placa */
	//uint32_t pinmask = 0; /* Mask for setting the pins that shall generate interrupts */
	for(int i=0; i<sizeof(buttons_pins); i++) {
		ret = gpio_pin_configure(gpio0_dev, buttons_pins[i], GPIO_INPUT | GPIO_PULL_UP);
		if (ret < 0) {
			printk("Error: gpio_pin_configure failed for button %d/pin %d, error:%d\n\r", i+1,buttons_pins[i], ret);
			return;
		} else {
			printk("Success: gpio_pin_configure for button %d/pin %d\n\r", i+1,buttons_pins[i]);
		}
	}

	/* configurar butoes da placa */
	//uint32_t pinmask = 0; /* Mask for setting the pins that shall generate interrupts */
	for(int i=0; i<sizeof(leds_pins); i++) {
		ret = gpio_pin_configure(gpio0_dev, leds_pins[i], GPIO_OUTPUT);
		if (ret < 0) {
			printk("Error: gpio_pin_configure failed for button %d/pin %d, error:%d\n\r", i+1,leds_pins[i], ret);
			return;
		} else {
			printk("Success: gpio_pin_configure for button %d/pin %d\n\r", i+1,leds_pins[i]);
		}
	}

}