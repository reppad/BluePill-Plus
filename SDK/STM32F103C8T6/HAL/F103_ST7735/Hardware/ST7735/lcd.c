#include "lcd.h"
#include "font.h"
#include "spi.h"
#include "tim.h"

//SPI��ʾ���ӿ�
//LCD_RST
#define LCD_RST_SET     HAL_GPIO_WritePin(LCD_RST_GPIO_Port,LCD_RST_Pin,GPIO_PIN_SET)
#define LCD_RST_RESET   HAL_GPIO_WritePin(LCD_RST_GPIO_Port,LCD_RST_Pin,GPIO_PIN_RESET)
//LCD_RS//dc  
#define LCD_RS_SET      HAL_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_SET)//PC4 
#define LCD_RS_RESET    HAL_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_RESET)
//LCD_CS  
#define LCD_CS_SET      HAL_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_SET)
#define LCD_CS_RESET    HAL_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_RESET)
//SPI Driver
#define SPI spi2
#define SPI_Drv (&hspi2)
#define delay_ms HAL_Delay
#define get_tick HAL_GetTick
//LCD_Brightness timer
#define LCD_Brightness_timer &htim2
#define LCD_Brightness_channel TIM_CHANNEL_1


static int32_t lcd_init(void);
static int32_t lcd_gettick(void);
static int32_t lcd_writereg(uint8_t reg,uint8_t* pdata,uint32_t length);
static int32_t lcd_readreg(uint8_t reg,uint8_t* pdata);
static int32_t lcd_senddata(uint8_t* pdata,uint32_t length);
static int32_t lcd_recvdata(uint8_t* pdata,uint32_t length);

/** @defgroup configure device interface 
  * @{
  */
ST7735_IO_t st7735_pIO = {
	lcd_init,
	NULL,
	NULL,
	lcd_writereg,
	lcd_readreg,
	lcd_senddata,
	lcd_recvdata,
	lcd_gettick
};

ST7735_Object_t st7735_pObj;
uint32_t st7735_id;
//extern unsigned char WeActStudiologo[];
extern ST7735_Ctx_t ST7735Ctx;
uint32_t id;
void LCD_Test(void)
{
	uint8_t text[20];
	
	//����ST7735 configure own ST7735 
	ST7735Ctx.Orientation = ST7735_ORIENTATION_PORTRAIT;//ST7735��ʾ����
	ST7735Ctx.Panel = BOE_Panel;			//ST7735 panel
	ST7735Ctx.Type = ST7735_1_8_inch_screen;		//ST7735����
	
	ST7735_RegisterBusIO(&st7735_pObj,&st7735_pIO);
	ST7735_LCD_Driver.Init(&st7735_pObj,ST7735_FORMAT_RBG565,&ST7735Ctx);
	ST7735_LCD_Driver.ReadID(&st7735_pObj,&st7735_id);
	
	LCD_SetBrightness(0);
	
//	ST7735_LCD_Driver.DrawBitmap(&st7735_pObj,0,0,WeActStudiologo);
	ST7735_LCD_Driver.FillRect(&st7735_pObj,0,0,ST7735Ctx.Width,ST7735Ctx.Height,BLACK);
	sprintf((char *)&text,"WeAct Studio");
	LCD_ShowString(18, 80, ST7735Ctx.Width, 16, 16, text);
	
	uint32_t tick=get_tick();
	while(HAL_GPIO_ReadPin(KEY_GPIO_Port,KEY_Pin) != GPIO_PIN_SET)
	{
		delay_ms(10);
		
		if(get_tick()-tick <= 1000) LCD_SetBrightness((get_tick()-tick)*100/1000);
		else if(get_tick()-tick <= 3000)
		{
			sprintf((char *)&text,"%03d",(get_tick()-tick-1000)/10);
			LCD_ShowString(ST7735Ctx.Width - 30,1,ST7735Ctx.Width,16,16,text);
			ST7735_LCD_Driver.FillRect(&st7735_pObj,0,ST7735Ctx.Height - 3,(get_tick()-tick-1000)*ST7735Ctx.Width/2000,3,0xFFFF);
		}
		else if(get_tick()-tick > 3000) break;
	}
	while(HAL_GPIO_ReadPin(KEY_GPIO_Port,KEY_Pin) == GPIO_PIN_SET)
	{
		delay_ms(10);
	}
	LCD_Light(0,300);
	
	ST7735_LCD_Driver.FillRect(&st7735_pObj,0,0,ST7735Ctx.Width,ST7735Ctx.Height,BRRED);
	BACK_COLOR = BRRED;		//���屳����ɫ
	
	sprintf((char *)&text,"WeAct Studio");
	LCD_ShowString(4, 4, ST7735Ctx.Width, 16, 16, text);
	sprintf((char *)&text, "STM32F1xx");
	LCD_ShowString(4, 22, ST7735Ctx.Width, 16, 16, text);
	sprintf((char *)&text,"LCD ID:0x%X",st7735_id);
	LCD_ShowString(4, 40, ST7735Ctx.Width, 16, 16, text);
	
	LCD_Light(100,200);
}

void LCD_SetBrightness(uint32_t Brightness)
{
	__HAL_TIM_SET_COMPARE(LCD_Brightness_timer,LCD_Brightness_channel,Brightness);
}

uint32_t LCD_GetBrightness(void)
{
	return __HAL_TIM_GET_COMPARE(LCD_Brightness_timer,LCD_Brightness_channel);
}

// ��Ļ�𽥱������߱䰵
// Brightness_Dis: Ŀ��ֵ
// time: �ﵽĿ��ֵ��ʱ��,��λ: ms
void LCD_Light(uint32_t Brightness_Dis,uint32_t time)
{
	uint32_t Brightness_Now;
	uint32_t time_now;
	float temp1,temp2;
	float k,set;
	
	Brightness_Now = LCD_GetBrightness();
	time_now = 0;
	if(Brightness_Now == Brightness_Dis)
		return;
	
	if(time == time_now)
		return;
	
	temp1 = Brightness_Now;
	temp1 = temp1 - Brightness_Dis;
	temp2 = time_now;
	temp2 = temp2 - time;
	
	k = temp1 / temp2;
	
	uint32_t tick=get_tick();
	while(1)
	{
		delay_ms(1);
		
		time_now = get_tick()-tick;
		
		temp2 = time_now - 0;
		
		set = temp2*k + Brightness_Now;
		
		LCD_SetBrightness((uint32_t)set);
		
		if(time_now >= time) break;
		
	}
}
	
uint16_t POINT_COLOR=0xFFFF;	//������ɫ
uint16_t BACK_COLOR=BLACK;  //����ɫ 
//��ָ��λ����ʾһ���ַ�
//x,y:��ʼ����
//num:Ҫ��ʾ���ַ�:" "--->"~"
//size:�����С 12/16
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)  

void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode)
{  							  
  uint8_t temp,t1,t;
	uint16_t y0=y;
	uint16_t x0=x;
	uint16_t colortemp=POINT_COLOR; 
  uint32_t h,w;
	
	uint16_t write[size][size==12?6:8];
	uint16_t count;
	
  ST7735_GetXSize(&st7735_pObj,&w);
	ST7735_GetYSize(&st7735_pObj,&h);
	
	//���ô���		   
	num=num-' ';//�õ�ƫ�ƺ��ֵ
	count = 0;
	
	if(!mode) //�ǵ��ӷ�ʽ
	{
		for(t=0;t<size;t++)
		{   
			if(size==12)temp=asc2_1206[num][t];  //����1206����
			else temp=asc2_1608[num][t];		 //����1608����
			
			for(t1=0;t1<8;t1++)
			{			    
				if(temp&0x80)
					POINT_COLOR=(colortemp&0xFF)<<8|colortemp>>8;
				else 
					POINT_COLOR=(BACK_COLOR&0xFF)<<8|BACK_COLOR>>8;
				
				write[count][t/2]=POINT_COLOR;
				count ++;
				if(count >= size) count =0;
				
				temp<<=1;
				y++;
				if(y>=h){POINT_COLOR=colortemp;return;}//��������
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=w){POINT_COLOR=colortemp;return;}//��������
					break;
				}
			}
		}
	}
	else//���ӷ�ʽ
	{
		for(t=0;t<size;t++)
		{   
			if(size==12)temp=asc2_1206[num][t];  //����1206����
			else temp=asc2_1608[num][t];		 //����1608���� 	                          
			for(t1=0;t1<8;t1++)
			{			    
				if(temp&0x80)
					write[count][t/2]=(POINT_COLOR&0xFF)<<8|POINT_COLOR>>8;
				count ++;
				if(count >= size) count =0;
				
				temp<<=1;
				y++;
				if(y>=h){POINT_COLOR=colortemp;return;}//��������
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=w){POINT_COLOR=colortemp;return;}//��������
					break;
				}
			}  	 
		}     
	}
	ST7735_FillRGBRect(&st7735_pObj,x0,y0,(uint8_t *)&write,size==12?6:8,size); 
	POINT_COLOR=colortemp;	    	   	 	  
}   

//��ʾ�ַ���
//x,y:�������
//width,height:�����С  
//size:�����С
//*p:�ַ�����ʼ��ַ
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint8_t *p)
{         
	uint8_t x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//�ж��ǲ��ǷǷ��ַ�!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//�˳�
        LCD_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }  
}

static int32_t lcd_init(void)
{
	int32_t result = ST7735_OK;
	LCD_RST_RESET;
	delay_ms(5);
	LCD_RST_SET;
	HAL_TIM_PWM_Start(LCD_Brightness_timer,LCD_Brightness_channel);
	return result;
}

static int32_t lcd_gettick(void)
{
	return HAL_GetTick();
}

static int32_t lcd_writereg(uint8_t reg,uint8_t* pdata,uint32_t length)
{
	int32_t result;
	LCD_CS_RESET;
	LCD_RS_RESET;
	result = HAL_SPI_Transmit(SPI_Drv,&reg,1,100);
	LCD_RS_SET;
	if(length > 0)
		result += HAL_SPI_Transmit(SPI_Drv,pdata,length,500);
	LCD_CS_SET;
	result /= -result;
	return result;
}

static int32_t lcd_readreg(uint8_t reg,uint8_t* pdata)
{
	int32_t result;
	LCD_CS_RESET;
	LCD_RS_RESET;
	
	result = HAL_SPI_Transmit(SPI_Drv,&reg,1,100);
	LCD_RS_SET;
	result += HAL_SPI_Receive(SPI_Drv,pdata,1,500);
	LCD_CS_SET;
	result /= -result;
	return result;
}

static int32_t lcd_senddata(uint8_t* pdata,uint32_t length)
{
	int32_t result;
	LCD_CS_RESET;
	//LCD_RS_SET;
	result =HAL_SPI_Transmit(SPI_Drv,pdata,length,100);
	LCD_CS_SET;
	result /= -result;
	return result;
}

static int32_t lcd_recvdata(uint8_t* pdata,uint32_t length)
{
	int32_t result;
	LCD_CS_RESET;
	//LCD_RS_SET;
	result = HAL_SPI_Receive(SPI_Drv,pdata,length,500);
	LCD_CS_SET;
	result /= -result;
	return result;
}
