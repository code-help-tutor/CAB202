WeChat: cstutorcs
QQ: 749389476
Email: tutorcs@163.com
#include <avr/io.h>
#include "timer.h"
#include "spi.h"
#include "buzzer.h"
#include <stdint.h>

#define MAX_SEQUENCE_LENGTH 20

//     ABCDEFG
//    xFABGCDE
#define SEGS_EF  0b00111110
#define SEGS_BC  0b01101011
#define SEGS_OFF 0b01111111
#define FAIL 0b01110111

volatile uint8_t segs [] = {SEGS_OFF, SEGS_OFF};  // segs initially off
uint8_t simonSequence[MAX_SEQUENCE_LENGTH];  // Simon Says sequence
uint8_t sequenceLength = 0;  // Length of the current sequence

uint8_t pb_sample = 0xFF;
uint8_t pb_sample_r = 0xFF;
uint8_t pb_changed, pb_rising, pb_falling;

static uint8_t vcount0 = 0, vcount1 = 0;

void pb_init(void) {
    // already configured as inputs

    // PBs PA4-7, enable pullup resistors
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

void pb_debounce(void) {
    uint8_t pb_sample_r_temp = pb_sample_r;
    pb_sample_r = pb_sample;
    pb_sample = PORTA.IN;
    pb_changed = pb_sample ^ pb_sample_r_temp;

    // Vertical counter update
    vcount1 = (vcount1 ^ vcount0) & pb_changed;
    vcount0 = ~vcount0 & pb_changed;

    pb_sample_r ^= vcount0 & vcount1;
}

void uart_init(void) {
    PORTB.DIRSET = PIN2_bm; // Enable PB2 as output (USART0 TXD)
    USART0.BAUD = 1389;     // 9600 baud @ 3.3 MHz
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;   // Enable Tx/Rx
}

void uart_putc(char data) {
    while (!(USART0.STATUS & USART_DREIF_bm)); // Wait for the Data Register to be empty
    USART0.TXDATAL = data;                     // Transmit the data by writing to the Data Register
}

uint8_t uart_getc(void) {
    while (!(USART0.STATUS & USART_RXCIF_bm));  // Wait for data
    return USART0.RXDATAL;
}

uint32_t STATE_LFSR = 0x09876272;  // Initial LFSR state

uint8_t generateSimonSequence(void) {
    // Apply the LFSR algorithm to generate the next step in the sequence
    uint8_t bit = STATE_LFSR & 1;
    STATE_LFSR >>= 1;
    if (bit == 1)
        STATE_LFSR ^= 0xE2023CAB;
    uint8_t step = STATE_LFSR & 0b11;

    return step;
}

typedef enum {
  WAIT,
  TONE1,
  TONE2,
  TONE3,
  TONE4,
  FAIL_STATE
} state_t;

int main(void) {
    pb_init();
    timer_init();
    spi_init();
    buzzer_init();
    buzzer_stop();

    state_t state = WAIT;
    uint8_t pb_previous_state = 0xFF;
    uint8_t pb_new_state = 0xFF;
    uint8_t userSequence[MAX_SEQUENCE_LENGTH];  // User input sequence
    uint8_t userInputIndex = 0;  // Index of the user input in the sequence

    while (1) {
        pb_previous_state = pb_new_state;
        pb_new_state = PORTA.IN;

        uint8_t pb_falling = (pb_previous_state ^ pb_new_state) & pb_previous_state;
        uint8_t pb_rising = (pb_previous_state ^ pb_new_state) & pb_new_state;

        switch (state) {
            case WAIT:
                if (pb_falling & PIN4_bm) {
                    state = TONE1;
                    buzzer_play();
                    segs[0] = SEGS_EF;
                    segs[1] = SEGS_OFF;
                    TCA0.SINGLE.PERBUF = TONE1_PER;
                    TCA0.SINGLE.CMP0BUF = TONE1_PER >> 1;

                    // Generate the next step in the sequence
                    simonSequence[sequenceLength] = generateSimonSequence();
                    sequenceLength++;
                } else if (pb_falling & PIN5_bm) {
                    // Handle other buttons (TONE2, TONE3, TONE4) similarly
                    // ...
                }
                break;

            // Handle other states (TONE2, TONE3, TONE4) similarly
            // ...

            case FAIL_STATE:
                if (pb_rising & (PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm)) {
                    // User pressed any button, restart the game
                    state = WAIT;
                    buzzer_stop();
                    segs[0] = FAIL;
                    segs[1] = FAIL;
                    sequenceLength = 0;
                    userInputIndex = 0;
                }
                break;
        }

        // Check user input
        if (state != WAIT && state != FAIL_STATE && pb_falling) {
            uint8_t button = 0;
            if (pb_falling & PIN4_bm)
                button = 0;
            else if (pb_falling & PIN5_bm)
                button = 1;
            else if (pb_falling & PIN6_bm)
                button = 2;
            else if (pb_falling & PIN7_bm)
                button = 3;

            // Check if the user pressed the correct button in the sequence
            if (button == simonSequence[userInputIndex]) {
                userInputIndex++;

                // If the user completed the sequence, generate the next step
                if (userInputIndex == sequenceLength) {
                    simonSequence[sequenceLength] = generateSimonSequence();
                    sequenceLength++;
                    userInputIndex = 0;
                }
            } else {
                state = FAIL_STATE;
                buzzer_play();
                segs[0] = FAIL;
                segs[1] = FAIL;
            }
        }
    }
}