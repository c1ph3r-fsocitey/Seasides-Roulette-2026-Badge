#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <avr/eeprom.h>

/* ===== PIN MAP ===== */
#define SER     PB3
#define SRCLK   PB4
#define RCLK    PB0
#define BTN_RX  PB1
#define TX      PB2

#define BIT_US 833

/* ===== EEPROM ===== */
uint8_t EEMEM ee_level;

/* ===== STATE ===== */
uint8_t lfsr = 0x5A;
uint8_t level = 0;

/* ===== FORWARD DECL ===== */
uint8_t classic_spin(void);

/* ===== STAGES ===== */
const char* stage_name[8] = {
"ROT13","BASE64","HEX","BINARY","CAESAR","HEX","HEX","HEX"
};

const char* challenge[8] = {
"FRN FVQRF TB N",
"YzFwaDNyLWZzb2NpdGV5",
"73686966745f7265676973746572",
"01110101 01100001 01110010 01110100",
"vslo_wkh_fkdrv",
"656570726f6d",
"6269745f62616e676572",
"73656173696465735f6f7665726c6f7264"
};

const char* solution[8] = {
"seasides_goa",
"c1ph3r-fsocitey",
"shift_register",
"uart",
"spin_the_chaos",
"eeprom",
"bit_banger",
"seasides_overlord"
};

/* ===== 74HC595 ===== */
void shift_out(uint8_t data){
    for(int8_t i=7;i>=0;i--){
        if(data & (1<<i)) PORTB |= (1<<SER);
        else PORTB &= ~(1<<SER);
        PORTB |= (1<<SRCLK);
        PORTB &= ~(1<<SRCLK);
    }
    PORTB |= (1<<RCLK);
    PORTB &= ~(1<<RCLK);
}

/* ===== UTILS ===== */
void delay_ms_var(uint16_t ms){
    while(ms--) _delay_ms(1);
}

/* ===== RANDOM ===== */
void init_timer(){ TCCR0B = (1<<CS01)|(1<<CS00); }

uint8_t random8(){
    lfsr ^= TCNT0;
    lfsr ^= lfsr << 3;
    lfsr ^= lfsr >> 5;
    lfsr ^= lfsr << 1;
    return lfsr;
}

/* ===== UART ===== */
void uart_tx(char c){
    PORTB &= ~(1<<TX); _delay_us(BIT_US);
    for(uint8_t i=0;i<8;i++){
        if(c & 1) PORTB |= (1<<TX);
        else PORTB &= ~(1<<TX);
        _delay_us(BIT_US);
        c >>= 1;
    }
    PORTB |= (1<<TX); _delay_us(BIT_US);
}

uint8_t uart_available(){ return !(PINB & (1<<BTN_RX)); }

char uart_rx(){
    _delay_us(BIT_US + BIT_US/2);
    char c=0;
    for(uint8_t i=0;i<8;i++){
        if(PINB & (1<<BTN_RX)) c |= (1<<i);
        _delay_us(BIT_US);
    }
    _delay_us(BIT_US);
    return c;
}

void uart_print(const char*s){ while(*s) uart_tx(*s++); }

/* ===== EFFECT ===== */
void blink_and_off(uint8_t led){
    for(uint8_t i=0;i<3;i++){
        shift_out(led); delay_ms_var(200);
        shift_out(0);   delay_ms_var(200);
    }
}

/* ===== ORIGINAL SPIN ===== */
uint8_t classic_spin(){
    uint8_t p = 1 << (random8() & 7);
    for(uint8_t s=30; s>5; s--)
        for(uint8_t i=0;i<8;i++){
            shift_out(p);
            p=(p<<1)|(p>>7);
            delay_ms_var(s);
        }
    for(uint8_t s=5; s<30; s++)
        for(uint8_t i=0;i<8;i++){
            shift_out(p);
            p=(p<<1)|(p>>7);
            delay_ms_var(s);
        }
    return p;
}

/* ===== STAGE-BASED ANIMATIONS ===== */
uint8_t run_mode(){

    if(level == 0) return classic_spin();

    if(level == 1){
        uint8_t p = 1 << (random8() & 7);
        for(uint8_t s=15; s>4; s--)
            for(uint8_t i=0;i<8;i++){
                shift_out(p);
                p=(p<<1)|(p>>7);
                delay_ms_var(s);
            }
        return p;
    }

    if(level == 2){
        for(uint16_t i=0;i<128;i++){
            shift_out(i);
            _delay_ms(12);
        }
        return 0;
    }

    if(level == 3){
        uint8_t p=0;
        for(uint8_t i=0;i<120;i++){
            p = random8();
            shift_out(p);
            delay_ms_var(8);
        }
        return p;
    }

    if(level == 4){
        uint8_t p=0;
        for(uint8_t i=0;i<100;i++){
            p = 1 << (random8() & 7);
            shift_out(p);
            uint8_t d = (random8()%50)+10;
            while(d--) _delay_ms(1);
        }
        return p;
    }

    if(level == 5){
        for(uint8_t i=0;i<6;i++){
            shift_out(0xFF);
            delay_ms_var(120);
            shift_out(0x00);
            delay_ms_var(120);
        }
        return 0xFF;
    }

    if(level == 6){
        uint8_t p=1;
        for(uint8_t i=0;i<24;i++){
            shift_out(p);
            delay_ms_var(140);
            p <<= 1;
            if(!p) p=1;
        }
        return p;
    }

    for(uint8_t i=0;i<30;i++){
        shift_out(0xFF);
        delay_ms_var(60);
        shift_out(0x00);
        delay_ms_var(60);
    }
    return 0xFF;
}

/* ===== STAGE DISPLAY ===== */
void show_stage(){
    uart_print("\r\n==== SEASIDES GOA ROULETTE ====\r\n");
    uart_print("STAGE ");
    uart_tx('1'+level);
    uart_print(": ");
    uart_print(stage_name[level]);
    uart_print("\r\n");
    uart_print(challenge[level]);
    uart_print("\r\n> ");
}

/* ===== MAIN ===== */
int main(void){
    DDRB = (1<<SER)|(1<<SRCLK)|(1<<RCLK)|(1<<TX);
    PORTB |= (1<<BTN_RX);

    init_timer();
    shift_out(0);

    /* EEPROM LOAD FIX */
    level = eeprom_read_byte(&ee_level);
    if(level == 0xFF) level = 0;
    if(level > 7) level = 7;

    _delay_ms(300);
    show_stage();

    char buf[32]; uint8_t idx=0;

    while(1){
        if(!(PINB & (1<<BTN_RX))){
            _delay_ms(30);
            if(!(PINB & (1<<BTN_RX))){
                uint8_t result = run_mode();
                blink_and_off(result);
            }
        }

        if(uart_available()){
            char c = uart_rx();
            if(c=='\r' || c=='\n'){
                buf[idx]=0;

                /* NEW: If user just presses Enter, re-show puzzle */
                if(idx == 0){
                    show_stage();
                }

                else if(strcmp(buf,solution[level])==0){

                    for(uint8_t i=0;i<6;i++){
                        shift_out(0xFF);
                        delay_ms_var(70);
                        shift_out(0x00);
                        delay_ms_var(70);
                    }

                    uart_print("\r\nLEVEL UP!\r\n");

                    if(level < 7){
                        level++;
                        eeprom_write_byte(&ee_level, level);
                        show_stage();
                    } else {
                        uart_print("\r\nALL STAGES CLEARED!\r\n");
                        level = 7;
                        eeprom_write_byte(&ee_level, level);
                    }
                }
                else{
                    uart_print("\r\nWrong.\r\n> ");
                }

                idx=0;
            }
            else if(idx<31){
                buf[idx++] = c;
                uart_tx('*');
            }
        }
    }
}
