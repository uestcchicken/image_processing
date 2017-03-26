#include <stdio.h>
#include <stdlib.h>

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
	bmp_file_out = fopen("sharpened.bmp", "wb+");

	BITMAPFILEHEADER file_header;
	BITMAPINFOHEADER info_header;
	
	//读写文件头
	fread(&file_header, sizeof(unsigned char), 14, bmp_file_in);
	fread(&info_header, sizeof(unsigned char), 40, bmp_file_in);
	fwrite(&file_header, sizeof(unsigned char), 14, bmp_file_out);
	fwrite(&info_header, sizeof(unsigned char), 40, bmp_file_out);

	//获取输入图像大小
	const int width = info_header.biWidth;
	const int height = info_header.biHeight;
	
	//单行字节数不为4的倍数的会补0直至4的倍数，mod为补0数量
	int mod = ((width * 3) % 4) > 0 ? 4 - ((width * 3) % 4) : 0;

	//按序对图中的每个像素做操作
	int i;
	int j;
	int p;
	int q;
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			//用于存储周围3*3个点的灰度值
			unsigned char f[3][3] = {0};
	
			//读取滤波器范围内各点灰度值
			for (p = 0; p < 3; p++)
				for (q = 0; q < 3; q++)
				{
					//若超出图像边界则按灰度值为0计算
					if ((i - 1 + p < 0) || (j - 1 + q < 0) || (i - 1 + p >= width) || (j - 1 + q >= height))
						f[p][q] = 0;
					
					//else
					{
						int offset = 54 + ((j - 1 + q) * (width * 3 + mod) + (i - 1 + p) * 3);
						fseek(bmp_file_in, offset, SEEK_SET);
						fread(&f[p][q], sizeof(unsigned char), 1, bmp_file_in);
					}
				}
			
			//使用拉普拉斯滤波器模板计算结果
			//中间8周围8个点-1的滤波器
			int laplacian = (int)f[1][1] * (8) - (int)f[0][0] - (int)f[0][1] - (int)f[0][2] - (int)f[1][0] - (int)f[1][2] - (int)f[2][0] - (int)f[2][1] - (int)f[2][2];
			//中间4周围4个点-1的滤波器
			//int laplacian = (int)f[1][1] * (4) - (int)f[0][1] - (int)f[1][0] - (int)f[1][2] - (int)f[2][1];
			
			laplacian = laplacian > 0 ? laplacian : -laplacian;
			laplacian = laplacian < 256 ? laplacian : 255;
			int after = (laplacian + (int)f[1][1]) < 256 ? (laplacian + (int)f[1][1]) : 255;
			//输出锐化后图像
			unsigned char result = (unsigned char)after;
			//输出拉普拉斯滤波图像
			//unsigned char result = (unsigned char)laplacian;
			
			//写入新图像
			fwrite(&result, sizeof(unsigned char), 1, bmp_file_out);
			fwrite(&result, sizeof(unsigned char), 1, bmp_file_out);
			fwrite(&result, sizeof(unsigned char), 1, bmp_file_out);
		}
		
		//每行末尾补mod个0
		unsigned char temp = 0;
		for(i = 0; i < mod; i++)
			fwrite(&temp, sizeof(unsigned char), 1, bmp_file_out);
	}

	fclose(bmp_file_in);
	fclose(bmp_file_out);
}

















