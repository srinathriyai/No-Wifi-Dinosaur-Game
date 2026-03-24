
#ifndef SPIAVR_H
#define SPIAVR_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "serialATmega.h"
#include "helper.h"


#include <stdio.h>

//B5 should always be SCK(spi clock) and B3 should always be MOSI. If you are using an
//SPI peripheral that sends data back to the arduino, you will need to use B4 as the MISO pin.
//The SS pin can be any digital pin on the arduino. Right before sending an 8 bit value with
//the SPI_SEND() funtion, you will need to set your SS pin to low. If you have multiple SPI
//devices, they will share the SCK, MOSI and MISO pins but should have different SS pins.
//To send a value to a specific device, set it's SS pin to low and all other SS pins to high.

// Outputs, pin definitions
#define PIN_SCK                   PORTB5//SHOULD ALWAYS BE B5 ON THE ARDUINO
#define PIN_MOSI                  PORTB3//SHOULD ALWAYS BE B3 ON THE ARDUINO
#define PIN_SS                    PORTB2

//-----------   S O N G     N O T E S-------------

#define NOTE_D5  247
#define NOTE_C5  415
#define NOTE_A4  494
#define NOTE_E5  587
#define NOTE_B4  740
#define NOTE_REST 0

// ----------------For 4D7G display-----------------
int numbers[10] = { // a b c d e f g
               0b11111100, // 0
               0b01100000, // 1
               0b11011010, // 2
               0b11110010, // 3
               0b01100110, // 4
               0b10110110, // 5
               0b10111110, // 6
               0b11100000, // 7
               0b11111110, // 8
               0b11110110  // 9
};

#define SRCLK 4
#define RCLK 3
#define SER 2

// --------sends individual digits (0-9) to the shift register--------
void displayNumber(int num) {
    // Loop through the 8 bits of the number
    for (int i = 0; i < 8; i++) {
        // Send each bit of the number to DS (PD2)
        PORTD = SetBit(PORTD, SER, (numbers[num] >> i) & 0b1);  // DS = PD2
        // PORTD = SetBit(PORTD, SER, (0xaa >> i) & 0b1);  // DS = PD2

        // Toggle the SRCLK (Shift Clock - PD4)
        PORTD = SetBit(PORTD, SRCLK, 1);  // Set SRCLK high
        _delay_us(1);                 // Short delay
        PORTD = SetBit(PORTD, SRCLK, 0);  // Set SRCLK low
        _delay_us(1);                 // Short delay
    }

    // Toggle the STCP (Latch - PD3)
    PORTD = SetBit(PORTD, RCLK, 1);  // Set STCP high
    _delay_us(1);                 // Short delay
    PORTD = SetBit(PORTD, RCLK, 0);  // Set STCP low
    _delay_us(1);                 // Short delay
}

// void outNum(int num){
// 	PORTD = nums[num] << 1;
//   	PORTB = SetBit(PORTB, 1 ,nums[num]&0x01);
// }
// -------------------------------------------------

void SPI_INIT(){
    DDRB |= (1 << PIN_SCK) | (1 << PIN_MOSI) | (1 << PIN_SS);//initialize your pins. 
    SPCR |= (1 << SPE) | (1 << MSTR); //initialize SPI coomunication
}

void SPI_SEND(char data)
{
    SPDR = data;//set data that you want to transmit
    while (!(SPSR & (1 << SPIF)));// wait until done transmitting
}

void HardwareReset(){
    PORTB = SetBit(PORTB,2,0);
    _delay_ms(200);
    PORTB = SetBit(PORTB,2,1);
    _delay_ms(200);
}

void Send_Command(char x) {
    PORTB = SetBit(PORTB,0,0);
    SPI_SEND(x);
    PORTB = SetBit(PORTB,0,1);
}

void Send_Data(char x) {
    PORTB = SetBit(PORTB,0,1);
    SPI_SEND(x);
}

void ST7735_init(){
    HardwareReset();
    Send_Command(0x01); // swreset
    _delay_ms(150);
    Send_Command(0x11); // slpout
    _delay_ms(200);
    Send_Command(0x3A); // color mode
    Send_Data(0x06); //for 18 bit color mode. You can pick any color mode you want
    _delay_ms(10);
    Send_Command(0x29); // display on
    _delay_ms(200);
}

//-----------------------------------------C U S T O M--------------------------------------------

void set_column(uint16_t c_start, uint16_t c_end) {
    Send_Command(0x2A);
    Send_Data(0);
    Send_Data(c_start);
    Send_Data(0);
    Send_Data(c_end);
}

void set_row(uint16_t r_start, uint16_t r_end) {
    Send_Command(0x2B);
    Send_Data(0);
    Send_Data(r_start);
    Send_Data(0);
    Send_Data(r_end);
}

void send_color(int red, int green, int blue, int pixels) {
    Send_Command(0x2C);
  for (int i = 0; i < pixels; ++i) {
    Send_Data(blue);
    Send_Data(green);
    Send_Data(red);
  }
}

//  G A M E     S C R E E N S

void black_screen()
{
    //Background
    set_row(0,130);
    set_column(0,129);
    send_color(0, 0, 0, 16770); // black
}


void displayTitleScreen() {

    //------- DINO -------

    // Letter D
    set_row(25, 51);
    set_column(30, 33); 
    send_color(0, 255, 0, 92); // Left vertical line

    set_row(25, 28);
    set_column(30, 45); 
    send_color(0, 255, 0, 35); // Top horizontal line

    set_row(47, 50);
    set_column(30, 40); 
    send_color(0, 255, 0, 31); // Bottom horizontal line

    set_row(25, 51);
    set_column(42, 45); 
    send_color(0, 255, 0, 86); // Right vertical curve

    // Letter I
    set_row(25, 28);
    set_column(50, 60); 
    send_color(0, 255, 0, 44); // Top horizontal line

    set_row(46, 59);
    set_column(50, 60); 
    send_color(0, 255, 0, 44); // Bottom horizontal line

    set_row(29, 48);
    set_column(54, 57);
    send_color(0, 255, 0, 72); // Middle vertical line

    // Letter N
    set_row(25, 51);
    set_column(65, 68); 
    send_color(0, 255, 0, 100); // Left vertical line

    set_row(25, 51);
    set_column(82, 85); 
    send_color(0, 255, 0, 100); // Right vertical line

    for (int i = 0; i < 25; i++) { 
        set_row(25 + i, 25 + i);
        set_column(65 + i / 1.5, 68 + i / 1.5); 
        send_color(0, 255, 0, 35); // Diagonal line
    }

    // Letter O
    set_row(25, 28);
    set_column(90, 105);
    send_color(0, 255, 0, 35); // Top horizontal line

    set_row(47, 50);
    set_column(90, 105); 
    send_color(0, 255, 0, 48); // Bottom horizontal line

    set_row(25, 51);
    set_column(90, 93); 
    send_color(0, 255, 0, 92); // Left vertical line

    set_row(25, 51);
    set_column(102, 105); 
    send_color(0, 255, 0, 92); // Right vertical line


    //------- -> PLAY -------

    // Letter P
    set_row(65, 79);
    set_column(42, 44); 
    send_color(255, 255, 255, 81); // Left Vertical line

    set_row(65, 68);
    set_column(42, 49); 
    send_color(255, 255, 255, 18); // Top horizontal line

    set_row(71, 74);
    set_column(42, 49); 
    send_color(255, 255, 255, 18); // Middle horizontal line

    set_row(65, 79);
    set_column(49, 51); 
    send_color(255, 255, 255, 20); // Left Vertical line

    // Letter L
    set_row(65, 79);
    set_column(55, 57); 
    send_color(255, 255, 255, 81); // Left Vertical line

    set_row(78, 81);
    set_column(55, 63); 
    send_color(255, 255, 255, 27); // Bottom horizontal line

    // Letter A
    set_row(65, 79);
    set_column(67, 69); 
    send_color(255, 255, 255, 81); // Left Vertical line

    set_row(65, 68);
    set_column(67, 74); 
    send_color(255, 255, 255, 18); // Top horizontal line

    set_row(71, 74);
    set_column(67, 74); 
    send_color(255, 255, 255, 18); // Middle horizontal line

    set_row(65, 79);
    set_column(74, 76); 
    send_color(255, 255, 255, 81); // Right Vertical line

    // Letter Y
    for (int i = 0; i < 6; i++) {
        set_row(65 + i, 65 + i);
        set_column(80 + i, 82 + i); 
        send_color(255, 255, 255, 81); // Left Vertical line
    }

    for (int i = 0; i < 6; i++) {
        set_row(65 + i, 65 + i);
        set_column(90 - i, 92 - i); 
        send_color(255, 255, 255, 81); // Right Vertical line
    }

    set_row(71, 79);
    set_column(85, 87); 
    send_color(255, 255, 255, 35); // Middle horizontal line
    
}

void displayGameScreen() {

    // GROUND
    set_row(80, 84);  
    set_column(0, 129); 
    send_color(255, 75, 0, 650);  // Dark orange color

}

void displayGameOverScreen()
{
    // Letter G
    set_row(45, 59);  
    set_column(11, 13); 
    send_color(255, 0, 0, 81);  // Left Vertical line

    set_row(45, 48); 
    set_column(11, 20); 
    send_color(255, 0, 0, 20);  // Top horizontal line

    set_row(51, 52); 
    set_column(16, 18); 
    send_color(255, 0, 0, 18);  // Middle horizontal line

    set_row(57, 60); 
    set_column(11, 18); 
    send_color(255, 0, 0, 24);  // Bottom horizontal line

    set_row(51, 59); 
    set_column(18, 20); 
    send_color(255, 0, 0, 28);  // Right Vertical line

    // Letter A
    set_row(45, 59); 
    set_column(24, 26); 
    send_color(255, 0, 0, 81);  // Left Vertical line

    set_row(45, 48); 
    set_column(24, 31); 
    send_color(255, 0, 0, 18);  // Top horizontal line

    set_row(51, 54); 
    set_column(24, 31); 
    send_color(255, 0, 0, 18);  // Middle horizontal line

    set_row(45, 59); 
    set_column(31, 33); 
    send_color(255, 0, 0, 81);  // Right Vertical line

    // Letter M
    set_row(45, 59); 
    set_column(36, 38); 
    send_color(255, 0, 0, 81);  // Left Vertical line

    // Diagonal line from left to center
    for (int i = 0; i < 6; i++) {
        set_row(45 + i, 45 + i);
        set_column(39 + i, 41 + i); 
        send_color(255, 0, 0, 81); // Diagonal line
    }

    // Diagonal line from center to right
    for (int i = 0; i < 6; i++) {
        set_row(45 + i, 45 + i);
        set_column(51 - i, 53 - i); 
        send_color(255, 0, 0, 81); // Diagonal line
    }

    set_row(45, 59); 
    set_column(51, 53); 
    send_color(255, 0, 0, 81);  // Right Vertical line


    // Letter E (moved 5 rows to the right)
    set_row(45, 59); 
    set_column(57, 59); 
    send_color(255, 0, 0, 81);  // Left Vertical line

    set_row(45, 48); 
    set_column(57, 64); 
    send_color(255, 0, 0, 18);  // Top horizontal line

    set_row(51, 54); 
    set_column(57, 64); 
    send_color(255, 0, 0, 18);  // Middle horizontal line

    set_row(58, 59); 
    set_column(57, 64); 
    send_color(255, 0, 0, 25);  // Bottom horizontal line

    // Letter O
    set_row(45, 59); 
    set_column(73, 75); 
    send_color(255, 0, 0, 81);  // Left Vertical line

    set_row(45, 48); 
    set_column(73, 80); 
    send_color(255, 0, 0, 18);  // Top horizontal line

    set_row(59, 59); 
    set_column(73, 80); 
    send_color(255, 0, 0, 18);  // Bottom horizontal line

    set_row(45, 59); 
    set_column(80, 82); 
    send_color(255, 0, 0, 81);  // Right Vertical line

    // Letter V
    // Left diagonal line (moved further right by 2 more rows and less steep)
    for (int i = 0; i < 15; i++) { 
        set_row(45 + i, 45 + i);               // Row increases as before
        set_column(85 + (i / 3), 87 + (i / 3)); // Column increases more slowly (i / 3)
        send_color(255, 0, 0, 81); // Diagonal line
    }

    for (int i = 0; i < 15; i++) { 
        set_row(45 + i, 45 + i);               // Row increases as before
        set_column(94 - (i / 3), 96 - (i / 3)); // Shifted columns 9 places to the right
        send_color(255, 0, 0, 81); // Diagonal line
    }


    // Letter E (left vertical line starts at column 98)
    set_row(45, 59); 
    set_column(99, 101);  // Shifted columns 1 step right
    send_color(255, 0, 0, 81);  // Left Vertical line

    set_row(45, 48); 
    set_column(99, 106);  // Shifted columns 1 step right
    send_color(255, 0, 0, 18);  // Top horizontal line

    set_row(51, 54); 
    set_column(99, 106);  // Shifted columns 1 step right
    send_color(255, 0, 0, 18);  // Middle horizontal line

    set_row(58, 59); 
    set_column(99, 106);  // Shifted columns 1 step right
    send_color(255, 0, 0, 25);  // Bottom horizontal line


    // Letter R
    set_row(45, 59); 
    set_column(110, 112); 
    send_color(255, 0, 0, 81);  // Left Vertical line

    set_row(45, 48); 
    set_column(110, 117); 
    send_color(255, 0, 0, 18);  // Top horizontal line

    set_row(51, 54); 
    set_column(110, 117); 
    send_color(255, 0, 0, 18);  // Middle horizontal line

    set_row(45, 59); 
    set_column(117, 119); 
    send_color(255, 0, 0, 20);  // Right Vertical line

    for (int i = 0; i < 7; i++) {  // Increased loop iteration for 7 steps (from row 53 to 59)
    set_row(53 + i, 53 + i);  // Start from row 53 and increment until row 59
    set_column(112 + i, 114 + i);  // Start from column 112 and increment until column 118
    send_color(255, 0, 0, 81);  // Diagonal line color
    }
}

//  S P R I T E S

void user_Dino()
{
    //USER - DINOSAUR
    set_row(67, 71);  // Head
    set_column(13, 17);
    send_color(0, 255, 0, 100);

    set_row(71, 76);  // Body
    set_column(9, 15);
    send_color(0, 255, 0, 100);

    set_row(76, 82);  // Left leg
    set_column(10, 12);
    send_color(0, 255, 0, 5);

    set_row(76, 82);  // Right leg
    set_column(13, 15);
    send_color(0, 255, 0, 5);

    set_row(74, 77);  // Tail
    set_column(6, 9);
    send_color(0, 255, 0, 8);
}

void sun() {
    set_row(15, 31);        
    set_column(97, 110);    
    send_color(255, 255, 0, 200);  // Top horizontal line

    set_row(28, 30);        
    set_column(97, 113);    
    send_color(255, 255, 0, 80);   // Bottom horizontal line

    set_row(15, 30);        
    set_column(97, 100);     
    send_color(255, 255, 0, 90);   // Left vertical line

    set_row(15, 30);        
    set_column(110, 113);   
    send_color(255, 255, 0, 90);   // Right vertical line
}

void dino_duck_image()
{
    set_row(70, 74);  // Head
    set_column(15, 19);
    send_color(0, 255, 0, 100);

    set_row(71, 76);  // Body
    set_column(9, 15);
    send_color(0, 255, 0, 100);

    set_row(76, 82);  // Left leg
    set_column(10, 12);
    send_color(0, 255, 0, 5);

    set_row(76, 82);  // Right leg
    set_column(13, 15);
    send_color(0, 255, 0, 5);

    set_row(74, 77);  // Tail
    set_column(6, 9);
    send_color(0, 255, 0, 8);
}




#endif /* SPIAVR_H */