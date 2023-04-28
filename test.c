#include <mega164a.h>
#include <delay.h>
#include <spi.h>
#include <stdint.h>
#define NUM_DEVICES 2     // Number of cascaded max7219's, or just 1
#define DEL 600    // Delay for scrolling speed in microseconds

// Buffer array of bytes to store current display data for each column in each cascaded device
uint8_t buffer [NUM_DEVICES*8];


interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
// Reinitialize Timer 0 value
TCNT0=0x01;
// Place your code here

}
#include <io.h>
#include <delay.h>

#define MAX7219_DIN_PIN  PORTB5
#define MAX7219_CS_PIN   PORTB4
#define MAX7219_CLK_PIN  PORTB7

#define MAX7219_DECODE_MODE_REG  0x09
#define MAX7219_INTENSITY_REG    0x0A
#define MAX7219_SCAN_LIMIT_REG   0x0B
#define MAX7219_SHUTDOWN_REG     0x0C
#define MAX7219_DISPLAY_TEST_REG 0x0F

// Macros to set SS line LOW (selected) or HIGH (deselected)
#define SLAVE_SELECT    PORTB &= ~(1 << MAX7219_CS_PIN)
#define SLAVE_DESELECT  PORTB |= (1 << MAX7219_CS_PIN)

// Function Prototypes

void initSPI(void);

void writeByte(uint8_t byte);

void writeWord(uint8_t address, uint8_t data);

void initMatrix(void);

void clearMatrix(void);

void initBuffer(void);

void pushBuffer(uint8_t x);

void displayMessage(uint8_t c);

void displayBuffer(void);

// Initializes SPI
void initSPI(void)
{
  DDRB  |= (1 << PORTB4);        // Set SS output
  PORTB |= (1 << PORTB4);      // Begin high (unselected)

  DDRB |= (1 << MAX7219_DIN_PIN);       // Output on MOSI
  DDRB |= (1 << MAX7219_CLK_PIN);       // Output on SCK

  SPCR |= (1 << MSTR);      // Clockmaster
  SPCR |= (1 << SPE);       // Enable SPI
}

// Send byte through SPI
void writeByte(uint8_t byte)
{
  SPDR = byte;                      // SPI starts sending immediately
  while(!(SPSR & (1 << SPIF)));     // Loop until complete bit set
}
// Sends word through SPI
void writeWord(uint8_t address, uint8_t data)
{
  writeByte(address);    // Write first byte
  writeByte(data);      // Write Second byte
}
// Initializes all cascaded devices
void initMatrix()
{
    uint8_t i;    // Var used in for loops

    // Set display brighness
    SLAVE_SELECT;
    for(i = 0; i < NUM_DEVICES; i++)   // Loop through number of cascaded devices
    {
        writeByte(0x0A); // Select Intensity register
        writeByte(0x02); // Set brightness
    }
    SLAVE_DESELECT;


    // Set display refresh
    SLAVE_SELECT;
    for(i = 0; i < NUM_DEVICES; i++)
    {
        writeByte(0x0B); // Select Scan-Limit register
        writeByte(0x07); // Select columns 0-7
    }
    SLAVE_DESELECT;


    // Turn on the display
    SLAVE_SELECT;
    for(i = 0; i < NUM_DEVICES; i++)
    {
        writeByte(0x0C); // Select Shutdown register
        writeByte(0x01); // Select Normal Operation mode
    }
    SLAVE_DESELECT;


    // Disable Display-Test
    SLAVE_SELECT;
    for(i = 0; i < NUM_DEVICES; i++)
    {
        writeByte(0x0F); // Select Display-Test register
        writeByte(0x00); // Disable Display-Test
    }
    SLAVE_DESELECT;
}

// Clears all columns on all devices
void clearMatrix(void)
{
    uint8_t i,x;    // Var used in for loops
    for(x = 1; x < 9; x++) // for all columns
    {
        SLAVE_SELECT;
        for(i = 0; i < NUM_DEVICES; i++)
        {
            writeByte(x);    // Select column x
            writeByte(0x00); // Set column to 0
        }
        SLAVE_DESELECT;
    }
}

// Initializes buffer empty
void initBuffer(void)
{
    uint8_t i;    // Var used in for loops
    for(i = 0; i < NUM_DEVICES*8; i++)
        buffer[i] = 0x00;
}

// Moves each byte forward in the buffer and adds next byte in at the end
void pushBuffer(uint8_t x)
{
    uint8_t i;    // Var used in for loops
    for(i = 0; i < NUM_DEVICES*8 - 1; i++)
        buffer[i] = buffer[i+1];

    buffer[NUM_DEVICES*8 - 1] = x;
}

// Takes a pointer to the beginning of a char array holding message, and array size, scrolls message.
void displayMessage(uint8_t c)
{
        pushBuffer(c);                        // Add empty column after character for letter spacing
        displayBuffer();                        // Display &
        delay_ms(DEL);                         // Delay

}

// Displays current buffer on the cascaded devices
void displayBuffer()
{
    uint8_t i,j,k;    // Var used in for loops
   for(i = 0; i < NUM_DEVICES; i++) // For each cascaded device
   {
       for(j = 1; j < 9; j++) // For each column
       {
           SLAVE_SELECT;

           for(k = 0; k < i; k++) // Write Pre No-Op code
               writeWord(0x00, 0x00);

           writeWord(j, buffer[j + i*8 - 1]); // Write column data from buffer

           for(k = NUM_DEVICES-1; k > i; k--) // Write Post No-Op code
               writeWord(0x00, 0x00);

           SLAVE_DESELECT;
       }
   }
}


void main(void)
{
// Declare your local variables here
 uint8_t letter_a[] = {0b00011000, 0b00100100, 0b00100100, 0b00100100,0b01111110,0b00100100,0b00100100,0b00100100};
 //int letter_b[] = {0b01111100, 0b01000100, 0b01000100, 0b01111100, 0b01000100, 0b01000100, 0b01111100, 0b00000000};
 int i=0;
// Crystal Oscillator division factor: 1
#pragma optsize-
CLKPR=(1<<CLKPCE);
CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Input/Output Ports initialization
// Port A initialization
// Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
DDRA=(0<<DDA7) | (0<<DDA6) | (0<<DDA5) | (0<<DDA4) | (0<<DDA3) | (0<<DDA2) | (0<<DDA1) | (0<<DDA0);
// State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T
PORTA=(0<<PORTA7) | (0<<PORTA6) | (0<<PORTA5) | (0<<PORTA4) | (0<<PORTA3) | (0<<PORTA2) | (0<<PORTA1) | (0<<PORTA0);

// Port B initialization
// Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
DDRB=(0<<DDB7) | (0<<DDB6) | (0<<DDB5) | (0<<DDB4) | (0<<DDB3) | (0<<DDB2) | (0<<DDB1) | (0<<DDB0);
// State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T
PORTB=(0<<PORTB7) | (0<<PORTB6) | (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);

// Port C initialization
// Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
DDRC=(0<<DDC7) | (0<<DDC6) | (0<<DDC5) | (0<<DDC4) | (0<<DDC3) | (0<<DDC2) | (0<<DDC1) | (0<<DDC0);
// State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T
PORTC=(0<<PORTC7) | (0<<PORTC6) | (0<<PORTC5) | (0<<PORTC4) | (0<<PORTC3) | (0<<PORTC2) | (0<<PORTC1) | (0<<PORTC0);

// Port D initialization
// Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
DDRD=(0<<DDD7) | (1<<DDD6) | (0<<DDD5) | (0<<DDD4) | (0<<DDD3) | (0<<DDD2) | (0<<DDD1) | (0<<DDD0);
// State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T
PORTD=(0<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 20000,000 kHz
// Mode: Normal top=0xFF
// OC0A output: Toggle on compare match
// OC0B output: Disconnected
// Timer Period: 0,01275 ms
// Output Pulse(s):
// OC0A Period: 0,0255 ms Width: 0,01275 ms
TCCR0A=(0<<COM0A1) | (1<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
TCCR0B=(0<<WGM02) | (0<<CS02) | (0<<CS01) | (1<<CS00);
TCNT0=0x01;
OCR0A=0x00;
OCR0B=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: Timer1 Stopped
// Mode: Normal top=0xFFFF
// OC1A output: Disconnected
// OC1B output: Disconnected
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (0<<CS10);
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer2 Stopped
// Mode: Normal top=0xFF
// OC2A output: Disconnected
// OC2B output: Disconnected
ASSR=(0<<EXCLK) | (0<<AS2);
TCCR2A=(0<<COM2A1) | (0<<COM2A0) | (0<<COM2B1) | (0<<COM2B0) | (0<<WGM21) | (0<<WGM20);
TCCR2B=(0<<WGM22) | (0<<CS22) | (0<<CS21) | (0<<CS20);
TCNT2=0x00;
OCR2A=0x00;
OCR2B=0x00;

// Timer/Counter 0 Interrupt(s) initialization
TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (1<<TOIE0);

// Timer/Counter 1 Interrupt(s) initialization
TIMSK1=(0<<ICIE1) | (0<<OCIE1B) | (0<<OCIE1A) | (0<<TOIE1);

// Timer/Counter 2 Interrupt(s) initialization
TIMSK2=(0<<OCIE2B) | (0<<OCIE2A) | (0<<TOIE2);

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
// INT2: Off
// Interrupt on any change on pins PCINT0-7: Off
// Interrupt on any change on pins PCINT8-15: Off
// Interrupt on any change on pins PCINT16-23: Off
// Interrupt on any change on pins PCINT24-31: Off
EICRA=(0<<ISC21) | (0<<ISC20) | (0<<ISC11) | (0<<ISC10) | (0<<ISC01) | (0<<ISC00);
EIMSK=(0<<INT2) | (0<<INT1) | (0<<INT0);
PCICR=(0<<PCIE3) | (0<<PCIE2) | (0<<PCIE1) | (0<<PCIE0);

// USART0 initialization
// USART0 disabled
UCSR0B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (0<<RXEN0) | (0<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);

// USART1 initialization
// USART1 disabled
UCSR1B=(0<<RXCIE1) | (0<<TXCIE1) | (0<<UDRIE1) | (0<<RXEN1) | (0<<TXEN1) | (0<<UCSZ12) | (0<<RXB81) | (0<<TXB81);

// Analog Comparator initialization
// Analog Comparator: Off
// The Analog Comparator's positive input is
// connected to the AIN0 pin
// The Analog Comparator's negative input is
// connected to the AIN1 pin
ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIC) | (0<<ACIS1) | (0<<ACIS0);
ADCSRB=(0<<ACME);
// Digital input buffer on AIN0: On
// Digital input buffer on AIN1: On
DIDR1=(0<<AIN0D) | (0<<AIN1D);

// ADC initialization
// ADC disabled
ADCSRA=(0<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (0<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);


// TWI initialization
// TWI disabled
TWCR=(0<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWEN) | (0<<TWIE);

// Inits
  initSPI();
  initMatrix();
  clearMatrix();
  initBuffer();


// Globally enable interrupts
#asm("sei")
PORTD.6 = 1; // turn on LED

  // Event loop
  while (1)
  {
  PORTD.6 = !PORTD.6;

   displayMessage(letter_a[i]);    // Display the message

   if(i%8==0){ i=0; clearMatrix();
   }
         i+=1;
  }

}