//Librerias Lorawan
#include "LoRaWan_APP.h"

#include "Arduino.h"

//agregadas nuevas para la prueba
#include <Wire.h>  
#include "HT_SSD1306Wire.h"
#include "images.h"


//Configuración de Parametros
#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;

int16_t txNumber;

int16_t rssi,rxSize;

bool lora_idle = true;

//agrego constructor
SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

//logo 




void setup() {
  Serial.begin(115200);
  Mcu.begin();
  factory_display.init(); // Inicializa la pantalla OLED
  factory_display.flipScreenVertically(); // Voltea la pantalla OLED (opcional)
  factory_display.setFont(ArialMT_Plain_16); // Establece la fuente de texto
  factory_display.drawString(0, 0, "Esperando dato"); // Muestra el mensaje "Hola mundo!" en la pantalla OLED
  factory_display.display(); // Muestra el contenido de la pantalla OLED
    
    txNumber=0;
    rssi=0;
  
    RadioEvents.RxDone = OnRxDone;
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                               LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                               LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                               0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

}

void loop() {
  if(lora_idle)
  {
    lora_idle = false;
    Serial.println("into RX mode");
    Radio.Rx(0);
  }
  Radio.IrqProcess( );

}



void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
  
rssi=rssi;
//rssi = "rssi: ";
//rssi += String(rssi,DEC);
rxSize=size;
memcpy(rxpacket, payload, size );
rxpacket[size]='\0';
Radio.Sleep( );
Serial.printf("\r\nPaquete Recibido \"%s\" con rssi %d , longitud %d\r\n",rxpacket,rssi,rxSize);
//lora_idle = true;

factory_display.clear(); // Limpia la pantalla OLED
//factory_display.drawString(0, 0, "Paquete Recibido:");
//factory_display.drawString(0, 12, rxpacket);
factory_display.drawString(0, 0, "RSSI:");
factory_display.drawString(0, 12, String(rssi));
factory_display.display(); // Muestra el contenido de la pantalla OLED
delay(2000);
factory_display.clear();

//Creo un array tipo byte de 3 numeros
byte pos_char[] = {0,0,0};
byte numChar = 1;

// recupero las posiciones de los caracteres @
for(byte i=0; i<rxSize; i++){
  if((rxpacket[i]=='@')&&(numChar == 3)){pos_char[2]= i,numChar=4;}
  if((rxpacket[i]=='@')&&(numChar == 2)){pos_char[1]= i,numChar=3;}
  if((rxpacket[i]=='@')&&(numChar == 1)){pos_char[0]= i,numChar=2;}
}
// una vez recuperados los caracteres procedo a recuperar los valores de humedad y temperatura
Serial.printf("\r\nnumChar1 \"%d\" numChar2 %d, numChar3 %d\r\n",pos_char[0],pos_char[1],pos_char[2]);

 String ValHmd;
 for (byte i=pos_char[0]+1; i<pos_char[1]; i++)
 {
  ValHmd = ValHmd + char(rxpacket[i]);
    
}
String ValTmp;
for(byte i=pos_char[2]+1; i<rxSize; i ++)
{
  ValTmp = ValTmp + char(rxpacket[i]);
}
//Convierto esos String a un valor entero con coma flotante
float f_ValHmd = ValHmd.toFloat();
float f_ValTmp = ValTmp.toFloat();

//imprimo los valores de humedad y temperatura
Serial.print("f_ValHmd: "); Serial.println(f_ValHmd);
Serial.print("f_ValTmp: "); Serial.println(f_ValTmp);

factory_display.clear();
factory_display.drawString(0, 0, "Hmd:");
factory_display.drawString(40, 0, String(f_ValHmd)); // 0,12
factory_display.drawString(0, 24, "Temp:");
factory_display.drawString(46, 24, String(f_ValTmp)); // 0.36
factory_display.display();
delay(3000);
factory_display.clear();


lora_idle = true;

}

