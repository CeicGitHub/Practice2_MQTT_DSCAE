#include "lwip/opt.h"

#if LWIP_IPV4 && LWIP_RAW && LWIP_NETCONN && LWIP_DHCP && LWIP_DNS

#include "lwip/altcp_tls.h" //FOR USE TLS

//Libraries
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_phy.h"
#include "lwip/api.h"
#include "lwip/apps/mqtt.h"
#include "lwip/dhcp.h"
#include "lwip/netdb.h"
#include "lwip/netifapi.h"
#include "lwip/prot/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"
#include "enet_ethernetif.h"
#include "lwip_mqtt_id.h"
#include "ctype.h"
#include "stdio.h"
#include "fsl_phyksz8081.h"
#include "fsl_enet_mdio.h"
#include "fsl_device_registers.h"
#include<stdio.h>
#include <stdlib.h>
#include <time.h>
//Libraries

//MAC Configuration
#ifndef configMAC_ADDR
#define configMAC_ADDR                     \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x12 \
    }
#endif

#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS //PHY_Address
#define EXAMPLE_PHY_OPS phyksz8081_ops				//PHY_operations
#define EXAMPLE_MDIO_OPS enet_ops  					//MDIO_Operations
#define EXAMPLE_CLOCK_FREQ CLOCK_GetFreq(kCLOCK_CoreSysClk) //ENET_Clock_Frequency

/* GPIO pin configuration. */
#define BOARD_LED1_GPIO       BOARD_LED_RED_GPIO
#define BOARD_LED1_GPIO_PIN   BOARD_LED_RED_GPIO_PIN
#define BOARD_LED2_GPIO       BOARD_LED_GREEN_GPIO
#define BOARD_LED2_GPIO_PIN   BOARD_LED_GREEN_GPIO_PIN
#define BOARD_LED3_GPIO       BOARD_LED_BLUE_GPIO
#define BOARD_LED3_GPIO_PIN   BOARD_LED_BLUE_GPIO_PIN
#define BOARD_SW_GPIO         BOARD_SW3_GPIO
#define BOARD_SW_GPIO_PIN     BOARD_SW3_GPIO_PIN
#define BOARD_SW_PORT         BOARD_SW3_PORT
#define BOARD_SW_IRQ          BOARD_SW3_IRQ
#define BOARD_SW_IRQ_HANDLER  BOARD_SW3_IRQ_HANDLER


#ifndef EXAMPLE_NETIF_INIT_FN //Interface_Initialization_Function
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif

#define EXAMPLE_MQTT_SERVER_HOST 	"broker.hivemq.com"	//MQTT_SERVER_HOSTNAME_OR_IP-ADDRESS
#define EXAMPLE_MQTT_SERVER_PORT  	 1883 //8883(TLS)			//MQTT_SERVER_PORT_NUMBER

#define INIT_THREAD_STACKSIZE 1024 					//Stack size of the temporary lwIP initialization
#define INIT_THREAD_PRIO DEFAULT_THREAD_PRIO		//Priority of the temporary lwIP initialization
#define APP_THREAD_STACKSIZE 1024					//Stack size of the temporary initialization
#define APP_THREAD_PRIO DEFAULT_THREAD_PRIO			//Priority of the temporary initialization

static void connect_to_mqtt(void *ctx);

static struct altcp_tls_config *tls_config;			//For Use TLS!!_NEW


//Mqtt_Client_Data_&_ID_String
static mdio_handle_t mdioHandle = {.ops = &EXAMPLE_MDIO_OPS};
static phy_handle_t phyHandle   = {.phyAddr = EXAMPLE_PHY_ADDRESS, .mdioHandle = &mdioHandle, .ops = &EXAMPLE_PHY_OPS};
static mqtt_client_t *mqtt_client;
static char client_id[40];

//MQTT_Client_Information
static const struct mqtt_connect_client_info_t mqtt_client_info = {
    .client_id   = (const char *)&client_id[0],
    .client_user = "Ceic_a",
    .client_pass = "ATceic9610290",
    .keep_alive  = 300,
    .will_topic  = NULL,
    .will_msg    = NULL,
    .will_qos    = 0,
    .will_retain = 0,

#if LWIP_ALTCP && LWIP_ALTCP_TLS
    .tls_config = NULL,
#endif
};

static ip_addr_t mqtt_addr; 				//MQTT broker IP address
static volatile bool connected = false;	   //Indicates connection to MQTT broker
volatile bool g_ButtonPress = false;

int temp = 0;
int temp_alarm = 0;
int liquido = 0;
int energia = 0;
char dato[40] = "b";
char a[40] = "a";
char b[40] = "b";
char estado[40] = "b";
char t[40] = "0";
char zero[40] = "0";
int res1 = 0;
int res2 = 0;
int res3 = 0;
int tiempo = 0;


void BOARD_SW_IRQ_HANDLER(void)
{
    /* Clear external interrupt flag. */
    GPIO_PortClearInterruptFlags(BOARD_SW_GPIO, 1U << BOARD_SW_GPIO_PIN);

    /* Change state of button. */
    if(g_ButtonPress == false){
    	g_ButtonPress = true;
    	SDK_ISR_EXIT_BARRIER;
    }else{
    	g_ButtonPress = false;
    	SDK_ISR_EXIT_BARRIER;
    }
}

static void init_button_and_led(void)
{
    /* Define the init structure for the input switch pin */
    gpio_pin_config_t sw_config = {
        kGPIO_DigitalInput,
        0,
    };

    /* Define the init structure for the output LED pin */
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput,
        0,
    };

    /* Print a note to terminal. */
    PRINTF("Init SW3 and LED pins\r\n");


    PORT_SetPinInterruptConfig(BOARD_SW_PORT, BOARD_SW_GPIO_PIN, kPORT_InterruptFallingEdge);
    EnableIRQ(BOARD_SW_IRQ);
    GPIO_PinInit(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, &sw_config);

    /* Init output LED GPIO. */
    LED_RED_INIT(0U);
    LED_GREEN_INIT(0U);
    LED_BLUE_INIT(0U);
    LED_RED_OFF();
    LED_GREEN_OFF();
    LED_BLUE_OFF();
}

//Called when subscription request finishes
static void mqtt_topic_subscribed_cb(void *arg, err_t err)
{
    const char *topic = (const char *)arg;

    if (err == ERR_OK)
    {
        PRINTF("Subscribed to the topic \"%s\".\r\n", topic);
    }
    else
    {
        PRINTF("Failed to subscribe to the topic \"%s\": %d.\r\n", topic, err);
    }
}

//Called when there is a message on a subscribed topic
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    LWIP_UNUSED_ARG(arg);

    PRINTF("Received %u bytes from the topic \"%s\": \"", tot_len, topic);
}

//Called when recieved incoming published message fragment
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    int i;

    LWIP_UNUSED_ARG(arg);

    for (i = 0; i < len; i++)
    {
        if (isprint(data[i]))
        {
            PRINTF("%c", (char)data[i]);
            dato[0] = data[0];
        }
        else
        {
            PRINTF("\\x%02x", data[i]);
            dato[0] = data[0];
        }
    }

    if (flags & MQTT_DATA_FLAG_LAST)
    {
        PRINTF("\"\r\n");
    }

}



int main(void)
{
    SYSMPU_Type *base = SYSMPU;
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    /* Disable SYSMPU. */
    base->CESR &= ~SYSMPU_CESR_VLD_MASK;

    /* Initialize lwIP from thread */
    if (sys_thread_new("main", stack_init, NULL, INIT_THREAD_STACKSIZE, INIT_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("main(): Task creation failed.", 0);
    }

    vTaskStartScheduler();

    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    return 0;
}
#endif

