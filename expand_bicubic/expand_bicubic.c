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

//绝对值函数
float absolute(float input)
{
	return input > 0 ? input : -input;
}

//幂函数
float power(float input, int power)
{
	int i;
	float output = 1.0;
	for(i = 0; i < power; i++)
		output *= input;
	return output;
}

//基函数
float S(float input)
{
	input = absolute(input);
	if (input < 1)
		return 1 - 2 * power(input, 2) + power(input, 3);
	else if (input < 2)
		return 4 - 8 * input + 5 * power(input, 2) - power(input, 3);
	else
		return 0;
}

void main()
{
	FILE *bmp_file_in;
	FILE *bmp_file_out;

	bmp_file_in = fopen("input.bmp", "rb");
	bmp_file_out = fopen("expanded_bicublic.bmp", "wb+");

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
	
	//目标像素位置m+u,n+v
	int m = 0;
	int n = 0;
	float u = 0;
	float v = 0;
	
	//按序对新图中的每个像素做操作
	for (j = 0; j < NEW_HEIGHT; j++)
	{
		for (i = 0; i < NEW_WIDTH; i++)
		{
			//对应像素在原图中的位置
			x = (float)i * mul_width;
			y = (float)j * mul_height;			

			//转换为m+u,n+v形式
			m = (int)x;
			u = x - m;
			n = (int)y;
			v = y - n;
			
			//用于存储周围16个点的灰度值
			unsigned char f[4][4] = {0};

			//根据每点坐标计算在原图文件中的读取位置，并读取灰度值
			int p = 0;
			int q = 0;
			int offset = 0;
			for (p = 0; p < 4; p++)
				for (q = 0; q < 4; q++)
				{
					//若超出图像边界则按灰度值为0计算
					if ((n - 1 + q < 0) || (m - 1 + p < 0) || (m - 1 + p >= old_width) || (n - 1 + q >= old_height))
						f[p][q] = 0;
					
					else
					{
						offset = 54 + ((n - 1 + q) * (old_width * 3 + mod_old) + (m - 1 + p) * 3);
						fseek(bmp_file_in, offset, SEEK_SET);
						fread(&f[p][q], sizeof(unsigned char), 1, bmp_file_in);
					}
				}
			
			//矩阵运算得到该点最终的灰度值
			float temp1 = S(1 + u) * (float)f[0][0] + S(u) * (float)f[0][1] + S(1 - u) * (float)f[0][2] + S(2-u) * (float)f[0][3];
			float temp2 = S(1 + u) * (float)f[1][0] + S(u) * (float)f[1][1] + S(1 - u) * (float)f[1][2] + S(2-u) * (float)f[1][3];
			float temp3 = S(1 + u) * (float)f[2][0] + S(u) * (float)f[2][1] + S(1 - u) * (float)f[2][2] + S(2-u) * (float)f[2][3];
			float temp4 = S(1 + u) * (float)f[3][0] + S(u) * (float)f[3][1] + S(1 - u) * (float)f[3][2] + S(2-u) * (float)f[3][3];
			
			char result = temp1 * S(1 + v) + temp2 * S(v) + temp3 * S(1 - v) + temp4 * S(2 - v);
			
			//写入新文件
			fwrite(&result, sizeof(unsigned char), 1, bmp_file_out);
			fwrite(&result, sizeof(unsigned char), 1, bmp_file_out);
			fwrite(&result, sizeof(unsigned char), 1, bmp_file_out);
		}
		
		//每行末尾补mod个0
		unsigned char temp = 0;
		for(i = 0; i < mod_new; i++)
			fwrite(&temp, sizeof(unsigned char), 1, bmp_file_out);
	}

	fclose(bmp_file_in);
	fclose(bmp_file_out);

}

