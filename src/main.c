/** \file main.c
 *  \brief Implementaçao de um modulo simples de Input/Output sensor/atuador com envio
 * 	de comandos pela UART 
 * 
 * 
 *  O sistema implementa 5 Input, valor de tempearatura obtido através do sensor tc74 através de comunicaçao i2c com o mesmo e 4 butoes da placa 
 *  que permite ler o estado em que cada butao se encontra. Para a obtençao destes valores foram implementadas 2 threads periodicas, 
 *  1 para temperatura e outra para os butoes, que irão periodicamento ler o valor de cada intput.
 *  O sistema tem 4 saídas e para a alteraçao das saídas foi implementada 1 thread periodica que ira atribuir a cada saida um estado que eta  guardado numa variavel.
 *  Também foi implementado um comunicaçao via uart para a inserçao de comandos para alteraçao dos periodos de cada thread, comando do tipo #TXYYYY! ou #tXYYY!,
 *  onde X pode ser T/t para thread temperartura, L/t para thread leds ou B/b para thread butoes e YYYY sao qutro digitos de tempo em ms.
 *  e alteraçao do estado dos leds, comando do tipo #Lxy! ou #lxy! , em que é o led que pretendemos alterar estado e y o estado.
 *  
 * 
 * Base documentation:
 *  Zephyr kernel:  
 *      https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/index.html#kernel-services
 *      
 *  DeviceTree 
 *      Board DTS can be found in BUILD_DIR/zephyr/zephyr.dts
 *      https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/dts/api-usage.html#dt-from-c  
 *
 *      HW info
 *      https://infocenter.nordicsemi.com/topic/struct_nrf52/struct/nrf52840.html
 *      Section: nRF52840 Product Specification -> Peripherals -> GPIO / GPIOTE
 * 
 *      Board specific HW info can be found in the nRF52840_DK_User_Guide_20201203. I/O pins available at pg 27
 * 
 * 
 *  \author Rodolfo Oliveira, NºMec: 88919
 *  \date 06/06/2023
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/types.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/timing/timing.h>   /* for timing services */
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>


#define I2C0_NODE DT_NODELABEL(tempsensor)/**< definir o "node identifier" do sensor*/


#define mem_size 10/**< Numero de elementos de dados para guradar */


#define Receive_Buff_Size 10/**< Define the size of the UART receive buffer */

/*definir o tamanho da stcak e prioridade das threads*/
#define Stacksize 1024 /**< tamanho da stack da thread*/
#define thread_tempread_prio 3 /**< prioridade da thread tempread*/
#define thread_print_prio 3/**< prioridade da thread para print de valores*/
#define thread_but_prio 3/**< prioridade da thread dos butoes*/
#define thread_led_prio 3/**< prioridade da thread dos leds*/
#define thread_cmd_prio 4/**< prioridade da thread da cmd*/

// array com id dos butoes 
const uint8_t buttons_pins[] = { 11,12,24,25};
//array com id dos leds
const uint8_t leds_pins[] = { 13,14,15,16};


#define GPIO0_NODE DT_NODELABEL(gpio0)/**< Get node ID for GPIO0, which has leds and buttons */ 


static const struct device * gpio0_dev = DEVICE_DT_GET(GPIO0_NODE);/**< Now get the device pointer for GPIO0 */

/*definir periocidade das threads (em ms)*/
volatile int thread_tempread_period = 5000; /**< Periodo da thread tempread */
#define thread_print_period 1000 /**< periodo estatico da thread print de valores */
volatile int thread_but_period = 10;/**< periodo da thread dos butoes*/
volatile int thread_led_period = 1000;/**< periodo da thread dos leds*/
#define thread_cmd_period 1000/**< periodo estatico da thread dos comandos*/


/* criar espaço de memoria para cada thread*/
K_THREAD_STACK_DEFINE(thread_tempread_stack, Stacksize);/**< Criar espaço de memoria para a thread tempread*/
K_THREAD_STACK_DEFINE(thread_print_stack, Stacksize);/**< Criar espaço de memoria para a thread print*/
K_THREAD_STACK_DEFINE(thread_but_stack, Stacksize);/**< Criar espaço de memoria para a thread dos butoes*/
K_THREAD_STACK_DEFINE(thread_led_stack, Stacksize);/**< Criar espaço de memoria para a thread dos leds*/
K_THREAD_STACK_DEFINE(thread_cmd_stack, Stacksize);/**< Criar espaço de memoria para a thread dos comandos*/

/*criar variaveis para a inf de cada thread */
struct k_thread thread_tempread_data;/**< data da thread tempread*/
struct k_thread thread_print_data;/**< data da thread print*/
struct k_thread thread_but_data;/**< data da thread dos butoes*/
struct k_thread thread_led_data;/**< data da thread dos leds*/
struct k_thread thread_cmd_data;/**< data da thread dos comandos*/

/* criar id de tarefas*/
k_tid_t thread_tempread_tid;/**< ID da tarefa da thread tempread*/
k_tid_t thread_print_tid;/**< ID da tarefa da thread print*/
k_tid_t thread_but_tid;/**< ID da tarefa da thread dos butoes*/
k_tid_t thread_led_tid;/**< ID da tarefa da thread dos leds*/
k_tid_t thread_cmd_tid;/**< ID da tarefa da thread dos comandos*/

/* variaveis globais (shared memory) para comunicar entre tarefas*/
volatile int temp = 0; /**< Var para guardar valores de temperatura*/
volatile int but1 =0; /**< Var para guardar valor lido do estado do butao 1*/
volatile int but2 =0; /**< Var para guardar valor lido do estado do butao 2*/
volatile int but3 =0;/**< Var para guardar valor lido do estado do butao 3*/
volatile int but4 =0; /**< Var para guardar valor lido do estado do butao 4*/
volatile int led0_state =1;/**< Var para guardar estado do led 1*/
volatile int led1_state =1;/**< Var para guardar estado do led 2*/
volatile int led2_state =0;/**< Var para guardar estado do led 3*/
volatile int led3_state =0;/**< Var para guardar estado do led 4*/
volatile int cmd = 0;/**< Var para indicaçao de que foi inserido comando quando tem o valor 1*/
volatile int res = 1;

/**variaveis internas */
static unsigned char cmdLen = 0; 
int SOF_C = -1;
int EOF_C = -1;
static char cmdString[Receive_Buff_Size];

/** definir buffer de transmisao da uart que guarda o conteudo a transmitir pela uart*/
static uint8_t tx_buf[]= {""};
/** definir buffer de receçao*/
static uint8_t rx_buf[Receive_Buff_Size] = {0};

/** definir o tamanho do receive buffer da uart*/
#define Receive_Buff_Size 10

/** definir o periodo de timeout da uart*/
#define Receive_Timeout 100

/** uart node identifier*/
#define UART_NODE DT_NODELABEL(uart0)

/** obter a configuraçao dos butoes da devicetree com alias*/
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
void thread_cmd_code(void *argA , void *argB, void *argC);

/*prototipos de funcoes */
void in_out_puts_config(void);
int CmdChar(unsigned char newChar);
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data);
void resetcmd(void);
int cmdProcess(void);

int ret;
/** defifnir o registo config do sensor -> 0000 0000 */
uint8_t config = 0x00;

/** obter o device struct e verificar se esta operacional */
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NODE);

/** Get the device pointer of the UART hardware */
const struct device *uart = DEVICE_DT_GET(UART_NODE);

/** definir estrutura de comandos e saidas possiveis */
#define MAX_CMDSTRING_SIZE 10   /**< tamanho maximo do comando */ 
#define SOF_SYM '#'             /**< Simbolo de inicio de comando */
#define EOF_SYM '!'             /**< Simbolo de fim de comando */
#define EXIT_SUCCESS    0;      /**< Successfull exit */
#define EMPTY_STRING   -1;      /**< String is empty */
#define STRING_FULL    -1;      /**< String is full */
#define CMD_NOT_FOUND  -2;      /**< Invalid CMD */
#define WRONG_STR_FORMAT -3;    /**< Wrong string format*/

/** \brief Funçao Main
 * 
 * Funçao main configura os devices I2C, inputs e outputs, e uart e cria as threads
 *
 */
void main(void)
{

	in_out_puts_config(); // config input and output pins

	/* checar devices */
	/*I2C*/
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

	 /* UART */
    if (!device_is_ready(uart)) {
        printk("UART device not ready\r\n");
        return;
    }
	
	ret = uart_callback_set(uart, uart_cb, NULL);
    if (ret) {
		return;
	}
	/* Send the data over UART by calling uart_tx() */
	ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_MS);
    if (ret) {
		return;
	}

    /* Start receiving by calling uart_rx_enable() and pass it the address of the receive  buffer */
	ret = uart_rx_enable(uart ,rx_buf,sizeof rx_buf,Receive_Timeout);
	if (ret) {
		return;
	}

	/*iniciar funçoe de tempo*/
	
	timing_init();
	timing_start();
	 /* criar tarefas*/
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

	thread_cmd_tid = k_thread_create(&thread_cmd_data, thread_cmd_stack,
        K_THREAD_STACK_SIZEOF(thread_cmd_stack), thread_cmd_code,
        NULL, NULL, NULL, thread_cmd_prio, 0, K_NO_WAIT);
		
	resetcmd();	
	return;
}

/** \brief callback de interrupçao da uart
 * 
 * Funçao de interrupçao que guarda valores inserido pelo utilizador num buffer
 * até que seja primido enter
 * 
 * 
 */
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data){
	int res = 1;
	switch (evt->type) {

	case UART_RX_RDY:
        if(evt->data.rx.len > 0){
            if(evt->data.rx.buf[evt->data.rx.offset] == '\r') { // espera que seja primido enter
                cmd = 1; // identifica que foi inserido um comando
                break;
            }
            res = CmdChar(evt->data.rx.buf[evt->data.rx.offset]);
        }
        break;

	case UART_RX_DISABLED:
		uart_rx_enable(dev ,rx_buf,sizeof rx_buf,Receive_Timeout);
		break;
		
	default:
		break;
    }
}
/** \brief Funçao para adicionar char introduzido pelo user num buffer 
 * ******************************** 
 * adiciona char á cmdString 	
 * \return 	0: if success 			         		        
 * \return -1: if cmd string full 	    
 * ******************************** 
 * */
int CmdChar(unsigned char newChar) {
	/* If cmd string not full add char to it */
	if (cmdLen < MAX_CMDSTRING_SIZE) {
		cmdString[cmdLen] = newChar;
		cmdLen ++;
		return EXIT_SUCCESS;
	}
	/* If cmd string full return error */
	return STRING_FULL;
}

/** \brief Funçao de reset do buffer de comandos
 * 
 * Resseta o buufer onde estao guardados o comandos bem como o valor de 
 * start of frame e end of frame
 * 
*/
void resetcmd(void) {
	cmdLen = 0;
	SOF_C = -1;
	EOF_C = -1;		
}

/** \brief Processa o cmdString que foi inserido para verificar comando 
 *                                                      
 * \return  0: comando valido e executado                         	    
 * \return	-1: string vazia ou comando incompleto                      
 * \return	-2: comando invalido encontrado                             
 * \return	-3: formato da string incorreto                             
 * ************************************************************ */
int cmdProcess(void){
	int i;
	/*encontrar simbolo inicial de comando e final de comando*/
	/* posiçao do primeiro #*/
	for(i=0;i<cmdLen;i++){
		if(cmdString[i] == '#'){
			SOF_C = i;
			break;
		}
	}
	/*identificar posicao do primeiro !*/
	for(i=0;i<cmdLen;i++){
		if(cmdString[i] == '!'){
			EOF_C = i;
			break;
		}
	}
	/* se encontrou primeiro ! do que #*/
	if(EOF_C < SOF_C){
		SOF_C = -1;
		EOF_C = -1;
	    return WRONG_STR_FORMAT;
	}
	/* se encontrou um # entre o primeiro # e !*/
	for(i=SOF_C+1;i<EOF_C;i++){
		if(cmdString[i] == '#'){
			return WRONG_STR_FORMAT;
		}
	}

	if(SOF_C == -1){
		return WRONG_STR_FORMAT;
	}
	if(EOF_C == -1){
		return WRONG_STR_FORMAT;
	}

	/* Detect empty cmd string */
	if(cmdLen == 0) {
		return EMPTY_STRING;
    } 
	/* ate aqui confirmo que tenho um comando do tipo #...! */
	/* detetar comandos*/
	/* comando para altera tempo(ms) das threads*/
	if(cmdString[SOF_C+1] == 'T' || cmdString[SOF_C+1] == 't'){
		if(cmdLen<8){
			return WRONG_STR_FORMAT;
		}
		for(i = SOF_C+3;i<EOF_C;i++){
			if(cmdString[i] < '0' || cmdString[i] > '9'){
				return CMD_NOT_FOUND;
			}
		}
		/* mudar periodo da thread dos leds*/
		if(cmdString[SOF_C+2] == 'L' || cmdString[SOF_C+2] == 'l'){
			if( (EOF_C - (SOF_C+3)) == 4){
				thread_led_period = (cmdString[SOF_C+3]-'0')*1000+(cmdString[SOF_C+4]-'0')*100+(cmdString[SOF_C+5]-'0')*10+(cmdString[SOF_C+6]-'0')*1;
				return EXIT_SUCCESS;
			}
			else{
				return CMD_NOT_FOUND;
			}
			return EXIT_SUCCESS;
		}
		/* mudar periodo da thread dos butoes*/
		else if(cmdString[SOF_C+2] == 'B' || cmdString[SOF_C+2] == 'b'){
			if( (EOF_C - (SOF_C+3)) == 4){
				thread_but_period = (cmdString[SOF_C+3]-'0')*1000+(cmdString[SOF_C+4]-'0')*100+(cmdString[SOF_C+5]-'0')*10+(cmdString[SOF_C+6]-'0')*1;
				return EXIT_SUCCESS;
			}
			else{
				return CMD_NOT_FOUND;
			}
			return EXIT_SUCCESS;
		}
		/* mudar periodo da thread da temp*/
		else if(cmdString[SOF_C+2] == 'T' || cmdString[SOF_C+2] == 't'){
			if( (EOF_C - (SOF_C+3)) == 4){
				thread_tempread_period = (cmdString[SOF_C+3]-'0')*1000+(cmdString[SOF_C+4]-'0')*100+(cmdString[SOF_C+5]-'0')*10+(cmdString[SOF_C+6]-'0')*1;
				return EXIT_SUCCESS;
			}
			else{
				return CMD_NOT_FOUND;
			}
		}
		return EXIT_SUCCESS;
	}
	/* verificar se é commando para alterar estado dos leds*/
	else if(cmdString[SOF_C+1] == 'L' || cmdString[SOF_C+1] == 'l'){
		/* tamanho deste tipo de comando é 5*/
		if((EOF_C - SOF_C) > 4 ){
			return CMD_NOT_FOUND;
		}
		else if(cmdString[SOF_C+2] < '0' || cmdString[SOF_C+2] > '3'){
			return CMD_NOT_FOUND;
		} 
		else if(cmdString[SOF_C+3] < '0' || cmdString[SOF_C+3] > '1'){
			return CMD_NOT_FOUND;
		} 
		else{
			if(cmdString[SOF_C+2] == '0'){
				led0_state = cmdString[SOF_C+3] - '0';
			}
			else if(cmdString[SOF_C+2] == '1'){
				led1_state = cmdString[SOF_C+3] - '0';
			}
			else if(cmdString[SOF_C+2] == '2'){
				led2_state = cmdString[SOF_C+3] - '0';
			}
			else if(cmdString[SOF_C+2] == '3'){
				led3_state = cmdString[SOF_C+3] - '0';
			}	
		}
		return EXIT_SUCCESS;
	}
	else{
		return CMD_NOT_FOUND;
	}
	return WRONG_STR_FORMAT;
}

/** \brief Thread de leitura da temperatura
 * 
 * Esta thread implementa comunicaçao I2C com o sensor tc74 para retirar amostras do valor da temperatura
 * Esta thread é periodica e o seu periodo pode ser alterado pela introduçao de um comando
 * 
 * 
*/
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
/** \brief Thread de Print dos valores de temperatura, estado dos leds e butoes e periodo correspondente a cada thread
 * 
 * Esta thread imprime os valores das variaveis que contêm o valor da temperatura e estados dos butoes e leds e 
 * periodo respetivo de cada thread, tempread, butoes e leds
 * Esta thread é periodica e o seu periodo é estatico
 * 
 * 
*/
void thread_print_code(void *argA , void *argB, void *argC){
	/* Timing variables to control task periodicity */
    int64_t fin_time=0, release_time=0;

	printk("Thread print init (periodic)\n");

	/* Compute next release instant */
    release_time = k_uptime_get() + thread_print_period;

	while(1){
		
		printk("%d  ",temp);
		printk("\n");
		printk("Temp Period: %d  ",thread_tempread_period);
		printk("\n");
		
		printk("%d ",but1);
		printk("%d ",but2);
		printk("%d ",but3);
		printk("%d ",but4);
		printk("\n");
		printk("But Period: %d  ",thread_but_period);
		printk("\n");

		printk("%d ",led0_state);
		printk("%d ",led1_state);
		printk("%d ",led2_state);
		printk("%d ",led3_state);
		printk("\n");
		printk("Led Period: %d  ",thread_led_period);
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

/** \brief Thread de leitura do estado dos butoes
 * 
 * Esta thread implementa a leitura do estado dos pinos correspondente a cada 1 dos 4 butoes existentes na placa
 * Esta thread é periodica e o seu periodo pode ser alterado pela introduçao de um comando
 * 
 * 
*/
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

/** \brief Thread de verificaçao/alteraçao do estado dos leds
 * 
 * Esta thread implementa a verificaçao se o estados dos led, output, precisa de ser alterado.
 * O estado dos leds pode ser alterado atraves da inserçao de comando via uart.
 * Esta thread é periodica e o seu periodo pode ser alterado pela introduçao de um comando
 * 
 * 
*/
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

/** \brief Thread de leitura da temperatura
 * 
 * Esta thread implementa a verificaçao se foi inserido um comando
 * e se o mesmo corresponde a um comando correto para a alteraçao do periodo das threads
 * ou alteraçao do estado dos leds.
 * Nesta implementaçao nao está disponivel a identificaçao de todas as possibilidades
 * de comandos mal inseridos, somente permite identificar os casos mais comuns de comando errados 
 * e verificar se mesmo a estrutura do comando esteja correta mas um dos char inserido nao for o esperado identifica 
 * como comando invalido
 * Esta thread é periodica e o seu periodo é estatico
 * 
*/
void thread_cmd_code(void *argA , void *argB, void *argC){

	/* Timing variables to control task periodicity */
    int64_t fin_time=0, release_time=0;

	printk("Thread tempread init (periodic)\n");

	/* Compute next release instant */
    release_time = k_uptime_get() + thread_cmd_period;

	while(1){
		
		if(cmd){
			res = cmdProcess();
			printk("\n\rcmdProcess output: %d\n\r", res);
			resetcmd();
			cmd=0;
		}

		/* Wait for next release instant */ 
        fin_time = k_uptime_get();
        if( fin_time < release_time) {
            k_msleep(release_time - fin_time); /* There are other variants, k_sleep(), k_usleep(), ... */
            release_time += thread_cmd_period;
		}
	}
	timing_stop();
}

/** \brief Configuraçao dos butoes
 * 
 *
 * 
*/
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

	/* configurar leds da placa */
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