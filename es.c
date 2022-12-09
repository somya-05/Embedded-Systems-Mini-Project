#include <LPC17xx.h>
#include<stdio.h>
#define RS 0x08000000; //0.27
#define EN 0x10000000; //0.28
#define DT 0x07800000; //0.23-0.26
void init(void);
void clearPorts(void);
void writeCmd(void);
void writeData(void);
void disp(char *);
void disp_with_delay(char *);
void move(int l, int c);
void clear_display(void);
unsigned int i1, j1;
float dist;
char str[16];
int cmd=0x0, data=0x00, flag=0, j,i;
int main (void)
{
char msg[16] = {""};
SystemInit();
SystemCoreClockUpdate();
init();
for(j1=0;j1<10000;j1++);
clear_display();
move(1, 0);
disp(&msg[0]);
LPC_PINCON->PINSEL4 = (1<<20)|(0<<22); // setting P2.10 as echo pin ( function 1 ) and P2.11 as trigger Output ( function 0 )
LPC_GPIO2->FIODIR = (0x1<<11); // setting P2.11 as Trigger Output LPC_SC->EXTINT=0x01;
LPC_SC->EXTMODE = 0x01;
LPC_SC->EXTPOLAR = 0x01;
LPC_TIM0->TCR=0x02;
LPC_TIM0->PR=0x02;
LPC_TIM0->CTCR=0x00;
LPC_TIM0->MR0 = 9;
//Set Timer 0 as trigger for sensor
LPC_TIM0->MCR = 0x03;
LPC_TIM1->TCR = 0x02;
//Timer 1 used to measure sensor distance
LPC_TIM1->PR = 0x2;
LPC_TIM2->TCR=0x02;
LPC_TIM2->PR=0x02;
LPC_TIM2->CTCR=0x00;
LPC_TIM2->MR1 = 59999;
    LPC_TIM2->MCR = (0x03<<3); NVIC_EnableIRQ(TIMER0_IRQn); NVIC_EnableIRQ(EINT0_IRQn); LPC_TIM0->TCR = 0x01; LPC_GPIO2->FIOSET = 1<<11; }
void init()
{
LPC_PINCON->PINSEL1 = 0xFC003FFF;
LPC_GPIO0->FIODIRH = 0x1F80;
clearPorts();
flag=0;
for(j=0;j<3200;j++);
for(i=0;i<3;i++) //wake up
{
cmd = 0x3<<23;
writeCmd();
for(j=0;j<30000;j++);
}
cmd = 0x2<<23; //return home
writeCmd();
for(j=0;j<30000;j++);
data = 0x28; //inform that there are 2 lines, default font writeData();
for(j=0;j<30000;j++);
data = 0x01; //clear display
writeData();
for(j=0;j<10000;j++);
data = 0x06; //increment cursor after writing, don't shift data writeData
();
for(j=0;j<800;j++);
data = 0x80; //move to first line, first column
writeData();
for(j=0;j<800;j++);
data = 0x0C; //switch on LCD, show cursor and blink writeData();
for(j=0;j<800;j++);
}
void clearPorts()
{
LPC_GPIO0->FIOCLRH = 0x1F80; }
void
writeCmd()
{
clearPorts(); LPC_GPIO0->FIOPIN = cmd; if(flag==0)
{
 
    LPC_GPIO0->FIOCLR = RS; }
else if(flag==1)
{
LPC_GPIO0->FIOSET = RS; }
LPC_GPIO0->FIOSET = EN; for(j=0;j<50;j++); LPC_GPIO0->FIOCLR = EN; }
void writeData()
{
cmd = (data& 0xF0)<<19; writeCmd();
cmd = (data & 0x0F)<<23; writeCmd();
for(j=0;j<1000;j++);
}
void disp(char *to_disp)
{
i = 0;
while(to_disp[i]!='\0')
{
data = to_disp[i];
flag=1;
writeData();
i++;
}
}
void disp_with_delay(char *to_disp) {
i = 0;
while(to_disp[i]!='\0')
{
data = to_disp[i];
flag=1;
writeData();
i++;
for(j=0;j<50000;j++);
}
}
void move(int l, int c)
{
flag=0;
if(l==1)
data = 0x80;
else
data = 0xC0;
data = data+c;
writeData();
for(j=0;j<800;j++);
 
    }
void clear_display() {
flag = 0;
data = 0x01; writeData(); for(j=0;j<10000;j++); }
void TIMER0_IRQHandler() {
LPC_TIM0->IR = 0x01; LPC_TIM2->TCR=0x01; LPC_GPIO2->FIOSET = 1<<11; LPC_TIM0->TCR=0x00;
}
void TIMER2_IRQHandler()
{
LPC_TIM2->IR = 0x01;
LPC_GPIO2->FIOCLR = 1<<11;
LPC_TIM2->TCR=0x00;
}
void EINT0_IRQHandler()
//Echo trigger handler
{
LPC_SC->EXTINT = 0x01;
if(LPC_SC->EXTPOLAR&0x01)
{
LPC_SC->EXTPOLAR = 0X00;
dist = echo_monitor();
sprintf(str , "%3.2fcm " , dist);
disp(&str[0]);
for(j1=0;j1<10000;j1++);
LPC_TIM0->TCR=0x02;
LPC_TIM0->TCR=0x01;
LPC_GPIO2->FIOSET = 1<<11;
}
}
int echo_monitor(){
float pulse_time = 0,distance=0;
while((LPC_GPIO0->FIOPIN & (0x1<<10)) == 0x0); //Wait till echo is low startTimer0(); //Initialize the echo timer
while((LPC_GPIO0->FIOPIN & (0x1<<10)) == 0x1<<16); //Wait till echo is high pulse_time = stopTimer0(); //Get count of echo timer
distance = (0.343*pulse_time)/2;
return round_val(distance);
}
void startTimer0(void)
{
LPC_TIM0->TCR = 0x02; //Reset Timer
 
LPC_TIM0->TCR = 0x01; //Enable timer }
unsigned int stopTimer0(void)
{
LPC_TIM0->TCR = 0x00; //Disable timer return LPC_TIM0->TC;
}
int round_val(float num){
float val = num - abs(num); if(val <= 0.5)
return (int)num;
else
return (int)num + 1; 
}
