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
	bmp_file_out = fopen("expanded_nearest.bmp", "wb+");

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

	//用于读取一个像素的RGB
	unsigned char target[3];
	
	//循环变量i,j
	int i = 0;
	int j = 0;
	
	//目标像素位置x,y
	int x = 0;
	int y = 0;
	
	//按序对新图中的每个像素做操作
	for (j = 0; j < NEW_HEIGHT; j++)
	{
		for (i = 0; i < NEW_WIDTH; i++)
		{
			//对应像素在原图中的位置，四舍五入取最邻近点坐标
			x = (int)(i * mul_width + 0.5);
			y = (int)(j * mul_height + 0.5);
		
			//根据最邻近点坐标计算在原图文件中的读取位置
			int offset = 54 + (y * (old_width * 3 + mod_old) + x * 3);
			
			//移动文件读写位置，读取目标像素，写入新文件
			fseek(bmp_file_in, offset, SEEK_SET);
			fread(target, sizeof(unsigned char), 3, bmp_file_in);
			fwrite(target, sizeof(unsigned char), 3, bmp_file_out);
		}
		
		//每行末尾补mod个0
		unsigned char temp = 0;
		for(i = 0; i < mod_new; i++)
			fwrite(&temp, sizeof(unsigned char), 1, bmp_file_out);
	}

	fclose(bmp_file_in);
	fclose(bmp_file_out);
}

