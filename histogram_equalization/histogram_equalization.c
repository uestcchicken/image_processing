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
	bmp_file_out = fopen("histogram_equalized.bmp", "wb+");

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

	//用于统计每个灰度值的出现次数
	int counter[256] = {0};
	
	unsigned char temp;
	int i, j, k;
	//按序对每个像素读取灰度值并记录
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			fread(&temp, sizeof(unsigned char), 1, bmp_file_in);
			counter[(int)temp]++;
			fseek(bmp_file_in, 2, SEEK_CUR);
		}
		fseek(bmp_file_in, mod, SEEK_CUR);
	}

	//校验像素总数
	int check_sum = 0;
	for(i = 0; i < 256; i++)
		check_sum += counter[i];
	printf("check_sum: %d\n", check_sum);

	//计算每个灰度值的出现频率
	float p[256] = {0.0};
	
	//使用直方图均衡算法进行处理
	for(i = 0; i < 256; i++)
		p[i] = (float)counter[i] / check_sum;

	//用于存储每个灰度值对应新灰度值的映射
	int f[256] = {0};	
	float sum_p = 0.0;
	for(i = 0; i < 256; i++)
	{
		sum_p = 0.0;		
		for(j = 0; j <= i; j++)
			sum_p += p[j];
		sum_p *= 255;
		f[i] = (int)sum_p + 0.5;
	}
					
	//输出映射表
	//for(i = 0; i < 256; i++)
	//	printf("%d --> %d\t", i, f[i]);

	//移动文件读写位置至第1个像素处
	fseek(bmp_file_in, 54, SEEK_SET);
	
	//按序对每个像素读取灰度值，并将映射的新灰度值写入新图像
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			for (k = 0; k < 3; k++)
			{
				fread(&temp, sizeof(unsigned char), 1, bmp_file_in);
				temp = (unsigned char)f[(int)temp];
				fwrite(&temp, sizeof(unsigned char), 1, bmp_file_out);
			}
		}
		//每行末尾补mod个0
		unsigned char t = 0;
		for(i = 0; i < mod; i++)
			fwrite(&t, sizeof(unsigned char), 1, bmp_file_out);
	}

	fclose(bmp_file_in);
	fclose(bmp_file_out);
}

















