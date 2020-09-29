#include "mbed.h"
#include "ESP8266Interface.h"

// Library to use https://github.com/ARMmbed/mbed-mqtt
#include <MQTTClientMbedOs.h>

int main()
{
    ESP8266Interface esp(MBED_CONF_APP_ESP_TX_PIN, MBED_CONF_APP_ESP_RX_PIN);
    
    //Store device IP
    SocketAddress deviceIP;
    //Store broker IP
    SocketAddress MQTTBroker;
    
    TCPSocket socket;
    MQTTClient client(&socket);
    
    printf("\nConnecting wifi..\n");

    int ret = esp.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);

    if(ret != 0)
    {
        printf("\nConnection error\n");
    }
    else
    {
        printf("\nConnection success\n");
    }

    esp.get_ip_address(&deviceIP);
    printf("IP via DHCP: %s\n", deviceIP.get_ip_address());
    
     // Use with IP
    //SocketAddress MQTTBroker(MBED_CONF_APP_MQTT_BROKER_IP, MBED_CONF_APP_MQTT_BROKER_PORT);
    
    // Use with DNS
    esp.gethostbyname(MBED_CONF_APP_MQTT_BROKER_HOSTNAME, &MQTTBroker, NSAPI_IPv4, "esp");
    MQTTBroker.set_port(MBED_CONF_APP_MQTT_BROKER_PORT);

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
    data.MQTTVersion = 3;
    char *id = MBED_CONF_APP_MQTT_ID;
    data.clientID.cstring = id;

    char buffer[64];
    sprintf(buffer, "Hello from Mbed OS %d.%d", MBED_MAJOR_VERSION, MBED_MINOR_VERSION);

    MQTT::Message msg;
    msg.qos = MQTT::QOS0;
    msg.retained = false;
    msg.dup = false;
    msg.payload = (void*)buffer;
    msg.payloadlen = strlen(buffer);

    socket.open(&esp);
    socket.connect(MQTTBroker);
    client.connect(data);
    
    while(1) {
        client.publish(MBED_CONF_APP_MQTT_TOPIC, msg);
        ThisThread::sleep_for(30000);
    }
    //client.yield(100);
    client.disconnect();

}


