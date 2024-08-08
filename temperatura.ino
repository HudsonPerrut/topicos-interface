#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

//Altere aqui para calibrar o sensor
static const float offset = 335.2; 
static const float ganho = 1.06154; 

//AUMENTE o valor do offset para DIMINUIR a temperatura
//DIMINUA o valor do offset para AUMENTAR a temperatura
//o valor de referencia do offset é 335.2 e para o ganho é 1.06154 

// Função para inicializar o ADC
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

// Função para inicializar a UART
void UART_init(uint16_t taxa_baud) {
    uint16_t valor_ubrr = F_CPU / (16UL * taxa_baud) - 1;

    // Define a taxa de baud rate
    UBRR0H = (uint8_t)(valor_ubrr >> 8);
    UBRR0L = (uint8_t)valor_ubrr;

    // Habilita a transmissor
    UCSR0B |= (1 << TXEN0);

    // Define o formato do frame: 8 bits de dados, 1 bit de parada, sem paridade
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}

// Função para transmitir um caractere via UART
void UART_transmitir(uint8_t dado) {
    while (!(UCSR0A & (1 << UDRE0)));    // Aguarda o buffer de transmissão vazio
    UDR0 = dado;                         // Coloca os dados no buffer e enviar
}

// Função para transmitir uma string via UART
void UART_transmitir_string(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        UART_transmitir(str[i]);
    }
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

int main(void) {
    ADC_init();                          // Inicializa o ADC
    UART_init(9600);                     // Inicializa o UART com taxa de baud rate 9600

    while (1) {
        uint16_t adc = ler_temperatura();

        // Conversão do valor ADC para temperatura em graus Celsius
        float celsius = (adc - offset) / ganho;

        char temperatura_str[7];
        float_to_string(celsius, temperatura_str);

        _delay_ms(5000);                   // Atraso de 1 segundo
        
        // Transmitir string de temperatura via UART
        UART_transmitir_string("Temperatura: ");
        UART_transmitir_string(temperatura_str);
        UART_transmitir('\n');

    }

    return 0;
}