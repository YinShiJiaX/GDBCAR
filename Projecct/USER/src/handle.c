#include "handle.h"

uint8 colour[2] = {255, 0},black=1,white=0; //0 和 1 分别对应的颜色
//注：山外的摄像头 0 表示 白色，1表示 黑色
uchar Weight[60] = { 5,  6,  8,  9, 11, 12, 14, 15, 17, 18,	// 50
					20, 21, 22, 23, 24, 25, 26, 27, 28, 29,	// 40
				    27, 27, 26, 25, 24, 23, 22, 21, 20, 19,	// 30
				    4,  3,  3,  2,  1,    1,  1,  1,  1,  1,	// 20
					1,  1,  1,  1,  1,    1,  1,  1,  1,  1,
					1,  1,  1,  1,  1,    1,  1,  1,  1,  1};	//加权平均参数
int32 Current_Point = 0;                            //当前中点值



uchar Left_Line[62], Right_Line[62], Mid_Line[62];	// 原始左右边界数据
uchar Left_Add_Line[62], Right_Add_Line[62];		// 左右边界补线数据
uchar Left_Add_Flag[62], Right_Add_Flag[62];		// 左右边界补线标志位
uchar Jump[62];
uchar Width_Real[62];	// 实际赛道宽度
uchar Width_Add[62];	// 补线赛道宽度
uchar Width_Min;		// 最小赛道宽度

uchar Foresight;	// 前瞻，用于速度控制
uchar Out_Side = 0;	// 丢线控制
uchar Line_Count;	// 记录成功识别到的赛道行数

uchar Left_Add_Start, Right_Add_Start;	// 左右补线起始行坐标
uchar Left_Add_Stop, Right_Add_Stop;	// 左右补线结束行坐标
float Left_Ka = 0, Right_Ka = 0;
float Left_Kb = 0, Right_Kb = 0;	// 最小二乘法参数

uchar Left_Max, Right_Min;
int32 Area_Left = 0, Area_Right = 0;	// 左右侧赛面积

uchar Starting_Line_Flag = 0;	// 起跑线标志位

/***************十字相关变量******************/
uchar Cross_Flag = 0;
uchar Cross_Flag_Count = 0;//用来记录是第几次扫描到十字了
/***************十字相关变量结束**************/

/***************环岛相关变量*****************/
 uchar Rotary_Island_Left = 0;
 uchar Rotary_Island_Right = 0;//用来判断环岛进出状态
 uchar Rotary_Island_Repair_Point[2];//补线点
 uchar Rotary_Island_Count0 = 0;
 uchar Rotary_Island_Count1 = 0;
 uchar Rotary_Island_Count2 = 0;
/***************环岛相关变量结束*****************/



/*
*	图像处理算法
*
*	说明：处理普通图像，包括十字
*/
void Image_Handle(uchar *data)
{
	uchar res;	// 用于结果状态判断
	uchar Add_Start_Max;
	uchar Width_Check;
	uchar Weight_Left, Weight_Right;
	uchar Mid_Left, Mid_Right;
	uchar Limit_Left, Limit_Right;
	uchar Cross_Line_Count = 0;//用于对环路中全白行计数
	float Result;	// 用于结果状态判断
	
	Line_Count = 0;	// 赛道行数复位
	Starting_Line_Flag = 0;	// 起跑线标志位复位
	Cross_Flag = 0;//十字标志位复位
	
	Left_Add_Start = 0;		// 复位补线起始行坐标
	Right_Add_Start = 0;
	Left_Add_Stop = 0;
	Right_Add_Stop = 0;
	Point_Weight_Adjust(Weight, 0, 20);


	/***************************** 第一行特殊处理 *****************************/	
	res = First_Line_Handle(data);
	if (res == 0)
	{
		Out_Side = 1;	// 丢线
		return;
	}
	Out_Side = 0;
	Line_Count = 59;
	/*************************** 第一行特殊处理结束 ***************************/
	

	/***********仅处理前40行图像，隔行后仅处理20行数据***************************/
	for (uchar i = 59; i >= 15;)	
	{
		i -= 2;	// 隔行处理，减小单片机负荷
		
		Jump[i] = Corrode_Filter(i, data, Limit_Left, Limit_Right);	// 使用腐蚀滤波算法先对本行赛道进行预处理，返回跳变点数量
		if (Jump[i] >= 5 && i>= 21)
		{
			Starting_Line_Flag = 1;
		}
		/**************************由上一行状态对本行进行边界限定******************************/
		if (Left_Add_Flag[i+2])
		{
			Limit_Left = Left_Line[i+2];
		}
		else
		{
			if (Jump[i] >= 2)
			{
				Limit_Left = Left_Add_Line[i+2];
			}
			else
			{
				Limit_Left = Left_Add_Line[i+2]+1;
			}
			
		}
		
		if (Right_Add_Flag[i+2])
		{
			Limit_Right = Right_Line[i+2];
		}
		else
		{
			if (Jump[i] >= 2)
			{
				Limit_Right = Right_Add_Line[i+2];
			}
			else
			{
				Limit_Right = Right_Add_Line[i+2]-1;
			}
		}
		/**************************由上一行状态对本行进行边界限定结束************************/


		/*************************赛道结束判定与扫描边界线************************************/	
		if (!data[i*160 + Mid_Line[i+2]])//前2行中点在本行为黑点，赛道结束
		{
			break;
		}
		else
		{
			Traversal_Mid_Line(i, data, Mid_Line[i+2], 1, 159, Left_Line, Right_Line, Left_Add_Line, Right_Add_Line);
		}
		/*************************赛道结束判定与扫描中线结束**********************************/		


		/**************************** 补线检测开始 ******************************************/
		if (Starting_Line_Flag)
		{
			Width_Check = 7;
		}
		else
		{
			Width_Check = 2;
		}
		if (Width_Real[i] > Width_Min+Width_Check)	// 赛道宽度变宽，可能是十字或环岛
		{
			if (Left_Add_Line[i] <  Left_Add_Line[i+2])
			{
				if (!Left_Add_Flag[i])
				{
					Left_Add_Flag[i] = 1;	// 强制认定为需要补线
				}
			}
			if (Right_Add_Line[i] > Right_Add_Line[i+2])
			{
				if (!Right_Add_Flag[i])
				{
					Right_Add_Flag[i] = 1;	// 强制认定为需要补线
				}
			}
			
			if (Left_Add_Flag[i] || Right_Add_Flag[i])
			{
				if (Left_Add_Stop && Right_Add_Stop)
				{
					break;
				}
			}
		}
		/**************************** 补线检测结束 ************************************/		
		
		/*************************** 第一轮补线开始 ***********************************/
		if (Left_Add_Flag[i])	// 左侧需要补线
		{
			if (i >= 53)	// 前三行补线不算
			{
				if (!Left_Add_Start)
				{
					Left_Add_Start = i;	// 记录补线开始行
					Left_Ka = 0;
					Left_Kb = Left_Add_Line[i+2];
				}
				Left_Add_Line[i] = Calculate_Add(i, Left_Ka, Left_Kb);	// 使用前一帧图像左边界斜率补线
			}
			else
			{
				if (!Left_Add_Start)	// 之前没有补线
				{
					Left_Add_Start = i;	// 记录左侧补线开始行
					Curve_Fitting(&Left_Ka, &Left_Kb, &Left_Add_Start, Left_Add_Line, Left_Add_Flag, 1);	// 使用两点法拟合直线
				}
				Left_Add_Line[i] = Calculate_Add(i, Left_Ka, Left_Kb);	// 补线完成
			}
		}
		else
		{
			if (Left_Add_Start)	// 已经开始补线
			{
				if (!Left_Add_Stop && !Left_Add_Flag[i+2])
				{
					if (Left_Add_Line[i] >= Left_Add_Line[i+2])
					{
						Left_Add_Stop = i;	// 记录左侧补线结束行
					}
				}
			}
		}
		
		if (Right_Add_Flag[i])	// 右侧需要补线
		{
			if (i >= 53)	// 前三行补线不算
			{
				if (!Right_Add_Start)
				{
					Right_Add_Start = i;	// 记录补线开始行
					Right_Ka = 0;
					Right_Kb = Right_Add_Line[i+2];
				}
				Right_Add_Line[i] = Calculate_Add(i, Right_Ka, Right_Kb);	// 使用前一帧图像右边界斜率补线
			}
			else
			{
				if (!Right_Add_Start)	// 之前没有补线
				{
					Right_Add_Start = i;	// 记录右侧补线开始行
					Curve_Fitting(&Right_Ka, &Right_Kb, &Right_Add_Start, Right_Add_Line, Right_Add_Flag, 2);	// 使用两点法拟合直线
				}
				Right_Add_Line[i] = Calculate_Add(i, Right_Ka, Right_Kb);	// 补线完成
			}
		}
		else
		{
			if (Right_Add_Start)	// 已经开始补线
			{
				if (!Right_Add_Stop && !Right_Add_Flag[i+2])
				{
					if (Right_Line[i] <= Right_Line[i+2])
					{
						Right_Add_Stop = i;	// 记录右侧补线结束行
					}
				}
			}
		}
		/*************************** 第一轮补线结束 ***************************/
		

		/**************************重新计算赛道宽度并计算中线****************************/
		Width_Add[i] = Right_Add_Line[i] - Left_Add_Line[i];	
		Mid_Line[i] = (Right_Add_Line[i] + Left_Add_Line[i]) / 2;
		/**************************重新计算赛道宽度并计算中线结束************************/


		/********************************更新最小赛道宽度并记录成功识别到的赛道行数*****************************/
		if (Width_Add[i] < Width_Min)
		{
			Width_Min = Width_Add[i];	
		}
		if (Width_Add[i] <= 20)	// 赛道宽度太小
		{
			break;
		}
		Line_Count = i;	// 记录成功识别到的赛道行数
		/********************************更新最小赛道宽度并记录成功识别到的赛道行数结束*****************************/
	}
	/***********仅处理前40行图像，隔行后仅处理20行数据结束***************************/


	/**************************十字和第一轮环岛检测*****************************************/
	
	for(uchar i = 15;i < 59;)
	{
		i += 2;

		if(Left_Add_Flag[i] && Right_Add_Flag[i])//十字检测
		{
			Cross_Line_Count++;
			if(Cross_Line_Count >= 14)
			{
				Cross_Flag = 1;
			}
		}

		/*第一轮右环岛检测, 左边为直道，右边需要补线*/
		if((Left_Line[31] <= (Left_Line[51] + 20)) && 
				(Left_Line[31] >= (Left_Line[51] + 10)) && 
				!Left_Add_Flag[i] && Right_Add_Flag[i])
		{
			Rotary_Island_Count0++;
			if(Rotary_Island_Count0 > 10 && Rotary_Island_Right == 0)
			{
				Rotary_Island_Right++;
				for(uchar i = 59;i>=15;) 
				{
					i -= 2;
					if(Left_Add_Flag[i] && Left_Add_Flag[i-2] && Left_Add_Flag[i-4])
					{
						Rotary_Island_Right = 0;
					}
				}
	 		}	
		}

		/*第一轮左环岛检测*/
		if((Right_Line[31] + 10 <= Right_Line[51]) && 
				(Right_Line[31] + 20 >= Right_Line[51]) && 
				Left_Add_Flag[i] && !Right_Add_Flag[i])
		{
			Rotary_Island_Count1++;
			if(Rotary_Island_Count1 > 10 && Rotary_Island_Left == 0)
			{
				Rotary_Island_Left++;
				for(uchar i = 59;i>=15;) 
				{
					i -= 2;
					if(Right_Add_Flag[i] && Right_Add_Flag[i-2] && Right_Add_Flag[-4])
					{
						Rotary_Island_Left = 0;
					}
				}
				// gpio_init(BUZZER,GPO,1);	    //开蜂鸣器
    		// systick_delay_ms(300);
    		// gpio_set(BUZZER,0);            //关蜂鸣器 
	 		}	
		}		
	}
	Rotary_Island_Count0 = 0;//只是用来计数的，用来判断第一次见到环岛
	Rotary_Island_Count1 = 0;
	/**************************十字和第一轮环岛检测结束***************************************/

	/*第二轮右环岛检测，底行右边需要补线*/
	if(Rotary_Island_Right == 1 && !Rotary_Island_Left)
	{
		if(Right_Add_Flag[59])
		{
			Rotary_Island_Right++; 
		}
	}
	/*第二轮左环岛检测, 底行左边需要补线*/	
	if(Rotary_Island_Left == 1 && !Rotary_Island_Right)
	{
		if(Left_Add_Flag[59])
		{
			Rotary_Island_Left++; 
		}
	}
	/*第三轮右环岛检测，找到右边的环岛拐点*/
	if(Rotary_Island_Right == 2 && !Rotary_Island_Left)
	{
		if(Travel_Turn_Point_For_Island(data, 0, 1))
		{
			Rotary_Island_Right++; 
		}
	}
	/*第三轮左环岛检测，找到右边的环岛拐点*/
	if(Rotary_Island_Left == 2 && !Rotary_Island_Right)
	{
		if(Travel_Turn_Point_For_Island(data, 1, 0))
		{
			Rotary_Island_Left++; 
		}
	}
	/*第四轮右环岛检测， 找到远处的补线点*/
	if(Rotary_Island_Right == 3 && !Rotary_Island_Left)
	{
		if(Travel_Repair_Point_For_Island(data, 0, 1))
		{
			Rotary_Island_Right++;
		}
	}
	/*第四轮左环岛检测*/
	if(Rotary_Island_Left == 3 && !Rotary_Island_Right)
	{
		if(Travel_Repair_Point_For_Island(data, 1, 0))
		{
			Rotary_Island_Left++;

		}
	}
	/*第五轮右环岛检测，由于小车进入环岛经历过渡带，左边界消失*/
	if(Rotary_Island_Right == 4 && !Rotary_Island_Left)
	{
		if(Left_Add_Flag[59])
		{
			Rotary_Island_Right++;
		}

	}
	/*第五轮左环岛检测*/
	if(Rotary_Island_Left == 4 && !Rotary_Island_Right)
	{
		if(!Left_Add_Flag[59])
		{
			Rotary_Island_Left++;
 
		}
	}
	/*第六轮右环岛检测，由于完全进入环岛，左边界重现*/
	if(Rotary_Island_Right == 5 && !Rotary_Island_Left)
	{
		if(!data[160*58 + 10])
		{
			Rotary_Island_Right++;
		}

	}

	/*第六轮左环岛检测*/
	/*******************************************/

	/*******************************************/


		/*第七轮右环岛检测，出环岛，发现图像中间左右出现缺口*/
	if(Rotary_Island_Right == 6 && !Rotary_Island_Left)
	{
		if(Left_Add_Flag[25])
		{
			Rotary_Island_Right++;
		}

	}

	/*第七轮左环岛检测*/
	/*******************************************/

	/*******************************************/

	/*第八轮右环岛检测，左边界底部消失*/
	if(Rotary_Island_Right == 7 && !Rotary_Island_Left)
	{
		if(Left_Add_Flag[59] && Left_Add_Flag[57] && Left_Add_Flag[55]&& Left_Add_Flag[53]&& Left_Add_Flag[51])
		{
			Rotary_Island_Right++;
		}

	}

	/*第八轮左环岛检测*/
	/*******************************************/

	/*******************************************/
	/*第九轮右环岛检测， 左边界重现*/
	if(Rotary_Island_Right == 8 && !Rotary_Island_Left)
	{
		if(!Left_Add_Flag[58] && !Left_Add_Flag[57] && !Left_Add_Flag[55]&& !Left_Add_Flag[53]&& !Left_Add_Flag[51])
		{
			Rotary_Island_Right++;
		}

	}

	/*第九轮左环岛检测*/
	/*******************************************/

	/*******************************************/

	/*第十轮右环岛检测， 发现最后出环标志，之后就正常跑*/
	if(Rotary_Island_Right == 9 && !Rotary_Island_Left)
	{
		if(!Right_Add_Flag[59] && !Right_Add_Flag[57] && !Right_Add_Flag[55]&& !Right_Add_Flag[53]&& !Right_Add_Flag[51])
		{
			Rotary_Island_Right++;
		}

	}

	/*第十轮左环岛检测*/
	/*******************************************/

	/*******************************************/

	/*************************** 第二轮补线修复开始 ***************************/
	if (!Cross_Flag)	// 不是环岛而且不是十字
	{
		if (Left_Add_Start)		// 左边界需要补线
		{
			//Line_Repair(Left_Add_Start, Left_Add_Stop, data, Left_Line, Left_Add_Line, Left_Add_Flag, 1);
		}
		if (Right_Add_Start)	// 右边界需要补线
		{
			//Line_Repair(Right_Add_Start, Right_Add_Stop, data, Right_Line, Right_Add_Line, Right_Add_Flag, 2);
		}
	}
	/*************************** 第二轮补线修复结束 ***************************/














	/********************对相应的特殊元素做处理********************************/

	/********************十字处理*********************************************/
	if(Cross_Flag)//十字
	{ 	
		for(uchar i = 59;i >= 15;)
		{
			i -= 2;
			Left_Add_Line[i] = 7;
			Right_Add_Line[i] = 159;
		}	
	}
	/********************十字处理结束*********************************************/

	/********************右环岛处理*********************************************/
	if(Rotary_Island_Right == 1 || Rotary_Island_Right == 2)
	{
		for(int i = 59;i >= 15;)
		{
			Right_Add_Line[i] = Left_Add_Line[i] + 110 + i - 59;
			i -= 2;
		}
	}
	if(Rotary_Island_Right == 4 || Rotary_Island_Right == 5)
	{				
		if(Travel_Repair_Point_For_Island(data, 0, 1))
		{
			Repair_For_In_Island(0, 1);
		}		
	}
	if(Rotary_Island_Right == 7 || Rotary_Island_Right == 8)
	{
		Repair_For_Out_Island(0, 1);

	}
	if(Rotary_Island_Right == 9)
	{
		for(int i = 59;i >= 15;)
		{
			Right_Add_Line[i] = Left_Add_Line[i] + 110 + i - 59;
			i -= 2;
		}

	}
	if(Rotary_Island_Right == 10)
	{
		Rotary_Island_Right = 0;
	}
	/********************右环岛处理结束*********************************************/




	/********************左环岛处理*********************************************/
	if(Rotary_Island_Left == 1 || Rotary_Island_Left ==2)
	{
		for(uchar i = 59;i >= 15;)
		{
			Left_Add_Line[i] = Right_Add_Line[i] - 110 + 59 - i;
			i -= 2;
		}

	}

	if(Rotary_Island_Left == 3 || Rotary_Island_Left ==4)
	{
		for(uchar i = 59;i >= 15;)
		{
			Left_Add_Line[i] = Right_Add_Line[i] - 110 + 59 - i;
			i -= 2;
		}

	}

	if(Rotary_Island_Left == 5)
	{
		Rotary_Island_Left = 0;

	}
	/********************左环岛处理结束*********************************************/
	
	
	/********************对相应的特殊元素做处理结束********************************/


	/****************************** 中线修复开始 ******************************/	
	Mid_Line_Repair(Line_Count, data);
	/****************************** 中线修复结束 ******************************/
}


uchar Fres = 0;	// 前瞻
void Fresight_Calculate(void)
{
//	if (Speed_Min < 140)
//	{
//		Fres = 0;
//	}
//	else if (Speed_Min < 150)
//	{
//		Fres = 1;
//	}
//	else if (Speed_Min < 160)
//	{
//		Fres = 2;
//	}
//	else if (Speed_Min < 170)
//	{
//		Fres = 3;
//	}
//	else if (Speed_Min >= 170)
//	{
//		Fres = 4;
//	}
  
  Fres = 0;
  
}


uchar Fresight[5][2] = {{21, 35},	// 130	前瞻控制
						{21, 35},	// 140
						{21, 33},	// 150
						{21, 31},	// 160
						{19, 29}};	// 170
/*
*	中线平均
*
*	说明：不做特殊处理，直接取均值
*/
int32 Point_Average(void)
{
	uchar i, Point;
	int32 Sum = 0;
	static uchar Last_Point = 80;
	
	Fresight_Calculate();	// 计算前瞻
	
	if (Line_Count <= Fresight[Fres][0])
	{
		Line_Count = Fresight[Fres][0];
	}
	/*
	if (Annulus_Flag)
	{
		if (Line_Count <= Fresight[Fres][1])
		{
			Line_Count = Fresight[Fres][1];
		}
	}
	*/
	
	if (Out_Side || Line_Count >= 57)	// 出界或者摄像头图像异常
	{
		if (Last_Point <= 80)
		{
			Point  = 1;			// 使用上次目标点
		}
		else
		{
			Point = 159;
		}
	}
	else
	{
		
		for (i = 61; i >= Line_Count+2;)
		{
			i -= 2;
			Sum += Mid_Line[i];
		}

		Point = Sum / ((61-Line_Count)/2);	// 对中线求平均
		
		Point = Point*0.8 + Last_Point*0.2;	// 低通滤波
		Point = Range_Protect(Point, 1, 159);		// 限幅，防止补偿溢出
    
		Last_Point = Point;	// 更新上次目标点
	}
	
	return Point;
}


/*
*	面积算法
*
*	说明：计算左右面积
*/
int32 Area_Calculate(void)
{
	char i;
	uchar Line_Start = 61;
	uint16 Area_Mid = 79;
	int32 Result;
	static int32 Result_Last = 0;
	
	Area_Left = 0;	// 左侧面积清零
	Area_Right = 0;	// 右侧面积清零
	
	Fresight_Calculate();	// 计算前瞻

	if (Line_Count <= Fresight[Fres][0])
	{
		Line_Count = Fresight[Fres][0];
	}
	
	if (Annulus_Flag)
	{
		if (Line_Count <= Fresight[Fres][1])
		{
			Line_Count = Fresight[Fres][1];
		}
	}
	
	for (i = Line_Start; i >= Line_Count+2;)
	{
		i -= 2;
		
		if (Mid_Line[i] == Area_Mid)
		{
			Area_Left += (Area_Mid - Left_Add_Line[i]);
			Area_Right += (Right_Add_Line[i] - Area_Mid);
		}
		else if (Right_Add_Line[i] <= Area_Mid)	// 仅左侧有赛道
		{
			Area_Left += Width_Add[i];
		}
		else if (Left_Add_Line[i] >= Area_Mid)	// 仅右侧有赛道
		{
			Area_Right += Width_Add[i];
		}
		else
		{
			Area_Left += (Area_Mid - Left_Add_Line[i]);
			Area_Right += (Right_Add_Line[i] - Area_Mid);
		}
	}
	
	Result = 50*(Area_Right - Area_Left)/(Area_Right + Area_Left);
	Result = Range_Protect(Result, -40, 40);
	
	Result = Result * 0.8 + Result_Last * 0.2;
	Result_Last = Result;
	
	return Result;
}

/*
*	最小二乘法计算斜率
*
*	说明：返回斜率作为转向控制量,结果放大100倍
*/
int32 Least_Squares(void)
{
	int32 Ave_x, Ave_y, Sum1, Sum2;
	uchar i;
	float f_Slope;
	int32 i_Slope;
	static int32 i_Last_Slope = 0;
	
	for (i = 61; i >= Line_Count; )
	{
		i -= 2;
	}
	i_Last_Slope = i_Slope;
	
	return i_Slope;
}

/*
*	斜率计算
*
*	说明：使用斜率作为转向控制量，返回斜率的倒数
*/
float Slope_Weight(uchar *Mid)
{
	float Slope;
	
	Slope = 1.0 * (Mid[Line_Count] - 80) / (60-Line_Count);
	
	return Slope;
}

/*
*	赛道角度计算
*
*	说明：返回结果为 右：0：180°，1：135°，2：90°，>= 1可能为直角突变
*					 左：0：180°，-1：135°，-2：90°，<= -1可能为直角突变
*/
float Calculate_Angle(uchar Point_1, uchar Point_2, uchar Point_3)
{
	char K1, K2;
	float Result;
	
	K1 = Point_2 - Point_1;
	K2 = Point_3 - Point_2;
	
	Result = (K2 - K1) * 0.5;
	
	return Result;
}

/*
*	两点法求直线
*
*	说明：拟合直线 y = Ka * x + Kb   Mode == 1代表左边界，Mode == 2代表右边界
*/
void Curve_Fitting(float *Ka, float *Kb, uchar *Start, uchar *Line, uchar *Add_Flag, uchar Mode)
{
	*Start += 4;
	if (Add_Flag[*Start] == 2)
	{
		if (*Start <= 51)
		{
			*Start += 2;
		}
		*Ka = 1.0*(Line[*Start+4] - Line[*Start]) / 4;
		if (Mode == 2)
		{
			if (*Ka < 0)
			{
				*Ka = 0;
			}
		}
		if (Mode == 1)
		{
			if (*Ka > 0)
			{
				*Ka = 0;
			}
		}
	}
	else
	{
		*Ka = 1.0*(Line[*Start+4] - Line[*Start]) / 4;
	}
	*Kb = 1.0*Line[*Start] - (*Ka * (*Start));
}

/*
*	计算补线坐标
*
*	说明：使用两点法计算拟合出的补线坐标
*/
uchar Calculate_Add(uchar i, float Ka, float Kb)	// 计算补线坐标
{
	float res;
	int32 Result;
	
	res = i * Ka + Kb;
	Result = Range_Protect((int32)res, 1, 159);
	
	return (uchar)Result;
}

uchar Test_Jump;
/*
*	腐蚀滤波
*
*	说明：将干扰滤除，并统计黑白跳变点数量，用于起跑线检测
*/
uchar Corrode_Filter(uchar i, uchar *data, uchar Left_Min, uchar Right_Max)
{
	uchar j;
	uchar White_Flag = 0;
	uchar Jump_Count = 0;	// 跳变点计数
	
	Test_Jump = 0;
	
	Right_Max = Range_Protect(Right_Max, 1, 159);	// 保留右侧部分区域，防止溢出
	
	for (j = Left_Min; j <= Right_Max; j++)	// 从左向右扫描，方向不影响结果
	{
		if (!White_Flag)	// 先查找白点，只滤黑点，不滤白点
		{
			if (data[i*160 + j])	// 检测到白点
			{
				White_Flag = 1;	// 开始找黑点
			}
		}
		else
		{
			if (!data[i*160 + j])	// 检测到黑点
			{
				Jump_Count++;	// 视为一次跳变
				
				Test_Jump = Jump_Count;
				
				if (!data[i*160 + j+1] && j+1 <= Right_Max)	// 连续两个黑点
				{
					if (!data[i*160 + j+2] && j+2 <= Right_Max)	// 连续三个黑点
					{
						if (!data[i*160 + j+3] && j+3 <= Right_Max)	// 连续四个黑点
						{
							if (!data[i*160 + j+4] && j+4 <= Right_Max)	// 连续五个黑点
							{
								if (!data[i*160 + j+5] && j+5 <= Right_Max)	// 连续六个黑点
								{
									if (!data[i*160 + j+6] && j+6 <= Right_Max)	// 连续七个黑点
									{
										if (!data[i*160 + j+7] && j+7 <= Right_Max)	// 连续八个黑点
										{
											if (!data[i*160 + j+8] && j+8 <= Right_Max)	// 连续九个黑点
											{
												if (!data[i*160 + j+9] && j+9 <= Right_Max)	// 连续十个黑点
												{
													if (!data[i*160 + j+10] && j+10 <= Right_Max)	// 连续11个黑点
													{
														White_Flag = 0;	// 认为不是干扰，不做任何处理，下次搜索白点
														j += 10;
													}
													else if (j+10 <= Right_Max)
													{
														/*
														data[i*160 + j] = 255;	// 仅有连续10个黑点，滤除掉
														data[i*160+j+1] = 255;	// 仅有连续10个黑点，滤除掉
														data[i*160+j+2] = 255;	// 仅有连续10个黑点，滤除掉
														data[i*160+j+3] = 255;	// 仅有连续10个黑点，滤除掉
														data[i*160+j+4] = 255;	// 仅有连续10个黑点，滤除掉
														data[i*160+j+5] = 255;	// 仅有连续10个黑点，滤除掉
														data[i*160+j+6] = 255;	// 仅有连续10个黑点，滤除掉
														data[i*160+j+7] = 255;	// 仅有连续10个黑点，滤除掉
														data[i*160+j+8] = 255;	// 仅有连续10个黑点，滤除掉
														data[i*160+j+9] = 255;	// 仅有连续10个黑点，滤除掉
														
														*/
														j += 10;
														
													}
													else
													{
														j += 10;
													}
												}
												else if (j+9 <= Right_Max)
												{
													/*
													data[i*160 + j] = 255;	// 仅有连续九个黑点，滤除掉
													data[i*160+j+1] = 255;	// 仅有连续九个黑点，滤除掉
													data[i*160+j+2] = 255;	// 仅有连续九个黑点，滤除掉
													data[i*160+j+3] = 255;	// 仅有连续九个黑点，滤除掉
													data[i*160+j+4] = 255;	// 仅有连续九个黑点，滤除掉
													data[i*160+j+5] = 255;	// 仅有连续九个黑点，滤除掉
													data[i*160+j+6] = 255;	// 仅有连续九个黑点，滤除掉
													data[i*160+j+7] = 255;	// 仅有连续九个黑点，滤除掉
													data[i*160+j+8] = 255;	// 仅有连续九个黑点，滤除掉
													*/
													j += 9;
												}
												else
												{
													j += 9;
												}
											}
											else if (j+8 <= Right_Max)
											{
												/*
												data[i*160 + j] = 255;	// 仅有连续八个黑点，滤除掉
												data[i*160+j+1] = 255;	// 仅有连续八个黑点，滤除掉
												data[i*160+j+2] = 255;	// 仅有连续八个黑点，滤除掉
												data[i*160+j+3] = 255;	// 仅有连续八个黑点，滤除掉
												data[i*160+j+4] = 255;	// 仅有连续八个黑点，滤除掉
												data[i*160+j+5] = 255;	// 仅有连续八个黑点，滤除掉
												data[i*160+j+6] = 255;	// 仅有连续八个黑点，滤除掉
												data[i*160+j+7] = 255;	// 仅有连续八个黑点，滤除掉
												*/
												j += 8;
											}	
											else
											{
												j += 8;
											}
										}
										else if (j+7 <= Right_Max)
										{
											/*
											data[i*160 + j] = 255;	// 仅有连续七个黑点，滤除掉
											data[i*160+j+1] = 255;	// 仅有连续七个黑点，滤除掉
											data[i*160+j+2] = 255;	// 仅有连续七个黑点，滤除掉
											data[i*160+j+3] = 255;	// 仅有连续七个黑点，滤除掉
											data[i*160+j+4] = 255;	// 仅有连续七个黑点，滤除掉
											data[i*160+j+5] = 255;	// 仅有连续七个黑点，滤除掉
											data[i*160+j+6] = 255;	// 仅有连续七个黑点，滤除掉										
											*/
											j += 7;
										}	
										else
										{
											j += 7;
										}
									}
									else if (j+6 <= Right_Max)
									{
										/*
										data[i*160 + j] = 255;	// 仅有连续六个黑点，滤除掉
										data[i*160+j+1] = 255;	// 仅有连续六个黑点，滤除掉
										data[i*160+j+2] = 255;	// 仅有连续六个黑点，滤除掉
										data[i*160+j+3] = 255;	// 仅有连续六个黑点，滤除掉
										data[i*160+j+4] = 255;	// 仅有连续六个黑点，滤除掉
										data[i*160+j+5] = 255;	// 仅有连续六个黑点，滤除掉
										*/
										j += 6;
									}	
									else
									{
										j += 6;
									}
								}
								else if (j+5 <= Right_Max)
								{
									/*
									data[i*160 + j] = 255;	// 仅有连续五个黑点，滤除掉
									data[i*160+j+1] = 255;	// 仅有连续五个黑点，滤除掉
									data[i*160+j+2] = 255;	// 仅有连续五个黑点，滤除掉
									data[i*160+j+3] = 255;	// 仅有连续五个黑点，滤除掉
									data[i*160+j+4] = 255;	// 仅有连续五个黑点，滤除掉									
									*/
									j += 5;
								}	
								else
								{
									j += 5;
								}
							}
							else if (j+4 <= Right_Max)
							{
								/*
								data[i*160 + j] = 255;	// 仅有连续四个黑点，滤除掉
								data[i*160+j+1] = 255;	// 仅有连续四个黑点，滤除掉
								data[i*160+j+2] = 255;	// 仅有连续四个黑点，滤除掉
								data[i*160+j+3] = 255;	// 仅有连续四个黑点，滤除掉
								*/
								
								j += 4;
							}	
							else
							{
								j += 4;
							}
						}
						else if (j+3 <= Right_Max)
						{
							/*
							data[i*160 + j] = 255;	// 仅有连续三个黑点，滤除掉
							data[i*160+j+1] = 255;	// 仅有连续三个黑点，滤除掉
							data[i*160+j+2] = 255;	// 仅有连续三个黑点，滤除掉
							*/
							
							j += 3;
						}	
						else
						{
							j += 3;
						}
					}
					else if (j+2 <= Right_Max)
					{
						data[i*160 + j] = 255;	// 仅有连续两个黑点，滤除掉
						data[i*160+j+1] = 255;	// 仅有连续两个黑点，滤除掉
						
						j += 2;
					}	
					else
					{
						j += 2;
					}
				}
				else if (j+1 <= Right_Max)
				{
					data[i*160 + j] = 255;	// 有一个黑点，滤除掉
					
					j += 1;
				}	
				else
				{
					j += 1;
				}
			}
		}
	}
	
	return Jump_Count;	// 返回跳变点计数
}

/*
*	尽头搜索
*
*	说明：从某一点开始竖直向上搜索，返回最远行坐标
*/
uchar Limit_Scan(uchar i, uchar *data, uchar Point)
{
	for ( ; i >= 20; i--)
	{
		if (!data[160*i + Point])	// 搜索到黑点
		{
			break;
		}
	}
	
	return i;	// 返回最远行坐标
}

/*
*	第一行特殊处理
*
*	说明：先使用第60行中点作为第59行(第一行)搜线起始位置，成功搜索到左右边界后
 	将第59行中点赋值给第60行便于下一帧图像使用。如果第60行中点在本行为黑点，再
 	分别使用左遍历和右遍历的方法搜索边界，以赛道宽度较大的结果作为第59行边界，
 	若仍然搜索不到边界或数据异常认为出界，出界返回0
*/
uchar First_Line_Handle(uchar *data)
{
	uchar i;	// 控制行
	uchar Weight_Left, Weight_Right;	// 左右赛道宽度
	uchar Mid_Left, Mid_Right;
	uchar res;
	
	i = 59;
	
	res = Corrode_Filter(i, data, 1, 159);	// 使用腐蚀滤波算法先对本行赛道进行预处理，返回跳变点数量
	Jump[59] = res;
	
	if (!data[i*160 + Mid_Line[61]])	// 第61行中点在第59行为黑点
	{
		Weight_Left = Traversal_Left(i, data, &Mid_Left, 1, 159);	// 从左侧搜索边界
		Weight_Right = Traversal_Right(i, data, &Mid_Right, 1, 159);// 从右侧搜索边界
		if (Weight_Left >= Weight_Right && Weight_Left)	// 左赛道宽度大于右赛道宽度且不为0
		{
			Traversal_Left_Line(i, data, Left_Line, Right_Line);	// 使用左遍历获取赛道边界
		}
		else if (Weight_Left < Weight_Right && Weight_Right)
		{
			Traversal_Right_Line(i, data, Left_Line, Right_Line);	// 使用右遍历获取赛道边界
		}
		else	// 说明没查到
		{
			return 0;
		}
	}
	else
	{
		Traversal_Mid_Line(i, data, Mid_Line[i+2], 1, 159, Left_Line, Right_Line, Left_Add_Line, Right_Add_Line);	// 从前一行中点向两边扫描
	}
	Left_Line[61] = Left_Line[59];
	Right_Line[61] = Right_Line[59];
	Left_Add_Line[61] = Left_Add_Line[59];
	Right_Add_Line[61] = Right_Add_Line[59];
	if (Left_Add_Flag[59] && Right_Add_Flag[59])
	{
		Mid_Line[59] = Mid_Line[61];
	}
	else
	{
		Mid_Line[59] = (Right_Line[59] + Left_Line[59]) / 2;
		Mid_Line[61] = Mid_Line[59];	// 更新第60行虚拟中点，便于下一帧图像使用
	}
	if (Left_Add_Flag[59])
	{
		Left_Add_Start = i;	// 记录补线开始行
		Left_Ka = 0;
		Left_Kb = Left_Add_Line[59];
	}
	if (Right_Add_Flag[i])
	{
		Right_Add_Start = i;	// 记录补线开始行
		Right_Ka = 0;
		Right_Kb = Right_Add_Line[59];
	}
	
	Width_Real[61] = Width_Real[59];
	Width_Add[61] = Width_Add[59];
	Width_Min = Width_Add[59];
	
	return 1;
}

/*
*	从左侧开始搜索边界，返回赛道宽度
*
*	说明：本函数仅仅作为探测赛道使用，仅返回赛道宽度，不保存边界数据
*/
uchar Traversal_Left(uchar i, uchar *data, uchar *Mid, uchar Left_Min, uchar Right_Max)
{
	uchar j, White_Flag = 0;
	uchar Left_Line = Left_Min, Right_Line = Right_Max;
	
	for (j = Left_Min; j <= Right_Max; j++)	// 边界坐标 1到159
	{
		if (!White_Flag)	// 先查找左边界
		{
			if (data[i*160 + j])	// 检测到白点
			{
				Left_Line = j;	// 记录当前j值为本行左边界
				White_Flag = 1;	// 左边界已找到，必有右边界
				
				continue;	// 结束本次循环，进入下次循环
			}
		}
		else
		{
			if (!data[i*160 + j])//检测黑点
			{
				Right_Line = j-1;//记录当前j值为本行右边界
				
				break;	// 左右边界都找到，结束循环
			}
		}
	}
	
	if (!White_Flag)	// 未找到左右边界
	{
		return 0;
	}
	else
	{
		*Mid = (Right_Line + Left_Line) / 2;
		
		return (Right_Line - Left_Line);
	}
}

/*
*	从右侧开始搜索边界，返回赛道宽度
*
*	说明：本函数仅仅作为探测赛道使用，仅返回赛道宽度，不保存边界数据
*/
uchar Traversal_Right(uchar i, uchar *data, uchar *Mid, uchar Left_Min, uchar Right_Max)
{
	uchar j, White_Flag = 0;
	uchar Left_Line = Left_Min, Right_Line = Right_Max;
	
	for (j = Right_Max; j >= Left_Min; j--)	// 边界坐标 1到159
	{
		if (!White_Flag)	// 先查找右边界
		{
			if (data[i*160 + j])	// 检测到白点
			{
				Right_Line = j;	// 记录当前j值为本行右边界
				White_Flag = 1;	// 右边界已找到，必有左边界
				
				continue;	// 结束本次循环，进入下次循环
			}
		}
		else
		{
			if (!data[i*160 + j])	//检测黑点
			{
				Left_Line = j+1;	//记录当前j值为本行左边界
				
				break;	// 左右边界都找到，结束循环
			}
		}
	}
	
	if (!White_Flag)	// 未找到左右边界
	{
		return 0;
	}
	else
	{
		*Mid = (Right_Line + Left_Line) / 2;
			
		return (Right_Line - Left_Line);
	}
}

/*
*	从左侧开始搜索边界，保存赛道边界，返回1成功 0失败
*
*	说明：本函数使用后将保存边界数据
*/
uchar Traversal_Left_Line(uchar i, uchar *data, uchar *Left_Line, uchar *Right_Line)
{
	uchar j, White_Flag = 0;
	
	Left_Line[i] = 1;
	Right_Line[i] = 159;
	
	for (j = 1; j < 160; j++)	// 边界坐标 1到159
	{
		if (!White_Flag)	// 先查找左边界
		{
			if (data[i*160 + j])	// 检测到白点
			{
				Left_Line[i] = j;	// 记录当前j值为本行左边界
				White_Flag = 1;		// 左边界已找到，必有右边界
				
				continue;	// 结束本次循环，进入下次循环
			}
		}
		else
		{
			if (!data[i*160 + j])	//检测黑点
			{
				Right_Line[i] = j-1;//记录当前j值为本行右边界
				
				break;	// 左右边界都找到，结束循环
			}
		}
	}
	if (White_Flag)
	{
		Left_Add_Line[i] = Left_Line[i];
		Right_Add_Line[i] = Right_Line[i];
		Width_Real[i] = Right_Line[i] - Left_Line[i];
		Width_Add[i] = Width_Real[i];
	}
	
	return White_Flag;	// 返回搜索结果
}

/*
*	从右侧开始搜索边界，保存赛道边界，返回1成功 0失败
*
*	说明：本函数使用后将保存边界数据
*/
uchar Traversal_Right_Line(uchar i, uchar *data, uchar *Left_Line, uchar *Right_Line)
{
	uchar j, White_Flag = 0;
	
	Left_Line[i] = 1;
	Right_Line[i] = 159;
	
	for (j = 159; j > 0; j--)	// 边界坐标 1到159
	{
		if (!White_Flag)	// 先查找右边界
		{
			if (data[i*160 + j])	// 检测到白点
			{
				Right_Line[i] = j;	// 记录当前j值为本行右边界
				White_Flag = 1;		// 右边界已找到，必有左边界
				
				continue;	// 结束本次循环，进入下次循环
			}
		}
		else
		{
			if (!data[i*160 + j])	//检测黑点
			{
				Left_Line[i] = j+1;//记录当前j值为本行左边界
				
				break;	// 左右边界都找到，结束循环
			}
		}
	}
	if (White_Flag)
	{
		Left_Add_Line[i] = Left_Line[i];
		Right_Add_Line[i] = Right_Line[i];
		Width_Real[i] = Right_Line[i] - Left_Line[i];
		Width_Add[i] = Width_Real[i];
	}
	
	return White_Flag;	// 返回搜索结果
}

/*
*	从中间向两边搜索边界
*
*	说明：本函数使用后将保存边界数据
*/
void Traversal_Mid_Line(uchar i, uchar *data, uchar Mid, uchar Left_Min, uchar Right_Max, uchar *Left_Line, uchar *Right_Line, uchar *Left_Add_Line, uchar *Right_Add_Line)
{
	uchar j;
	
	Left_Add_Flag[i] = 1;	// 初始化补线标志位
	Right_Add_Flag[i] = 1;
	
	Left_Min = Range_Protect(Left_Min, 1, 159);	// 限幅，防止出错
	if (Left_Add_Flag[i+2])
	{
		Left_Min = Range_Protect(Left_Min, Left_Add_Line[i+2], 159);
	}
	Right_Max = Range_Protect(Right_Max, 1, 159);
	if (Right_Add_Flag[i+2])
	{
		Right_Max = Range_Protect(Right_Max, 1, Right_Add_Line[i+2]);
	}
	
	Right_Line[i] = Right_Max;
	Left_Line[i] = Left_Min;	// 给定边界初始值
	
	for (j = Mid; j >= Left_Min; j--)	// 以前一行中点为起点向左查找边界
	{
		if (!data[i*160 + j])	// 检测到黑点
		{
			Left_Add_Flag[i] = 0;	//左边界不需要补线，清除标志位
			Left_Line[i] = j+1;	//记录当前j值为本行实际左边界
			Left_Add_Line[i] = j+1;	// 记录实际左边界为补线左边界
			
			break;
		}
	}
	for (j = Mid; j <= Right_Max; j++)	// 以前一行中点为起点向右查找右边界
	{
		if (!data[i*160 + j])	//检测到黑点
		{
			Right_Add_Flag[i] = 0;		//右边界不需要补线，清除标志位
			Right_Line[i] = j-1;	//记录当前j值为本行右边界
			Right_Add_Line[i] = j-1;	// 记录实际右边界为补线左边界
			
			break;
		}
	}
	if (Left_Add_Flag[i+2])	// 左边界前一行需要补线
	{
		if (Left_Add_Line[i] <= Left_Add_Line[i+2])	// 本行限定就要严格
		{
			Left_Add_Flag[i] = 1;
		}
	}
	if (Right_Add_Flag[i+2])// 右边界前一行需要补线
	{
		if (Right_Add_Line[i] >= Right_Add_Line[i+2])	// 本行限定就要严格
		{
			Right_Add_Flag[i] = 1;
		}
	}
	if (Left_Add_Flag[i])	// 左边界需要补线
	{
		
		if (!data[(i-2)*160 + Left_Add_Line[i+2]] || !data[(i-4)*160 + Left_Add_Line[i+2]])	// 可能是反光干扰
		{
			Left_Add_Flag[i] = 0;	//左边界不需要补线，清除标志位
			Left_Line[i] = Left_Add_Line[i+2];		//记录当前j值为本行实际左边界
			Left_Add_Line[i] = Left_Add_Line[i+2];	// 记录实际左边界为补线左边界
		}
		else
		{
			if (i >= 55)	// 前6行
			{
				Left_Add_Line[i] = Left_Line[59];	// 使用底行数据
			}
			else
			{
				Left_Add_Line[i] = Left_Add_Line[i+2];	// 使用前2行左边界作为本行左边界
			}
		}
	}
	if (Right_Add_Flag[i])	// 右边界需要补线
	{
		if (!data[(i-2)*160 + Right_Add_Line[i+2]] || !data[(i-4)*160 + Right_Add_Line[i+2]])	// 可能是反光干扰
		{
			Right_Add_Flag[i] = 0;	//左边界不需要补线，清除标志位
			Right_Line[i] = Right_Add_Line[i+2];		//记录当前j值为本行实际左边界
			Right_Add_Line[i] = Right_Add_Line[i+2];	// 记录实际左边界为补线左边界
		}
		else
		{
			if (i >= 55)	// 前6行
			{
				Right_Add_Line[i] = Right_Line[59];	// 使用底行数据
			}
			else
			{
				Right_Add_Line[i] = Right_Add_Line[i+2];	// 使用前2行右边界作为本行右边界
			}
		}
	}
	Width_Real[i] = Right_Line[i] - Left_Line[i];			// 计算实际赛道宽度
	Width_Add[i] = Right_Add_Line[i] - Left_Add_Line[i];	// 计算补线赛道宽度
	
//	data[80 * i + Right_Line[i] + 2] = 0;//右移2位显示左边界，方便调试观察
//	data[80 * i + Left_Line[i] - 2] = 0;//左移2位显示右边界，方便调试观察
}

/*
*	补线修复
*
*	说明：有始有终才使用，直接使用两点斜率进行补线
*/
void Line_Repair(uchar Start, uchar Stop, uchar *data, uchar *Line, uchar *Line_Add, uchar *Add_Flag, uchar Mode)
{
	float res;
	uchar i, End;	// 控制行
	uchar Hazard_Width;
	float Ka, Kb;
	/****左补线，右边开始行小于左边停止行，有始有终，左边没有障碍物****/
	if ((Mode == 1) && (Right_Add_Start <= Stop) && Stop && Start <= 53)	// 左边界补线
	{
		for (i = Start+2; i >= Stop+2;)
		{
			i -= 2;
			Line_Add[i] = Range_Protect(Right_Add_Line[i] - Width_Add[i+2]+5, 1, Right_Add_Line[i]); 
			Width_Add[i] = Width_Add[i+2]-3;
			
			if (Width_Add[i] <= 20)
			{
				Line_Count = i;
				break;
			}
		}
	}
	else
	{
		if (Stop)	// 有始有终
		{
			if (((Mode == 1 && Left_Add_Start >= 55) || (Mode == 2 && Right_Add_Start >= 55))
				&& ((Mode == 1 && Line_Add[Left_Add_Stop] > Line_Add[59]) 
				|| (Mode == 2 && Line_Add[Right_Add_Stop] < Line_Add[59])))// 只有较少行需要补线
			{
				if(Stop >= Line_Count + 4)
				{
					End = Stop - 4;
				}
				else if(Stop >= Line_Count + 2)
				{
					End = Stop - 2;
				}
				else
				{
					End = 0;
				}
				if (End)
				{
					Ka = 1.0*(Line_Add[Stop] - Line_Add[End]) / (Stop - End);
					Kb = 1.0*Line_Add[Stop] - (Ka * Stop);
					
					if (Mode == 1)
					{
						if (Line_Add[59] > 59 * Ka + Kb)
						{
							Ka = 1.0*(Line_Add[Start] - Line_Add[Stop-2]) / (Start - (Stop-2));
							Kb = 1.0*Line_Add[Start] - (Ka * Start);
						}
					}
					else if (Mode == 2)
					{
						if (Line_Add[59] < 59 * Ka + Kb)
						{
							Ka = 1.0*(Line_Add[Start] - Line_Add[Stop-2]) / (Start - (Stop-2));
							Kb = 1.0*Line_Add[Start] - (Ka * Start);
						}
					}
				}
				else
				{
					Ka = 1.0*(Line_Add[Start] - Line_Add[Stop]) / (Start - Stop);
					Kb = 1.0*Line_Add[Start] - (Ka * Start);
				}
				
				for (i = 61; i > Stop; )
				{
					i -= 2;
					res = i * Ka + Kb;
					Line_Add[i] = Range_Protect((int32)res, 1, 159);
				}
			}
			else	// 将起始行和结束行计算斜率补线
			{
				if (Start <= 57)
				{
					Start +=2;
				}
				if (Stop >= Line_Count + 4)
				{
					Stop -= 4;
				}
				else if (Stop >= Line_Count + 2)
				{
					Stop -= 2;
				}
				Ka = 1.0*(Line_Add[Start] - Line_Add[Stop]) / (Start - Stop);
				Kb = 1.0*Line_Add[Start] - (Ka * Start);
				
				if (Mode == 1)
				{
					if (Line_Add[59] > 59 * Ka + Kb)
					{
						Ka = 1.0*(Line_Add[Start] - Line_Add[Stop]) / (Start - Stop);
						Kb = 1.0*Line_Add[Start] - (Ka * Start);
					}
				}
				else if (Mode == 2)
				{
					if (Line_Add[59] < 59 * Ka + Kb)
					{
						Ka = 1.0*(Line_Add[Start] - Line_Add[Stop]) / (Start - Stop);
						Kb = 1.0*Line_Add[Start] - (Ka * Start);
					}
				}
				
				for (i = Stop; i <= Start; )
				{
					i += 2;
					res = i * Ka + Kb;
					Line_Add[i] = Range_Protect((int32)res, 1, 159);
				}
			}
		}
	}
	if ((Mode == 2) && (Left_Add_Start <= Stop) && Stop && Start <= 53)	// 右边界补线
	{
		for (i = Start+2; i >= Stop+2;)
		{
			i -= 2;
			Line_Add[i] = Range_Protect(Left_Add_Line[i] + Width_Add[i+2]-5, Left_Add_Line[i], 159); 
			Width_Add[i] = Width_Add[i+2]-3;
			
			if (Width_Add[i] <= 20)
			{
				Line_Count = i;
				break;
			}
		}
	}
	else
	{
		if (Stop)	// 有始有终
		{
			if (((Mode == 1 && Left_Add_Start >= 55) || (Mode == 2 && Right_Add_Start >= 55))
				&& ((Mode == 1 && Line_Add[Left_Add_Stop] > Line_Add[59]) 
				|| (Mode == 2 && Line_Add[Right_Add_Stop] < Line_Add[59])))// 只有较少行需要补线
			{
				if(Stop >= Line_Count + 4)
				{
					End = Stop - 4;
				}
				else if(Stop >= Line_Count + 2)
				{
					End = Stop - 2;
				}
				else
				{
					End = 0;
				}
				if (End)
				{
					Ka = 1.0*(Line_Add[Stop] - Line_Add[End]) / (Stop - End);
					Kb = 1.0*Line_Add[Stop] - (Ka * Stop);
					
					if (Mode == 1)
					{
						if (Line_Add[59] > 59 * Ka + Kb)
						{
							Ka = 1.0*(Line_Add[Start] - Line_Add[Stop-2]) / (Start - (Stop-2));
							Kb = 1.0*Line_Add[Start] - (Ka * Start);
						}
					}
					else if (Mode == 2)
					{
						if (Line_Add[59] < 59 * Ka + Kb)
						{
							Ka = 1.0*(Line_Add[Start] - Line_Add[Stop-2]) / (Start - (Stop-2));
							Kb = 1.0*Line_Add[Start] - (Ka * Start);
						}
					}
				}
				else
				{
					Ka = 1.0*(Line_Add[Start] - Line_Add[Stop]) / (Start - Stop);
					Kb = 1.0*Line_Add[Start] - (Ka * Start);
				}
				
				for (i = 61; i > Stop; )
				{
					i -= 2;
					res = i * Ka + Kb;
					Line_Add[i] = Range_Protect((int32)res, 1, 159);
				}
			}
			else	// 将起始行和结束行计算斜率补线
			{
				if (Start <= 57)
				{
					Start +=2;
				}
				if (Stop >= Line_Count + 4)
				{
					Stop -= 4;
				}
				else if (Stop >= Line_Count + 2)
				{
					Stop -= 2;
				}
				Ka = 1.0*(Line_Add[Start] - Line_Add[Stop]) / (Start - Stop);
				Kb = 1.0*Line_Add[Start] - (Ka * Start);
				
				if (Mode == 1)
				{
					if (Line_Add[59] > 59 * Ka + Kb)
					{
						Ka = 1.0*(Line_Add[Start] - Line_Add[Stop]) / (Start - Stop);
						Kb = 1.0*Line_Add[Start] - (Ka * Start);
					}
				}
				else if (Mode == 2)
				{
					if (Line_Add[59] < 59 * Ka + Kb)
					{
						Ka = 1.0*(Line_Add[Start] - Line_Add[Stop]) / (Start - Stop);
						Kb = 1.0*Line_Add[Start] - (Ka * Start);
					}
				}
				
				for (i = Stop; i <= Start; )
				{
					i += 2;
					res = i * Ka + Kb;
					Line_Add[i] = Range_Protect((int32)res, 1, 159);
				}
			}
		}
	}
	
}

/*
*	中线修复
*
*	说明：普通弯道丢线使用平移赛道方式，中点到达边界结束
*/
void Mid_Line_Repair(uchar count, uchar *data)
{
	float Gradient_Sum = 0, k;

	for (uchar i = 61; i >= count+2;)
	{
		i -= 2;
		Mid_Line[i] = (Right_Add_Line[i] + Left_Add_Line[i]) / 2;	// 计算赛道中点
		Width_Add[i] = Right_Add_Line[i] - Left_Add_Line[i];		// 计算赛道宽度
		/*************************** 上位机显示边界 ***************************/
		data[i*160 + Left_Add_Line[i] + 6] = 0;	// 上位机显示补线后的左边界，不用时屏蔽
		data[i*160 + Right_Add_Line[i] - 6] = 0;	// 上位机显示补线后的右边界，不用时屏蔽
		data[i*160 + Mid_Line[i]] = 0;			// 上位机显示中线，不用时屏蔽
		data[i*160 + Left_Line[i] + 1] = 0;		// 上位机显示原始左边界，不用时屏蔽
		data[i*160 + Right_Line[i] - 1] = 0;		// 上位机显示原始右边界，不用时屏蔽
		/*************************** 上位机显示边界结束***************************/
	}
	

	Mid_Line[61] = Mid_Line[59];
}


/****************** 新算法 ******************/

/*
*	加权平均
*
*	说明：权重是乱凑的，效果不好
*/
uchar Point_Weight(void)
{
	uchar i ,Point, Point_Mid;
	static char Last_Point = 80;
	int32 Sum = 0, Weight_Count = 0;
	
	if (Line_Count <= 20)
	{
		Line_Count = 20;
	}
	
	if (Out_Side || Line_Count >= 53)	//出界或者摄像头图像异常
	{
		if (Last_Point == 80)
		{
			Point = Last_Point;
		}
		else if (Last_Point < 80)
		{
			Point = 1;
		}
		else if (Last_Point > 80)
		{
			Point = 159;
		}
	}
	else
	{
		for (i = 61; i >= Line_Count; )		//使用加权平均
		{
			i -= 2;
			Sum += Mid_Line[i] * Weight[59-i];
			Weight_Count += Weight[59-i];
		}
		Point = Range_Protect(Sum / Weight_Count, 1, 159);
		Point = Range_Protect(Point, 2, 158);
		Last_Point = Point;
		
		/***** 使用最远行数据和目标点作为前瞻 *****/
		if (Line_Count >= 25)
		{
			Point_Mid = Mid_Line[60-30];
		}
		else
		{
			Point_Mid = Mid_Line[60-Line_Count];
		}
	}
	Foresight = 0.8 * Error_Transform(Point_Mid, 80)	//使用最远行偏差和加权偏差确定前瞻
			  + 0.2 * Error_Transform(Point, 	 80);
	
	return Point;
}

int8 Error_Transform(int8 Data, int8 Set)
{
	int8 Error;
	
	Error = Set - Data;
	if (Error < 0)
	{
		Error = -Error;
	}
	
	return Error;
}



/*!
*  @brief      二值化图像解压（空间 换 时间 解压）
*  @param      dst             图像解压目的地址
*  @param      src             图像解压源地址
*  @param      srclen          二值化图像的占用空间大小
*  @since      v5.0            img_extract(img, imgbuff,CAMERA_SIZE);
*  Sample usage:
*/
void img_extract(uint8 *dst, uint8 *src, uint32 srclen)
{
  uint8 tmpsrc;
  while(srclen --)
  {
    tmpsrc = *src++;
    *dst++ = colour[ (tmpsrc >> 7 ) & 0x01 ];
    *dst++ = colour[ (tmpsrc >> 6 ) & 0x01 ];
    *dst++ = colour[ (tmpsrc >> 5 ) & 0x01 ];
    *dst++ = colour[ (tmpsrc >> 4 ) & 0x01 ];
    *dst++ = colour[ (tmpsrc >> 3 ) & 0x01 ];
    *dst++ = colour[ (tmpsrc >> 2 ) & 0x01 ];
    *dst++ = colour[ (tmpsrc >> 1 ) & 0x01 ];
    *dst++ = colour[ (tmpsrc >> 0 ) & 0x01 ];
  }
}

/*!
*  @brief      二值化图像压缩（空间 换 时间 压缩）
*  @param      dst             图像压缩目的地址
*  @param      src             图像压缩源地址
*  @param      srclen          二值化图像的占用空间大小
*  @since      v5.0            img_extract(img, imgbuff,CAMERA_SIZE);
*  Sample usage:
*/
void img_recontract(uint8 *dst, uint8 *src, uint32 srclen)
{
  uint8 tmpsrc;
  uint8 i, j, k;
  uint32 Count = 0;
  
  for (i = 0; i < 60/2; i++)
  {
	  for (j = 0; j < 160/8; j++)
	  {
		  Count = (i*2+1)*160+j*8;
		  for (k = 0; k < 8; k++)		  
		  {
			  tmpsrc <<= 1;
			  if (!dst[Count++])
			  {
				  tmpsrc |= 1;
			  }
		  }
		  *src++ = tmpsrc;
	  }
  }
}

/*!
*  @brief      取边界线
*  @param      dst             图像压缩目的地址
*  @param      src             图像压缩源地址
*  @param      srclen          二值化图像的占用空间大小
*  @since      v5.0            img_extract(img, imgbuff,CAMERA_SIZE);
*  Sample usage:
*/
void img_getline(uint8 *dst, uint8 *src, uint32 srclen)
{
  uint8 tmpsrc,buff_y,buff_x;
  
  while(srclen --)
  {
    tmpsrc=0;
    buff_x = *(dst + 80);
    buff_y = *dst++;
    *(dst-1)=colour[white];
    if(*dst != buff_y || (buff_y != buff_x && srclen > 9))
    {
      tmpsrc = tmpsrc + 0x80;
      *(dst-1)=colour[black];
    }
    
    buff_x = *(dst + 80);
    buff_y = *dst++;
    *(dst-1)=colour[white];
    if(*dst != buff_y || (buff_y != buff_x && srclen > 9))
    {
      tmpsrc = tmpsrc + 0x40;
      *(dst-1)=colour[black];
    }
    
    buff_x = *(dst + 80);
    buff_y = *dst++;
    *(dst-1)=colour[white];
    if(*dst != buff_y || (buff_y != buff_x && srclen > 9))
    {
      tmpsrc = tmpsrc + 0x20;
      *(dst-1)=colour[black];
    }
    
    buff_x = *(dst + 80);
    buff_y = *dst++;
    *(dst-1)=colour[white];
    if(*dst != buff_y || (buff_y != buff_x && srclen > 9))
    {
      tmpsrc = tmpsrc + 0x10;
      *(dst-1)=colour[black];
    }
    
    buff_x = *(dst + 80);
    buff_y = *dst++;
    *(dst-1)=colour[white];
    if(*dst != buff_y || (buff_y != buff_x && srclen > 9))
    {
      tmpsrc = tmpsrc + 0x08;
      *(dst-1)=colour[black];
    }
    
    buff_x = *(dst + 80);
    buff_y = *dst++;
    *(dst-1)=colour[white];
    if(*dst != buff_y || (buff_y != buff_x && srclen > 9))
    {
      tmpsrc = tmpsrc + 0x04;
      *(dst-1)=colour[black];
    }
    
    buff_x = *(dst + 80);
    buff_y = *dst++;
    *(dst-1)=colour[white];
    if(*dst != buff_y || (buff_y != buff_x && srclen > 9))
    {
      tmpsrc = tmpsrc + 0x02;
      *(dst-1)=colour[black];
    }
    
    buff_x = *(dst + 80);
    buff_y = *dst++;
    *(dst-1)=colour[white];
    if((*dst != buff_y && srclen % 10 !=0 ) || (buff_y != buff_x && srclen > 9))
    {
      tmpsrc = tmpsrc + 0x01;
      *(dst-1)=colour[black];
    }
    
    *src++ = tmpsrc;
  }
}

/*!
*  @brief      大津法阈值
*  @param      image           图像源地址
*  @param      col             图像col
*  @param      row             图像row
*  @since      v5.0            image_threshold = OtsuThreshold(image[0],COL,ROW);
*  Sample usage:
*/
uint8 OtsuThreshold( uint8 *image, uint16 col, uint16 row)
{
     uint16 OtsuThresholdOut;
     uint16 GrayList[256]={0};   //灰度值个数统计数组
     uint16 i, pixelSum = col * row;   //图像大小
     uint16 Camera_Graylevel=0;
     uint16 Graylevel=0;         //灰度值
     uint16 cnts=0,cnt=0;
     float avfor=0,avback=0; 
     float total=0,total_low=0,total_high=0,Variance=0,VarianceMax=0;
     //清空直方图
     for(Graylevel=0;Graylevel<256;Graylevel++)
     {
       GrayList[Graylevel]=0;
     }
     //计算直方图

       for(i=0;i<pixelSum;i++)
       {   
         Camera_Graylevel=*(image+i);//获取灰度值
         GrayList[Camera_Graylevel]+=1;//直方图统计图像所有像素点
         total+=Camera_Graylevel;//total计算图像的灰度值总和
       }

     for(Graylevel=0;Graylevel<256;Graylevel++)
     {
       cnt=GrayList[Graylevel];//当前灰度值的计数数量
       if(cnt==0)   continue;
       total_low+=cnt*Graylevel;//计算灰度值从0到当前值的像素点灰度值的总和
       cnts+=cnt;//计算灰度值从0到当前值的像素点数目的总和

       if(cnts==pixelSum)  break;//统计完所有像素点，之后的灰度值已经没有任何像素点符合，计算完毕，退出
       
       total_high=total-total_low;//背景灰度=灰度总和-前景灰度(还没有归一化)
       avfor=(int)(total_low/cnts);//前景归一化

         avback=(int)(total_high/(pixelSum-cnts));//背景归一化 
         Variance=(avback-avfor)*(avback-avfor)*(cnts*pixelSum-cnts*cnts)/(pixelSum*pixelSum);//计算方差 
           
       if(VarianceMax<Variance)
       {
         VarianceMax=Variance;
         OtsuThresholdOut=Graylevel;//方差最大的灰度值即为大津阈值
       }
     }
     return OtsuThresholdOut;
}

/*
 * 大津法计算阈值并对图像进行二值化
*/
void Handle_Gray(void)
{
	uchar *Image_Process = image[0];
	uchar image_threshold;
	
	image_threshold = OtsuThreshold(image[0],COL,ROW);
	
	for(int i = 0; i < ROW*COL; i++)
	{
		if (*Image_Process >= image_threshold) 
		{
			*Image_Process = 255;
		}
		else
		{
			*Image_Process = 0;
		}
		Image_Process++;
	}
	
}

/*
 *
 * 求两点之间距离
 * 
*/
double distance(double x1, double y1, double x2, double y2)
{
	double distance;//两点间距离
	double x, y;
	x= x1 - x2;
	y= y1 - y2;
	distance = sqrt(x*x + y*y);
	return distance;
}

/*
 *
 * 判断三点是否在一条直线上 ，共线返回1
*/
int collinear(double x1, double y1, double x2, double y2, double x3, double y3)
{
 	double k1,k2;
 	double kx1,ky1,kx2,ky2;
 	if(x1==x2 && x2==x3) return 1;//三点横坐标都相等，共线
 	else
	{
		kx1=x2 - x1;
		kx2=x2 - x3;
		ky1=y2 - y1;
		ky2=y2 - y3;
		k1=ky1/kx1;
		k2=ky2/kx2;
		if(k1==k2) return 1;//AB与BC斜率相等，共线
		else return 0;//不共线
	}
}


/*
 *
 * 计算曲率
*/
double curvature(double x1, double y1, double x2, double y2, double x3, double y3)
{
	double curvature;//求得的曲率
	if(collinear(x1, y1, x2, y2, x3, y3)==1)//判断三点是否共线
	{
		curvature = 0.0;//三点共线时将曲率设为某个值，0
	}
	else
	{
		double radius;//曲率半径
		double dis,dis1,dis2,dis3;//距离
		double sinA;//ab确定的边所对应的角A的sin值
		dis1 = distance(x1, y1, x2, y2);
		dis2 = distance(x2, y2, x3, y3);
		dis3 = distance(x1, y1, x3, y3);
		dis = dis2*dis2+dis3*dis3-dis1*dis1;
		sinA = dis/(2*dis2*dis3);//余弦定理
		radius = 0.5*dis1/sinA;
		curvature = 1/radius;
	}
	return curvature;
}

/*
 *
 * 用于判断环岛的拐点是否到达图像的右侧中间附近
 * Side_Of_Island 为 1 代表左圆环，为 0 代表右圆环
 * Size_Of_Island 为 1 代表小圆环，为 0 代表大圆环
*/

int Travel_Turn_Point_For_Island(uchar *data,int Side_Of_Island, int Size_Of_Island)
{
	uchar Line[60];
	
	if(Side_Of_Island == 0)
	{
		if(Size_Of_Island == 1)
		{
			for(uchar i = 34; i >= 25;i--)
			{
				Line[i] =  159;
				for(uchar j = 159; j >= 80;j--)
				{
					if(data[160*i + j] )
					{
						Line[i] = j;
						break;
					}
				}
			}
			if(Line[25] > Line[34])
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}
	else
	{
		if (Size_Of_Island == 1) 
		{
			/* code */
		}
		else
		{
			for(uchar i = 30; i >= 20;i--)
			{
				Line[i] =  1;
				for(uchar j = 0; j <= 80;j++)
				{
					if(data[160*i + j] )
					{
						Line[i] = j;
						break;
					}
				}
			}
			if(Line[20] < Line[30])
			{
				return 1;
			}
			else
			{
				return 0;
			}
			
		}
	}	
}

/*
 *
 * 用于寻找环岛补线点
 * Side_Of_Island 为 1 代表左圆环，为 0 代表右圆环
 * Size_Of_Island 为 1 代表小圆环，为 0 代表大圆环
*/
int Travel_Repair_Point_For_Island(uchar *data,int Side_Of_Island, int Size_Of_Island)
{
	uchar count = 0;
	if(Side_Of_Island == 0)
	{
		if(Size_Of_Island == 1)
		{
			for(uchar  i = 55; i >= 7;)
			{
				i-=3;
				for(uchar j = 1;j <= 120;j++)
				{
					if((!data[i*160 + j] && !data[i*160 + j + 1] && data[i*160 + j + 2] && data[i*160 + j + 3]) 
					|| (data[i*160 + j] && data[i*160 + j + 1] && !data[i*160 + j + 2] && !data[i*160 + j + 3]))
					{
						count++;
					}
					if(count >= 3)
					{
						Rotary_Island_Repair_Point[0] = i;
						Rotary_Island_Repair_Point[1] = j;
						return 1;
					}
				}
				count = 0;
			}
		}
		else
		{
			/* code */
		}
		
	}
	else
	{
		if (Size_Of_Island == 1) 
		{
			/* code */
		}
		else
		{
			for(uchar  i = 55; i >= 25;)
			{
				i-=3;
				for(uchar j = 1;j <= 120;j++)
				{
					if((!data[i*160 + j] && !data[i*160 + j + 1] && data[i*160 + j + 2] && data[i*160 + j + 3]) 
					|| (data[i*160 + j] && data[i*160 + j + 1] && !data[i*160 + j + 2] && !data[i*160 + j + 3]))
					{
						count++;
					}
					if(count >= 3)
					{
						Rotary_Island_Repair_Point[0] = i;
						Rotary_Island_Repair_Point[1] = j;
						return 1;
					}
				}
				count = 0;
			}
		}
		
		
	}
	return 0;
}

/*
 *
 * 用于进入环岛补线
 * Side_Of_Island 为 1 代表左圆环，为 0 代表右圆环
 * Size_Of_Island 为 1 代表小圆环，为 0 代表大圆环
*/
void Repair_For_In_Island(int Side_Of_Island, int Size_Of_Island)
{
	float k;
	k = ((float)Rotary_Island_Repair_Point[0] - 59)/((float)Rotary_Island_Repair_Point[1] - 5);
	if(Side_Of_Island == 0) 
	{
		if(Size_Of_Island == 1)
		{
			for(uchar i = 59; i >= Rotary_Island_Repair_Point[0]; )
			{
				i-=2;
				Left_Add_Line[i] = (uchar)((i - 59)/k + 5);
			}

		}
	}
	else
	{
		/* code */
	}
}


/*
 *
 * 用于出环岛补线
 * Side_Of_Island 为 1 代表左圆环，为 0 代表右圆环
 * Size_Of_Island 为 1 代表小圆环，为 0 代表大圆环
*/
void Repair_For_Out_Island(int Side_Of_Island, int Size_Of_Island)
{
	if(Side_Of_Island == 0) 
	{
		if(Size_Of_Island == 1)
		{
			for(uchar i = 59; i >= 15;)
			{
				Left_Add_Line[i] = 40;
				Right_Add_Line[i] = 159;
				i-=2;
			}
		}
	}
	else
	{
		/* code */
	}

}

/*
*	图像算法参数初始化
*
*	说明：仅影响第一行特殊处理
*/
void Image_Para_Init(void)
{	
	Mid_Line[61] = 80;
	Left_Line[61] = 1;
	Right_Line[61] = 159;
	Left_Add_Line[61] = 1;
	Right_Add_Line[61] = 159;
	Width_Real[61] = 158;
	Width_Add[61] = 158;
	Right_Add_Flag[61] = 0;
	Left_Add_Flag[61] = 0;
}

/*************************************************************
 *Function:PointWeightAdjust
 *Description:Adjust the weight array
 * - PointWeight: Original PointWeight array.
 * - AdjustCount: The count you want to adjust.
 * - Direction  : Adjust to Right(0) or Left(1)
 ************************************************************/
void Point_Weight_Adjust(uchar *PointWeight, uchar Direction, uchar AdjustCount)
{
  char i;
	/* 权重数组向左调整 */
	if(Direction == 1)
	{
		for(i = AdjustCount; i < 60; i++)
		{
			PointWeight[i] = PointWeight[i - AdjustCount];
			
		}
		for(i = 0; i < AdjustCount; i++)
		{
			PointWeight[i] = 1;
		}

	}
	/* 权重数组向右调整 */
	if(Direction == 0)
	{
		for(i = 0; i < 60 - AdjustCount; i++)
		{
			PointWeight[i] = PointWeight[i + AdjustCount];
			
		}
		for(i = 60 - AdjustCount; i < 60; i++)
		{
			PointWeight[i] = 1;
		}

	}


}