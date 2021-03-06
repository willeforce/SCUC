/***
This is the SCUC module for the project of Northwest.

**/
#include <ilcplex/ilocplex.h>
#include <fstream>
#include <math.h>
#include <ilconcert/ilomodel.h>

/************************ Define all the files ***************************************************/
#define OUTFILEDATA "./Output/Check.dat"    
#define OUTFILERESULT "./Output/Result.dat"
#define lp "./Output/Model.lp"     
#define SYSTEMDATA "./Input/SystemData.dat"   
#define APPDATA "./Input/AppData.dat"    
#define THUNITDATA "./Input/ThUnitData.dat"
#define NETDATA "./Input/NetData.dat"
#define GAMADATA "./Input/GamaData.dat"
#define HYDDATA "./Input/HydData.dat"		
#define WINDDATA "./Input/WindData.dat"		
#define SOLARDATA "./Input/SolarData.dat"
/************************ File End ***************************************************************/

ILOSTLBEGIN
/***************** typedef为 IloArray<IloNumArray> 起别名Matrix2，以便于后面运用时的书写**********/
typedef IloArray<IloNumArray> Matrix2; 
typedef IloArray<Matrix2> Matrix3;
typedef IloArray<Matrix3> Matrix4;
typedef IloArray<Matrix4> Matrix5;

typedef IloArray<IloNumVarArray>VarMatrix2;
typedef IloArray<VarMatrix2>VarMatrix3;
typedef IloArray<VarMatrix3>VarMatrix4;
typedef IloArray<VarMatrix4>VarMatrix5;
typedef IloArray<VarMatrix5>VarMatrix6;

/************************ Define Global Variables *********************************************************/
IloInt cycle; //调度时段数
IloInt demandNum; //负荷节点个数
IloInt lineNum; //传输线个数
IloInt busNum;
IloInt sectionNum; //断面个数
IloInt thUnitNum; //火电机组台数
IloInt hyUnitNum; //水电机组台数
IloInt wFieldNum; //风场数量
IloInt sPlantNum; //光伏电厂数量

const double _INF=1E-7;//add by hx

//************************************
// Method:    readSystemData
// FullName:  readSystemData
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: IloEnv env
// Parameter: IloInt & cycle, time periods,24;
// Parameter: IloInt & demandNum,demandNum, number of load, 11;
// Parameter: IloInt & lineNum,number of transmission line, 39;
// Parameter: IloInt & busNum, number of bus, 31;
// Parameter: IloInt & sectionNum,numberof section, 1 
// Parameter: IloInt & thUnitNum, number of thermal unit, 16;
// Parameter: IloInt & hyUnitNum, number of hydroelectric station
// Parameter: IloInt & wFieldNum, number of wind field, 3;	
// Parameter: IloInt & sPlantNum, number of solar farm
//************************************
void readSystemData(IloEnv env,
					IloInt& cycle,
					IloInt& demandNum,
					IloInt& lineNum,
					IloInt& busNum,
					IloInt& sectionNum,
					IloInt& thUnitNum,
					IloInt& hyUnitNum, 
					IloInt& wFieldNum,
					IloInt& sPlantNum
					)
{
	ifstream fin(SYSTEMDATA,ios::in);
	if(!fin) 
		env.out()<<"problem with file:" << SYSTEMDATA<<endl;
	fin>>cycle;
	fin>>demandNum;
	fin>>lineNum;
	fin>>busNum;
	fin>>sectionNum;   //0
	fin>>thUnitNum;		// 16
	fin>>hyUnitNum;		// 1
	fin>>wFieldNum;		// 1
	fin>>sPlantNum;		// 1
	fin.close();
	env.out()<<"System date is done"<<endl;
}

/********************* readNetData by Xuan ****************************************/


//************************************
// Method:    readNetData
// FullName:  readNetData
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: IloEnv env
// Parameter: IloNumArray & unitLocation	Thermal unit所在bus编号 
// Parameter: IloNumArray & demandLocation 负载所在bus编号
// Parameter: IloNumArray & demand	各负载需求系数
// Parameter: IloNumArray & sysDemand	系统在每个时刻的总负荷
// Parameter: IloNumArray & lineCap	传输能力
// Parameter: IloNumArray & upSectionCap	断面功率上限
// Parameter: IloNumArray & downSectionCap	断面功率下限
// Parameter: IloNumArray & linSectionNum	每个断面包含线路条数
// Parameter: Matrix2 & linSection	每个断面具体包含的线路
//************************************
void readNetData(IloEnv env,
				 IloNumArray& unitLocation,
				 IloNumArray& demandLocation,            
				 IloNumArray& demand,                    
				 IloNumArray& sysDemand,					
				 IloNumArray& lineCap,					
				 IloNumArray& upSectionCap,				
				 IloNumArray& downSectionCap,				
				 IloNumArray& linSectionNum,				
				 Matrix2& linSection					
				 )
{
	ifstream fin(NETDATA,ios::in);
	if(!fin) env.out()<<"problem with file:" << NETDATA <<endl;
	int i,j,t;
	for(i=0;i<thUnitNum;i++)
	{
		fin>> unitLocation[i];
	}
	for(i=0;i<demandNum;i++)
	{
		fin>>demandLocation[i];
	}

	for(i=0;i<demandNum;i++)
	{
		fin>> demand[i];
	}

	for(t=0;t<cycle;t++)
	{
		fin>>sysDemand[t];
	}

	for(i=0;i<lineNum;i++)
	{
		fin>> lineCap[i];
	}

	for(i=0;i<sectionNum;i++)
	{
		fin>>upSectionCap[i];
	}

	for(i=0;i<sectionNum;i++)
	{
		fin>>downSectionCap[i];
	}

	for(i=0;i<sectionNum;i++)
	{
		fin>>linSectionNum[i];
	}

	for(i=0;i<sectionNum;i++)
	{
		for(j=0;j<linSectionNum[i];j++)
		{
			fin>>linSection[i][j];
		}
	}
	fin.close();
	env.out()<<"Net Data is done "<<endl;
}
/*********************** End readNetData by Xuan *********************************/

//************************************
// Method:    readGama
// FullName:  readGama
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: char * gamgadata
// Parameter: IloEnv env
// Parameter: Matrix2 & gama
//************************************
int readGama(char* gamgadata,IloEnv env,Matrix2& gama)
{
	int i,j;
	ifstream fin(gamgadata,ios::in);		
	if(!fin) 
		env.out()<<"problem with "<< gamgadata<<endl;
	//read gama
	for(i=0;i<lineNum;i++)
	{
		for(j=0;j<busNum;j++)
		{
			fin>>gama[i][j];
		}
	}
	return 0;
}


//************读取火电机组数据函数************************
//************************************
// Method:    readThUnitData
// FullName:  readThUnitData
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: char * thunitdata
// Parameter: IloEnv env
// Parameter: IloNumArray & thminPower
// Parameter: IloNumArray & thmaxPower
// Parameter: IloNumArray & thminDown
// Parameter: IloNumArray & thminUp
// Parameter: IloNumArray & thcoldUpTime
// Parameter: IloNumArray & thfuelCostPieceNum
// Parameter: IloNumArray & thhotUpCost
// Parameter: IloNumArray & thcoldUpCost
// Parameter: IloNumArray & thdelta
// Parameter: IloNumArray & thfirstLast
// Parameter: IloNumArray & thmaxR
// Parameter: IloNumArray & tha
// Parameter: IloNumArray & thb
// Parameter: Ilo
//************************************
int readThUnitData(char* thunitdata,
				   IloEnv env,                                   //定义环境变量
				   IloNumArray& thminPower,                        //机组最小发电量
				   IloNumArray& thmaxPower,                        //机组最大发电量
				   IloNumArray& thminDown,                         //机组最小关机时间
				   IloNumArray& thminUp,                           //机组最小开机时间
				   IloNumArray& thcoldUpTime,                      //冷启动时间
				//   IloNumArray& thfuelCostPieceNum,                //燃料费用曲线段数
				   IloNumArray& thhotUpCost,                       //热启动费用
				   IloNumArray& thcoldUpCost,			           //冷启动费用 IloNumArray& hotUpTime
				   IloNumArray& thdelta,						   //爬升
				   IloNumArray& thfirstLast,                       //首末开机约束，取0，1
				   IloNumArray& thmaxR,                            //机组最大备用	
				 //  IloNumArray& tha,							   //燃料费用曲线上系数a		
				   IloNumArray& thb,							   //燃料费用曲线上系数b		
				   IloNumArray& thc,						       //燃料费用曲线上系数c
				   IloNumArray& thminFuelCost,                        //机组最小发电费用
				   IloNumArray& thmaxFuelCost,                        //机组最大发电费用
				   IloNumArray& thinitState,                        //机组初始状态
				   IloNumArray& thinitPower                        //机组初始发电量			 
				   )
{
	ifstream fin(thunitdata,ios::in);
	if(!fin) env.out()<<"problem with file:"<<thunitdata<<endl;
	int i;

	//read minPower
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thminPower[i];
	}

	//read maxPower
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thmaxPower[i];
	}

	//read minDown
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thminDown[i];
	}

	//read minUp
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thminUp[i];
	}

	//read coldUpTime
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thcoldUpTime[i];
	}

	//read thfuelCostPieceNum
	/*
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thfuelCostPieceNum[i];
	}
*/
	//read hotUpCost
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thhotUpCost[i];
	}

	//read coldUpCost
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thcoldUpCost[i];
	}

	//read delta
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thdelta[i];
	}

	//read firstlast
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thfirstLast[i];
	}

	//read maxR
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thmaxR[i];
	}

	//read a
	/*
	for(i=0;i<thUnitNum;i++)
	{
		fin>>tha[i];
	}
*/
	//read b
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thb[i];
	}
	//read c
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thc[i];
	}

	for(i=0;i<thUnitNum;i++)
	{
	//	thminFuelCost[i]=tha[i]*thminPower[i]*thminPower[i]+thb[i]*thminPower[i]+thc[i];
	//	thmaxFuelCost[i]=tha[i]*thmaxPower[i]*thmaxPower[i]+thb[i]*thmaxPower[i]+thc[i];
		thminFuelCost[i]=thb[i]*thminPower[i]+thc[i];
		thmaxFuelCost[i]=thb[i]*thmaxPower[i]+thc[i];
	}
	//read thinitState
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thinitState[i];
	}
	//read thinitPower
	for(i=0;i<thUnitNum;i++)
	{
		fin>>thinitPower[i];
	}


	return 0;
}


//************************************
// Method:    pieceThUnitData
// FullName:  pieceThUnitData
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: IloEnv env
// Parameter: IloNumArray & tha	燃料费用曲线上系数a
// Parameter: IloNumArray & thb	燃料费用曲线上系数b
// Parameter: IloNumArray & thc
// Parameter: IloNumArray & thminPower	机组最小发电量
// Parameter: IloNumArray & thmaxPower	机组最大发电量
// Parameter: IloNumArray & thfuelCostPieceNum	分段数
// Parameter: Matrix2 & thminPiecePower
// Parameter: Matrix2 & thmaxPiecePower
// Parameter: Matrix2 & thminFuelPieceCost
// Parameter: Matrix2 & thmaxFuelPieceCost
// Parameter: Matrix2 & thfuelCostPieceSlope
//************************************
/*
int pieceThUnitData(IloEnv env,
					//IloNumArray& tha,							   		
					IloNumArray& thb,							   	
					IloNumArray& thc,							   
					IloNumArray& thminPower,                        
					IloNumArray& thmaxPower,						
				//	IloNumArray& thfuelCostPieceNum,                
					Matrix2& thminPiecePower,             
					Matrix2& thmaxPiecePower,
					Matrix2& thminFuelPieceCost,
					Matrix2& thmaxFuelPieceCost
				//	Matrix2& thfuelCostPieceSlope
					) 
{
	int i,j,pieceNum;
	IloNum d;
	for(i=0;i<thUnitNum;i++)
	{
	//  pieceNum=thfuelCostPieceNum[i];
	//	d=(thmaxPower[i]-thminPower[i])/pieceNum;
		d=thmaxPower[i]-thminPower[i];
		thminPiecePower[i][0]=thminPower[i];
	//	thminFuelPieceCost[i][0]=tha[i]*thminPiecePower[i][0]*thminPiecePower[i][0]+thb[i]*thminPiecePower[i][0]+thc[i];
		thminFuelPieceCost[i][0]=thb[i]*thminPiecePower[i][0]+thc[i];

	//	thmaxPiecePower[i][pieceNum-1]=thmaxPower[i];
		thmaxPiecePower[i][0]=thmaxPower[i];
	//	thmaxFuelPieceCost[i][pieceNum-1]=tha[i]*thmaxPiecePower[i][pieceNum-1]*thmaxPiecePower[i][pieceNum-1]+thb[i]*thmaxPiecePower[i][pieceNum-1]+thc[i];
	//	thmaxFuelPieceCost[i][pieceNum-1]=thb[i]*thmaxPiecePower[i][pieceNum-1]+thc[i];
		thmaxFuelPieceCost[i][0]=thb[i]*thmaxPiecePower[i][0]+thc[i];
	/*	for(j=1;j<pieceNum;j++)
		{
			thminPiecePower[i][j]=thminPiecePower[i][j-1]+d;
			thmaxPiecePower[i][j-1]=thminPiecePower[i][j];
			//thminFuelPieceCost[i][j]=tha[i]*thminPiecePower[i][j]*thminPiecePower[i][j]+thb[i]*thminPiecePower[i][j]+thc[i];
			thminFuelPieceCost[i][j]=thb[i]*thminPiecePower[i][j]+thc[i];
			thmaxFuelPieceCost[i][j-1]=thminFuelPieceCost[i][j];			
		}
		for(j=0;j<pieceNum;j++)
		{
			thfuelCostPieceSlope[i][j]=(thmaxFuelPieceCost[i][j]-thminFuelPieceCost[i][j])/d;
		}
		//
	}
	return 0;
}
*/
//风电数据读取函数
//************************************
// Method:    readWindData
// FullName:  readWindData
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: IloEnv env
// Parameter: IloNumArray & windLocation, the location of wind field
// Parameter: IloNumArray & maxWindPower, upper bound of wind power
// Parameter: IloNumArray & minWindPower,lower bound of wind power
//************************************
void readWindData(IloEnv env,
				  IloNumArray& windLocation,		
				  IloNumArray& maxWindPower,		
				  IloNumArray& minWindPower		
				  )
{
	ifstream fin(WINDDATA,ios::in);
	if(!fin) env.out()<<"problem with file:" << WINDDATA <<endl;
	int w = 0;

	for(w=0;w<wFieldNum;w++)
	{
		fin>>windLocation[w];
	}

	for(w=0;w<wFieldNum;w++)
	{
		fin>>maxWindPower[w];
	}

	for(w=0;w<wFieldNum;w++)
	{
		fin>>minWindPower[w];
	}
	fin.close();
	env.out()<<"Wind data is done"<<endl;
}


//水电数据读取函数
//************************************
// Method:    readHydData
// FullName:  readHydData
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: IloEnv env
// Parameter: IloNumArray & hyUnitLocation, location of hydro
// Parameter: IloNumArray & maxhyUnitPower, upper of hydropower
// Parameter: IloNumArray & minhyUnitPower, lower of hydropower
//************************************
void readHydData(IloEnv env,
				 IloNumArray& hyUnitLocation,	//水电厂位置
				 IloNumArray& maxhyUnitPower,	//水电出力上限
				 IloNumArray& minhyUnitPower		//水电出力下限
				 )
{
	ifstream fin(HYDDATA,ios::in);
	if(!fin) env.out()<<"problem with file:" << HYDDATA <<endl;
	int i,k;

	for(k=0;k<hyUnitNum;k++)
	{
		fin>>hyUnitLocation[k];
	}

	for(k=0;k<hyUnitNum;k++)
	{
		fin>>maxhyUnitPower[k];
	}

	for(k=0;k<hyUnitNum;k++)
	{
		fin>>minhyUnitPower[k];
	}
	fin.close();
	env.out()<<"Hydro data is done"<<endl;
}

//光伏数据读取函数
//************************************
// Method:    readSolarData
// FullName:  readSolarData
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: IloEnv env
// Parameter: IloNumArray & sPlantLocation, location of Solar Power Plant
// Parameter: Matrix2 & maxSPlantPower, upper bound of Solar Power Plant
// Parameter: Matrix2 & minSPlantPower, lower bound of Solar Power Plant
//************************************
void readSolarData(IloEnv env,
				   IloNumArray& sPlantLocation,	//光电厂位置
				   Matrix2& maxSPlantPower,	//光电厂最大预测出力
				   Matrix2& minSPlantPower		//光电厂最大预测出力
				   )
{
	ifstream fin(SOLARDATA,ios::in);
	if(!fin) env.out()<<"problem with file:" << SOLARDATA <<endl;
	int i,j;

	for(i=0;i<sPlantNum;i++)
	{
		fin>>sPlantLocation[i];
	}

	for(i=0;i<sPlantNum;i++)
	{
		for(j=0;j<cycle;j++)
		{
			fin>>maxSPlantPower[i][j];
		}
	}

	for(i=0;i<sPlantNum;i++)
	{
		for(j=0;j<cycle;j++)
		{
			fin>>minSPlantPower[i][j];
		}
	}
	fin.close();
	env.out()<<"Solar data is done"<<endl;
}

int main(int argc, char *argv[])
{

	IloEnv env;
	int i,j,l,d,day;
	int s = 0;
	int w = 0;
	int k = 0;
	int t = 0;
	float r= 0.15; // Pay attention to this variable, it should be input by reading file.
	try
	{
		IloModel model(env);
		IloTimer timer(env);
		timer.start();
		//*************读取系统变量***************************	
		readSystemData(env,cycle,demandNum,lineNum,busNum,sectionNum,thUnitNum,hyUnitNum,wFieldNum,sPlantNum);

		ifstream fin(APPDATA,ios::in);
		if(!fin) 
			env.out()<<"problem with file:"<<APPDATA<<endl;

		IloNumArray sysDemand(env,cycle+1);
		IloNumArray sysReserve(env,cycle+1);
		Matrix2 windPower(env,wFieldNum);
		for (w=0;w<wFieldNum;++w)
		{
			windPower[w]=IloNumArray(env,cycle+1);
		}
		day=1;
		//sysDemand
		for(t=1;t<cycle+1;t++)
			fin>>sysDemand[t];
		//sysReserve
		for(t=1;t<cycle+1;t++)
			fin>>sysReserve[t];
		for(w = 0;w < wFieldNum; w++)
		{
			for( t=1;t<cycle+1;++t) {

				fin >> windPower[w][t];

			}
		}

		//*************读取网络数据，王圭070411**************************		
		IloNumArray unitLocation(env,thUnitNum);
		IloNumArray demandLocation(env,demandNum);
		IloNumArray demand(env,demandNum);
		IloNumArray lineCap(env,lineNum);	
		IloNumArray upSectionCap(env,sectionNum);							//断面功率上限
		IloNumArray downSectionCap(env,sectionNum);						//断面功率下限
		IloNumArray linSectionNum(env,sectionNum);						//断面中线路的个数
		Matrix2 linSection(env,sectionNum);									//断面中的具体线路,矩阵[断面号][线路号]
		for(int i_sec=0;i_sec<sectionNum;i_sec++)
		{
			linSection[i_sec]=IloNumArray(env,linSectionNum[i_sec]);
		}


		readNetData(env,unitLocation, demandLocation,demand,sysDemand,lineCap
			,upSectionCap,downSectionCap,linSectionNum,linSection);	



		//*************定义机组变量**************************
		IloNumArray thminPower(env,thUnitNum);                                    //最小发电量
		IloNumArray thmaxPower(env,thUnitNum);                                    //最大发电量
		IloNumArray thminFuelCost(env,thUnitNum);
		IloNumArray thmaxFuelCost(env,thUnitNum);
		IloNumArray thminUp(env,thUnitNum);                                       //最小开机时间
		IloNumArray thminDown(env,thUnitNum);                                     //最小关机时间
		IloNumArray thcoldUpTime(env,thUnitNum);
		IloNumArray thfuelCostPieceNum(env,thUnitNum);
		IloNumArray thhotUpCost(env,thUnitNum);
		IloNumArray thcoldUpCost(env,thUnitNum);
		IloNumArray thdelta(env,thUnitNum);			
		IloNumArray thfirstLast(env,thUnitNum);
		IloNumArray thmaxR(env,thUnitNum);
	//	IloNumArray tha(env,thUnitNum);
		IloNumArray thb(env,thUnitNum);
		IloNumArray thc(env,thUnitNum);
		IloNumArray thinitState(env,thUnitNum);
		IloNumArray thinitPower(env,thUnitNum);



		//*************读火电取机组数据***************************
		readThUnitData(THUNITDATA,env,
			thminPower,                        //机组最小发电量
			thmaxPower,                        //机组最大发电量
			thminDown,                         //机组最小关机时间
			thminUp,                           //机组最小开机时间
			thcoldUpTime,                      //冷启动时间
		//	thfuelCostPieceNum,                //燃料费用曲线段数
			thhotUpCost,                       //热启动费用
			thcoldUpCost,			           //冷启动费用
			thdelta,						      //爬升
			thfirstLast,                       //首末开机约束，取0，1
			thmaxR,                          //机组最大备用	
			//tha,							     //燃料费用曲线上系数a		
			thb,							     //燃料费用曲线上系数b		
			thc,
			thminFuelCost,
			thmaxFuelCost,
			thinitState,
			thinitPower
			);
		//风电数据读取
		IloNumArray windLocation(env,wFieldNum);		//风电场位置
		IloNumArray maxWindPower(env,wFieldNum);		//风电场出力上限
		IloNumArray minWindPower(env,wFieldNum);		//风电场出力下限
		readWindData(env,windLocation,maxWindPower,minWindPower);		//读取函数

		//水电数据读取
		IloNumArray hyUnitLocation(env,hyUnitNum);		//水电机组位置
		IloNumArray maxHyUnitPower(env,hyUnitNum);		//水电机组出力上限
		IloNumArray minHyUnitPower(env,hyUnitNum);		//水电机组出力下限
		readHydData(env,hyUnitLocation,maxHyUnitPower,minHyUnitPower);	//读取函数

		//光伏数据读取
		IloNumArray sPlantLocation(env,sPlantNum);	//光电厂位置
		Matrix2 maxSPlantPower(env,sPlantNum);		//光电厂最大预测出力
		for(s=0;s<sPlantNum;s++)
		{
			maxSPlantPower[s]=IloNumArray(env,cycle);
		}
		Matrix2 minSPlantPower(env,sPlantNum);		//光电厂最小预测出力
		for(s=0;s<sPlantNum;s++)
		{
			minSPlantPower[s]=IloNumArray(env,cycle);
		}
		readSolarData(env,sPlantLocation,maxSPlantPower,minSPlantPower);	//读取函数
		//******************读取火电分段线性数据*************
	/*	Matrix2      thfuelCostPieceSlope(env,thUnitNum);
		for(i=0;i<thUnitNum;i++)
		{
			thfuelCostPieceSlope[i]=IloNumArray(env,thfuelCostPieceNum[i]);
		}

		Matrix2      thminFuelPieceCost(env,thUnitNum);
		for(i=0;i<thUnitNum;i++)
		{
			thminFuelPieceCost[i]=IloNumArray(env,thfuelCostPieceNum[i]);
		}

		Matrix2      thmaxFuelPieceCost=Matrix2(env,thUnitNum);
		for(i=0;i<thUnitNum;i++)
		{
			thmaxFuelPieceCost[i]=IloNumArray(env,thfuelCostPieceNum[i]);
		}

		Matrix2     thminPiecePower(env,thUnitNum);
		for(i=0;i<thUnitNum;i++)
		{
			thminPiecePower[i]=IloNumArray(env,thfuelCostPieceNum[i]);
		}

		Matrix2      thmaxPiecePower(env,thUnitNum);
		for(i=0;i<thUnitNum;i++)
		{
			thmaxPiecePower[i]=IloNumArray(env,thfuelCostPieceNum[i]);
		}
*/
/*
		pieceThUnitData(env,
		//	tha,							   //燃料费用曲线上系数a		
			thb,							   //燃料费用曲线上系数b		
			thc,							   //燃料费用曲线上系数b
			thminPower,                        //机组最小发电量
			thmaxPower,                        //机组最大发电量
			thfuelCostPieceNum,
			thminPiecePower,
			thmaxPiecePower,
			thminFuelPieceCost,
			thmaxFuelPieceCost,
			//thfuelCostPieceSlope
			); 
*/
		//************读取gama*********
		Matrix2      gama(env, lineNum);
		for(i=0;i<lineNum;i++)
		{
			gama[i]=IloNumArray(env, busNum);
		}
		readGama(GAMADATA,env,gama);

		//输出
		ofstream tfile(OUTFILEDATA,ios::out);
		if(!tfile)
			cout<<"cannot open "<<OUTFILEDATA<<endl;
		tfile<<"cycle="<<cycle<<endl;

		tfile<<"thUnitNum="<<thUnitNum<<endl;

		tfile<<"lineNum="<<lineNum<<endl;
		tfile<<"busNum="<<busNum<<endl;
		tfile<<"demandNum="<<demandNum<<endl;

		tfile<<endl<<"***********************网络数据****************"<<endl;
		tfile<<endl<<"unit location"<<endl;
		for (i=0;i<thUnitNum;i++)
		{
			tfile<<unitLocation[i]<<"\t";
		}

		tfile<<endl<<"demand location"<<endl;
		for (i=0;i<demandNum;i++)
		{
			tfile<<demandLocation[i]<<"\t";
		}
		tfile<<endl<<"demand***************After changed**********"<<endl;

		for (i=0;i<demandNum;i++)
		{
			tfile<<demand[i]<<"\t";
		}

		tfile<<endl<<"line capacity"<<endl;
		for (i=0;i<lineNum;i++)
		{
			tfile<<lineCap[i]<<"\t";
		}

		tfile<<"***************************火电数据************************88"<<endl;
		tfile<<endl<<"Gama: ---------Begin-------"<<endl; 
		tfile<<"busNum "<<busNum<<" linNum "<<lineNum<<endl;
		for(j=0;j<lineNum;j++)
		{
			for(i=0;i<busNum;i++)
			{
				if(i%10==0) tfile<<endl;
				tfile<<gama[j][i]<<"  ";
			}
			tfile<<endl;
		}	
		tfile<<"  ---------End----"<<endl; 

		tfile<<endl<<"thminPower"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thminPower[i]<<" ";
		}
		tfile<<endl<<"thmaxPower"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thmaxPower[i]<<" ";
		}
		tfile<<endl<<"thminDown"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thminDown[i]<<"  ";
		}
		tfile<<endl<<"thminUp"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thminUp[i]<<"  ";
		}
		tfile<<endl<<"thcoldUpTime"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thcoldUpTime[i]<<"  ";
		}
		tfile<<endl<<"thfuelCostPieceNum"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thfuelCostPieceNum[i]<<" ";
		}
		tfile<<endl<<"thhotUpCost"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thhotUpCost[i]<<"  ";
		}
		tfile<<endl<<"thcoldUpCost"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thcoldUpCost[i]<<" ";
		}
		tfile<<endl<<"thdelta"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thdelta[i]<<" ";
		}
		tfile<<endl<<"thfirstLast"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thfirstLast[i]<<" ";
		}
		tfile<<endl<<"thmaxR"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thmaxR[i]<<" ";
		}
		/*
		tfile<<endl<<"tha"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<tha[i]<<"  ";
		}
		*/
		tfile<<endl<<"thb"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thb[i]<<"  ";
		}
		tfile<<endl<<"thc"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thc[i]<<"  ";
		}
		tfile<<endl<<"thminFuelCost"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thminFuelCost[i]<<" ";
		}
		tfile<<endl<<"thmaxFuelCost"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thmaxFuelCost[i]<<" ";
		}
		tfile<<endl<<"thinitState"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thinitState[i]<<"\t";
		}
		tfile<<endl<<"thinitPower"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			tfile<<thinitPower[i]<<" ";
		}
		/*
		tfile<<endl<<"**************火电机组分段线性数据***********"<<endl;
		tfile<<"thminFuelPieceCost"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			for(j=0;j<thfuelCostPieceNum[i];j++)
			{
				tfile<<thminFuelPieceCost[i][j]<<"  ";
			}
			tfile<<endl;
		}
		tfile<<endl<<"thmaxFuelPieceCost"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			for(j=0;j<thfuelCostPieceNum[i];j++)
			{
				tfile<<thmaxFuelPieceCost[i][j]<<"  ";
			}
			tfile<<endl;
		}
		tfile<<endl<<"thminPiecePower"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			for(j=0;j<thfuelCostPieceNum[i];j++)
			{
				tfile<<thminPiecePower[i][j]<<"  ";
			}
			tfile<<endl;
		}
		tfile<<endl<<"thmaxPiecePower"<<endl;
		for(i=0;i<thUnitNum;i++)                                                        
		{
			for(j=0;j<thfuelCostPieceNum[i];j++)
			{
				tfile<<thmaxPiecePower[i][j]<<"  ";
			}
			tfile<<endl;
		}
		*/
		/*
		tfile<<endl<<"thfuelCostPieceSlope"<<endl;
		for(i=0;i<thUnitNum;i++)
		{
			for(j=0;j<thfuelCostPieceNum[i];j++)
			{
				tfile<<thfuelCostPieceSlope[i][j]<<"  ";
			}
			tfile<<endl;
		}
		*/
		tfile<<endl<<"Demand of Bus"<<endl;
		//传输线安全约束
		for(t=0;t<cycle;t++)
		{		
			for(d=0;d<demandNum;d++)
				{
					tfile << sysDemand[t]*demand[d]<<"  ";
				}
			tfile << endl;
		}
		tfile.close();

		//************************火电变量定义***********************


		/************************************************************************/
		/*			Define variable of thermal unit		By Chun-Ting            */
		/*	1. State of unit, 1 is up and 0 is down;                            */
		/*	2. thermalPower[i][t], power generated by unit i at time t,in MW;   */
		/*	3. thermalR[i][t], spinning contribution of unit i at time t,in MW; */
		/*	4. startUp[i][t], action of start up of unit i at time t,0 or 1;    */
		/*	5. shutDown[i][t], action of shut down of unit i at time t,0 or 1;  */
		/*	6. upCost[i][t], startup cost of unit i at time t;					*/
		/*	7. fuelCost[i][t], fuel cost of unit i at time t;					*/
		/************************************************************************/
		VarMatrix2 state(env,thUnitNum);
		VarMatrix2 thermalPower(env,thUnitNum);
		VarMatrix2 thermalR(env,thUnitNum);
		VarMatrix2 thermalRN(env,thUnitNum);
		VarMatrix2 startUp= VarMatrix2(env,thUnitNum);
		VarMatrix2 shutDown= VarMatrix2(env,thUnitNum);
		VarMatrix2 upCost(env,thUnitNum);
		VarMatrix2 fuelCost=VarMatrix2(env,thUnitNum);
		for(i = 0; i < thUnitNum; i++)
		{
			state[i] = IloNumVarArray(env, cycle+1, 0, 1, ILOINT);
			thermalPower[i]=IloNumVarArray(env,cycle+1,0,thmaxPower[i],ILOFLOAT);
			thermalR[i]=IloNumVarArray(env,cycle+1,0,thmaxR[i],ILOFLOAT);
			thermalRN[i]=IloNumVarArray(env,cycle+1,0,thmaxR[i],ILOFLOAT);
			startUp[i] = IloNumVarArray(env, cycle+1, 0, 1, ILOINT);
			shutDown[i]=IloNumVarArray(env,cycle+1,0,1,ILOINT);	
			upCost[i]=IloNumVarArray(env,cycle+1,0,thcoldUpCost[i],ILOFLOAT);
			fuelCost[i]=IloNumVarArray(env,cycle+1,0,thmaxFuelCost[i],ILOFLOAT);
		}



		//水电变量
		VarMatrix2 hyPower(env,hyUnitNum);
		for(k=0;k<hyUnitNum;k++)
		{
			hyPower[k]=IloNumVarArray(env,cycle,minHyUnitPower[k],maxHyUnitPower[k],ILOFLOAT);
		}

		//VarMatrix2 outputPower(env,outputNum);
		VarMatrix2 windR(env,wFieldNum);



		for(w=0;w<wFieldNum;++w)
			windR[w]=IloNumVarArray(env,cycle+1,0,maxWindPower[w],ILOFLOAT);

		//************与分段线性有关的变量*************************
		//分段线性状态
		/*
		VarMatrix3 pieceState = VarMatrix3(env, thUnitNum);
		for(i = 0; i < thUnitNum; i++)
		{
			pieceState[i] = VarMatrix2(env, cycle+1);
			for(t = 0; t < cycle+1; t++)
			{
				pieceState[i][t] = IloNumVarArray(env, thfuelCostPieceNum[i], 0, 1, ILOINT);
			}
		}
		//发电量
		VarMatrix3 thermalPiecePower=VarMatrix3(env,thUnitNum);
		for(i=0;i<thUnitNum;i++)
		{
			thermalPiecePower[i]=VarMatrix2(env,cycle+1);
			for(t=0;t<cycle+1;t++)
			{
				thermalPiecePower[i][t]=IloNumVarArray(env,thfuelCostPieceNum[i],0,thmaxPower[i],ILOFLOAT);
			}
		}
		//分段线性部分上的燃料费用
		VarMatrix3 fuelPieceCost=VarMatrix3(env,thUnitNum);
		for(i=0;i<thUnitNum;i++)
		{	
			fuelPieceCost[i]=VarMatrix2(env,cycle+1);
			for(t=0;t<cycle+1;t++)
			{
				fuelPieceCost[i][t]=IloNumVarArray(env,thfuelCostPieceNum[i],0,thmaxFuelCost[i],ILOFLOAT);
			}
		}
		*/
		//***********************************************约束******************************************
		/***************  Within the multi power system, the constraint is still ok or not ? *************/
		//可行解约束
		for(t=1;t<cycle+1;t++)
		{
			IloExpr thsummaxp(env);
			for(i=0;i<thUnitNum;i++)
			{
				thsummaxp+=thmaxPower[i]*state[i][t];
			}

			model.add(thsummaxp>=sysDemand[t]+sysReserve[t]);
		}
		//可行解中的一个约束
		for(t=1;t<cycle+1;t++)
		{
			IloExpr thsumminp(env);

			for(i=0;i<thUnitNum;i++)
			{
				thsumminp+=thminPower[i]*state[i][t];
			}

			model.add(thsumminp<=sysDemand[t]);
		}
		/******************************************** End *************************************************************/
		//***********************火电机组约束***********************
		//初始状态约束
		for(i = 0; i < thUnitNum; i++)
		{
			if(thinitState[i] < 0)
			{
				model.add(state[i][0] == 0);
				model.add(thermalPower[i][0] == 0);
				model.add(thermalR[i][0]==0);
			}
			else if(thinitState[i] > 0)
			{
				model.add(state[i][0] == 1);								
				model.add(thermalR[i][0]==0);
			}
		}

		//初始开关机约束
		for(i = 0; i< thUnitNum; i++)
		{
			model.add(startUp[i][0] == 0);
			model.add(shutDown[i][0] == 0);		
		}

		//开关机状态约束（即状态转移约束）
		for(i = 0; i < thUnitNum; i++)
		{
			for(t = 1; t < cycle+1; t++)
			{
				model.add(state[i][t]-state[i][t-1]-startUp[i][t]+shutDown[i][t]==0);
				model.add(startUp[i][t] + shutDown[i][t]<=1);
			}
		}

		//最小开关机时间约束
		for(i = 0; i < thUnitNum; i++)
		{
			for(t = 1; t < cycle+1; t++) //从1时刻到cycle
			{
				IloNum temp1 = IloMin(cycle, t+thminUp[i]-1);//the minimum of an array of numeric expressions or over a numeric expression    and a constant in C++
				IloNum temp2 = IloMin(cycle, t+thminDown[i]-1);
				IloExpr sum1(env);
				IloExpr sum2(env);
				for(k = t+1; k <= temp1; k++)
				{
					sum1 += shutDown[i][k];
				}
				for(k = t+1; k <= temp2; k++)
				{
					sum2 += startUp[i][k];
				}
				model.add(startUp[i][t] + sum1 <= 1);
				model.add(shutDown[i][t] + sum2 <= 1);
			}
		}

		//初始开关机时间约束
		for(i=0;i<thUnitNum;i++)
		{
			if(thinitState[i]<0&&IloAbs(thinitState[i])<thminDown[i])
			{
				for(t=1;t<=(thminDown[i]+thinitState[i]);t++)
				{
					model.add(startUp[i][t]==0);
					model.add(state[i][t]==0);
				}
			}
			if(thinitState[i]>0&&thinitState[i]<thminUp[i])
			{
				for(t=1;t<=(thminUp[i]-thinitState[i]);t++)
				{
					model.add(shutDown[i][t]==0);
					model.add(state[i][t]==1);
				}
			}
		}
		//开关机操作约束（从1时刻开始）
		for(i=0;i<thUnitNum;i++)
		{
			for(t=1;t<cycle+1;t++)
			{
				IloNum temp=IloMin(cycle,(t+thminUp[i]+thminDown[i]-1));
				IloExpr sum1(env);
				IloExpr sum2(env);
				for(j=t;j<=temp;j++)
				{
					sum1+=startUp[i][j];
				}
				for(j=t;j<=temp;j++)
				{
					sum2+=shutDown[i][j];
				}
				model.add(sum1-1<=0);
				model.add(sum2-1<=0);
			}
		}



		//燃料费用曲线分段后的约束
		//*****************燃料费用约束，发电量约束，备用约束******************		
		/*
		for(i=0;i<thUnitNum;i++)
		{
			for(t=1;t<cycle+1;t++)
			{
				for(j=0;j<thfuelCostPieceNum[i];j++)
				{					
					model.add(thermalPiecePower[i][t][j]<=thmaxPiecePower[i][j]*pieceState[i][t][j]);
					model.add(thermalPiecePower[i][t][j]>=thminPiecePower[i][j]*pieceState[i][t][j]);
					model.add(fuelPieceCost[i][t][j]<=thmaxFuelPieceCost[i][j]*pieceState[i][t][j]);
					model.add(fuelPieceCost[i][t][j]>=thminFuelPieceCost[i][j]*pieceState[i][t][j]);
					//model.add(fuelPieceCost[i][t][j]==thminFuelPieceCost[i][j]*pieceState[i][t][j]+thfuelCostPieceSlope[i][j]*(thermalPiecePower[i][t][j]-thminPiecePower[i][j]*pieceState[i][t][j]));
					//修改by xjlei
					//model.add(fuelPieceCost[i][t][j]==thminFuelPieceCost[i][j]*pieceState[i][t][j]+(thmaxFuelPieceCost[i][j]-thminFuelPieceCost[i][j])/(thmaxPiecePower[i][j]-thminPiecePower[i][j])*(thermalPiecePower[i][t][j]-thminPiecePower[i][j]*pieceState[i][t][j])-tha[i]/8*(thmaxPiecePower[i][j]-thminPiecePower[i][j])*(thmaxPiecePower[i][j]-thminPiecePower[i][j])*pieceState[i][t][j]);
					model.add(fuelPieceCost[i][t][j]==thminFuelPieceCost[i][j]*pieceState[i][t][j]+(thmaxFuelPieceCost[i][j]-thminFuelPieceCost[i][j])/(thmaxPiecePower[i][j]-thminPiecePower[i][j])*(thermalPiecePower[i][t][j]-thminPiecePower[i][j]*pieceState[i][t][j]));
				}
			}
		}
		
		for(i=0;i<thUnitNum;i++)
		{
			for(t=1;t<cycle+1;t++)
			{					
					model.add(thermalPiecePower[i][t][j]<=thmaxPiecePower[i][j]*pieceState[i][t][j]);
					model.add(thermalPiecePower[i][t][j]>=thminPiecePower[i][j]*pieceState[i][t][j]);
					model.add(fuelPieceCost[i][t][j]<=thmaxFuelPieceCost[i][j]*pieceState[i][t][j]);
					model.add(fuelPieceCost[i][t][j]>=thminFuelPieceCost[i][j]*pieceState[i][t][j]);
					//model.add(fuelPieceCost[i][t][j]==thminFuelPieceCost[i][j]*pieceState[i][t][j]+thfuelCostPieceSlope[i][j]*(thermalPiecePower[i][t][j]-thminPiecePower[i][j]*pieceState[i][t][j]));
					//修改by xjlei
					//model.add(fuelPieceCost[i][t][j]==thminFuelPieceCost[i][j]*pieceState[i][t][j]+(thmaxFuelPieceCost[i][j]-thminFuelPieceCost[i][j])/(thmaxPiecePower[i][j]-thminPiecePower[i][j])*(thermalPiecePower[i][t][j]-thminPiecePower[i][j]*pieceState[i][t][j])-tha[i]/8*(thmaxPiecePower[i][j]-thminPiecePower[i][j])*(thmaxPiecePower[i][j]-thminPiecePower[i][j])*pieceState[i][t][j]);
					model.add(fuelPieceCost[i][t][j]==thminFuelPieceCost[i][j]*pieceState[i][t][j]+(thmaxFuelPieceCost[i][j]-thminFuelPieceCost[i][j])/(thmaxPiecePower[i][j]-thminPiecePower[i][j])*(thermalPiecePower[i][t][j]-thminPiecePower[i][j]*pieceState[i][t][j]));
			}
		}
		//发电状态只能同时处于分段上的一段
		for(i=0;i<thUnitNum;i++)
		{
			for(t=1;t<cycle+1;t++)
			{
				IloExpr sum(env);
				for(j=0;j<thfuelCostPieceNum[i];j++)
				{
					sum +=pieceState[i][t][j];
				}
				model.add(startUp[i][t]-sum<=0);
				model.add(sum-1+shutDown[i][t]<=0);
				model.add(sum-state[i][t]==0);
			}
		}
		
		//发电量约束
		for(i=0;i<thUnitNum;i++)
		{
			for(t=1;t<cycle+1;t++)
			{
				IloExpr sum(env);
				for(j=0;j<thfuelCostPieceNum[i];j++)
				{
					sum+=thermalPiecePower[i][t][j];
				}
				model.add(thermalPower[i][t]==sum);
			}
		}
		*/
		//the constraint of fuel cost
		for(i=0;i<thUnitNum;i++)
		{
			for(t=1;t<cycle+1;t++)
			{
				model.add (fuelCost[i][t] == thb[i]*thermalPower[i][t]+thc[i]);
			}
		}

		//备用

		for(i=0;i<thUnitNum;i++)
		{
			for(t=1;t<cycle+1;t++)
			{
				model.add(thermalR[i][t]==IloMin(thmaxPower[i]*state[i][t]-thermalPower[i][t],thmaxR[i]*state[i][t]));
			}
		}

		//启动费用约束	  
		for(i=0;i<thUnitNum;i++)
		{
			for(t=1;t<cycle+1;t++)
			{
				model.add(upCost[i][t]<=thcoldUpCost[i]*startUp[i][t]);
				model.add(upCost[i][t]>=thhotUpCost[i]*startUp[i][t]);
			}
		}

		//爬升，首末开机


		//***************************************************
		//********************add by hx,爬升约束***********************
		for (i=0;i<thUnitNum; i++)
		{
			for (t=1; t<cycle+1;t++)
			{
				model.add(IloAbs(thermalPower[i][t]-thermalPower[i][t-1])<=thdelta[i]);
			}

		}

		//**********************************************约束**************************************************
		//负荷平衡约束
		for( t=0;t<cycle;t++)
		{
			IloExpr thsum(env);			//火电
			IloExpr hydsum(env);		//水电
			IloExpr windsum(env);		//风电
			IloExpr solarsum(env);		//光电
			IloExpr demandsum(env);		//需求
			IloExpr secsum(env);		//断面
			for(int i=0;i<thUnitNum;i++)
			{
				thsum+=thermalPower[i][t+1];
			}

			for(int k=0;k<hyUnitNum;k++)
			{
				hydsum+=hyPower[k][t];
			}

			for(int w=0;w<wFieldNum;w++)
			{
				windsum+=windPower[w][t];
			}

			for(int s=0;s<sPlantNum;s++)
			{
				solarsum+=maxSPlantPower[s][t];
			}

			//model.add(sysDemand[t]==thsum);	
			model.add(sysDemand[t]==thsum+hydsum+windsum+solarsum);
		}

		/************************************************************************/
		/*					Spinning reserve	       		By Chun-Ting	    */
		/************************************************************************/

		for(t=1;t<cycle+1;t++)
		{
			IloExpr thsum(env);
			IloExpr thsumN(env);
			IloExpr windRP(env);
			IloExpr windRN(env);
			//IloExpr outputRP(env);
			//IloExpr outputRN(env);
			for(i=0;i<thUnitNum;i++)
			{
				model.add(thermalR[i][t]<=IloMin(thmaxPower[i]*state[i][t]-thermalPower[i][t],thmaxR[i]*state[i][t]));
				model.add(thermalRN[i][t]==IloMin(thermalPower[i][t]-thminPower[i]*state[i][t],thmaxR[i]*state[i][t]));
				thsum+=thermalR[i][t];
				thsumN+=thermalRN[i][t];
			}
			for(w=0;w<wFieldNum;++w)
			{
				model.add(windR[w][t]==IloMin(r*windPower[w][t],maxWindPower[w]-windPower[w][t]));
				windRP+=windR[w][t];
				windRN+=r*windPower[w][t];
			}
			model.add(thsum>=sysReserve[t]+windRN);
		}


			/**************************************************************/
			//传输线安全约束
			for(t=0;t<cycle;t++)
			{		
				for(l=0;l<lineNum;l++)
				{
					IloExpr thsum(env);			//火电
					IloExpr hydsum(env);		//水电
					IloExpr windsum(env);		//风电
					IloExpr solarsum(env);		//光电
					IloExpr demandsum(env);		//需求
					IloExpr secsum(env);		//断面
					for(i=0;i<thUnitNum;i++)
					{
						thsum+=gama[l][unitLocation[i]-1]*thermalPower[i][t];
					}

					for(k=0;k<hyUnitNum;k++)
					{
						hydsum+=gama[l][hyUnitLocation[k]-1]*hyPower[k][t];
					}

					for(w=0;w<wFieldNum;w++)
					{
						windsum+=gama[l][windLocation[w]-1]*windPower[w][t];
					}

					for(s=0;s<sPlantNum;s++)
					{
						solarsum+=gama[l][sPlantLocation[s]-1]*maxSPlantPower[s][t];
					}

					for(d=0;d<demandNum;d++)
					{
						demandsum+=gama[l][demandLocation[d]-1]*sysDemand[t]*demand[d];
					}

					model.add(thsum+hydsum+windsum+solarsum-demandsum+lineCap[l]>=0);
					model.add(thsum+hydsum+windsum+solarsum-demandsum-lineCap[l]<=0);
				}
			}

			//断面约束
			for(t=0;t<cycle;t++)
			{
				for(int i_sec=0;i_sec<sectionNum;i_sec++)
				{
					IloExpr thsum(env);			//火电
					IloExpr hydsum(env);		//水电
					IloExpr windsum(env);		//风电
					IloExpr solarsum(env);		//光电
					IloExpr demandsum(env);		//需求
					IloExpr secsum(env);		//断面
					int i_linsec=0;
					l=linSection[i_sec][0];
					while(l<=linSectionNum[i_sec])
					{				
						for(i=0;i<thUnitNum;i++)
						{
							thsum+=gama[l][unitLocation[i]-1]*thermalPower[i][t];
						}

						for(k=0;k<hyUnitNum;k++)
						{
							hydsum+=gama[l][hyUnitLocation[k]-1]*hyPower[k][t];
						}

						for(w=0;w<wFieldNum;w++)
						{
							windsum+=gama[l][windLocation[w]-1]*windPower[w][t];
						}

						for(s=0;s<sPlantNum;s++)
						{
							solarsum+=gama[l][sPlantLocation[s]-1]*maxSPlantPower[s][t];
						}

						for(d=0;d<demandNum;d++)
						{
							demandsum+=gama[l][demandLocation[d]-1]*sysDemand[t]*demand[d];
						}

						secsum+=thsum+hydsum+windsum+solarsum-demandsum;

						i_linsec++;
						l=linSection[i_sec][i_linsec];
					}
					model.add(secsum>=downSectionCap[i_sec]);
					model.add(secsum<=upSectionCap[i_sec]);
				}
			}

			//建立优化目标函数
			IloExpr obj(env);
			for(t=1;t<cycle+1;t++)
			{
				for(i=0;i<thUnitNum;i++)  
				{
					obj+=fuelCost[i][t]+upCost[i][t];
				}

			}

			IloObjective objective = IloMinimize(env, obj);
			model.add(objective);

			IloCplex cplex(model);
			cplex.setParam(cplex.EpGap,0.001);//relative MIP gap tolerance
			//	cplex.setParam(cplex.NodeFileInd,3);
			//	cplex.setParam(cplex.TiLim,100);

			cplex.extract(model);
			cplex.solve();
			cplex.exportModel(lp);//与IloCplex.importModel对应，可以读回模型

			env.out() << "Solution status = " << cplex.getStatus() << endl;
			env.out() << "Solution value  = " << cplex.getObjValue() << endl;
			env.out() << "solution time   = " <<timer.getTime()<<endl;
			env.out() << "EpGap           = " <<cplex.getParam(cplex.EpGap)<<endl;


			ofstream outf(OUTFILERESULT,ios::out);
			if(!outf)
				cout<<"cannot open 'result.dat'"<<OUTFILERESULT<<endl;

			outf<<"Result"<<endl;
			outf<<"Solution status\t"<<cplex.getStatus()<<endl;
			outf<<"Solution value\t"<<cplex.getObjValue()<<endl;
			outf<<"Solution time\t"<<timer.getTime()<<endl;
			outf<<"EpGap\t"<<cplex.getParam(cplex.EpGap)<<endl; 
			double allFuelCost=0;
			for(i=0;i<thUnitNum;i++)
			{
				for(t=1;t<cycle+1;t++)
				{
					allFuelCost+=cplex.getValue(fuelCost[i][t]);
				}
			}
			cout<<endl<<"allFuelCost="<<allFuelCost<<endl;
			outf<<"allFuelCost\t"<<allFuelCost<<endl;
			if(!outf)
				cout<<"cannot open 'result.dat'"<<OUTFILERESULT<<endl;
			outf<<"state"<<endl;
			for(t=1;t<cycle+1;t++)
			{
				for(i=0;i<thUnitNum;i++)
				{
					if(cplex.getValue(state[i][t])<_INF)
						outf<<"0\t";
					else
						outf<<cplex.getValue(state[i][t])<<"\t";
				}
				outf<<endl;        
			}

			outf<<endl<<"thermalPower"<<endl;
			for(t=1;t<cycle+1;t++)
			{
				for(i=0;i<thUnitNum;i++)
				{
					if(cplex.getValue(thermalPower[i][t])<_INF)
						outf<<"0\t";
					else
						outf<<cplex.getValue(thermalPower[i][t])<<"\t";
				}
				outf<<endl;

			}

			outf<<endl<<"thermalR"<<endl;
			for(t=1;t<cycle+1;t++)
			{
				for(i=0;i<thUnitNum;i++)
				{
					if(cplex.getValue(thermalR[i][t])<1e-7)
						outf<<"0\t";
					else
						outf<<cplex.getValue(thermalR[i][t])<<"\t";
				}
				outf<<endl;
			}




			outf<<"current[l][t]"<<endl;
			double current[100][200];
			for (t = 1; t < cycle+1; t++)
			{				 		 
				for(l= 0; l< lineNum; l++)
				{
					double gamap=0;
					for (i = 0; i < thUnitNum; i++)
					{
						gamap+=gama[l][unitLocation[i]-1]*cplex.getValue(thermalPower[i][t]);
					}
					double gamaD=0;
					for (d = 0; d < demandNum; d++)
					{
						gamaD+=gama[l][demandLocation[d]-1]*sysDemand[t]*demand[d];
					} 
					current[l][t]=gamap-gamaD;	
					outf<<current[l][t]<<"\t";

				}
				outf<<endl;
			}

			outf<<endl;

		}
		catch (IloException& e) 
		{
			cerr << "Concert exception caught: " << e << endl;
		}

		catch (...) 
		{
			cerr << "Unknown exception caught" << endl;
		}

		env.end();
		system("pause"); 
		return 0;
	}
