// a.	Digital I – push button to flip card
// b.	Digital I/O – debouncing push button
// c.	Digital O – Buzzer when snap is fucked / LED when snap is good / LED when snap
// d.	Analog Input – tilt sensor for snap bc tinker cad can suck my nuts
// e.	Analog Output (PWM) – hardware-based; OR software-based “bit-banging”
// f.	Serial I/O – log the reaction time to serial
// g.	LCD – scores and numbers
// h.	Timers – log reaction time to serial

// pressure sensor: 

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <math.h> 
#include <stdio.h> 
#include <inttypes.h>
#include <string.h>

//Change the values in these defines to reflect 
//  how you've hooked up the screen
//In 4-pin mode only DATA4:7 are used

#define LCD_USING_4PIN_MODE (1)

// #define LCD_DATA0_DDR (DDRD)
// #define LCD_DATA1_DDR (DDRD)
// #define LCD_DATA2_DDR (DDRD)
// #define LCD_DATA3_DDR (DDRD)
#define LCD_DATA4_DDR (DDRD)
#define LCD_DATA5_DDR (DDRD)
#define LCD_DATA6_DDR (DDRD)
#define LCD_DATA7_DDR (DDRD)


// #define LCD_DATA0_PORT (PORTD)
// #define LCD_DATA1_PORT (PORTD)
// #define LCD_DATA2_PORT (PORTD)
// #define LCD_DATA3_PORT (PORTD)
#define LCD_DATA4_PORT (PORTD)
#define LCD_DATA5_PORT (PORTD)
#define LCD_DATA6_PORT (PORTD)
#define LCD_DATA7_PORT (PORTD)

// #define LCD_DATA0_PIN (0)
// #define LCD_DATA1_PIN (1)
// #define LCD_DATA2_PIN (2)
// #define LCD_DATA3_PIN (3)
#define LCD_DATA4_PIN (4)
#define LCD_DATA5_PIN (5)
#define LCD_DATA6_PIN (6)
#define LCD_DATA7_PIN (7)


#define LCD_RS_DDR (DDRB)
#define LCD_ENABLE_DDR (DDRB)

#define LCD_RS_PORT (PORTB)
#define LCD_ENABLE_PORT (PORTB)

#define LCD_RS_PIN (1)
#define LCD_ENABLE_PIN (0)





//DATASHEET: https://s3-us-west-1.amazonaws.com/123d-circuits-datasheets/uploads%2F1431564901240-mni4g6oo875bfbt9-6492779e35179defaf4482c7ac4f9915%2FLCD-WH1602B-TMI.pdf

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

void lcd_init(void);
void lcd_write_string(uint8_t x, uint8_t y, char string[]);
void lcd_write_char(uint8_t x, uint8_t y, char val);


void lcd_clear(void);
void lcd_home(void);

void lcd_createChar(uint8_t, uint8_t[]);
void lcd_setCursor(uint8_t, uint8_t); 

void lcd_noDisplay(void);
void lcd_display(void);
void lcd_noBlink(void);
void lcd_blink(void);
void lcd_noCursor(void);
void lcd_cursor(void);
void lcd_leftToRight(void);
void lcd_rightToLeft(void);
void lcd_autoscroll(void);
void lcd_noAutoscroll(void);
void scrollDisplayLeft(void);
void scrollDisplayRight(void);

size_t lcd_write(uint8_t);
void lcd_command(uint8_t);


void lcd_send(uint8_t, uint8_t);
void lcd_write4bits(uint8_t);
void lcd_write8bits(uint8_t);
void lcd_pulseEnable(void);

uint8_t _lcd_displayfunction;
uint8_t _lcd_displaycontrol;
uint8_t _lcd_displaymode;

#define SET_BIT(reg, pin) (reg) |= (1 << (pin))
#define CLEAR_BIT(reg, pin) (reg) &= ~(1 << (pin))
#define WRITE_BIT(reg, pin, value) (reg) = (((reg) & ~(1 << (pin))) | ((value) << (pin)))
#define BIT_VALUE(reg, pin) (((reg) >> (pin)) & 1)
#define BIT_IS_SET(reg, pin) (BIT_VALUE((reg),(pin))==1)

//uart definitions
#define  RX_BUFFER_SIZE  64
#define  TX_BUFFER_SIZE  64
#define BAUD (9600)
#define UBRR (F_CPU/16/BAUD-1)
unsigned char rx_buf;
static volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
static volatile uint8_t tx_buffer_head;
static volatile uint8_t tx_buffer_tail;
static volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
static volatile uint8_t rx_buffer_head;
static volatile uint8_t rx_buffer_tail;


//initalise button states
int button_1_state, button_2_state, counter = 0;
unsigned char Serial_data; 

//runs whenver timer 0 overflows
ISR(TIMER0_OVF_vect) {
	// debounce button by checking its state every overflow (approx. 60kHz)
    // all buttons are wired to DDRD
    if (BIT_IS_SET(PIND, 3) ) {
        button_1_state = 1;
	}
    else
    {
        button_1_state = 0;
    }

    if (BIT_IS_SET(PIND, 2) ) {
        button_2_state = 1;
	}
    else
    {
        button_2_state = 0;
    }

    counter++;  
}

//Turn on LED (all LED's are wired to DDRB/PORTB)
void LED_on(int pin)
{
    SET_BIT(PORTB, pin);
}

//Turn off LED (all LED's are wired to DDRB/PORTB)
void LED_off(int pin)
{
    CLEAR_BIT(PORTB, pin);
}

//Turn on Buzzer
void BUZZ_on()
{
    SET_BIT(PORTB, 2);
}

//Turn off Buzzer
void BUZZ_off()
{
    CLEAR_BIT(PORTB, 2);
}

int read_sensor1()
{
  CLEAR_BIT(ADMUX, MUX0);
  //turn on ADSC
  ADCSRA |= (1 << ADSC);

	// Wait for conversion to complete.
	while ( ADCSRA & (1 << ADSC) ) {}

    //get value
    int out = ADC;
    
    return out;
}

int read_sensor2()
{
  SET_BIT(ADMUX,MUX0);
  //turn on ADSC
  ADCSRA |= (1 << ADSC);

	// Wait for conversion to complete.
	while ( ADCSRA & (1 << ADSC) ) {}

    //get value
    int out = ADC;
    
    return out;
}

int button_press(int pin)
{
    //all buttons are wired to DDRD
    if ( BIT_IS_SET(PIND, pin) ) {
        return 1;
	}
    return 0;
}

void PWM_output()
{
    //set duty cycle
    OCR0A |= 128;
    BUZZ_on();
    
}


uint16_t time(int overflow_count)
{
    return (overflow_count * 256.0 + TCNT0) * 1024.0 / 16000000.0;
}

unsigned char serial_get_character()
{
    while ( !(UCSR0A & (1<<RXC0)) ); // wait for data to be received

    return UDR0;
}

void serial_put_character(uint8_t character)
{
    // while buffer is full
    while (!( UCSR0A & (1<<UDRE0)));

    //send data
    UDR0 = character;
}

void serial_put_string(int len, char input[])
{
  //while(*input > 0) serial_put_character(*input++);
  for (int i = 0; i < (len-1); i++)
    {
        serial_put_character(input[i]);
    }
}

// Reverses a string 'str' of length 'len' 
void reverse(char* str, int len) 
{ 
    int i = 0, j = len - 1, temp; 
    while (i < j) { 
        temp = str[i]; 
        str[i] = str[j]; 
        str[j] = temp; 
        i++; 
        j--; 
    } 
} 
  
// Converts a given integer x to string str[].  
// d is the number of digits required in the output.  
// If d is more than the number of digits in x,  
// then 0s are added at the beginning. 
int intToStr(int x, char str[], int d) 
{ 
    int i = 0; 
    while (x) { 
        str[i++] = (x % 10) + '0'; 
        x = x / 10; 
    } 
  
    // If number of digits required is more, then 
    // add 0s at the beginning 
    while (i < d) 
        str[i++] = '0'; 
  
    reverse(str, i); 
    str[i] = '\0'; 
    return i; 
} 
  
// Converts a floating-point/double number to a string. 
void ftoa(float n, char* res, int afterpoint) 
{ 
    // Extract integer part 
    int ipart = (int)n; 
  
    // Extract floating part 
    float fpart = n - (float)ipart; 
  
    // convert integer part to string 
    int i = intToStr(ipart, res, 0); 
  
    // check for display option after point 
    if (afterpoint != 0) { 
        res[i] = '.'; // add dot 
  
        // Get the value of fraction part upto given no. 
        // of points after dot. The third parameter  
        // is needed to handle cases like 233.007 
        fpart = fpart * pow(10, afterpoint); 
  
        intToStr((int)fpart, res + i + 1, afterpoint); 
    } 
} 

void setup() 
{
    //### Setup Outputs ###//
    // Setup LED outputs
    SET_BIT(DDRB,2);
    SET_BIT(DDRB,3);

    // Setup Buzzer Output
    SET_BIT(DDRB,2);

    //### Setup Digital Inputs ###//
    //Setup Button Input
    CLEAR_BIT(DDRD,2);
    CLEAR_BIT(DDRD,3);

    //### setup analoug input (potentiome/pressure sensor) ###//
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADMUX = (1 << REFS0);

    //### Setup Debouncing Timer ###//
    CLEAR_BIT(TCCR0B,WGM02);
    SET_BIT(TCCR0B,CS02);
    CLEAR_BIT(TCCR0B,CS01);
    SET_BIT(TCCR0B,CS00);
    SET_BIT(TIMSK0, TOIE0);
    
    //### Setup Timer for PWM (bit bashing) ###//
    CLEAR_BIT(TCCR2B,WGM22);
    SET_BIT(TCCR2B,CS22);
    CLEAR_BIT(TCCR2B,CS21);
    SET_BIT(TCCR2B,CS20);
    SET_BIT(TIMSK2, TOIE2);

    //enable overflow and interupts
    sei();

    //### Setup UART ###//
    UBRR0H = (unsigned char)(UBRR>>8);
    UBRR0L = (unsigned char)(UBRR);
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C =(3 << UCSZ00);

    //## Setup LCD ##//
    // set up the LCD in 4-pin or 8-pin mode
    lcd_init();

    serial_put_string(15,"Setup Complete");
    serial_put_character(10);
}



char rand_card()
{
    return (rand() % (4)) + 48;
}

void welcome()
{
    //Print welcome message
    lcd_write_string(2,0,"Welcome to:");
    lcd_write_string(4,1,"SNAP!!!!");
  	_delay_ms(3000);
  	lcd_clear();	
  
    //instructions
    lcd_write_string(0,0,"Press the button");
    lcd_write_string(2,1,"to flip card");
  	_delay_ms(3000);
  	lcd_clear();
}

void display_score(char score, int player)
{
    if (player == 1)
    {
        lcd_write_char(0,0,score);
    }
    if (player == 2)
    {
        lcd_write_char(14,0,score);
    }
}

void display_card(char num)
{    
    lcd_write_char(12,1,'~');
    _delay_ms(300);
    lcd_write_char(12,1,num);
}


void titles()
{

  	lcd_write_string(2,0,"<# of Cards>");
  	lcd_write_string(2,1,"Current #");
   
}

void startup()
{
      welcome();
    
  	  //titles();
}



int check_snap(char current_card, char last_card)
  {
    if (current_card == last_card)
    {
      return 1;
    }
    return 0;
  }

void game()
{
  char player1_score = '9';
  char player2_score = '9';
  float show_time = 0;
  float press_time = 0;
  char reaction_time[64];
  char last_card = 0;
  char current_card = 0;

    startup();
    lcd_clear();
	//game loop
  	while (1)
    {
      
      titles();
      display_score(player1_score, 1);
      display_score(player2_score, 2);
      //see if player two has pressed thier button
      if (button_2_state)
      {
        player1_score -= 1;
        last_card = current_card;
        current_card = rand_card();
        display_card(current_card);
        show_time = time(counter);
      }
      
      //see if player two has pressed their button
      if (button_1_state)
      {
        player2_score -= 1;
        last_card = current_card;
        current_card = rand_card();
        display_card(current_card);
        show_time = time(counter);   
      }

      if (player1_score == '0')
      {
        lcd_clear();
        lcd_write_string(2,0,"Player 1");
        lcd_write_string(3,1,"Wins!!!");
        _delay_ms(3000);
        game();
      }
      if (player2_score == '0')
      {
        lcd_clear();
        lcd_write_string(2,0,"Player 2");
        lcd_write_string(4,1,"Wins!!!");
        _delay_ms(3000);
        game();
      }

      //check if player has 'snapped'
      if (read_sensor2() >=900)
      {
        press_time = time(counter);

        lcd_clear();
        lcd_write_string(2,0,"PLAYER 1");
        lcd_write_string(2,1,"SNAPPED!!!");
        serial_put_string(15,"Reaction time");
        serial_put_character(10);
        intToStr((press_time-show_time), reaction_time, 2);        
        serial_put_string(3,(char*) reaction_time);
        serial_put_string(5,"00ms");
        serial_put_character(10);

        if (check_snap(current_card,last_card))
        {
          LED_on(4);
          player2_score = '9';
        }
        else
        {
          PWM_output();
          _delay_ms(1000);
          BUZZ_off();
          player1_score = '9';
        }

        _delay_ms(3000);
      }
      if (read_sensor1() >= 900)
      {
        lcd_clear();
        lcd_write_string(2,0,"PLAYER 2");
        lcd_write_string(2,1,"SNAPPED!!!");
        serial_put_string(15,"Reaction time");
        serial_put_character(10);
        intToStr((press_time-show_time), reaction_time, 2);        
        serial_put_string(3,(char*) reaction_time);
        serial_put_string(5,"00ms");
        serial_put_character(10);

        if (check_snap(current_card,last_card))
        {
          LED_on(3);
          player1_score = '9';
        }
        else
        {
          PWM_output();
          _delay_ms(1000);
          BUZZ_off();
          player2_score = '9';
        }

        _delay_ms(3000);
        LED_off(3);
        LED_off(4);
      }
    }	
}

int main(void) {
    setup();   
    game(); 
}



//### LCD Function Definitions, retreived from : https://blackboard.qut.edu.au/bbcswebdav/pid-8642620-dt-content-rid-32682172_1/courses/CAB202_20se2/Topic11/CAB202%20Topic%2011%20%E2%80%93%20LCD.html
void lcd_init(void){
  //dotsize
  if (LCD_USING_4PIN_MODE){
    _lcd_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  } else {
    _lcd_displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
  }
  
  _lcd_displayfunction |= LCD_2LINE;

  // RS Pin
  LCD_RS_DDR |= (1 << LCD_RS_PIN);
  // Enable Pin
  LCD_ENABLE_DDR |= (1 << LCD_ENABLE_PIN);
  
  #if LCD_USING_4PIN_MODE
    //Set DDR for all the data pins
    LCD_DATA4_DDR |= (1 << LCD_DATA4_PIN);
    LCD_DATA5_DDR |= (1 << LCD_DATA5_PIN);
    LCD_DATA6_DDR |= (1 << LCD_DATA6_PIN);    
    LCD_DATA7_DDR |= (1 << LCD_DATA7_PIN);


#else
    //Set DDR for all the data pins
    LCD_DATA0_DDR |= (1 << LCD_DATA0_PIN);
    LCD_DATA1_DDR |= (1 << LCD_DATA1_PIN);
    LCD_DATA2_DDR |= (1 << LCD_DATA2_PIN);
    LCD_DATA3_DDR |= (1 << LCD_DATA3_PIN);
    LCD_DATA4_DDR |= (1 << LCD_DATA4_PIN);
    LCD_DATA5_DDR |= (1 << LCD_DATA5_PIN);
    LCD_DATA6_DDR |= (1 << LCD_DATA6_PIN);
    LCD_DATA7_DDR |= (1 << LCD_DATA7_PIN);
  #endif 

  // SEE PAGE 45/46 OF Hitachi HD44780 DATASHEET FOR INITIALIZATION SPECIFICATION!

  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
  _delay_us(50000); 
  // Now we pull both RS and Enable low to begin commands (R/W is wired to ground)
  LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
  LCD_ENABLE_PORT &= ~(1 << LCD_ENABLE_PIN);
  
  //put the LCD into 4 bit or 8 bit mode
  if (LCD_USING_4PIN_MODE) {
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    lcd_write4bits(0b0111);
    _delay_us(4500); // wait min 4.1ms

    // second try
    lcd_write4bits(0b0111);
    _delay_us(4500); // wait min 4.1ms
    
    // third go!
    lcd_write4bits(0b0111); 
    _delay_us(150);

    // finally, set to 4-bit interface
    lcd_write4bits(0b0010); 
  } else {
    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
    _delay_us(4500);  // wait more than 4.1ms

    // second try
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
    _delay_us(150);

    // third go
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
  }

  // finally, set # lines, font size, etc.
  lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);  

  // turn the display on with no cursor or blinking default
  _lcd_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  lcd_display();

  // clear it off
  lcd_clear();

  // Initialize to default text direction (for romance languages)
  _lcd_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

/********** high level commands, for the user! */
void lcd_write_string(uint8_t x, uint8_t y, char string[]){
  lcd_setCursor(x,y);
  for(int i=0; string[i]!='\0'; ++i){
    lcd_write(string[i]);
  }
}

void lcd_write_char(uint8_t x, uint8_t y, char val){
  lcd_setCursor(x,y);
  lcd_write(val);
}

void lcd_clear(void){
  lcd_command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  _delay_us(2000);  // this command takes a long time!
}

void lcd_home(void){
  lcd_command(LCD_RETURNHOME);  // set cursor position to zero
  _delay_us(2000);  // this command takes a long time!
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void lcd_createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  lcd_command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    lcd_write(charmap[i]);
  }
}


void lcd_setCursor(uint8_t col, uint8_t row){
  if ( row >= 2 ) {
    row = 1;
  }
  
  lcd_command(LCD_SETDDRAMADDR | (col + row*0x40));
}

// Turn the display on/off (quickly)
void lcd_noDisplay(void) {
  _lcd_displaycontrol &= ~LCD_DISPLAYON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}
void lcd_display(void) {
  _lcd_displaycontrol |= LCD_DISPLAYON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// Turns the underline cursor on/off
void lcd_noCursor(void) {
  _lcd_displaycontrol &= ~LCD_CURSORON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}
void lcd_cursor(void) {
  _lcd_displaycontrol |= LCD_CURSORON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// Turn on and off the blinking cursor
void lcd_noBlink(void) {
  _lcd_displaycontrol &= ~LCD_BLINKON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}
void lcd_blink(void) {
  _lcd_displaycontrol |= LCD_BLINKON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// These commands scroll the display without changing the RAM
void scrollDisplayLeft(void) {
  lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void scrollDisplayRight(void) {
  lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void lcd_leftToRight(void) {
  _lcd_displaymode |= LCD_ENTRYLEFT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

// This is for text that flows Right to Left
void lcd_rightToLeft(void) {
  _lcd_displaymode &= ~LCD_ENTRYLEFT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

// This will 'right justify' text from the cursor
void lcd_autoscroll(void) {
  _lcd_displaymode |= LCD_ENTRYSHIFTINCREMENT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

// This will 'left justify' text from the cursor
void lcd_noAutoscroll(void) {
  _lcd_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

/*********** mid level commands, for sending data/cmds */

inline void lcd_command(uint8_t value) {
  //
  lcd_send(value, 0);
}

inline size_t lcd_write(uint8_t value) {
  lcd_send(value, 1);
  return 1; // assume sucess
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void lcd_send(uint8_t value, uint8_t mode) {
  //RS Pin
  LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
  LCD_RS_PORT |= (!!mode << LCD_RS_PIN);

  if (LCD_USING_4PIN_MODE) {
    lcd_write4bits(value>>4);
    lcd_write4bits(value);
  } else {
    lcd_write8bits(value); 
  } 
}

void lcd_pulseEnable(void) {
  //Enable Pin
  LCD_ENABLE_PORT &= ~(1 << LCD_ENABLE_PIN);
  _delay_us(1);    
  LCD_ENABLE_PORT |= (1 << LCD_ENABLE_PIN);
  _delay_us(1);    // enable pulse must be >450ns
  LCD_ENABLE_PORT &= ~(1 << LCD_ENABLE_PIN);
  _delay_us(100);   // commands need > 37us to settle
}

void lcd_write4bits(uint8_t value) {
  //Set each wire one at a time

  LCD_DATA4_PORT &= ~(1 << LCD_DATA4_PIN);
  LCD_DATA4_PORT |= ((value & 1) << LCD_DATA4_PIN);
  value >>= 1;

  LCD_DATA5_PORT &= ~(1 << LCD_DATA5_PIN);
  LCD_DATA5_PORT |= ((value & 1) << LCD_DATA5_PIN);
  value >>= 1;

  LCD_DATA6_PORT &= ~(1 << LCD_DATA6_PIN);
  LCD_DATA6_PORT |= ((value & 1) << LCD_DATA6_PIN);
  value >>= 1;

  LCD_DATA7_PORT &= ~(1 << LCD_DATA7_PIN);
  LCD_DATA7_PORT |= ((value & 1) << LCD_DATA7_PIN);

  lcd_pulseEnable();
}

void lcd_write8bits(uint8_t value) {
  //Set each wire one at a time

  #if !LCD_USING_4PIN_MODE
    LCD_DATA0_PORT &= ~(1 << LCD_DATA0_PIN);
    LCD_DATA0_PORT |= ((value & 1) << LCD_DATA0_PIN);
    value >>= 1;

    LCD_DATA1_PORT &= ~(1 << LCD_DATA1_PIN);
    LCD_DATA1_PORT |= ((value & 1) << LCD_DATA1_PIN);
    value >>= 1;

    LCD_DATA2_PORT &= ~(1 << LCD_DATA2_PIN);
    LCD_DATA2_PORT |= ((value & 1) << LCD_DATA2_PIN);
    value >>= 1;

    LCD_DATA3_PORT &= ~(1 << LCD_DATA3_PIN);
    LCD_DATA3_PORT |= ((value & 1) << LCD_DATA3_PIN);
    value >>= 1;

    LCD_DATA4_PORT &= ~(1 << LCD_DATA4_PIN);
    LCD_DATA4_PORT |= ((value & 1) << LCD_DATA4_PIN);
    value >>= 1;

    LCD_DATA5_PORT &= ~(1 << LCD_DATA5_PIN);
    LCD_DATA5_PORT |= ((value & 1) << LCD_DATA5_PIN);
    value >>= 1;

    LCD_DATA6_PORT &= ~(1 << LCD_DATA6_PIN);
    LCD_DATA6_PORT |= ((value & 1) << LCD_DATA6_PIN);
    value >>= 1;

    LCD_DATA7_PORT &= ~(1 << LCD_DATA7_PIN);
    LCD_DATA7_PORT |= ((value & 1) << LCD_DATA7_PIN);
    
    lcd_pulseEnable();
  #endif
}

