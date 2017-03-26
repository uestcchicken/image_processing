#include <stdio.h>
#include <stdlib.h>

//目标图像大小
#define NEW_WIDTH 1600
#define NEW_HEIGHT 1600

typedef unsigned short WORD;
typedef unsigned int DWORD;

//14字节bmp文件头
typedef struct BMP_FILE_HEADER
{
    WORD bType;         
    DWORD bSize;         
    WORD bReserved1;    
    WORD bReserved2;      
    DWORD bOffset;          
} BITMAPFILEHEADER;

//40字节bmp信息头
typedef struct BMP_INFO
{
    DWORD bInfoSize;     
    DWORD biWidth;       
    DWORD biHeight;   
    WORD bPlanes;        
    WORD bBitCount;     
    DWORD bCompression;  
    DWORD bmpImageSize;
    DWORD bXPelsPerMeter; 
    DWORD bYPelsPerMeter;  
    DWORD bClrUsed;     
    DWORD bClrImportant;
} BITMAPINFOHEADER;

void main()
{
	FILE *bmp_file_in;
	FILE *bmp_file_out;

	bmp_file_in = fopen("input.bmp", "rb");
	bmp_file_out = fopen("expanded_bilinear.bmp", "wb+");

	BITMAPFILEHEADER file_header;
	BITMAPINFOHEADER info_header;

	//读取bmp文件头
	fread(&file_header, sizeof(unsigned char), 14, bmp_file_in);
	fread(&info_header, sizeof(unsigned char), 40, bmp_file_in);
	
	//获取输入图像大小
	const int old_width = info_header.biWidth;
	const int old_height = info_header.biHeight;
	
	//单行字节数不为4的倍数的会补0直至4的倍数，mod为补0数量
	int mod_old = ((old_width * 3) % 4) > 0 ? 4 - ((old_width * 3) % 4) : 0;
	int mod_new = ((NEW_WIDTH * 3) % 4) > 0 ? 4 - ((NEW_WIDTH * 3) % 4) : 0;

	//设定目标文件的文件头，并写入目标文件
	info_header.biWidth = NEW_WIDTH;
	info_header.biHeight = NEW_HEIGHT;
	fwrite(&file_header, sizeof(unsigned char), 14, bmp_file_out);
	fwrite(&info_header, sizeof(unsigned char), 40, bmp_file_out);

	//图像放缩倍数
	float mul_width = (float)old_width / NEW_WIDTH;
	float mul_height = (float)old_height / NEW_HEIGHT;

	//循环变量i,j
	int i = 0;
	int j = 0;
	
	//目标像素位置x,y
	float x = 0;
	float y = 0;
	
	//目标像素周围四点的横纵坐标
	int x1 = 0;
	int y1 = 0;
	int x2 = 0;
	int y2 = 0;

	//目标像素周围四点的灰度值
	unsigned char f11 = 0;
	unsigned char f12 = 0;
	unsigned char f21 = 0;
	unsigned char f22 = 0;

	//按序对新图中的每个像素做操作
	for (j = 0; j < NEW_HEIGHT; j++)
	{
		for (i = 0; i < NEW_WIDTH; i++)
		{
			//对应像素在原图中的位置
			x = (float)i * mul_width;
			y = (float)j * mul_height;			

			//计算周围4点横纵坐标
			x1 = (int)x;
			x2 = x1 + 1;
			y1 = (int)y;
			y2 = y1 + 1;

			//根据每点坐标计算在原图文件中的读取位置，并读取灰度值
			int offset11 = 54 + (y1 * (old_width * 3 + mod_old) + x1 * 3);
			fseek(bmp_file_in, offset11, SEEK_SET);
			fread(&f11, sizeof(unsigned char), 1, bmp_file_in);
			
			int offset12 = 54 + (y1 * (old_width * 3 + mod_old) + x2 * 3);
			fseek(bmp_file_in, offset12, SEEK_SET);
			fread(&f12, sizeof(unsigned char), 1, bmp_file_in);

			int offset21 = 54 + (y2 * (old_width * 3 + mod_old) + x1 * 3);
			fseek(bmp_file_in, offset21, SEEK_SET);
			fread(&f21, sizeof(unsigned char), 1, bmp_file_in);

			int offset22 = 54 + (y2 * (old_width * 3 + mod_old) + x2 * 3);
			fseek(bmp_file_in, offset22, SEEK_SET);
			fread(&f22, sizeof(unsigned char), 1, bmp_file_in);

			//根据周围4点的灰度值计算目标点的灰度值
			float f_down = (float)f11 * ((float)x2 - x) + (float)f21 * (x - (float)x1);
			float f_up = (float)f12 * ((float)x2 - x) + (float)f22 * (x - (float)x1);
			char f_mid = f_down * ((float)y2 - y) + f_up * (y - (float)y1);

			//写入新文件
			fwrite(&f_mid, sizeof(unsigned char), 1, bmp_file_out);
			fwrite(&f_mid, sizeof(unsigned char), 1, bmp_file_out);
			fwrite(&f_mid, sizeof(unsigned char), 1, bmp_file_out);
		}
		
		//每行末尾补mod个0
		unsigned char temp = 0;
		for(i = 0; i < mod_new; i++)
			fwrite(&temp, sizeof(unsigned char), 1, bmp_file_out);
	}

	fclose(bmp_file_in);
	fclose(bmp_file_out);
}

