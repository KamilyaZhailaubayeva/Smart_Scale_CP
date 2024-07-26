/*
* Kamilya Zhailaubayeva 201531425
* Superviser: Akhan Almagambetov
* Project title: Smart scale for determining the perfect ratio of grain-to-water
* Description: Smart scale where the user may select the type of grain being
*              used, the number of adult servings, and the portion size based 
*              on calories for determining the weights of the needed grain
*              and water with the appropriate cooking time.
*/

/* Press the red button to set the grain type, number of adults, 
 * portion size and to see the weights of required grain, water, 
 * and the appropriate cooking time (Red button is the central button)
 *
 * Press green and yellow buttons to see available options:
 * Press the green button to choose upwards (Green button is right button)
 * Press the yellow button to choose backwards. (Yellow button is left button)
 * 
 * Press the blue button to go back in setting the grain type, number of adults, 
 * portion size and to see the weights of required grain, water, 
 * and the appropriate cooking time (Blue button is the lowest button)
 *
 * Before entering numbers to the serial monitor, 
   it requires to reach the displayed grain, water, and time information
 * Serial monitor simulates the scale (entered value is a weight on the scale)
   ‘9999’ is a limit of numbers which can be received by serial monitor.
*/

// defining CPU speed
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>       // xc.h library
#include <avr/interrupt.h>  // interrupt library
#include <util/delay.h>   // delay library

#define RS 4  // Reset connected to PC4
#define EN 5  // Enable button connected to PC5

volatile int countR = 0; // Counts the number of RED button pressings
volatile int countY = 0; // Counts the number of YELLOW button pressings
volatile int countG = 0; // Counts the number of GREEN button pressings
volatile int countB = 0; // Counts the number of BLUE button pressings

volatile int X = 0; 
volatile int Z = 1;    // X and Z are required to check ANY buttons pressings
volatile int RtoBX = 0;
volatile int RtoBZ = 1;  // RtoBX & RtoBZ are required to check RED & BLUE buttons pressings
volatile int GtoYX = 0;
volatile int GtoYZ = 1;  // GtoYX & GtoYZ are required to check YELLOW & GREEN buttons pressings
volatile int RB = 0;     // The difference between Red and Blue button pressings

void setup_IO_INT(void);      // Setup I/O ports and interrupts
void usartInit(void);         // USART initialization
void pushData(int);         // Send data
void pushCmd(int);          // Send command
void pushCmdUpper(int);       // Send command with only four upper bits
void display_init();        // The display initialization routine
static inline void enableTiming();  // Pulling enable high and low to send data

int main () {
  
  // Temporary data variables:
  int numm = 0;      // variable to store obtained digit from console
  int GrainW = 0;    // Weight of grain in grams
  int WaterW = 0;    // Weight of water in grams
  int DataW = 0;
  int type = 0;      // The number of types
  int num = 0;       // The number of adults
  int cal = 0;       // The number of calory selection
  int inf = 0;       // The resulted info (1-grain, 2-water, 3-time)
  int CookT1_d1 = 0;  // 1st digit of the 1st cooking time
  int CookT1_d2 = 0;  // 2nd digit of the 1st cooking time
  int CookT2_d1 = 0;  // 1st digit of the 2nd cooking time
  int CookT2_d2 = 0;  // 2nd digit of the 2nd cooking time
  int PS = 0;    // Portion Size for each grain (in grams)
  int PS_short = 0;  // Shortened portion size to not exceed the largest possible number in Tinkercad 
  int CalT = 0;    // Calories total
  int CalN = 0;    // Counts of calories selection
  
  // Arrays containing instructions and grain types:
  int TOn[] = {' ', ' ', ' ', ' ', 'W', 'e', 'l', 'c', 'o', 'm', 'e', '!'};
  int G00[] = {'S', 'e', 'l', 'e', 'c', 't', ' ', 'g', 'r', 'a', 'i', 'n', ':'};
  int G01[] = {'S', 'e', 'r', 'v', 'i', 'n', 'g', ' ', 's', 'i', 'z', 'e', ':'};
  int G02[] = {'R', 'e', 'q', 'u', 'i', 'r', 'e', 'd', ' ', 'g', 'r', 'a', 'i', 'n', ':'};
  int G03[] = {' ', 'g', 'r', 'a', 'm', 's'};
  int G04[] = {'R', 'e', 'q', 'u', 'i', 'r', 'e', 'd', ' ', 'w', 'a', 't', 'e', 'r', ':'};
  int G05[] = {'C', 'o', 'o', 'k', 'i', 'n', 'g', ' ', 't', 'i', 'm', 'e', ':'};
  int G06[] = {' ', 'm', 'i', 'n'};
  int G09[] = {'S', 'e', 'l', 'e', 'c', 't', ' ', 'p', 'o', 'r', 't', 'i', 'o', 'n', ':'};
  int G010[] = {'c', 'a', 'l'};
  int G1[] = {'A', 'm', 'a', 'r', 'a', 'n', 't', 'h'};
  int G2[] = {'B', 'a', 'r', 'l', 'e', 'y'};
  int G3[] = {'B', 'u', 'c', 'k', 'w', 'h', 'e', 'a', 't'};
  int G4[] = {'B', 'u', 'l', 'g', 'u', 'r'}; 
  int G5[] = {'F', 'a', 'r', 'r', 'o'};
  int G6[] = {'M', 'i', 'l', 'l', 'e', 't'};
  int G7[] = {'Q', 'u', 'i', 'n', 'o', 'a'};
  int G8[] = {'R', 'o', 'l', 'l', 'e', 'd', ' ', 'O', 'a', 't', 's'};
  int G9[] = {'W', 'h', 'i', 't', 'e', ' ', 'R', 'i', 'c', 'e'};
  int G10[] = {'W', 'i', 'l', 'd', ' ', 'R', 'i', 'c', 'e'};
  
  // Lengths of Arrays:
  int TOn_len = sizeof TOn / sizeof *TOn;
  int G00_len = sizeof G00 / sizeof *G00;
  int G01_len = sizeof G01 / sizeof *G01;
  int G02_len = sizeof G02 / sizeof *G02;
  int G03_len = sizeof G03 / sizeof *G03;
  int G04_len = sizeof G04 / sizeof *G04;
  int G05_len = sizeof G05 / sizeof *G05;
  int G06_len = sizeof G06 / sizeof *G06;
  int G09_len = sizeof G09 / sizeof *G09;
  int G010_len = sizeof G010 / sizeof *G010;
  int G1_len = sizeof G1 / sizeof *G1;
  int G2_len = sizeof G2 / sizeof *G2;
  int G3_len = sizeof G3 / sizeof *G3;
  int G4_len = sizeof G4 / sizeof *G4;
  int G5_len = sizeof G5 / sizeof *G5;
  int G6_len = sizeof G6 / sizeof *G6;
  int G7_len = sizeof G7 / sizeof *G7;
  int G8_len = sizeof G8 / sizeof *G8;
  int G9_len = sizeof G9 / sizeof *G9;
  int G10_len = sizeof G10 / sizeof *G10;
  
  setup_IO_INT(); // Setting up I/O ports and interrupts
  display_init(); // Display initialization
  
  for (int i = 0; i < TOn_len; i++){
    //  Printing the instructions in TOn: "Welcome!"
    pushData(TOn[i]);    
  }
  
  sei();  // Enable global interrupts

  while (1){
    
    if (X == Z) {
    
      if (RtoBX == RtoBZ) {
        
        RB = abs(countR - countB)%4;
        
        if (RB == 1) {
          pushCmd(0x01); // Display clear
      
          for (int i = 0; i < G00_len; i++){
            //  Printing the instruction in G00: "Select grain:"
          pushData(G00[i]); 
          }
          
          pushCmd(0xC0); // Starting from a new line
          
          type = 1; // First grian option 
          for (int i = 0; i < G1_len; i++){
             //  Printing the 1st grain type: "Amaranth"
           pushData(G1[i]);
          }
          cal = 371;
          
          countY = 0;  //Restart counting Yellow button pressings
          countG = 91;  //Restart counting Green button pressings
        }
        
        if (RB == 2) {
          pushCmd(0x01); // Display clear
        
          for (int i = 0; i < G01_len; i++){
        //  Printing the instruction in G01: "Serving size:"
        pushData(G01[i]);
          }
          
          pushCmd(0xC0); // Starting from a new line
          
          num = 1; // First serving size
          pushData('0' + num);
          
          countY = 0;  //Restart counting Yellow button pressings
          countG = 91;  //Restart counting Green button pressings
        }
        
        if (RB == 3) {
          pushCmd(0x01); // Display clear
        
          for (int i = 0; i < G09_len; i++){
        //  Printing the instruction in G09: "Select portion:"
        pushData(G09[i]);
          }
          
          PS = 60;
          PS_short = 6; // First shortened portion size
          
          pushCmd(0xC0); // Starting from a new line
          
          CalT = (cal*PS_short)/10; 
          
          // 1st Portion Size requires 2 digits only, thus:
          pushData('0' + PS_short); // Printing the 1st digit
          // Extending portion size by adding '0' as the last digit:
          pushData('0'); // Printing the 2nd digit
      
          pushData('g'); 
          pushData('-');
      
          // Calory number contains 3 digits in any case, thus:
          int digitC_1 = CalT/ 100; // Calculating the 1st digit
          int digitC_2 = (CalT % 100)/10; // Calculating the 2nd digit
          int digitC_3 = CalT % 10; // Calculating the 3rd digit
         
          pushData('0' + digitC_1); // Printing the 1st digit
          pushData('0' + digitC_2); // Printing the 2nd digit
          pushData('0' + digitC_3); // Printing the 3rd digit 

          for (int i = 0; i < G010_len; i++){
          //  Printing the instruction in G010: "cal"
          pushData(G010[i]);
          }
          
          countY = 0;  //Restart counting Yellow button pressings
          countG = 91;  //Restart counting Green button pressings
        }
        
        if (RB == 0) {
          pushCmd(0x01); // Display clear
        
          for (int i = 0; i < G02_len; i++){
        //  Printing the instruction in G02: "Required grain:"
        pushData(G02[i]);
          }
          
          GrainW = num*PS;
          inf = 1;
          DataW = GrainW;
          pushCmd(0xC0); // Starting from a new line
          
          if (GrainW < 100){ 
            int digit_1 = GrainW / 10; // Calculating the 1st digit
            int digit_2 = GrainW % 10; // Calculating the 2nd digit
          
            pushData('0' + digit_1); // Printing the 1st digit
            pushData('0' + digit_2); // Printing the 2nd digit  
          } 
          // If the weight requires three digits
          else if (GrainW >= 100) { 
            int digit_1 = GrainW / 100; // Calculating the 1st digit
            int digit_2 = (GrainW % 100)/10; // Calculating the 2nd digit
            int digit_3 = GrainW % 10; // Calculating the 3rd digit
          
            pushData('0' + digit_1); // Printing the 1st digit
            pushData('0' + digit_2); // Printing the 2nd digit
            pushData('0' + digit_3); // Printing the 3rd digit
          }
        
          for (int i = 0; i < G03_len; i++){
            //  Printing the instruction in G03: "grams"
            pushData(G03[i]);
          }  
          
          countY = 0;  //Restart counting Yellow button pressings
          countG = 91;  //Restart counting Green button pressings
        }
        
        RtoBX += 1;
      }
      
      
      if (GtoYX == GtoYZ) {
        
        pushCmd(0x01); // Display clear    
        
        if (RB == 1) { // Selecting grain type
          
          type = abs(countG - countY)%10;
          
          for (int i = 0; i < G00_len; i++){
            //  Printing the instruction in G00: "Select grain:"
          pushData(G00[i]); 
          }
          
          pushCmd(0xC0); // Starting from a new line  
          
          if (type==1) {
            for (int i = 0; i < G1_len; i++){
              //  Printing the 1st grain type: "Amaranth"
            pushData(G1[i]);
            }
            cal = 371;
          }      
          else if (type==2) {
            for (int i = 0; i < G2_len; i++){
              //  Printing the 2nd grain type: "Barley"
              pushData(G2[i]);
            }
            cal = 354;
          }
          else if (type==3) {
            for (int i = 0; i < G3_len; i++){
              //  Printing the 3rd grain type: "Buckwheat"
            pushData(G3[i]);
            }
            cal = 343;
          }
          else if (type==4) {
            for (int i = 0; i < G4_len; i++){
              //  Printing the 4th grain type: "Bulgur"
              pushData(G4[i]);
            }
            cal = 342;
          }
          else if (type==5) {
            for (int i = 0; i < G5_len; i++){
              //  Printing the 5th grain type: "Farro"
            pushData(G5[i]);
            }
            cal = 378;
          }
          else if (type==6) {
            for (int i = 0; i < G6_len; i++){
              //  Printing the 6th grain type: "Millet"
            pushData(G6[i]);
            }
            cal = 378;
          }
          else if (type==7) {
            for (int i = 0; i < G7_len; i++){
              //  Printing the 7th grain type: "Quinoa"
            pushData(G7[i]);
            }
            cal = 368;
          }
          else if (type==8) {
            for (int i = 0; i < G8_len; i++){
              //  Printing the 8th grain type: "Rolled Oats"
              pushData(G8[i]); 
            }
            cal = 395;
          }           
          else if (type==9) {
            for (int i = 0; i < G9_len; i++){
              //  Printing the 9th grain type: "White Rice"
              pushData(G9[i]);  
            }
            cal = 358;
          }
          else if (type==0) {
            for (int i = 0; i < G10_len; i++){
              //  Printing the 10th grain type: "Wild Rice"
              pushData(G10[i]);    
            }
            cal = 357;
          }
        }
        
        if (RB == 2) { // Selecting serving size
          
          for (int i = 0; i < G01_len; i++){
        //  Printing the instruction in G01: "Serving size:"
        pushData(G01[i]);
          }
          
          pushCmd(0xC0); // Starting from a new line
          
          num = abs(countG - countY)%10;
          
          // Printing the number of adults:
          if (num==0) {
            num=10;
          pushData('1');
          pushData('0');
          }
          else {
          pushData('0' + num);
          }
        }
        
        if (RB == 3) { // Selecting portion size
          
          CalN = abs(countG - countY)%5;
          if (CalN == 1) {
            PS = 60;
            PS_short = 6;
          }
          else if (CalN == 2) {
            PS = 80;
            PS_short = 8;
          }
          else if (CalN == 3) {
            PS = 100;
            PS_short = 10;
          }
          else if (CalN == 4) {
            PS = 120;
            PS_short = 12;
          }
          else if (CalN == 0) {
            PS = 140;
            PS_short = 14;
          }
          
          for (int i = 0; i < G09_len; i++){
        //  Printing the instruction in G09: "Select portion:"
        pushData(G09[i]);
          }
          
          pushCmd(0xC0); // Starting from a new line
          
          CalT = (cal*PS_short)/10; 
          
          // Portion Size contains 2 or 3 digits, thus, if it contains 2 digits:
          if (PS_short < 10){    
          pushData('0' + PS_short); // Printing the 1st digit
            // Extending portion size by adding '0' as the last digit:
          pushData('0'); // Printing the 2nd digit
          } 
          // If the portion size requires 3 digits
          else if (PS_short >= 10) { 
          int digitPS_short_1 = PS_short / 10; // Calculating the 1st digit
          int digitPS_short_2 = PS_short % 10; // Calculating the 2nd digit
         
          pushData('0' + digitPS_short_1); // Printing the 1st digit
          pushData('0' + digitPS_short_2); // Printing the 2nd digit
            // Extending portion size by adding '0' as the last digit:
          pushData('0'); // Printing the 3rd digit
          } 
      
          pushData('g'); 
          pushData('-');
      
          // Calory number contains 3 digits in any case, thus:
          int digitC_1 = CalT/ 100; // Calculating the 1st digit
          int digitC_2 = (CalT % 100)/10; // Calculating the 2nd digit
          int digitC_3 = CalT % 10; // Calculating the 3rd digit
         
          pushData('0' + digitC_1); // Printing the 1st digit
          pushData('0' + digitC_2); // Printing the 2nd digit
          pushData('0' + digitC_3); // Printing the 3rd digit 

          for (int i = 0; i < G010_len; i++){
          //  Printing the instruction in G010: "cal"
          pushData(G010[i]);
          } 
           
        }
        
        if (RB == 0) { // Showing the resulted info
          
          inf = abs(countG - countY)%3;         
          
          /* Calculating the required grain and 
         water with appropriate cooking time
           based on the type and portion size: */
          if (type==1) {
            GrainW = num*PS;
            WaterW = (25*DataW)/10;
            CookT1_d1 = 2;
            CookT1_d2 = 0;
            CookT2_d1 = 2;
            CookT2_d2 = 5;
          }
        else if (type==2) {
            GrainW = num*PS;
            WaterW = 3*DataW;
            CookT1_d1 = 4;
            CookT1_d2 = 5;
            CookT2_d1 = 6;
            CookT2_d2 = 0;
          }
          else if (type==3) {
            GrainW = num*PS;
            WaterW = 2*DataW;
            CookT1_d1 = 2;
            CookT1_d2 = 0;
            CookT2_d1 = 3;
            CookT2_d2 = 0;
          }
          else if (type==4) {
            GrainW = num*PS;
            WaterW = 2*DataW;
            CookT1_d1 = 1;
            CookT1_d2 = 0;
            CookT2_d1 = 1;
            CookT2_d2 = 5;
          }
          else if (type==5) {
            GrainW = num*PS;
            WaterW = 3*DataW;
            CookT1_d1 = 2;
            CookT1_d2 = 5;
            CookT2_d1 = 4;
            CookT2_d2 = 0;
          } 
        else if (type==6) {
            GrainW = num*PS;
            WaterW = (25*DataW)/10;
            CookT1_d1 = 2;
            CookT1_d2 = 0;
            CookT2_d1 = 3;
            CookT2_d2 = 0;
          } 
          else if (type==7) {
            GrainW = num*PS;
            WaterW = 2*DataW;
            CookT1_d1 = 1;
            CookT1_d2 = 5;
            CookT2_d1 = 2;
            CookT2_d2 = 0;
          }
          else if (type==8) {
            GrainW = num*PS;
            WaterW = 2*DataW;
            CookT1_d1 = 0;
            CookT1_d2 = 5;
            CookT2_d1 = 1;
            CookT2_d2 = 0;
          } 
          else if (type==9) {
            GrainW = num*PS;
            WaterW = 2*DataW;
            CookT1_d1 = 1;
            CookT1_d2 = 5;
            CookT2_d1 = 2;
            CookT2_d2 = 5;
          } 
          else if (type==0) {
            GrainW = num*PS;
            WaterW = 4*DataW;
            CookT1_d1 = 4;
            CookT1_d2 = 5;
            CookT2_d1 = 6;
            CookT2_d2 = 0;
          }
          
          
          // Firstly, it shows the weight of the required grain:
          if (inf==1) {
            
            DataW = GrainW;
        
            for (int i = 0; i < G02_len; i++){
            //  Printing the instruction in G02: "Required grain:"
              pushData(G02[i]);
            }
        
            pushCmd(0xC0); // Starting from a new line
          
            if (GrainW < 100){ 
              int digit_1 = GrainW / 10; // Calculating the 1st digit
              int digit_2 = GrainW % 10; // Calculating the 2nd digit
          
              pushData('0' + digit_1); // Printing the 1st digit
              pushData('0' + digit_2); // Printing the 2nd digit  
            } 
            // If the weight requires three digits
            else if (GrainW >= 100) { 
              int digit_1 = GrainW / 100; // Calculating the 1st digit
              int digit_2 = (GrainW % 100)/10; // Calculating the 2nd digit
              int digit_3 = GrainW % 10; // Calculating the 3rd digit
          
              pushData('0' + digit_1); // Printing the 1st digit
              pushData('0' + digit_2); // Printing the 2nd digit
              pushData('0' + digit_3); // Printing the 3rd digit
            }
        
            for (int i = 0; i < G03_len; i++){
             //  Printing the instruction in G03: "grams"
             pushData(G03[i]);
            }  
          }
          
          // Secondly, it shows the weight of the required water:
          else if (inf==2) {
            
            for (int i = 0; i < G04_len; i++){
              //  Printing the instruction in G04: "Required water:"
            pushData(G04[i]);
            }
        
            pushCmd(0xC0); // Starting from a new line
          
            if (WaterW < 1000){ 
              int digitW_1 = WaterW / 100; // Calculating the 1st digit
              int digitW_2 = (WaterW % 100)/10; // Calculating the 2nd digit
              int digitW_3 = WaterW % 10; // Calculating the 3rd digit
          
              pushData('0' + digitW_1); // Printing the 1st digit
              pushData('0' + digitW_2); // Printing the 2nd digit
              pushData('0' + digitW_3); // Printing the 3rd digit  
            }
            // If the weight requires four digits
            else if (WaterW >= 1000) {
              int digitW_1 = WaterW / 1000; // Calculating the 1st digit
              int digitW_2 = (WaterW % 1000)/100; // Calculating the 2nd digit
              int digitW_3 = (WaterW % 100)/10; // Calculating the 3rd digit
              int digitW_4 = WaterW % 10; // Calculating the 4th digit
          
              pushData('0' + digitW_1); // Printing the 1st digit
              pushData('0' + digitW_2); // Printing the 2nd digit
              pushData('0' + digitW_3); // Printing the 3rd digit
              pushData('0' + digitW_4); // Printing the 4th digit
            }
          
            for (int i = 0; i < G03_len; i++){
              //  Printing the instructions in G03: "grams"
              pushData(G03[i]);
            }
          
          }
          
          // Finally, it shows the appropriate cooking time:
          else if (inf==0) {
            
            for (int i = 0; i < G05_len; i++){
              //  Printing the instruction in G05: "Cooking time:"
              pushData(G05[i]);
            }
        
            pushCmd(0xC0); // Starting from a new line
        
            pushData('0' + CookT1_d1); // Printing the 1st digit
            pushData('0' + CookT1_d2); // Printing the 2nd digit
            pushData('-'); // Printing the 2nd digit
            pushData('0' + CookT2_d1); // Printing the 2nd digit
            pushData('0' + CookT2_d2); // Printing the 2nd digit
         
            for (int i = 0; i < G06_len; i++){
              //  Printing the instruction in G06: "min"
              pushData(G06[i]);
            }
          }     
          
        }
        GtoYX += 1;
      }
      X += 1;
    }
   
    //SCALE PART:
    // deleted to increase functionality of Arduino
              
  }
  
  return 0;
}

// Interrupt Service Routine for PD7..PD0
ISR(PCINT2_vect) {
  
  if ((PIND & (1 << PIND2)) == 0){ // YELLOW (Left) button is pressed
    countY += 1;
    GtoYZ = GtoYX;
    Z = X;
  }
  if ((PIND & (1 << PIND3)) == 0) { // RED (Central) button is pressed
    countR += 1;
    RtoBZ = RtoBX;
    Z = X;
  }
  if ((PIND & (1 << PIND4)) == 0) { // GREEN (Right) button is pressed
    countG += 1;
    GtoYZ = GtoYX;
    Z = X;
  }
  if ((PIND & (1 << PIND5)) == 0) { // BLUE (lowest) button is pressed
    if ((countR != countB)&(countR > countB)&(RB != 1)) {
      countB += 1;
      RtoBZ = RtoBX;
      Z = X;
    }
  }
  
}

void setup_IO_INT(){
  
  /* PORTD data-direction register declaration
  PIND2, PIND3, PIND4, PIND5 are inputs:*/
  DDRD = 0;   
  PORTD |= ((1 << PORTD2)|(1 << PORTD3)|(1 << PORTD4)|(1 << PORTD5));  // enable internal pull-up resistor on PIND2, PIND3, PIND4
  
  // Enabling interrupts:
  PCICR |= (1 << PCIE2);  // EN INT for PD7..PD0
  PCMSK2 |= (1 << PCINT18)|(1 << PCINT19)|(1 << PCINT20)|(1 << PCINT21); // EN INT for PD2, PD3, PD4, PD5
  
}


void display_init(){
    
  _delay_ms(1000); // Waiting for the microcontroller to initialize after power cycling
  DDRC = 0b00111111; // Setting the DDR for Port C
  PORTC = 0b00111111;
  pushCmdUpper(0x30); // Sending the command of 0x30 (only the upper nibble)
  _delay_ms(5);
  pushCmdUpper(0x30); // Sending the command of 0x30 (only the upper nibble)
  _delay_ms(1);
  pushCmdUpper(0x30); // Sending the command of 0x30 (only the upper nibble)
  pushCmdUpper(0x20); // Sending the command of 0x20 (only the upper nibble)
  pushCmd(0x28); // Specifying the number of display lines (1) and character font (0)
  pushCmd(0x08); // Display off
  pushCmd(0x01); // Display clear
  pushCmd(0x06); // Setting entry mode
  pushCmd(0x0F); // Display on
  _delay_ms(1000);
  
}


static inline void enableTiming(){
  
  _delay_ms(1);
  PORTC |= (1 << EN); // pull EN logic-high
  _delay_ms(1);
  PORTC &= ~(1 << EN); // pull EN logic-low
  _delay_ms(1);

}

void pushData (int data){
  
  // send upper 4 bits of data
  /*/ keep upper bits of PORTC 
  and write shifted upper 4 bits of data to PORTC3...0 */
  PORTC = (PORTC & 0b11110000) | (data >> 4); 
  
  PORTC |= (1 << RS); // pull RS logic-high to send data
  enableTiming(); // pull enable high and low to send data
  
  /* keep upper bits of PORTC 
  and write remaining 4 bits of data to PORTC3...0 */
  PORTC = (PORTC & 0b11110000) | (data & 0b00001111);
  enableTiming();
  
}

void pushCmd (int data){
  
  PORTC = (PORTC & 0b11110000) | (data >> 4);
  PORTC &= ~(1 << RS); // pull RS logic-low to send command
  enableTiming();
  PORTC = (PORTC & 0b11110000) | (data & 0b00001111);
  enableTiming();
  
}

void pushCmdUpper (int data){ 
    
  // send upper 4 bits of data
  /*/ keep upper bits of PORTC 
  and write shifted upper 4 bits of data to PORTC3...0 */
  PORTC = (PORTC & 0b11110000) | (data >> 4);
  
  PORTC &= ~(1 << RS);  // pull RS logic-low to send command
  enableTiming();
  
}
