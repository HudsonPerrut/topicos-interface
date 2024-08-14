#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>

#include <SPI.h>
#include "printf.h"
#include "RF24.h"

#define CE_PIN 7
#define CSN_PIN 8

#define MSG 0
#define ACK 1
#define RTS 2
#define CTS 3

#define TIMEOUT 1000 //milisegundos

static const float offset = 335.2; 
static const float ganho = 1.06154;

RF24 radio(CE_PIN, CSN_PIN);
uint64_t address[2] = { 0x3030303030LL, 0x3030303030LL};
 
byte payload[10] = {0,1,2,3,4,5,6,7,8,9};
byte payloadRx[10] = "          ";
uint8_t origem=26;
uint8_t indice=0;
uint8_t id_rede = 1;
uint8_t vtempInt,vtempDec;
float celsius;


void ADC_init() {
    ADMUX |= (1 << REFS1) | (1 << REFS0);   // Utiliza a referência interna de tensão de 1.1V
    ADCSRA |= (1 << ADEN);                  // Habilita o ADC
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);  // Define o prescaler do ADC como 128
}

// Função para ler o sensor de temperatura interno
uint16_t ler_temperatura() {
    // Seleciona o canal ADC 8 para o sensor de temperatura
    ADMUX = (ADMUX & 0xF0) | 0x08;

    ADCSRA |= (1 << ADSC);                  // Inicia a conversão ADC
    while (ADCSRA & (1 << ADSC));            // Aguarda a conclusão da conversão

    return ADC;
}

// Função para converter um valor float para uma string
void float_to_string(float valor, char* str) {
    int32_t inteira = (int32_t)valor;
    int32_t decimal = (int32_t)(valor * 100) % 100;

    if (valor < 0) {
        str[0] = '-';
        inteira = -inteira;
        decimal = -decimal;
    } else {
        str[0] = ' ';
    }

    str[1] = '0' + inteira / 10;
    str[2] = '0' + inteira % 10;
    str[3] = '.';

    str[4] = '0' + decimal / 10;
    str[5] = '0' + decimal % 10;
    str[6] = '\0';
}


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  radio.setPALevel(RF24_PA_MAX);  // RF24_PA_MAX is default.
  radio.setChannel(50);
  radio.setPayloadSize(sizeof(payload));  // float datatype occupies 4 bytes
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setDataRate(RF24_2MBPS);

  radio.openWritingPipe(address[0]);  // always uses pipe 0
  radio.openReadingPipe(1, address[1]);  // using pipe 1

  //For debugging info
  printf_begin();             // needed only once for printing details
  //radio.printDetails();       // (smaller) function that prints raw register values
  radio.printPrettyDetails(); // (larger) function that prints human readable data

   ADC_init();

}

void printPacote(byte *pac, int tamanho){
      Serial.print(F("Rcvd "));
      Serial.print(tamanho);  // print the size of the payload
      Serial.print(F(" O: "));
      Serial.print(pac[0]);  // print the payload's value
      Serial.print(F(" D: "));
      Serial.print(pac[1]);  // print the payload's value
      Serial.print(F(" C: "));
      Serial.print(pac[2]);  // print the payload's value
      Serial.print(F(" i: "));
      Serial.print(pac[3]);  // print the payload's value
      Serial.print(F(" : "));
      for(int i=4;i<tamanho;i++){
        Serial.print(pac[i]);
      }
      Serial.println();  // print the payload's value
}

bool aguardaMsg(int tipo){
    radio.startListening();
    unsigned long tempoInicio = millis();
    while(millis()-tempoInicio<TIMEOUT){
      if (radio.available()) {              // is there a payload? get the pipe number that recieved it
        uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
        radio.read(&payloadRx[0], bytes);             // fetch payload from FIFO
        if(payloadRx[1]==origem && payloadRx[3]==tipo){
          
          radio.stopListening();
          return true;
        }
      }
      radio.flush_rx();
      delay(10);      
    }
    radio.stopListening();
    return false;
}
  

bool sendPacket(byte *pacote, int tamanho, int destino, int controle){
    pacote[0]=id_rede;
    pacote[1]=destino;
    pacote[2]=origem;
    pacote[3]=controle;
    pacote[4]=indice;
    for(int i=0;i<tamanho;i++){
      Serial.print(pacote[i]);
    }
    Serial.println();
   
    while(1){
        
       radio.startListening();
       delayMicroseconds(50);
       radio.stopListening();
       if (!radio.testCarrier()) {
          return radio.write(&pacote[0], tamanho);
          
       }else{
        Serial.println("Meio Ocupado");
        delayMicroseconds(270);
       }
       radio.flush_rx();
    }
}


void loop() {

  uint16_t adc = ler_temperatura();

  // Conversão do valor ADC para temperatura em graus Celsius
  celsius = (adc - offset) / ganho;

 // char temperatura_str[7];
  //float_to_string(celsius, temperatura_str);


  //if (Serial.available()) {
    // change the role via the serial monitor

    //char c = toupper(Serial.read());
    //if (c == 'T') {
      // Become the TX node
      unsigned long start_timer = micros();                // start the timer
      bool report = sendPacket(&payload[0], sizeof(payload), 2, RTS);  // transmit & save the report
      report = aguardaMsg(CTS);
      if(report){
        vtempInt = (int32_t)celsius;
        vtempDec = (int32_t)(celsius * 100) % 100;

        payload[5] = vtempInt / 10;
        payload[6] = vtempInt % 10;

        payload[7] = vtempDec / 10;
        payload[8] = vtempDec % 10;

        if(celsius < 0) {
          payload[9] = 1;
        }else{
          payload[9] = 0;
        }
        -============Serial.print(celsius);
        Serial.println("\n");
        sendPacket(&payload[0], sizeof(payload), 2, MSG);
        report = aguardaMsg(ACK);
      }
      
      unsigned long end_timer = micros();                  // end the timer
      if(report){
         Serial.println("Sucesso!");
      }else{
         Serial.println("FALHA!");
      }

   //}
  //}

    radio.flush_rx();
    delay(2000);

}
