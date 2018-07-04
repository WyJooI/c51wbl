#include<reg52.h>
sbit cs=P2^0;
sbit clk=P2^1;
sbit dat=P2^2;
sbit key=P2^3;
sbit P_kl=P1^0;
sbit P_jjj=P1^1;
sbit P_jl=P1^2;
sbit P_out=P1^3;
sbit P_lum=P1^4;
sbit P_hl1=P1^7;
sbit P_hl2=P1^6;
sbit P_hl3=P1^5;
#define RESET 0xa4
#define TEST 0xbf
#define DECODE0 0x80	//下载数据方式0--0-9 ehlp
#define DECODE1 0xc8	 //下载数据方式1  --0-9a-f
#define READ 0x15		  //读取键盘
#define UNDECODE 0x90	   //下载不译码
#define RTL_CYCLE 0xa3	   //循环左移
#define RTR_CYCLE 0xa2		//循环右移
#define RTL_UNCYL 0xa1	   //左移
#define RTR_UNCYL 0xa0	   //右移
#define ACTCTL 0x98		   //xiaoyin
#define SEGON 0xe0		   //duanliang
#define SEGOFF 0xc0
#define BLINKCTL 0x88	   //闪烁

unsigned char PWM_T,tk,tp,Q_fa,tm;//PWM_T是pwm周期,tk档位数值，tp是pwm单位时间,Q_fa是工作状态标志位,tm倒计时单位时间；
signed char sec1,sec2,mi1,mi2,Nset,*set;	//sec1秒十位，sec2秒个位，mi1分十位，mi2分个位，set当前设定位
void S_Delay(void)
{
    unsigned char i;
	for(i=0;i<8;i++);
}
void L_Delay(void)
{
    unsigned char i;
	for(i=0;i<0x30;i++);
}
void L_NDelay(unsigned char n)
{
    unsigned int i,j;
	for(i=0;i<n;i++)
	for(j=0;j<0x450;j++);
}
void baojin(int i)
{
	int n=i;
	while(i--){
	    P_jl=0;
	    L_NDelay(10);
	    P_jl=1; 
		if(n!=1)L_NDelay(30);
	}
}
void send_byte(unsigned char out_b)
{
    unsigned char i;
	cs=0;
	L_Delay();
	for(i=0;i<8;i++){
	    if(out_b&0x80){
		    dat=1;			
		}
		else{
		    dat=0;
		}
		clk=1;
		S_Delay();
		clk=0;
		S_Delay();
		out_b=out_b*2;
	}
	dat=0;
}
unsigned char receive_byte(void)
{
	unsigned char i, in_byte;
	dat=1;				
	L_Delay();
	for (i=0;i<8;i++)
	{
		clk=1;
		S_Delay();     //输出键盘数据建立时间
		in_byte=in_byte*2;
		if (dat)
		{
			in_byte=in_byte|0x01;
		}
		clk=0;
		S_Delay();
	}
	dat=0;
	return (in_byte);
}
void write(unsigned char cmd, unsigned char dta)
{
	send_byte (cmd);
	send_byte (dta);
}
unsigned char read(unsigned char command)
{
	send_byte(command);
	return(receive_byte());
}
void display()
{
    unsigned char cmd=DECODE1,i=0;
	write(DECODE1,mi1);
	write(DECODE1+1,mi2);
	write(DECODE1+2,sec1);
	write(DECODE1+3,sec2);
	write(DECODE1+5,tk);
	if(Nset==1)i=0xf7;
	else if(Nset==2)i=0xfb;
	else if(Nset==3)i=0xfd;
	else if(Nset==4)i=0xfe;
	else if(Nset==5)i=0xdf;
	else i=0xff;
	write(BLINKCTL,i);
	if(P_kl==0)P_lum=0;
	else P_lum=1;
	////////////////////////////////////闪烁未写   火显示
}
void work()
{
    unsigned char i=0;
	TMOD=0x01;
	TH0=(65536-45870)/256;
	TL0=(65536-48570)/256;
	EA=1;
	ET0=1;
	TR0=1;
	tm=0;  tp=0;  P_out=0; PWM_T=10;
	while(Q_fa){
	    display();
	    if((mi1+mi2+sec1+sec2)==0)Q_fa=0;
		if(P_lum==1)Q_fa=0;
	}
	display();
	EA=0;
	TR0=0;
	P_out=1;
}
void T0_time()interrupt 1
{
    TH0=(65536-45870)/256;
	TL0=(65536-48570)/256;
	tm++;
	if(tm==20){
	    tp++;
		if(tp==tk)P_out=1;
		if(tp==PWM_T){tp=0;P_out=0;}
		sec2--;
		if(sec2<0){sec1--;sec2=9;}
		if(sec1<0){mi2--;sec1=5;}
		if(mi2<0){mi1--;mi2=9;}
		tm=0;
	}
}
void keydewi(unsigned char key)
{
    /*
        k7 03  k8 0b	  k9 13	 ka 16
        k4 02  k5 0a	  k6 12	 kb 1a
        k1 01  k2 09      k3 11  kc 19   
        k0 00  kf 08	  ke 10	 kd 18
        开始：kd   切换：kf  火力：ke   数字键：k0-k9
	*/
    if(key==0xff){return;}
	else { 
	while(read(READ)!=0xff);
	baojin(1);
	if(key==0x18){Q_fa=1; return;}	//开始
	else if(key==0x08){    
		Nset++; 			 //切换
		if(Nset>4)Nset=1; 
		if(Nset==1)set=&sec2;
		if(Nset==2)set=&sec1;
		if(Nset==3)set=&mi2;
		if(Nset==4)set=&mi1;
		L_Delay();
	//	P1=0;
		return;
	}			
	else if(key==0x10){ Nset=5; set=&tk;  return; }			//火力
	
	else if(key==0x00){*set=0;return;}
	else if(key==0x01){*set=1;return;} else if(key==0x09){*set=2;return;} else if(key==0x11){*set=3;return;}
	else if(key==0x02){*set=4;return;} else if(key==0x0a){*set=5;return;} else if(key==0x12){*set=6;return;}
	else if(key==0x03){*set=7;return;} else if(key==0x0b){*set=8;return;} else if(key==0x13){*set=9;return;}
	
	}
}
void main()
{
	while(1){
        mi1=0;mi2=0;sec1=0;sec2=0;
  	    send_byte(RESET);
	    Q_fa=0;	 tk=3; Nset=1; set=&sec2; P1=0xff;
	    while(1){
		    if(Q_fa==1&&P_lum==0){break;}
			if(Q_fa==1&&P_lum==1){baojin(3);}
			Q_fa=0;
			keydewi(read(READ));
			display();
			
		}
		if(Q_fa==1&&P_lum==0){
		    Nset=5;
			work();
			baojin(5);
		}
	 }
}
