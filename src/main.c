/*
 * Paulo Pedreiras, 2022/02
 * Simple Digital Output example
 * 
 * Toggles periodically LED1, which is internally connected to P0.13 
 *
 * Base documentation:
 *        
 *      HW info:
 *          https://infocenter.nordicsemi.com/topic/struct_nrf52/struct/nrf52840.html
 *          Section: nRF52840 Product Specification -> Peripherals -> GPIO / GPIOTE
 * 
 *          Board specific HW info can be found in the nRF52840_DK_User_Guide_20201203. I/O pins available at pg 27
 *
 *      Peripherals:
 *          https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/reference/peripherals/gpio.html 
 * 
 * 
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <string.h>
#include <timing/timing.h>
#include <stdio.h>


/* Refer to dts file */
#define GPIO0_NID DT_NODELABEL(gpio0) 
#define BOARDLED1 0xd /* Pin at which LED is connected. Addressing is direct (i.e., pin number) */
#define BOARDLED2 0xe
#define BOARDLED3 0xf
#define BOARDLED4 0x10

#define BOARDBUT1 0xb
#define BOARDBUT2 0xc
#define BOARDBUT3 0x18
#define BOARDBUT4 0x19
bool n1, n2, n3, n4;
int n;
int c1=0,c2=0,c3=0,c4=0;



/* Int related declarations */
static struct gpio_callback but1_cb_data, but2_cb_data, but3_cb_data, but4_cb_data; /* Callback structure */

/* Callback function and variables*/

void but1press_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins){ //-------------- BUT1 ISR
    
    /* MINUS */

    n++;

}

void but2press_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins){ //-------------- BUT2 ISR
    
    /* PLUS */

    n--;

}

void but3press_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins){ //-------------- BUT3 ISR
    
    /* CONFIRM */

    if (!n1){
      n1=!n1;
    }
    else if (!n2){
      n2=!n2;
    }
    else if (!n3){
      n3=!n3;
    }
    else if(!n4){
      n4=!n4;
    }
    else{
      return;
    }

}

void but4press_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins){ //-------------- BUT4 ISR
    
    /* RESET */
    printk("\x1b[2J\n");
    printk("RESETING...\r");
    if (!n1){
      n=0;
      n1=!n1;
      n2=!n2;
      n3=!n3;
      n4=!n4;
    }
    else if (!n2){
      n=0;
      n2=!n2;
      n3=!n3;
      n4=!n4;
    }
    else if (!n3){
      n=0;
      n3=!n3;
      n4=!n4;
    }
    else if(!n4){
      n=0;
      n4=!n4;
    }
    else{
      return;
    }

}




/* Main function */
void main(void) {

    /* Local vars */    
    const struct device *gpio0_dev;         /* Pointer to GPIO device structure */
    int ret=0;                              /* Generic return value variable */
    int ledstate=0;                         /* Led state var */



    /* Task init code */
    printk("ALL demo test config tool \n");

    /* Bind to GPIO 0 */
    gpio0_dev = device_get_binding(DT_LABEL(GPIO0_NID));
    if (gpio0_dev == NULL) {
        printk("Failed to bind to GPIO0\n\r");        
	return;
    }
    else {
        printk("Bind to GPIO0 successfull \n\r");        
    }
    
    /* Configure PIN --------------------------------------------------------------------PIN-CONFIG----------------------*/
    ret = gpio_pin_configure(gpio0_dev, BOARDLED1, GPIO_OUTPUT_ACTIVE); // LED 1
    if (ret < 0) {
        printk("gpio_pin_configure() failed with error %d\n\r", ret);        
	return;
    } 
    
    ret = gpio_pin_configure(gpio0_dev, BOARDLED2, GPIO_OUTPUT_ACTIVE);  // LED 2
    if (ret < 0) {
        printk("gpio_pin_configure() failed with error %d\n\r", ret);        
	return;
    } 
    
    ret = gpio_pin_configure(gpio0_dev, BOARDLED3, GPIO_OUTPUT_ACTIVE); // LED 3
    if (ret < 0) {
        printk("gpio_pin_configure() failed with error %d\n\r", ret);        
	return;
    } 
    
    ret = gpio_pin_configure(gpio0_dev, BOARDLED4, GPIO_OUTPUT_ACTIVE); // LED 4
    if (ret < 0) {
        printk("gpio_pin_configure() failed with error %d\n\r", ret);        
	return;
    }
    
    ret = gpio_pin_configure(gpio0_dev, BOARDBUT1, GPIO_INPUT | GPIO_PULL_UP); // But 1
    if (ret < 0) {
        printk("Error %d: Failed to configure BUT 1 \n\r", ret);
	return;
    }

    ret = gpio_pin_configure(gpio0_dev, BOARDBUT2, GPIO_INPUT | GPIO_PULL_UP); // But 2
    if (ret < 0) {
        printk("Error %d: Failed to configure BUT 2 \n\r", ret);
	return;
    }

    ret = gpio_pin_configure(gpio0_dev, BOARDBUT3, GPIO_INPUT | GPIO_PULL_UP); // But 3
    if (ret < 0) {
        printk("Error %d: Failed to configure BUT 3 \n\r", ret);
	return;
    }

    ret = gpio_pin_configure(gpio0_dev, BOARDBUT4, GPIO_INPUT | GPIO_PULL_UP); // But 4
    if (ret < 0) {
        printk("Error %d: Failed to configure BUT 4 \n\r", ret);
	return;
    }

    /* Set interrupt HW - which pin and event generate interrupt -----------------------------INTERRUPT----------------------------------------------*/
    ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT1, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
	printk("Error %d: failed to configure interrupt on BUT1 pin \n\r", ret);
        n++;
	return;
    }

     ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT2, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
	printk("Error %d: failed to configure interrupt on BUT2 pin \n\r", ret);
        n--;
	return;
    }

     ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT3, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
	printk("Error %d: failed to configure interrupt on BUT3 pin \n\r", ret);
        return;
    }

     ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT4, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
	printk("Error %d: failed to configure interrupt on BUT4 pin \n\r", ret);
	return;
    }

     /* Set callback -----------------------------------------------------------------------CALLBACK--------------------------------------------*/
    gpio_init_callback(&but1_cb_data, but1press_cbfunction, BIT(BOARDBUT1));
    gpio_add_callback(gpio0_dev, &but1_cb_data);

    gpio_init_callback(&but2_cb_data, but2press_cbfunction, BIT(BOARDBUT2));
    gpio_add_callback(gpio0_dev, &but2_cb_data);

    gpio_init_callback(&but3_cb_data, but3press_cbfunction, BIT(BOARDBUT3));
    gpio_add_callback(gpio0_dev, &but3_cb_data);

    gpio_init_callback(&but4_cb_data, but4press_cbfunction, BIT(BOARDBUT4));
    gpio_add_callback(gpio0_dev, &but4_cb_data);
        

    /* Blink loop */
    while(1) {  
        n1 = 0; n2 = 0; n3 = 0; n4 = 0;
        n = 0;
        
         
        gpio_pin_set(gpio0_dev, BOARDLED1, 1);
        gpio_pin_set(gpio0_dev, BOARDLED2, 1);
        gpio_pin_set(gpio0_dev, BOARDLED3, 1);
        gpio_pin_set(gpio0_dev, BOARDLED4, 1);             
        
        while (!n1){
          printk("\x1b[2J\n");
          printk("C1 -> %d  C2 -> %d  C3 -> %d  C4 -> %d\n\r",c1,c2,c3,c4);
          printk("Numero 1: %d\r",n);
          printk("Press BUT3 to enter\r");
          k_msleep(200);
        }
        c1 = n;
        gpio_pin_set(gpio0_dev, BOARDLED1, 0);

        while (!n2){
          printk("\x1b[2J\n");
          printk("C1 -> %d  C2 -> %d  C3 -> %d  C4 -> %d\n\r",c1,c2,c3,c4);
          printk("Numero 2: %d\r",n);
          printk("Press BUT3 to enter\r");
          k_msleep(200);
        }
        c2=n;
        gpio_pin_set(gpio0_dev, BOARDLED2, 0);

        while (!n3){
          printk("\x1b[2J\n");
          printk("C1 -> %d  C2 -> %d  C3 -> %d  C4 -> %d\n\r",c1,c2,c3,c4);
          printk("Numero 3: %d\r",n);
          printk("Press BUT3 to enter\r");
          k_msleep(200);
        }
        c3=n;
        gpio_pin_set(gpio0_dev, BOARDLED3, 0);

        while (!n4){
          printk("\x1b[2J\n");
          printk("C1 -> %d  C2 -> %d  C3 -> %d  C4 -> %d\n\r",c1,c2,c3,c4);
          printk("Numero 4: %d\r",n);
          printk("Press BUT3 to enter\r");
          k_msleep(200);
        }
        c4=n;
        gpio_pin_set(gpio0_dev, BOARDLED4, 0);    
        /* Toggle led status */
        if(c1==4 && c2==5 && c3==8 && c4==6){
          printk("\x1b[2J\n");
          printk("CODE USED: %d %d %d %d ---- HELL YEAH\r",c1,c2,c3,c4);
          printk("CONGRATS!!");
          k_msleep(10000);
        }
        else{
          
          printk("CODE USED: %d %d %d %d ---- WRONG!!!\r",c1,c2,c3,c4);
          printk("Try again in 5s");
          k_msleep(5000); 
        }        

    }
    
    return;
} 