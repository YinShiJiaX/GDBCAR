#ifndef __HANDLE_H__
#define __HANDLE_H__

#include "headfile.h"

//�ⲿ��������
extern uchar Annulus_Count[10];
extern uchar Foresight;
extern uchar Fres;	// ǰհ����
extern uchar Annulus_Flag;	// ��·��־λ
extern uchar Annulus_Mode;	// ��·״̬��־λ
extern uchar Annulus_Times;	// ��·����
extern int16 Annulus_Delay;	// ��·��ʱ
extern uchar Out_Side;
extern uchar Line_Count;
extern uchar Left_Add_Start, Right_Add_Start;
extern uchar Starting_Line_Flag;
extern uchar Left_Max, Right_Min;
extern uchar Left_Line[62];
extern uchar Right_Line[62];
extern uchar Mid_Line[62];
extern uchar Left_Add_Line[62];
extern uchar Right_Add_Line[62];
extern int32 Area_Left, Area_Right;
extern int32 Current_Point;
//��������
void Image_Para_Init(void);
void Annulus_Control(void);
void Annulus_Handle(uchar *data);
void Image_Handle(uchar *data);
void Fresight_Calculate(void);
int32 Area_Calculate(void);
int32 Point_Average(void);
float Slope_Weight(uchar *Mid);
uchar First_Line_Handle(uchar *data);
uchar Traversal_Left(uchar i, uchar *data, uchar *Mid, uchar Left_Min, uchar Right_Max);
uchar Traversal_Right(uchar i, uchar *data, uchar *Mid, uchar Left_Min, uchar Right_Max);
uchar Traversal_Left_Line(uchar i, uchar *data, uchar *Left_Line, uchar *Right_Line);
uchar Traversal_Right_Line(uchar i, uchar *data, uchar *Left_Line, uchar *Right_Line);
uchar Corrode_Filter(uchar i, uchar *data, uchar Left_Min, uchar Right_Max);
void Traversal_Mid_Line(uchar i, uchar *data, uchar Mid, uchar Left_Min, uchar Right_Max,
uchar *Left_Line, uchar *Right_Line, uchar *Left_Add_Line, uchar *Right_Add_Line);
void Line_Repair(uchar Start, uchar Stop, uchar *data, uchar *Line, uchar *Line_Add, uchar *Add_Flag, uchar Mode);
void Mid_Line_Repair(uchar count, uchar *data);
void Handle_Gray(void);
int Travel_Turn_Point_For_Island(uchar *data,int Side_Of_Island, int Size_Of_Island);
int Travel_Repair_Point_For_Island(uchar *data,int Side_Of_Island, int Size_Of_Island);
void Repair_For_In_Island(uchar *data,int Side_Of_Island, int Size_Of_Island);

float Calculate_Angle(uchar Point_1, uchar Point_2, uchar Point_3);
void Curve_Fitting(float *Ka, float *Kb, uchar *Start, uchar *Line, uchar *Add_Flag, uchar Mode);
uchar Calculate_Add(uchar i, float Ka, float Kb);
uchar Limit_Scan(uchar i, uchar *data, uchar Point);
uchar Point_Weight(void);
char Error_Transform(uchar Data, uchar Set);
double curvature(double x1, double y1, double x2, double y2, double x3, double y3);
int collinear(double x1, double y1, double x2, double y2, double x3, double y3);
double distance(double x1, double y1, double x2, double y2);

void img_extract(uint8 *dst, uint8 *src, uint32 srclen);        //��ѹ �̡̡�
void img_recontract(uint8 *dst, uint8 *src, uint32 srclen);     //ѹ���̡̡�
					//ͼ����Դ  //����Ŀ���ַ  //ͼ����


uint8 OtsuThreshold( uint8 *image, uint16 col, uint16 row);   //�����ֵ

#endif
