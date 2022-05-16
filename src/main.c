/** \file main.c
* \brief Codigo da maquina de vendas
*
*  Este codigo simula um funcionamento de uma maquina de vendas
* que funciona atraves de uma maquina de estados
*
* \author Daniel Barra de Almeida        85111
* \author Marco  Antonio da Silva Santos 83192
* \date 08/05/2022
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
/* Pin at which LED is connected. Addressing is direct (i.e., pin number) */
#define BOARDLED1 0xD   /* P0.13 - Prod1 */
#define BOARDLED2 0xE   /* P0.14 - Prod2 */
#define BOARDLED3 0xF   /* P0.15 - Prod3 */
#define BOARDLED4 0x10  /* P0.16 - Buyable */

#define BOARDBUT1 0xB   /* P0.11 - UP */
#define BOARDBUT2 0xC   /* P0.12 - Select */
#define BOARDBUT3 0x18  /* P0.24 - Return */
#define BOARDBUT4 0x19  /* P0.25 - Down */

#define BOARDBUT5 0x1C  /* P0.28 - 0.1 EUR */
#define BOARDBUT6 0x1D  /* P0.29 - 0.2 EUR */
#define BOARDBUT7 0x1E  /* P0.30 - 0.5 EUR */
#define BOARDBUT8 0x1F  /* P0.31 - 1.0 EUR */

#define BEER 0
#define TUNA 1
#define COFFE 2
#define TRYBUY 3
#define MONEYDELIVER 4



int last_state;
int state = BEER;         /*Machine Initial State*/
int money_cent = 0;       /*Money inserted*/
bool but1 = false, but2 = false, but3 = false, but4 = false, but5 = false, but6 = false, but7 = false, but8 = false; /* Flags butoes primidos */
bool buyed = false;       /* Produto comprado com sucesso */
bool no_money = false;    /* Nao tem dinheiro suficiente */
bool delivering_money = false;  /* Devolver o dinheiro */
int prod_price_cent;      /* Preço do produto em centimos */
int prod_d[3]= {0, 0, 0}; /* Contagem de produtos vendidos no total */
char product_names[3][20]={"Cerveja", "Sandwich Tuna", "Cafe"};

const struct device *gpio0_dev;         /* Pointer to GPIO device structure */

/* Int related declarations */
static struct gpio_callback but1_cb_data; /* Callback structure */

/** \brief  Callback function.
*  E chamada sempre que um botao e primido.
*
* Sempre que um botao e pressionado esta funcao ativa
* uma flag para sinalizar que acoes tem de ser efetuadas 
*/

/* Callback function and variables*/
void but1press_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins){

   /* Test each button ...*/
    if(BIT(BOARDBUT1) & pins) {
        /* Update global var*/
        but1 = true; 
    }

    if(BIT(BOARDBUT2) & pins) {
        /* Update global var*/        
        but2 = true;
    }

    if(BIT(BOARDBUT3) & pins) {
        /* Update global var*/        
        but3 = true;
    }

    if(BIT(BOARDBUT4) & pins) {
        /* Update global var*/        
        but4 = true;
    }
    if(BIT(BOARDBUT5) & pins) {
        /* Update global var*/
        but5 = true; 
    }

    if(BIT(BOARDBUT6) & pins) {
        /* Update global var*/        
        but6 = true;
    }

    if(BIT(BOARDBUT7) & pins) {
        /* Update global var*/        
        but7 = true;
    }

    if(BIT(BOARDBUT8) & pins) {
        /* Update global var*/        
        but8 = true;
    }
}





/** \brief  Input Checker function
*  Esta funcao verifica se foram recebidos comandos
*
* Analisa que butoes foram primidos e altera
* variaveis necessarias correspondentes
*/
void checkInputs(void){

  if (but5){            /* BOARDBUT5 - P0.28 - Insert 0.1 EUR */
    but5 = false;
    money_cent += 10;
  }

  if (but6){            /* BOARDBUT6 - P0.29 - Insert 0.2 EUR */
    but6 = false;
    money_cent+=20;
  }

  if (but7){            /* BOARDBUT7 - P0.30 - Insert 0.5 EUR */
    but7 = false;
    money_cent+=50;
  }

  if (but8){            /* BOARDBUT8 - P0.31 - Insert 1.0 EUR */
    but8 = false;
    money_cent+=100;
  }
}

void update_terminal(void){
  printk("\x1b[H");   // Send cursor to home
  printf("\x1B[?25l");   // Hide cursor
  printk("-------------------------------------\n");
  printk("Maquina de Vendas feita por:\n");
  printk(" Daniel Barra de Almeida        85111\n");
  printk(" Marco  Antonio da Silva Santos 83192\n");
  printk("-------------------------------------\n");
  printk("---------------BEM VINDO-------------\n");
  printk("-------------------------------------\n");

  printk("      Produto: %s\x1b[0K\n",product_names[state]);
  printk("      Preco: %d.%d EUR\x1b[0K\n",prod_price_cent/100,prod_price_cent%100);
  
  if (buyed){
    buyed = false;
    printk("Entregando %s, dinheiro restante: %d.%d EUR\x1b[0K\n", product_names[state], money_cent/100, money_cent%100);
    prod_d[state]++;
    k_msleep(2000);
  }
  else if(no_money){
    no_money = false;
    printk("Sem dinheiro suficiente. %s custa %d.%d EUR,tem: %d.%d EUR\x1b[0K\n", product_names[state],prod_price_cent/100,prod_price_cent%100, money_cent/100, money_cent%100);
    k_msleep(2000);
  }
  else{
    printk("        Dinheiro disponivel: %d.%d EUR\x1b[0K\n",money_cent/100,money_cent%100);
  }
  
  printk("-------------------------------------\x1b[0K\n");
  printk("Total de produtos Entregues:\x1b[0K\n");
  printk("                    Cerveja:%d\x1b[0K\n",prod_d[0]);
  printk("                    Sandwich Tunaa:%d\x1b[0K\n",prod_d[1]);
  printk("                    Cafe:%d\x1b[0K\n",prod_d[2]);
  printk("-------------------------------------\n");
  printk("\x1b[0J"); /* Clear from cursor to end of screen*/


  if (delivering_money){
    delivering_money = false;
    printk("Devolvendo: %d.%d EUR\x1b[0K\n",money_cent/100,money_cent%100);
    printk("Obrigado e volte sempre!\x1b[0K\n");
    printk("-------------------------------------\n");
    money_cent=0;
    k_msleep(2000);    
  }


}

/** \brief  Configuration function
*  Configura a placa
*
* Esta funcao faz as configuracoes necessarias
* para a interface da placa ser utilizada adequadamente
*/
void config(void){

  /* Local vars */    

  int ret=0;                              /* Generic return value variable */


  /* Bind to GPIO 0 */
  gpio0_dev = device_get_binding(DT_LABEL(GPIO0_NID));
  if (gpio0_dev == NULL) {
      printk("Failed to bind to GPIO0\n\r");        
      return;
  }

  /* Configure PIN --------------------------------------------------------------------PIN-CONFIGS-------------------------------------------------*/
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

  ret = gpio_pin_configure(gpio0_dev, BOARDBUT5, GPIO_INPUT | GPIO_PULL_UP); // But 5
  if (ret < 0) {
      printk("Error %d: Failed to configure BUT 5 \n\r", ret);
      return;
  }

  ret = gpio_pin_configure(gpio0_dev, BOARDBUT6, GPIO_INPUT | GPIO_PULL_UP); // But 6
  if (ret < 0) {
      printk("Error %d: Failed to configure BUT 6 \n\r", ret);
      return;
  }

  ret = gpio_pin_configure(gpio0_dev, BOARDBUT7, GPIO_INPUT | GPIO_PULL_UP); // But 7
  if (ret < 0) {
      printk("Error %d: Failed to configure BUT 7 \n\r", ret);
      return;
  }

  ret = gpio_pin_configure(gpio0_dev, BOARDBUT8, GPIO_INPUT | GPIO_PULL_UP); // But 8
  if (ret < 0) {
      printk("Error %d: Failed to configure BUT 8 \n\r", ret);
      return;
  }

  /* Set interrupt HW - which pin and event generate interrupt -----------------------------INTERRUPT----------------------------------------------*/
  ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT1, GPIO_INT_EDGE_TO_ACTIVE); // BUT 1
  if (ret != 0) {
      printk("Error %d: failed to configure interrupt on BUT1 pin \n\r", ret);
      return;
  }

  ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT2, GPIO_INT_EDGE_TO_ACTIVE);  // BUT 2
  if (ret != 0) {
      printk("Error %d: failed to configure interrupt on BUT2 pin \n\r", ret);
      return;
  }

  ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT3, GPIO_INT_EDGE_TO_ACTIVE);  // BUT 3
  if (ret != 0) {
      printk("Error %d: failed to configure interrupt on BUT3 pin \n\r", ret);
      return;
  }

  ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT4, GPIO_INT_EDGE_TO_ACTIVE); // BUT 4
  if (ret != 0) {
      printk("Error %d: failed to configure interrupt on BUT4 pin \n\r", ret);
      return;
  }

  ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT5, GPIO_INT_EDGE_TO_ACTIVE); // BUT 5
  if (ret != 0) {
      printk("Error %d: failed to configure interrupt on BUT5 pin \n\r", ret);
      return;
  }


  ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT6, GPIO_INT_EDGE_TO_ACTIVE); // BUT 6
  if (ret != 0) {
      printk("Error %d: failed to configure interrupt on BUT6 pin \n\r", ret);
      return;
  }

  ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT7, GPIO_INT_EDGE_TO_ACTIVE); // BUT 7
  if (ret != 0) {
      printk("Error %d: failed to configure interrupt on BUT7 pin \n\r", ret);
      return;
  }

  ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT8, GPIO_INT_EDGE_TO_ACTIVE); // BUT 8
  if (ret != 0) {
      printk("Error %d: failed to configure interrupt on BUT8 pin \n\r", ret);
      return;
  }

   /* Set callback -----------------------------------------------------------------------CALLBACK------------------------------------------------*/
  gpio_init_callback(&but1_cb_data, but1press_cbfunction, BIT(BOARDBUT1)| BIT(BOARDBUT2)| BIT(BOARDBUT3) | BIT(BOARDBUT4)| BIT(BOARDBUT5)| BIT(BOARDBUT6) | BIT(BOARDBUT7) | BIT(BOARDBUT8));
  gpio_add_callback(gpio0_dev, &but1_cb_data);

}

/** \brief  main function
*  Funcao de funcionamento
*
* Esta e a funcao principal de funcionamento
*/
/* Main function */
void main(void) {
    config();
        
    printk("\x1b[2J"); /* Clear screen */

    /* Working loop */
    while(1) {
      checkInputs(); /* Verifica se houve inputs */


      switch(state){
        
        case BEER:                         /* Estado 0 - Cerveja */
          prod_price_cent = 100;

          gpio_pin_set(gpio0_dev, BOARDLED1, 0);
          gpio_pin_set(gpio0_dev, BOARDLED2, 1);
          gpio_pin_set(gpio0_dev, BOARDLED3, 1);
          
          if (money_cent >= prod_price_cent){   /* LED4 acende se for possivel comprar */
            gpio_pin_set(gpio0_dev, BOARDLED4, 0);
          }
          else {
            gpio_pin_set(gpio0_dev, BOARDLED4, 1);
          }
          if (but1){            /*go to TUNA state */
              but1 = false;
              state = TUNA;  
          }
          if (but3){              /* go to COFFE state */
            but3 = false;
            state = COFFE;            
          }
          if (but2){            /* go to TRYBUY state  */
            but2 = false;
            last_state = state;
            state = TRYBUY;
          }
          if (but4){            /* go to MONEYDELIVER state */
            but4 = false;
            last_state = state;
            state = MONEYDELIVER;      
          }


          break;

        case TUNA:                         /* Estado 1 - Sandwich Tuna */
          prod_price_cent = 150;

          gpio_pin_set(gpio0_dev, BOARDLED1, 1);
          gpio_pin_set(gpio0_dev, BOARDLED2, 0);
          gpio_pin_set(gpio0_dev, BOARDLED3, 1);
          
          if (money_cent >= prod_price_cent){   /* LED4 acende se for possivel comprar */
            gpio_pin_set(gpio0_dev, BOARDLED4, 0);
          }
          else {
            gpio_pin_set(gpio0_dev, BOARDLED4, 1);
          }

          if (but1){            /*go to COFFE state */
              but1 = false;
              state = COFFE;  
          }
          if (but3){              /* go to BEER state */
            but3 = false;
            state = BEER;            
          }
          if (but2){            /* go to TRYBUY state  */
            but2 = false;
            last_state = state;
            state = TRYBUY;
          }
          if (but4){            /* go to MONEYDELIVER state */
            but4 = false;
            last_state = state;
            state = MONEYDELIVER;      
          }

          break;


        case COFFE:                         /* Estado 2 - Cafe */
          prod_price_cent = 50;

          gpio_pin_set(gpio0_dev, BOARDLED1, 1);
          gpio_pin_set(gpio0_dev, BOARDLED2, 1);          
          gpio_pin_set(gpio0_dev, BOARDLED3, 0);
          if (money_cent >= prod_price_cent){   /* LED4 acende se for possivel comprar */
            gpio_pin_set(gpio0_dev, BOARDLED4, 0);
          }
          else {
            gpio_pin_set(gpio0_dev, BOARDLED4, 1);
          }
          if (but1){            /*go to BEER state */
              but1 = false;
              state = BEER;  
          }
          if (but3){              /* go to TUNA state */
            but3 = false;
            state = TUNA;            
          }
          if (but2){            /* go to TRYBUY state  */
            but2 = false;
            last_state = state;
            state = TRYBUY;
          }
          if (but4){            /* go to MONEYDELIVER state */
            but4 = false;
            last_state = state;
            state = MONEYDELIVER;      
          }

          break;

          case TRYBUY:
            if (money_cent >= prod_price_cent){
              money_cent = money_cent - prod_price_cent;
              buyed = true;     
            }
            else{
              no_money = true;  
            }
            update_terminal();
            state = last_state;

            break;

          case MONEYDELIVER:
            if (money_cent > 0){
              delivering_money = true;
            }
            update_terminal();
            state = last_state;

            break;
      }

      update_terminal();  
          

      //k_msleep(200); // Atraso para o terminal nao piscar demasiado
    }
    return;
}
