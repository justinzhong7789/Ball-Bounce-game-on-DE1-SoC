#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
	
#define white 0xffff
#define black 0
#define netX 159
#define yBound 239
#define defaultleftPlateX 64
#define defaultrightPlateX 224
#define plateY 229
#define defaultBallX 158
#define defaultBallY 50
#define defaultBallVx 4
#define defaultBallVy 0
#define a_y 1 //mimic downward acceleration
#define defaultNetNum 5
#define defaultVy 20
#define makeleftArrow 0xe06b
#define makerightArrow 0xe074
#define make_a 0x1c
#define make_d 0x23
#define breakleftArrow 0xe0f06b
#define breakrightArrow 0xe0f074
#define break_a 0xf01c
#define break_d 0xf023
#define make_enter 0x5a
#define break_enter 0xf05a
//========= global function declarations==============
void clear_screen();
void plot_pixel(int x, int y, short int line_color);
void wait_for_vsync();
void draw_char(int * char_buffer, int x, int y, char c);
void draw_plates();
void draw_net();
void draw_ball();
void ball_movement();
void erase_ball();
void moveplate();
void reset_board();
void bounce();
void reset_ball_status();
void erase_plate();
void read_ps2();
void move_plate();
void bounceNet();
void resetNet();
int getOne(int a);
int getTen(int a);
void display_score();
//=====================================================
//========= global variable declarations===============
volatile int  pixel_buffer_start;	
volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
volatile int * char_buffer = (int *) 0xc9000000;
volatile int * ps2_base = (int *) 0xff200100;
volatile int * hex_base = (int *) 0xff200020;
int leftPlateX = 64;
int rightPlateX = 224;
int netNum = defaultNetNum;
int ball_loc[2] = {defaultBallX, defaultBallY};
int ball_v[2] = {defaultBallVx, defaultBallVy};
int prev_ball_loc[2];
int leftScore=0;
int rightScore = 0;
int bounceNum = 1;
bool gameOver = false;
bool ball_fell = false;
bool a_pressed  = false;
bool d_pressed = false;
bool left_arrow_pressed = false;
bool right_arrow_pressed = false;
bool enter_pressed = false;
char byte1=0, byte2=0, byte3=0; //used to store the ps2 keyboard
int ps2_data, RVALID;
char segpattern[] ={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
//======================================
int main (void){
	*(pixel_ctrl_ptr+1) = 0xc8000000;
	wait_for_vsync();
	//*(pixel_ctrl_ptr+1) = 0xc0000000;
	pixel_buffer_start = *(pixel_ctrl_ptr+1);
enter:
	while(enter_pressed){
		read_ps2();
	}
	netNum = defaultNetNum;
	ball_loc[0] = defaultBallX;
	ball_loc[1] = defaultBallY;
	ball_v[1] = defaultBallVy;
	ball_v[0] = defaultBallVx;
	leftPlateX = defaultleftPlateX;
	rightPlateX = defaultrightPlateX;
	leftScore = 0;
	rightScore = 0;
	clear_screen();
	draw_ball();
	draw_plates();
	draw_net();
	wait_for_vsync();
	
	
	while(!gameOver){
		while(!ball_fell){ 
			
			ball_movement();
			read_ps2();
			draw_net();
			move_plate();
			wait_for_vsync();
			gameOver = (leftScore>100 || rightScore>100);
			bounce();
			bounceNet();
			if(enter_pressed){
				goto enter;
			}
		}
		ball_fell = false;
		display_score();
		reset_board();
	}
	while(1){
		if(enter_pressed){
			goto enter;
		}
	}
}
void display_score(){
	int rightScoreOne = getOne(rightScore);
	int rightScoreTen = getTen(rightScore);
	int leftScoreTen = getTen(leftScore);
	int leftScoreOne = getOne(leftScore);
	char seg[4];
	seg[0] = segpattern[rightScoreOne];
	seg[1] = segpattern[rightScoreTen];
	seg[2] = segpattern[leftScoreOne];
	seg[3] = segpattern[leftScoreTen];
		
	*(hex_base) =*(int*) seg;
}
int getTen(int a){
	int b= a;
	int ten = 0;
	while(b>=10){
		b= b-10;
		ten++;
		
	}
	return ten;
}
int getOne(int a){
	if(a<10) {return a;}
	else{
		while(a>=10){
			a= a-10;
		}
		return a;
	}
}
void bounceNet(){
	if(ball_loc[0]>netX ){
		if(ball_loc[0]+ball_v[0] <= netX){
			if(ball_loc [1]> yBound-netNum *10){
				ball_v[0] =-1* ball_v[0];
			}
		}
	}
	else if(ball_loc[0]<netX){
		if(ball_loc[0]+ball_v[0] >= netX){
			if(ball_loc [1]> yBound-netNum *10){
				ball_v[0] =-1* ball_v[0];
			}
		}
	}

}
void move_plate(){
	erase_plate();
	if(a_pressed){leftPlateX = leftPlateX -2;}
	if(d_pressed){leftPlateX = leftPlateX +2;}
	if(left_arrow_pressed){rightPlateX = rightPlateX -2;}
	if(right_arrow_pressed){rightPlateX = rightPlateX +2;}
	draw_plates();

}

void erase_plate(){
	for(int i = 0;i<=30; i++){
		for (int j = 0; j<4; j++){
			if(leftPlateX+i <= 159){
				plot_pixel(leftPlateX+i, plateY+j, 0);
			}
			if(rightPlateX+i <= 319){
				plot_pixel(rightPlateX+i, plateY+j, 0);
			}
		}
	}
}



void reset_board(){
	erase_plate();
	erase_ball();
	leftPlateX = defaultleftPlateX;
	rightPlateX = defaultrightPlateX;
	reset_ball_status();
	netNum = defaultNetNum;
	resetNet();
	bounceNum = 0;
}
void resetNet(){
	int clear_start = yBound-defaultNetNum*10;
	while(clear_start>=0){
		for(int i=0;i< 2;i++){
			plot_pixel(netX+i, clear_start, black);
			
		}
		clear_start--;
	}
}
void reset_ball_status(){
	ball_loc[0] = defaultBallX;
	ball_loc[1] = defaultBallY;
	ball_v[0] = defaultBallVx;
	ball_v[1] = defaultBallVy;
	
}

void ball_movement(){
	//prev_ball_loc[0] = ball_loc[0];
	//prev_ball_loc[1] = ball_loc[1];
	erase_ball();
	ball_v[1] = ball_v[1] + a_y;
	ball_loc[1] = ball_loc[1]+ball_v[1];
	
	ball_loc[0] = ball_loc[0]+ball_v[0];
	draw_ball();
}

void draw_ball(){
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			if(ball_loc[0]+i<320 && ball_loc[1]+j < 240){
				plot_pixel(ball_loc[0]+i, ball_loc[1]+j, white);
			}
		}
	}
}
void erase_ball(){
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			if(ball_loc[0]+i < 320 &&ball_loc[1] <240){
				plot_pixel(ball_loc[0]+i,ball_loc[1]+j, 0);
			}
		}
	
	}
}
void draw_net(){
	for(int i=0;i<netNum;i++){
		for(int j=0; j< 10; j++){
			int start = yBound - i*10;
			for(int k =0;k<2;k++){
				if(j<5){
					plot_pixel(netX+k, start-j, white);
				}
			}
		}	
		
	}
}
void draw_plates(){
	//draw plate to be 10 pixel wide, and 3 in height
	for(int i = 0;i<=30; i++){
		for (int j = 0; j<4; j++){
			if(leftPlateX+i <= 159){
				plot_pixel(leftPlateX+i, plateY+j, white);
			}
			if(rightPlateX+i <= 319){
				plot_pixel(rightPlateX+i, plateY+j, white);
			}
		}
	}

}
void draw_char(int * char_buffer, int x, int y, char c){
	*(char *)(char_buffer + (y<<7)+(x)) = c;

}
void clear_screen(){
	for(int x=0;x<320;x++){
		for(int y = 0; y<240;y++){
			plot_pixel(x,y,0);
		}
	}
}
void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void wait_for_vsync(){
	*pixel_ctrl_ptr = 1;
	int sBit = *(pixel_ctrl_ptr +3)& 0b1;
	while(sBit != 0){sBit = *(pixel_ctrl_ptr +3)& 0b1;}
 	pixel_buffer_start = *(pixel_ctrl_ptr +1); // new back buffer
	
	
}
void bounce(){
	if(ball_loc[1] + ball_v[1] >= plateY){//if ball is gonna drop below the plate
		// ball has a velocity 20 at default when reaching the plate at start
		//check which plate it falls on 
		if(ball_loc[0] +ball_v[0] >= leftPlateX &&ball_loc[0] +ball_v[0] <leftPlateX +30){
			//fals on left plate
			bounceNum++;
			if(ball_loc[0] + ball_v[0] >=leftPlateX+12 && ball_loc[0] +ball_v[0]<=leftPlateX+17){
			//falls in middle, go straight up
				ball_v[1] = -1 * defaultVy;
				ball_v[0] = 0 ;
			}
			//0-2
			else if(ball_loc[0] + ball_v[0] >=leftPlateX && ball_loc[0] +ball_v[0] <=leftPlateX+2){
				ball_v[1] = -4;
				ball_v[0] = -10 ; 
			}
			//3-5
			else if(ball_loc[0] + ball_v[0] >=leftPlateX+3 && ball_loc[0] +ball_v[0] <=leftPlateX+5){
				ball_v[1] = -8;
				ball_v[0] = -7;
			}
			//6-8
			else if(ball_loc[0] + ball_v[0] >=leftPlateX+6 && ball_loc[0] +ball_v[0]<=leftPlateX+8){
				ball_v[1] = -12;
				ball_v[0] = -5;
			}
			//9-11
			else if(ball_loc[0] + ball_v[0] >=leftPlateX+9 && ball_loc[0] +ball_v[0] <=leftPlateX){
				ball_v[1] = -16;
				ball_v[0] = -2;
			}
			//18-20
			else if(ball_loc[0] + ball_v[0] >=leftPlateX+18 && ball_loc[0] +ball_v[0] <=leftPlateX+20){
				ball_v[1] = -16;
				ball_v[0] = 2;
			}
			//21-23
			else if(ball_loc[0] + ball_v[0] >=leftPlateX+21 && ball_loc[0] +ball_v[0] <=leftPlateX+23){
				ball_v[1] = -12;
				ball_v[0] = 5;
			}
			//24-26
			else if(ball_loc[0] + ball_v[0] >=leftPlateX+24 && ball_loc[0] +ball_v[0] <=leftPlateX+29){
				ball_v[1] = -8;
				ball_v[0] = 7;
			}
			//27-29
			else if(ball_loc[0] + ball_v[0] >=leftPlateX+27 && ball_loc[0] +ball_v[0] <=leftPlateX+29){
				ball_v[1] = -4;
				ball_v[0] = 10;
			}
		}
		else if(ball_loc[0] + ball_v[0] >=rightPlateX  && ball_loc[0] + ball_v[0] < rightPlateX +30){
			//fall on the right plate
			bounceNum++;
			if(ball_loc[0] + ball_v[0] >=rightPlateX+12 && ball_loc[0] +ball_v[0]<=rightPlateX+17){
			//falls in middle, go straight up
				ball_v[1] = -1 * defaultVy;
				ball_v[0] = 0 ;
			}
			//0-2
			else if(ball_loc[0] + ball_v[0] >=rightPlateX && ball_loc[0] +ball_v[0] <=rightPlateX+2){
				ball_v[1] = -4;
				ball_v[0] = -10;
			}
			//3-5
			else if(ball_loc[0] + ball_v[0] >=rightPlateX+3 && ball_loc[0] +ball_v[0] <=rightPlateX+5){
				ball_v[1] = -8;
				ball_v[0] = -7;
			}
			//6-8
			else if(ball_loc[0] + ball_v[0] >=rightPlateX+6 && ball_loc[0] +ball_v[0] <= rightPlateX+8){
				ball_v[1] = -12;
				ball_v[0] = -5;
			}
			//9-11
			else if(ball_loc[0] + ball_v[0] >=rightPlateX+9 && ball_loc[0] +ball_v[0] <=rightPlateX+11){
				ball_v[1] = -16;
				ball_v[0] = -2;
			}
			//18-20
			else if(ball_loc[0] + ball_v[0] >=rightPlateX+18 && ball_loc[0] +ball_v[0] <=rightPlateX+20){
				ball_v[1] = -16;
				ball_v[0] = 2;
			}
			//21-23
			else if(ball_loc[0] + ball_v[0] >=rightPlateX+21 && ball_loc[0] +ball_v[0] <=rightPlateX+23){
				ball_v[1] = -12;
				ball_v[0] = 5;
			}
			//24-26
			else if(ball_loc[0] + ball_v[0] >=rightPlateX+24 && ball_loc[0] +ball_v[0]<=rightPlateX+29){
				ball_v[1] = -8;
				ball_v[0] = 7;
			}
			//27-29
			else if(ball_loc[0] + ball_v[0] >=rightPlateX+27 && ball_loc[0] +ball_v[0] <=rightPlateX+29){
				ball_v[1] = -4;
				ball_v[0] = 10;
			}
		}
		else{
			//ball doesnt fall on the plate, check who failed 
			ball_fell = true;
			if(ball_loc[0] <netX){rightScore++;}
			else if(ball_loc[0]>netX){leftScore++;}
		}
		if(bounceNum%10==0){
			netNum++;
		}
		
	}
}
void read_ps2(){
	ps2_data = *(ps2_base);
	RVALID = ps2_data & 0x8000;
	if(RVALID) {
		byte1 = byte2;
		byte2 = byte3;
		byte3 = ps2_data & 0xff;
	}
	if((byte2<<8|byte3) == makeleftArrow){
		left_arrow_pressed = true;
	}
	if((byte2<<8|byte3) == makerightArrow){
		right_arrow_pressed = true;
	}
	if((byte3) == make_a){
		a_pressed = true;
	}
	if((byte3) == make_d){
		d_pressed = true;
	}
	if((byte1<< 16|byte2<<8|byte3) == breakleftArrow){
		left_arrow_pressed = false;
	}
	if((byte1<< 16|byte2<<8|byte3) == breakrightArrow){
		right_arrow_pressed = false;
	}
	if((byte2<<8|byte3)== break_a){
		a_pressed = false;
	}
	if((byte2<<8|byte3) == break_d){
		d_pressed = false;
	}
	if(byte3 == make_enter){
		enter_pressed = true;
	}
	if((byte2<<8|byte3) == break_enter){
		enter_pressed  = false;
	}
}
