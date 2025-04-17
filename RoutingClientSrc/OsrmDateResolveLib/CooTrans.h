#pragma once

#include <iostream>
#include <math.h>
#include <vector>

#include <QPair>

//#include <opencv2/opencv.hpp>

double iPI = 0.0174532925199433; //3.1415926535898/180.0
double PI = 3.1415926535898;
// 54年北京坐标系参数
double a = 6378245.0;   // 长轴
double f = 1.0 / 298.3; // 扁率   (a-b)/a

// 80年西安坐标系参数
// double a=6378140.0;
// double f=1/298.257;

// WGS84坐标系参数
// double a = 6378137.0;
// double f = 1 / 298.257223563;

double ZoneWide = 6.0;     // 带宽
double e2 = 2 * f - f * f; // e为第一偏心率，可以算可以直接提供，e2 = e * e
//cv::Mat R;
//输入为高斯坐标，输出为经纬度
QPair<double, double> GaussXYtoLL(double X, double Y)
{
	int ProjNo = (int)(X / 1000000); // 查找带号
	double longitude0 = (ProjNo - 1) * ZoneWide + ZoneWide / 2;
	longitude0 = longitude0 * iPI; // 中央经线

	double X0 = ProjNo * 1000000 + 500000;
	double Y0 = 0;
	double xval = X - X0;
	double yval = Y - Y0; // 带内大地坐标

	double e1 = (1.0 - sqrt(1 - e2)) / (1.0 + sqrt(1 - e2));
	double ee = e2 / (1 - e2);
	double M = yval;
	double u = M / (a * (1 - e2 / 4 - 3 * e2 * e2 / 64 - 5 * e2 * e2 * e2 / 256));
	double fai = u + (3 * e1 / 2 - 27 * e1 * e1 * e1 / 32) * sin(2 * u) + (21 * e1 * e1 / 16 - 55 * e1 * e1 * e1 * e1 / 32) * sin(4 * u) + (151 * e1 * e1 * e1 / 96) * sin(6 * u) + (1097 * e1 * e1 * e1 * e1 / 512) * sin(8 * u);
	double C = ee * cos(fai) * cos(fai);
	double T = tan(fai) * tan(fai);
	double N = a / sqrt(1.0 - e2 * sin(fai) * sin(fai)); // 该点的卯酉圈曲率半径
	double R = a * (1 - e2) / sqrt((1 - e2 * sin(fai) * sin(fai)) * (1 - e2 * sin(fai) * sin(fai)) * (1 - e2 * sin(fai) * sin(fai)));
	double D = xval / N;
	// 计算经度(Longitude) 纬度(Latitude)
	double longitude = longitude0 + (D - (1 + 2 * T + C) * D * D * D / 6 + (5 - 2 * C + 28 * T - 3 * C * C + 8 * ee + 24 * T * T) * D * D * D * D * D / 120) / cos(fai);
	double latitude = fai - (N * tan(fai) / R) * (D * D / 2 - (5 + 3 * T + 10 * C - 4 * C * C - 9 * ee) * D * D * D * D / 24 + (61 + 90 * T + 298 * C + 45 * T * T - 256 * ee - 3 * C * C) * D * D * D * D * D * D / 720);
	// 转换为度 DD
	return QPair<double, double>(longitude / iPI, latitude / iPI );
}


