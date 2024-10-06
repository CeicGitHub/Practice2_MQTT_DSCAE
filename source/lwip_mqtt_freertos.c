//BY:CESAR_EDUARDO_INDA_CENICEROS_752964

#include "lwip/opt.h" // ¡This library needs to be here!
#if LWIP_IPV4 && LWIP_RAW && LWIP_NETCONN && LWIP_DHCP && LWIP_DNS
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
//#include <cstdlib>

//MAC_Configuration
#ifndef configMAC_ADDR
#define configMAC_ADDR                     \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x99 \
    }
#endif

#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS 		//Address_PHY interface
#define EXAMPLE_MDIO_OPS enet_ops 							//MDIO operations
#define EXAMPLE_PHY_OPS phyksz8081_ops			    		//PHY operations
#define EXAMPLE_CLOCK_FREQ CLOCK_GetFreq(kCLOCK_CoreSysClk) //ENET clock frequency

//LED_Configuration_PIN
#define BOARD_LED1_GPIO       BOARD_LED_RED_GPIO
#define BOARD_LED1_GPIO_PIN   BOARD_LED_RED_GPIO_PIN
#define BOARD_LED2_GPIO       BOARD_LED_GREEN_GPIO
#define BOARD_LED2_GPIO_PIN   BOARD_LED_GREEN_GPIO_PIN
#define BOARD_LED3_GPIO       BOARD_LED_BLUE_GPIO
#define BOARD_LED3_GPIO_PIN   BOARD_LED_BLUE_GPIO_PIN

//SW3_Button_Configuration
#define BOARD_SW_GPIO         BOARD_SW3_GPIO
#define BOARD_SW_GPIO_PIN     BOARD_SW3_GPIO_PIN
#define BOARD_SW_PORT         BOARD_SW3_PORT
#define BOARD_SW_IRQ          BOARD_SW3_IRQ
#define BOARD_SW_IRQ_HANDLER  BOARD_SW3_IRQ_HANDLER

#ifndef EXAMPLE_NETIF_INIT_FN  				   //Network interface initialization function
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init //EXAMPLE_NETIF_INIT_FN
#endif

#define EXAMPLE_MQTT_SERVER_HOST "broker.hivemq.com" // MQTT server host name or IP address
#define EXAMPLE_MQTT_SERVER_PORT 1883 				 //MQTT server port number
//#define EXAMPLE_MQTT_SERVER_PORT 8883 			//TLS PORT

#define INIT_THREAD_STACKSIZE 1024				//Stack size of the temporary lwIP initialization thread
#define INIT_THREAD_PRIO DEFAULT_THREAD_PRIO	//Priority of the temporary lwIP initialization thread
#define APP_THREAD_STACKSIZE 1024				//Stack size of the temporary initialization thread
#define APP_THREAD_PRIO DEFAULT_THREAD_PRIO		//Priority of the temporary initialization thread

static void connect_to_mqtt(void *ctx);

static mdio_handle_t mdioHandle = {.ops = &EXAMPLE_MDIO_OPS};
static phy_handle_t phyHandle   = {.phyAddr = EXAMPLE_PHY_ADDRESS, .mdioHandle = &mdioHandle, .ops = &EXAMPLE_PHY_OPS};

static mqtt_client_t *mqtt_client; 			//MQTT client data
static ip_addr_t mqtt_addr; 				//MQTT broker IP address
static char client_id[40];  				//MQTT client ID string
static volatile bool connected = false;		//Indicates connection to MQTT broker

// MQTT client information
static const struct mqtt_connect_client_info_t mqtt_client_info = {
    .client_id   = (const char *)&client_id[0],
    .client_user = "Ceic",
    .client_pass = "AEIOU123",
    .keep_alive  = 300,
    .will_topic  = NULL,
    .will_msg    = NULL,
    .will_qos    = 0,
    .will_retain = 0,

#if LWIP_ALTCP && LWIP_ALTCP_TLS
    .tls_config = NULL,
#endif
};

//Variables_Int
int temp = 0;
int temp_alarm = 0;
int fisura = 0;
int protection = 0;
int res1 = 0;
int res2 = 0;
int res3 = 0;
int tiempo = 0;
//Variables_Int

//Variables_Char
char dato[40] = "b";
char a[40] = "a";
char b[40] = "b";
char estado[40] = "b";
char t[40] = "0";
char zero[40] = "0";
//Variables_Char

volatile bool g_ButtonPress = false;


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

    gpio_pin_config_t sw_config = {     //Define the_Init structure for the input switch pin
        kGPIO_DigitalInput,
        0,
    };

    gpio_pin_config_t led_config = {	//Define the_Init structure for the output LED pin
        kGPIO_DigitalOutput,
        0,
    };

    PRINTF("Init_SW3 and LED pins\r\n"); // Print a note to terminal


    PORT_SetPinInterruptConfig(BOARD_SW_PORT, BOARD_SW_GPIO_PIN, kPORT_InterruptFallingEdge);
    EnableIRQ(BOARD_SW_IRQ);
    GPIO_PinInit(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, &sw_config);

    //Init_output LED GPIO
    LED_RED_INIT(0U);
    LED_GREEN_INIT(0U);
    LED_BLUE_INIT(0U);
    LED_RED_OFF();
    LED_GREEN_OFF();
    LED_BLUE_OFF();
}

// Called when subscription request finishes
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

// Called when there is a message on a subscribed topic
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    LWIP_UNUSED_ARG(arg);

    PRINTF("Received %u bytes from the topic \"%s\": \"", tot_len, topic);
}

//Called when recieved_incoming published message fragment.
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

//Subscribe to MQTT topics
static void mqtt_subscribe_topics(mqtt_client_t *client)
{
    char topic[40] = {0};
    err_t err;
    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb,
                            LWIP_CONST_CAST(void *, &mqtt_client_info));

    sprintf(topic, "Ceic/ARE YOU OK?");
    err = mqtt_subscribe(client, topic, 0, mqtt_topic_subscribed_cb, LWIP_CONST_CAST(void *, "I'M_ALIVE"));
    if (err == ERR_OK) PRINTF("Subscribing to the topic \"%s\" with QoS %d...\r\n", topic, 0);
    else PRINTF("Failed to subscribe to the topic \"%s\" with QoS %d: %d.\r\n", topic, 0, err);

    sprintf(topic, "Ceic/tiempo");
    err = mqtt_subscribe(client, topic, 0, mqtt_topic_subscribed_cb, LWIP_CONST_CAST(void *, "EMERGENCY CONTROL"));
    if (err == ERR_OK) PRINTF("Subscribing to the topic \"%s\" with QoS %d...\r\n", topic, 0);
    else PRINTF("Failed to subscribe to the topic \"%s\" with QoS %d: %d.\r\n", topic, 0, err);
}

//Called when connection state changes
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t *)arg;
    connected = (status == MQTT_CONNECT_ACCEPTED);

    switch (status)
    {
        case MQTT_CONNECT_ACCEPTED:
            PRINTF("MQTT client \"%s\" connected.\r\n", client_info->client_id);
            mqtt_subscribe_topics(client);
            break;

        case MQTT_CONNECT_DISCONNECTED:
            PRINTF("MQTT client \"%s\" not connected.\r\n", client_info->client_id);
            /* Try to reconnect 1 second later */
            sys_timeout(1000, connect_to_mqtt, NULL);
            break;

        case MQTT_CONNECT_TIMEOUT:
            PRINTF("MQTT client \"%s\" connection timeout.\r\n", client_info->client_id);
            /* Try again 1 second later */
            sys_timeout(1000, connect_to_mqtt, NULL);
            break;

        case MQTT_CONNECT_REFUSED_PROTOCOL_VERSION:
        case MQTT_CONNECT_REFUSED_IDENTIFIER:
        case MQTT_CONNECT_REFUSED_SERVER:
        case MQTT_CONNECT_REFUSED_USERNAME_PASS:

        case MQTT_CONNECT_REFUSED_NOT_AUTHORIZED_:
            PRINTF("MQTT client \"%s\" connection refused: %d.\r\n", client_info->client_id, (int)status);
            /* Try again 5 seconds later */
            sys_timeout(5000, connect_to_mqtt, NULL);
            break;

        default:
            PRINTF("MQTT client \"%s\" connection status: %d.\r\n", client_info->client_id, (int)status);
            /* Try again 5 seconds later */
            sys_timeout(5000, connect_to_mqtt, NULL);
            break;
    }
}

//Starts connecting to MQTT broker. To be called on tcpip_thread
static void connect_to_mqtt(void *ctx)
{
    LWIP_UNUSED_ARG(ctx);
    PRINTF("Connecting to MQTT broker at %s...\r\n", ipaddr_ntoa(&mqtt_addr));
    mqtt_client_connect(mqtt_client, &mqtt_addr, EXAMPLE_MQTT_SERVER_PORT, mqtt_connection_cb,
                        LWIP_CONST_CAST(void *, &mqtt_client_info), &mqtt_client_info);
}

//Called when publish request finishes
static void mqtt_message_published_cb(void *arg, err_t err)
{
    const char *topic = (const char *)arg;

    if (err == ERR_OK)
    {

    }
    else
    {
        PRINTF("Failed to publish to the topic \"%s\": %d.\r\n", topic, err);
    }
}

static void publish_temp(void *ctx)
{
	LWIP_UNUSED_ARG(ctx);
	char topic[40] = {0};
	char message[40] = {0};
	sprintf(topic, "Ceic/temp");
	sprintf(message, "%d", temp);
	PRINTF("Going to publish to the topic: \"%s\", message:  \"%s\"...\r\n", topic, message);
	mqtt_publish(mqtt_client, topic, message, strlen(message), 1, 0, mqtt_message_published_cb, (void *)topic);
	SDK_DelayAtLeastUs(10000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
}

static void publish_alarma_temp(void *ctx)
{
	LWIP_UNUSED_ARG(ctx);
	char topic[40] = {0};
	char message[40] = {0};
	sprintf(topic, "Ceic/alarma_temp");
	sprintf(message, "%d", temp_alarm);
	PRINTF("Going to publish to the topic: \"%s\", message:  \"%s\"...\r\n", topic, message);
	mqtt_publish(mqtt_client, topic, message, strlen(message), 1, 0, mqtt_message_published_cb, (void *)topic);
	SDK_DelayAtLeastUs(10000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
}


static void publish_fisura(void *ctx)
{
	LWIP_UNUSED_ARG(ctx);
	char topic[40] = {0};
	char message[40] = {0};
	sprintf(topic, "Ceic/fisura");
	sprintf(message, "%d", fisura);
   	PRINTF("Going to publish to the topic: \"%s\", message:  \"%s\"...\r\n", topic, message);
	mqtt_publish(mqtt_client, topic, message, strlen(message), 1, 0, mqtt_message_published_cb, (void *)topic);
	SDK_DelayAtLeastUs(10000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
}

static void publish_protection(void *ctx)
{
	LWIP_UNUSED_ARG(ctx);
	char topic[40] = {0};
	char message[40] = {0};
	sprintf(topic, "Ceic/protection");
	sprintf(message, "%d", protection);
	PRINTF("Going to publish to the topic: \"%s\", message:  \"%s\"...\r\n", topic, message);
	mqtt_publish(mqtt_client, topic, message, strlen(message), 1, 0, mqtt_message_published_cb, (void *)topic);
	SDK_DelayAtLeastUs(10000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
}


static void app_thread(void *arg)
{
    struct netif *netif = (struct netif *)arg;
    struct dhcp *dhcp;
    err_t err;
    int i;

    PRINTF("Getting IP address from DHCP...\r\n");    //Wait_for_address_DHCP

    do
    {
        if (netif_is_up(netif))
        {
            dhcp = netif_dhcp_data(netif);
        }
        else
        {
            dhcp = NULL;
        }

        sys_msleep(20U);

    } while ((dhcp == NULL) || (dhcp->state != DHCP_STATE_BOUND));

    PRINTF("\r\nIPv4 Address     : %s\r\n", ipaddr_ntoa(&netif->ip_addr));
    PRINTF("IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&netif->netmask));
    PRINTF("IPv4 Gateway     : %s\r\n\r\n", ipaddr_ntoa(&netif->gw));

    /*Check if there is an IP address or host configured, use the function "netconn_netconn_gethostbyname()"
     for both the IP address and the host name.*/

    if (ipaddr_aton(EXAMPLE_MQTT_SERVER_HOST, &mqtt_addr) && IP_IS_V4(&mqtt_addr))
    {
        err = ERR_OK;    //Already an IP address
    }
    else
    {
        /* Resolve MQTT broker's host name to an IP address */
        PRINTF("Resolving \"%s\"...\r\n", EXAMPLE_MQTT_SERVER_HOST);
        err = netconn_gethostbyname(EXAMPLE_MQTT_SERVER_HOST, &mqtt_addr);
    }

    if (err == ERR_OK)
    {
        /* Start connecting to MQTT broker from tcpip_thread */
        err = tcpip_callback(connect_to_mqtt, NULL);
        if (err != ERR_OK)
        {
            PRINTF("Failed to invoke broker connection on the tcpip_thread: %d.\r\n", err);
        }
    }
    else
    {
        PRINTF("Failed to obtain IP address: %d.\r\n", err);
    }

    init_button_and_led();
    srand(time(NULL));

    //Publish_some_Messages
    for (;;)
    {
        if (connected)
        {
        	temp = rand()%21;
        	PRINTF("Temperature: %d \r\n", temp);
            err = tcpip_callback(publish_temp, NULL);
            if (err != ERR_OK)
            {
                PRINTF("The 'temperature' topic wasn't published: %d.\r\n", err);
            }

        	if (temp > 10)
        	{
        		temp_alarm = 1;
                err = tcpip_callback(publish_alarma_temp, NULL);
                if (err != ERR_OK)
                {
                    PRINTF("The 'alarm_temperature' topic wasn't published:: %d.\r\n", err);
                }
        		PRINTF("Temperature out of the normal range!! \r\n");
        		LED_BLUE_ON();
                sys_msleep(4000U);
                LED_BLUE_OFF();
                temp_alarm = 0;
                err = tcpip_callback(publish_alarma_temp, NULL);
                if (err != ERR_OK)
                {
                    PRINTF("Failed to invoke publishing of a temperature alarm on the tcpip_thread: %d.\r\n", err);
                }
        	}

        	fisura = rand()%2;
        	PRINTF("Was a fisure_caused?? %d \r\n", fisura);
        	PRINTF("Yes:1 - No:0 \r\n");
        	if (fisura == 1)
			{
                err = tcpip_callback(publish_fisura, NULL);
                if (err != ERR_OK)
                {
                    PRINTF("Failed to invoke publishing of a fissure change on the tcpip_thread: %d.\r\n", err);
                }
        		PRINTF("A fissure was detected!! \r\n");
        		LED_RED_ON();
        		LED_BLUE_ON();
				LED_GREEN_ON();
				sys_msleep(4000U);
				LED_RED_OFF();
				LED_BLUE_OFF();
				LED_GREEN_OFF();
				fisura = 0;
			}
            err = tcpip_callback(publish_fisura, NULL);
            if (err != ERR_OK)
            {
                PRINTF("The fissure topic was not published: %d.\r\n", err);
            }

            PRINTF("protection? %d \r\n", protection);
    		err = tcpip_callback(publish_protection, NULL);
    		if (err != ERR_OK)
    		{
            PRINTF("Failed to invoke publishing of a message about protection on the tcpip_thread: %d.\r\n", err);
    		}

        	if(g_ButtonPress == true)
        	{
        		protection = 1;
        		err = tcpip_callback(publish_protection, NULL);
        		if (err != ERR_OK)
        		{
                PRINTF("Failed to invoke publishing of a message about protection on the tcpip_thread: %d.\r\n", err);
        		}
        		PRINTF("Protection Barrier failure!! \r\n");
				LED_RED_ON();
				LED_BLUE_ON();
				sys_msleep(4000U);
				LED_RED_OFF();
				LED_BLUE_OFF();
        		protection = 0;
        		err = tcpip_callback(publish_protection, NULL);
        		if (err != ERR_OK)
        		{
                PRINTF("The 'protection' topic was not published: %d.\r\n", err);
        		}

            	//Operate received topics
            	res1 = strcmp(dato, a);
            	res2 = strcmp(dato, b);
            	if((res1 == 0 ) || (res2 == 0)){
            		memcpy(estado, dato, 1);
            	}else{
            		memcpy(t, dato, 1);
            	}
            	res3 = strcmp(estado, b);

            	if(res3 != 0){
            		PRINTF("Door locked by the user \r\n");
    				tiempo = atoi(t);
    				PRINTF("The lock time is: %d \r\n", tiempo);
    				int i;
    				LED_RED_ON();
    				for(i=0; i< tiempo; i++){
    					sys_msleep(2000U);
    					PRINTF("Wait for unlock: %d \r\n", i+1);
    				}
    				LED_RED_OFF();

    				LED_GREEN_ON();
    				for(i=0; i< 2; i++){
    					sys_msleep(2000U);
    					PRINTF("Opening Time: %d \r\n", i+1);
    				}
    				LED_GREEN_OFF();
    				LED_RED_ON();
    				for(i=0; i< tiempo; i++){
    					sys_msleep(2000U);
    					PRINTF("Wait for unlock: %d \r\n", i+1);
    				}
    				LED_RED_OFF();
            	}
            	else{
            		PRINTF("Door_Unlocked \r\n");
            	}

        	}else{
        		protection = 0;
        	}
            PRINTF("\r\n");
            PRINTF("\r\n");
            PRINTF("\r\n");
            sys_msleep(5000U);
        }
    }
    vTaskDelete(NULL);
}

static void generate_client_id(void)
{
    uint32_t mqtt_id[MQTT_ID_SIZE];
    int res;

    get_mqtt_id(&mqtt_id[0]);

    res = snprintf(client_id, sizeof(client_id), "nxp_%08lx%08lx%08lx%08lx", mqtt_id[3], mqtt_id[2], mqtt_id[1],
                   mqtt_id[0]);
    if ((res < 0) || (res >= sizeof(client_id)))
    {
        PRINTF("snprintf failed: %d\r\n", res);
        while (1)
        {
        }
    }
}

static void stack_init(void *arg)
{
    static struct netif netif;
    ip4_addr_t netif_ipaddr, netif_netmask, netif_gw;
    ethernetif_config_t enet_config = {
        .phyHandle  = &phyHandle,
        .macAddress = configMAC_ADDR,
    };

    LWIP_UNUSED_ARG(arg);
    generate_client_id();

    mdioHandle.resource.csrClock_Hz = EXAMPLE_CLOCK_FREQ;

    IP4_ADDR(&netif_ipaddr, 0U, 0U, 0U, 0U);
    IP4_ADDR(&netif_netmask, 0U, 0U, 0U, 0U);
    IP4_ADDR(&netif_gw, 0U, 0U, 0U, 0U);

    tcpip_init(NULL, NULL);

    LOCK_TCPIP_CORE();
    mqtt_client = mqtt_client_new();
    UNLOCK_TCPIP_CORE();
    if (mqtt_client == NULL)
    {
        PRINTF("mqtt_client_new() failed.\r\n");
        while (1)
        {
        }
    }

    netifapi_netif_add(&netif, &netif_ipaddr, &netif_netmask, &netif_gw, &enet_config, EXAMPLE_NETIF_INIT_FN,
                       tcpip_input);
    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(&netif);

    netifapi_dhcp_start(&netif);

    PRINTF("\r\n************************************************\r\n");
    PRINTF(" MQTT client example\r\n");
    PRINTF("************************************************\r\n");

    if (sys_thread_new("app_task", app_thread, &netif, APP_THREAD_STACKSIZE, APP_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("stack_init(): Task creation failed.", 0);
    }

    vTaskDelete(NULL);
}

int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    SYSMPU_Type *base = SYSMPU;
    base->CESR &= ~SYSMPU_CESR_VLD_MASK; 	// Disable SYSMPU

    // Initialize lwIP from thread
    if (sys_thread_new("main", stack_init, NULL, INIT_THREAD_STACKSIZE, INIT_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("main(): Task creation failed.", 0);
    }

    vTaskStartScheduler();

    return 0;   //Will not get here unless a task calls vTaskEndScheduler
}
#endif
