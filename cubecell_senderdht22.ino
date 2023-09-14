// librerias para Lora
#include "LoRaWan_APP.h"
#include "Arduino.h"

// Librerias para el sensor
#include "DHT.h"

#define DHTPIN GPIO12

#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
float ValHum, ValTem = 0;

//lora configuacion

#ifndef LoraWan_RGB
#define LoraWan_RGB 0
#endif

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

float txNumber;
bool lora_idle=true;

void setup() {
 Serial.begin(115200);

 txNumber=0;


    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );


 
 
dht.begin();

}

void loop() {

  if(lora_idle == true)
  {
    delay(1000);

    ValHum = dht.readHumidity();
    ValTem = dht.readTemperature();

    if (isnan(ValHum) || isnan(ValTem)) {
      //ValHum = 0;
      //ValTem = 0;
    Serial.println(F("Error de lectura del sensor DHT21"));
    return;
  }
  Serial.print("Humedad: "); Serial.print(ValHum); Serial.print("% Temperatura: ");
  Serial.print(ValTem); Serial.print("°C");

  //envio de paquetes lora
sprintf(txpacket,"Hmd@%0.2f@Tmp@%0.2f",ValHum,ValTem); //comienza envio de paquete
Serial.printf("\r\nEnviando paquete \"%s\" , longitud %d\r\n",txpacket, strlen(txpacket));
turnOnRGB(COLOR_SEND,0); //change rgb color
Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out 
lora_idle = false;
  }
Radio.IrqProcess();
}

void OnTxDone( void )
{
  turnOffRGB();
  Serial.println("TX done......");
  lora_idle = true;
}

void OnTxTimeout( void )
{
  turnOffRGB();
  Radio.Sleep( );
  Serial.println("TX Timeout......");
  lora_idle = true;
}
