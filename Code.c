#include <p18cxxx.h>
#include <BCDlib.h>
#include <delays.h>

#define Hi PORTBbits.RB7 // High temperature indicator
#define Med PORTBbits.RB6 // Medium temp. indicator
#define Lo PORTBbits.RB5 // Low temperature indicator
#define DegC FLAGS.B0 // DegC = 1, C else F
#define HiTemp 76 // A/D reading for T = 30 C
#define LoTemp 51 // A/D reading for T = 20 C

void Setup(void);
void Control(void);
void C_or_F(void);
void TestPB(void);
void InitMuxedDisplay(void);
void ISR(void);

char Digits[5]; 
unsigned char Qstates, val, i, j;
char SSCodes[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 
                  0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x00};

void main(void) {
    Setup();
    while (1) {
        Control(); // indicate hi, lo or medium temperature
        C_or_F(); // use current A/D result
        TestPB(); // start A/D & test PB
    }
}

void Setup(void) {
    ADCON0bits.ADON = 1; // turn on A/D, AN0
    ADCON0bits.CHS = 7; // select AN7: take input from LM35 sensor
    ADCON1bits.PVCFG = 0b10; // connect Vref+ to FVR
    ADCON2 = 0b00001001; // LEFT justify (0b0), Tconv = 22us, Tacq = 4us
    VREFCON0 = 0b10010000; // set Vref to 1.024V
    
    ANSELB = 0x00; TRISB &= 0x1F; // RB7..RB5 outputs, RB0 input
    INTCON2bits.INTEDG0 = 0; // RB0 reacts to falling edge
    INTCON2bits.RBPU = 0; // enable PORTB internal pull-ups
    
    DegC = 1; // start with Celsius
    InitMuxedDisplay();
}

void InitMuxedDisplay(void) {       
    TRISD = 0x00;                   // PORTD output
    TRISA &= 0b11110000;            // RA3..RA0: output, rest: input
    T0CON = 0b11010101;             // divide clock by 64, 8-bit mode
    INTCONbits.T0IE = 1;            // enable interrupt
    INTCONbits.GIE = 1;
    
    i = 0;                          // start with digits[0]
    Qstates = 0b00001000;           // start with MSD
}

void Control(void) {
    PORTB &= 0b00011111; // reset all LEDs
    if (ADRESH > HiTemp)
        Hi = 1; // T > 30, turn on high LED
    else if (ADRESH >= LoTemp)
        Med = 1; // 20 <= T <= 30, turn on med. LED
    else
        Lo = 1; // else turn on low LED
}

void TestPB(void) { // C or F
    ADCON0bits.GO = 1; // start AD conversion
    if (INTCONbits.INT0IF) { // if RB0 pressed
        INTCONbits.INT0IF = 0;
        DegC = ~DegC; // toggle unit
    }
}

void C_or_F(void) {
    float Celsius,Fahrenheit; // use float +0.5 for rounding instead of +128
    Celsius = 102.4 * ADRESH / 256.0; // convert AD output to Celsius
    Fahrenheit = 18.0 * Celsius + 320.0;
    if (DegC) {
        Bin2Bcd((unsigned char)(Celsius + 0.5), Digits);
    } else {
        INTCONbits.GIE = 0; // disable interrupt while shifting digits
        Bin2BcdE((unsigned int)(Fahrenheit + 0.5), Digits); // result = digits[1,2,3,4]
        for (j=0; j<4; j++)
            Digits[j] = Digits[j+1]; // shift result to digits[0,1,2,3]
        INTCONbits.GIE = 1; // enable interrupt, resolved flickering
    } 
}

#pragma code ISR = 0x0008
#pragma interrupt ISR

void ISR(void) { // display value every 8ms
    unsigned char val;
    INTCONbits.TMR0IF = 0;          // acknowledge interrupt
    TMR0L = 256 - 125;              // 64us * 125 = 8ms
    PORTD = 0x00;                   // remove shadowing
    PORTA = Qstates;                // display shows 1 digit at a time
    
    val = SSCodes[Digits[i]];
    if (DegC && i==3)
        val = 0x63;                 // C degree symbol
    else if (~DegC && i==2)
        val |= 0x80;                // DP for F
    PORTD = val;   
    
    Qstates >>= 1;                  // move to next transistor
    i++;                            // display next digit of Digits
    if (i == 4){
        i = 0;
        Qstates = 0b00001000;       // back to MSD
    }
}