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

/*definir periocidade das threads (em ms)*/
#define thread_tempread_period 5000
#define thread_print_period 20000

/* criar espaço de memoria para cada thread*/
K_THREAD_STACK_DEFINE(thread_tempread_stack, Stacksize);
K_THREAD_STACK_DEFINE(thread_print_stack, Stacksize);

/*criar variaveis para a inf de cada thread */
struct k_thread thread_tempread_data;
struct k_thread thread_print_data;

/* criar id de tarefas*/
k_tid_t thread_tempread_tid;
k_tid_t thread_print_tid;

/* defifnir array cicular para guardar dados */
typedef struct{
	int data[mem_size]; /*array para guardar dados */
	int idx;	/* ponteiro para a proxima posiçao a guardar dados*/
}buffer;


/* variaveis globais (shared memory) para comunicar entre tarefas*/
buffer temp_buffer; /* buffer para guardar valores de temperatura*/



/* prototipos de cada thread */
void thread_tempread_code(void *argA , void *argB, void *argC);
void thread_print_code(void *argA , void *argB, void *argC);

int ret;
/* defifnir o registo config do sensor -> 0000 0000 */
uint8_t config = 0x00;

/* obter o device struct e verificar se esta operacional */
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NODE);

void main(void)
{
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
			//temp_reading = 256 - temp_reading;
			//temp_sinal = 1;
			// temperatura negativa
			printk("-%d \n", temp_reading);
			temp_buffer.data[temp_buffer.idx] = -256 + temp_reading; // guardar o valor da temperatura no buffer de dados
			temp_buffer.idx = (temp_buffer.idx + 1)% mem_size; // incrementar a posiçao a guardar dados
		}
		else{
			//temperatura positiva
			//temp_sinal = 0;
			printk("%d \n", temp_reading);
			temp_buffer.data[temp_buffer.idx] = temp_reading; // guardar o valor da temperatura no buffer de dados
			temp_buffer.idx = (temp_buffer.idx + 1)% mem_size; // incrementar a posiçao a guardar dados
		}
		/*
		if(temp_sinal == 1){
			printk("-%d \n", temp_reading);
		}
		else{
			printk("%d \n", temp_reading);
		}*/

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
		for(int i=0; i<mem_size;i++){
			printk("%d  ",temp_buffer.data[i]);
		}
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