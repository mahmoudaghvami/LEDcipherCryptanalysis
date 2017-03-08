//#define _CRT_SECURE_NO_DEPRECATE
//#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>
//#include <string.h>
//#include <iostream>     
//#include "LEDalgorithm.h"
//#include <stdio.h>
//
//int xorProfil[256][256];
//
//void initXorProfile()
//{
//	for (int i = 0; i < 256; i++)
//	{
//		for (int j = 0; j < 256; j++)
//		{
//			xorProfil[i][j] = 0;
//		}
//	}
//}
//
//void printXorProfile()
//{
//	FILE *f = fopen("XorProfile.txt", "w");
//	for (int i = 0; i < 256; i++)
//	{
//		for (int j = 0; j < 256; j++)
//		{
//			fprintf(f, "%d\t", xorProfil[i][j]);
//		}
//		fprintf(f, "\n");
//	}
//}
//
//void GeneratXorProfile()
//{
//	int deltax;
//	int deltay;
//	initXorProfile();
//	FILE *f = fopen("XorProfile.txt", "w");
//	for (int i = 0; i < 256; i++)
//	{
//		for (int j = 0; j < 256; j++)
//		{
//			deltax ;
//			deltay ;
//			xorProfil[deltax][deltay]++;
//		}
//	}
//	printXorProfile();
//}
//
