
#include "msp.h"
#include "Clock.h"

//void (*TimerA2Task)(void);
//void TimerA2_Init(void(*task)(void), uint16_t period){
//    TimerA2Task = task;
//    TIMER_A2->CTL = 0x0280;
//    TIMER_A2->CCTL[0] = 0x0010;
//    TIMER_A2->CCR[0] = (period -1);
//    TIMER_A2->EX0 = 0x0005;
//    NVIC->IP[3] = (NVIC->IP[3]&0xFFFFFF00)|0x00000040;
//    NVIC->ISER[0] = 0x000010000;
//    TIMER_A2->CTL |= 0x0014;
//}
//void TA2_0_IRQHandler(void){
//    TIMER_A2->CCTL[0] &= ~0x0001;
//    (*TimerA2Task)();
//}

void PWM_Init34(uint16_t period, uint16_t duty3, uint16_t duty4){
    P2->DIR |= 0xC0;
    P2->SEL0 |= 0xC0;
    P2->SEL1 &= ~0xC0;

    TIMER_A0->CCTL[0] = 0x0800;
    TIMER_A0->CCR[0] = period;

    TIMER_A0->EX0 = 0x0000;

    TIMER_A0->CCTL[3] = 0x0040;
    TIMER_A0->CCR[3] = duty3;
    TIMER_A0->CCTL[4] = 0x0040;
    TIMER_A0->CCR[4] = duty4;

    TIMER_A0->CTL = 0x02F0;
}
#define START_SWITCH_PIN (1 << 1) // Assuming the switch is connected to P1.1

void Switch_Init(void){
    P1->SEL0 &= ~START_SWITCH_PIN; // Configure P1.1 as I/O
    P1->SEL1 &= ~START_SWITCH_PIN; // Configure P1.1 as I/O
    P1->DIR &= ~START_SWITCH_PIN;  // Configure P1.1 as input
    P1->REN |= START_SWITCH_PIN;   // Enable pull-up/pull-down resistors
    P1->OUT |= START_SWITCH_PIN;   // Select pull-up mode
}

void WaitForStart(void){
    while((P1->IN & START_SWITCH_PIN) != 0); // Wait for the switch to be pressed
}



void Motor_Init(){
    P3->SEL0 &= ~0xC0;
    P3->SEL1 &= ~0xC0;
    P3->DIR |= 0xC0;
    P3->OUT &= ~0xC0;

    P5->SEL0 &= ~0x30;
    P5->SEL1 &= ~0x30;
    P5->DIR |= 0x30;
    P5->OUT &= ~0x30;

    P2->SEL0 &= ~0xC0;
    P2->SEL1 &= ~0xC0;
    P2->DIR |= 0xC0;
    P2->OUT &= ~0xC0;

    PWM_Init34(15000, 0,0);
}

void PWM_Duty3(uint16_t duty3){
    TIMER_A0 -> CCR[3] = duty3;
}
void PWM_Duty4(uint16_t duty4){
    TIMER_A0->CCR[4] = duty4;
}

void Move(uint16_t leftDuty, uint16_t rightDuty){
    P3->OUT |= 0xC0;
    PWM_Duty3(rightDuty);
    PWM_Duty4(leftDuty);
}
void Left_Forward(){
    P5->OUT &= ~0x10;
}
void Left_Backward(){
    P5->OUT |= 0x10;
}
void Right_Forward(){
    P5->OUT &= ~0x20;
}
void Right_Backward(){
    P5->OUT |= 0x20;
}

void moveForward(x,y,delay){
    Left_Forward();
    Right_Forward();
    Move(x,y);
    Clock_Delay1ms(delay);
}
void moveBackward(x,y,delay){
    Left_Backward();
    Right_Backward();
    Move(x,y);
    Clock_Delay1ms(delay);
}


//its kind of hard to visualize for me, but if something is mixed up, its probably these functions, try replacing the forward with backwards
//if it's wrong, sorry!

void rotate_Right(x,y,delay){
    Left_Forward();
    Right_Backward();
    Move(x,y);
    Clock_Delay1ms(delay);
}
void rotate_Left(x,y,delay){
    Right_Forward();
    Left_Backward();
    Move(x,y);
    Clock_Delay1ms(delay);
}

void cancelRotate(delay){
    Left_Forward();
    Right_Forward();
    Move(0,0);
    Clock_Delay1ms(delay);
}


void IR_Init(){
    P2->SEL0 &= ~0x07;
    P2->SEL1 &= ~0x07;
    P2->DIR |= 0x07;
    P2->OUT &= ~0x07;
    P5->SEL0 &= ~0x08;
    P5->SEL1 &= ~0x08;
    P5->DIR |= 0x08;
    P5->OUT &= ~0x08;

    P9->SEL0 &= ~0x04;
    P9->SEL1 &= ~0x04;
    P9->DIR |= 0x04;
    P9->OUT &= ~0x04;

    P7->SEL0 &= ~0xFF;
    P7->SEL1 &= ~0xFF;
    P7->DIR |= 0xFF;


    Clock_Delay1ms(10);

}


void trackMovement(){
    int step=0;
    int node=0;
    int angle=0;
    int adjusting=0;


    while(1){

        //IR LEDs
        P5->OUT |= 0x08;
        P9->OUT |= 0X04;

        P7->DIR = 0xFF;
        P7->OUT = 0xFF;

        Clock_Delay1us(10);

        P7->DIR = 0x00;

        Clock_Delay1us(1000);

        int S0 = P7->IN & BIT0;
        int S1 = P7->IN & BIT1;
        int S2 = P7->IN & BIT2;
        int S3 = P7->IN & BIT3;
        int S4 = P7->IN & BIT4;
        int S5 = P7->IN & BIT5;
        int S6 = P7->IN & BIT6;
        int S7 = P7->IN & BIT7;
        //left most is 7

        if(S2&&S3&&S4&&S5){
            if(S0&&S7){
                if(node==0){
                    moveForward(3000,3000,300);
                }
                else if(node>8){
                    moveForward(3000,3000,1000);
                }
            }
            else{
                switch(step){
                    case(0):
                        cancelRotate(500);
                        moveForward(3000,3000,300);
                        cancelRotate(500);
                        rotate_Left(3000,3000,450);
                        cancelRotate(500);
                        moveForward(2000,2000,50);
                        step++;
                        node++;
                        break;
                    case(1):
                        if(node<8){
                            if(angle==1){
                                moveForward(2200,2400,600);
                                cancelRotate(500);
                                rotate_Right(3000,3000,400);
                            }
                            else if(angle==2){
                                moveForward(2400,2200,600);
                                cancelRotate(500);
                                rotate_Right(3000,3000,400);
                            }
                            else{
                                moveForward(3000,3000,300);
                                cancelRotate(500);
                                rotate_Right(3000,3000,450);
                            }
                            cancelRotate(500);
                            moveForward(3000,3000,100);
                            node++;
                            break;
                        }
                        else{
                            //char text[70];
                            //sprintf(text, "Else Node %d", node);
                            //printf("%s\n", text);
                            cancelRotate(500);
                            moveForward(3000,3000,400);
                            cancelRotate(500);
                            rotate_Right(3000,3000,1700);
                            cancelRotate(500);
                            moveForward(3000,3000,400);
                            node++;
                            step++;
                            break;
                        }
                    case(2):
                        if(node<17){
                            cancelRotate(500);
                            moveForward(3000,3000,180);
                            cancelRotate(500);
                            rotate_Left(3000,3000,1500);
                            cancelRotate(500);
                            moveForward(3000,3000,400);
                            node++;
                            break;
                        }

                        else{
                            moveForward(1500,1500,100);
                            step++;
                            break;
                        }
                }

            }

        }
        else if(S2&&!S5){ //if turned slightly left, turn slight right
            angle=2;
//            cancelRotate(200);
            moveForward(2000,500,10);
        }
        else if(!S2&&S5){
            angle=1;//if turned slightly right, turn left
//            cancelRotate(200);
            moveForward(500,2000,10);
        }
        else if(S3&&S4){
            angle=0;//move forward if 4 and 5 detects black
            adjusting=0;
            moveForward(3000,3000,50);
        }
        else if(!S2&&!S3&&!S4&&!S5){
//            cancelRotate(300);
            if(S0||S1||S6||S7){
                if(node<9){
                    if(angle == 1){

                        rotate_Right(1500,1500,100);
                        moveForward(2500,2500,180);

                        adjusting++;

                    }
                    else if(angle == 2){
                        rotate_Left(1500,1500,100);
                        moveForward(2200,2200,180);
                        adjusting++;

                    }

                }else if(node>9){
                    moveBackward(1500,1800,1500);
    //                rotate_Right(1500,1500,500);
                    moveForward(2500,2700,180);

                }

            }
        }

        P5->OUT &= ~0x08;
        P9->OUT &= ~0x04;

        Clock_Delay1ms(10);

    }
}

   int main(void){

    Clock_Init48MHz();
    Motor_Init();
    IR_Init();
    Switch_Init();
    WaitForStart();
    trackMovement();

}
