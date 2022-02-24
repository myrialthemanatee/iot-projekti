//#include <iostream>
//#include <stdlib.h>
//#include <time.h>
#include "mbed.h"
// Library to use https://github.com/ARMmbed/mbed-mqtt
#include <MQTTClientMbedOs.h>
#include "ESP8266Interface.h"


const int sensorAddr = 0x4c << 1; //ox98

signed int x_out, y_out, z_out;  // values to print out as 16 bit or 32 integer, two's complement
signed int x_out_6bit, y_out_6bit, z_out_6bit;  // values to print out with 6 bit two's complement accuracy

I2C i2c(D0, D1);
// main() runs in its own thread in the OS

void convert32bit(unsigned int x_in, unsigned int y_in, unsigned int z_in);

// void tallentelu declaration

char command[2];
char data[3];

using namespace std;

char testing[64];


int x_recent[5] = {-12, 5, 15, -7, 6};
int y_recent[5] = {-3, 6, 8, -20, 14};
int z_recent[5] = {-9, 13, 9, -18, 1};

int x;
int y;
int z;
float sum_x;
float sum_y;
float sum_z;
float average_x;
float average_y;
float average_z;


int x_compare[5] = {-12, 5, 15, -7, 6};
int y_compare[5] = {-3, 6, 8, -20, 14};
int z_compare[5] = {-9, 13, 9, -18, 1};

int x1;
int y1;
int z1;
float sum_x_compare;
float sum_y_compare;
float sum_z_compare;
float average_x_compare;
float average_y_compare;
float average_z_compare;

void tallentelu();
void vertailu();
// arr x_recent[]; 5 viimeisintä
// arr y_recent[];
// arr z_recent[];

// boolean xyz;

// int x;
// int y;
// int z; 

int main()
{
    command[0] = 0x07;
    command[1] = 0x00;
    i2c.write(sensorAddr,command, 2);

    command[0] = 0x06;
    command[1] = 0x10;
    i2c.write(sensorAddr,command, 2);

    command[0] = 0x08;
    command[1] = 0x00;
    i2c.write(sensorAddr,command, 2);
    
    command[0] = 0x07;
    command[1] = 0x01;
    i2c.write(sensorAddr,command, 2);
    
    ThisThread::sleep_for(500ms);
    
    while (true) {
        unsigned int x,y,z;
        ThisThread::sleep_for(1000ms); // There will be new values every 1000ms
                    // or delay(10) every 10 ms


        i2c.write(0x00);  // register to read


        i2c.read(sensorAddr, data, 3);
        x = data[0];
        y = data[1];
        z = data[2];

        convert32bit(x,y,z);

        printf("x:         %d\n", x);
        printf("y:         %d\n", y);
        printf("z:         %d\n", z);
        
        printf("x_out: %d\n", x_out);
        printf("y_out: %d\n", y_out);
        printf("z_out: %d\n", z_out);

        x_out_6bit = x_out/67108864;
        y_out_6bit = y_out/67108864;
        z_out_6bit = z_out/67108864;

        printf("x_out_6bit:        %d\n", x_out_6bit);
        printf("y_out_6bit:        %d\n", y_out_6bit);
        printf("z_out_6bit:        %d\n", z_out_6bit);
    }
	tallentelu();
    vertailu();
    printf(testing, "perille tuli");
    
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
    char *id = (char*) MBED_CONF_APP_MQTT_ID;
    data.clientID.cstring = id;

    char buffer[64];
    //sprintf(buffer, "nyt testataan", MBED_MAJOR_VERSION, MBED_MINOR_VERSION);
    sprintf(buffer, "{\"d\":{\"Sensor\":\"Pot1 \",\"SampleNr\",\"VoltageScaled\"}}");
    
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
        
        // Sleep time must be less than TCP timeout
        // TODO: check if socket is usable before publishing
        ThisThread::sleep_for(30s);
    }
    //client.yield(100);
    client.disconnect();

}

void convert32bit(unsigned int x_in, unsigned int y_in, unsigned int z_in)
//void convert16bit(int x_in, int y_in, int z_in)
{
    signed int x_32;
    signed int y_32;
    signed int z_32;
    x_32 = x_in << 26;    // shift left the get the sign bit as leftmost bit
    y_32 = y_in << 26;
    z_32 = z_in << 26;

    x_out = x_32;   // the value. This time no mathematics
    y_out = y_32;   // 16 bit two's complement -2 147 483 648  .... 2 147 483 647
    z_out = z_32;   // 9,81 m/s2 = 1 g 
                //  1 g =  1 431 655 764‬ = 0101 0101 0101 0101 0101 0101 0101 0100
                // -1 g = -1 431 655 764‬ = 1010 1010 1010 1010 1010 1010 1010 1100
}

void tallentelu() {
    sum_x = 0;
    x = sizeof(x_recent) / sizeof(x_recent[0]);
    for (int i = 0; i < x; i++){
        sum_x += x_recent[i];
        average_x = sum_x/x;
        //cout<<"This is the number of integers in x \n"<<x;
        //cout<<"Average of all x elements is \n"<<average_x;
        }

    sum_y = 0;
    y = sizeof(y_recent) / sizeof(y_recent[0]);
    for (int j = 0; j < y; j++){
        sum_y += y_recent[j];
        average_y = sum_y/y;
        //cout<<"This is the number of integers in y \n"<<y;
        //cout<<"Average of all y elements is \n"<<average_y;
        }

    sum_z = 0;
    z = sizeof(z_recent) / sizeof(z_recent[0]);
    for (int k = 0; k < z; k++){
        sum_z += z_recent[k];
        average_z = sum_z/z;
        //cout<<"This is the number of integers in z \n"<<z;
        //cout<<"Average of all z elements is \n"<<average_z;
        }
}

void vertailu() {
    sum_x_compare = 0;
    x1 = sizeof(x_compare) / sizeof(x_compare[0]);
    for (int l = 0; l < x1; l++){
        sum_x_compare += x_compare[l];
        average_x_compare = sum_x_compare/x1;
        //cout<<"This is the number of integers in x1 \n"<<x1;
        //cout<<"Average of all x1 elements is \n"<<average_x_compare;
        }

    sum_y_compare = 0;
    y1 = sizeof(y_compare) / sizeof(y_compare[0]);
    for (int m = 0; m < y1; m++){
        sum_y_compare += y_compare[m];
        average_y_compare = sum_y_compare/y1;
        //cout<<"This is the number of integers in y1 \n"<<y1;
        //cout<<"Average of all y1 elements is \n"<<average_y_compare;
        }

    sum_z_compare = 0;
    z1 = sizeof(z_compare) / sizeof(z_compare[0]);
    for (int n = 0; n < z1; n++){
        sum_z_compare += z_compare[n];
        average_z_compare = sum_z_compare/z1;
        //cout<<"This is the number of integers in z1 \n"<<z1;
        //cout<<"Average of all z1 elements is \n"<<average_z_compare;
        }
    }