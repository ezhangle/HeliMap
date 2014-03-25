#define M_PI 3.14159265358979323846
const double R_MAJOR = 6378137.0;
//eccentricity angle of the WGS84 spheroid
const double eccent_e = 0.081819191310869760;

struct Coordinate
{
	double lat;
	double lon;
	Coordinate(double l,double ln)
	{
		lat=l;
		lon=ln;
	}
	Coordinate()
	{
		lat=0.0f;
		lon=0.0f;
	}
	void ToRad()
	{
		lat*=M_PI/180;
		lon*=M_PI/180;
	}
	void ToDeg()
	{
		lat*=180/M_PI;
		lon*=180/M_PI;
	}
	void ToScreen()
	{
		printf("Lat:%f Lon:%f\n",lat,lon);
	}
};
struct Point
{
	float x;
	float y;
	Point(float tx,float ty)
	{
		x=tx;
		y=ty;
	}
	Point()
	{
		x=0.0f;
		y=0.0f;
	}
};
float GetDistance(Coordinate point1,Coordinate point2) 
{
	//haversine formula for calculating distance of two gps points, accurate to ~.5ft
	//equivalent to law of cosines but more accurate for computer arithmetic

	float radius=6371.0f; //radius of earth
	float deltalat=point2.lat-point1.lat;
	float deltalon=point2.lon-point1.lon;

	deltalat*=M_PI/180; //convert to radians
	deltalon*=M_PI/180;//convert to radians
	point1.ToRad();//convert to radians
	point2.ToRad();//convert to radians

	float a=sin(deltalat/2)*sin(deltalat/2)+sin(deltalon/2)*sin(deltalon/2)*cos(point1.lat)*cos(point2.lat);//square of half the chord length between the points
	float c=2*atan2(sqrt(a),sqrt(1-a)); //angular distance in radians
	float distance=radius*c;
	return distance; //returns kilometers
}
float GetBearing(Coordinate point1,Coordinate point2)
{
	//returns bearing in degrees on azimuth compass dial
	/////////360////////
	/////			////
	//270  azimuth	//90
	/////			////
	////////180/////////
	float deltalat=point2.lat-point1.lat;
	float deltalon=point2.lon-point1.lon;

	deltalat*=M_PI/180; //convert to radians
	deltalon*=M_PI/180; //convert to radians
	point1.ToRad(); //convert to radians
	point2.ToRad();//convert to radians

	float y=sin(deltalon)*cos(point2.lat);
	float x=cos(point1.lat)*sin(point2.lat)-sin(point1.lat)*cos(point2.lat)*cos(deltalon);
	float bearing=atan2(y,x)*180/M_PI; //convert to degrees
	//atan2 return +180 to -180 so we need to convert to 0-360
	bearing+=360;
	bearing=std::fmod(bearing,360.0f);
	return bearing;
}
Coordinate GetCenterPoint(std::vector<Coordinate> coordList)
{
	
	int total=coordList.size();

	double X = 0;
	double Y = 0;
	double Z = 0;

	for(int i=0;i<total;i++)
	{
		Coordinate CurCoord=coordList.at(i);
		double lat = CurCoord.lat * M_PI / 180;
		double lon = CurCoord.lon * M_PI / 180;

		double x = cos(lat) *cos(lon);
		double y = cos(lat) * sin(lon);
		double z = sin(lat);

		X += x;
		Y += y;
		Z += z;
	}

	X = X / total;
	Y = Y / total;
	Z = Z / total;

	double Lon = atan2(Y, X);
	double Hyp = sqrt(X * X + Y * Y);
	double Lat = atan2(Z, Hyp);

	return  Coordinate(Lat * 180 / M_PI, Lon * 180 / M_PI);
}

Point GetProjection(Coordinate coord) //Mercator Projection System
{
	Point returnval=Point(0,0);
	coord.ToRad();
	returnval.x=R_MAJOR*coord.lon;

	if (coord.lat > 89.5)
		coord.lat = 89.5;
	if (coord.lat < -89.5)
		coord.lat = -89.5;

	double phi = coord.lat;
	double sinphi = sin(phi);
	double com = eccent_e * sinphi;
	com = pow( ( (1.0 - com)/(1.0 + com) ), .5 * eccent_e );
	double ts = tan(.5 * ( (M_PI*.5f) - phi) ) / com;
	double y = 0 - R_MAJOR * log(ts);

	returnval.y=y;
	return returnval;
}

float Re_Range(float x,float inmin,float inmax,float outmin,float outmax)
{
	float newval=(x - inmin) * (outmax - outmin) / (inmax - inmin) + outmin;
	return newval;
}


 class ObjectsToDraw
{
	std::vector<Coordinate> ObstacleCoords;
	std::vector<Point> ObstacleScreenPoints;
	std::vector<Point> DropScreenPoints;
	std::vector<Coordinate> DropPoints;
	Coordinate HeliCoord;
	Point HeliScreenPoint;
	float minx,miny,maxx, maxy;
public:
	ObjectsToDraw(std::vector<Coordinate> Obstacles,std::vector<Coordinate> Drops,Coordinate Heli)
	{
		ObstacleCoords=Obstacles;
		DropPoints=Drops;
		HeliCoord=Heli;
		minx=99999999999999999.0f;
		miny=99999999999999999.0f;
		maxx=-99999999999999999.0f;
		maxy=-99999999999999999.0f;
	}

	void CalculateScreens()
	{
		//we need to clear arrays so we don't have duplicates
		ObstacleScreenPoints.clear();
		DropScreenPoints.clear();
		
		//now we iterate all entities to be drawn, and find their extremes so we can zoom in
		for(int i=0;i<ObstacleCoords.size();i++)
		{
			Point screen=GetProjection(ObstacleCoords.at(i)); //get mercator projection
			//find min and max values for re-ranging, this is effectively zooming to fit our objects
			CalcZoom(screen);
			ObstacleScreenPoints.push_back(screen);
		}
		//same thing as obstacles but do it for heli now
		HeliScreenPoint=Point(0,0);
		HeliScreenPoint=GetProjection(HeliCoord);
		CalcZoom(HeliScreenPoint);

		for(int i=0;i<DropPoints.size();i++)
		{
			Point screen=GetProjection(DropPoints.at(i));
			CalcZoom(screen);
			DropScreenPoints.push_back(screen);
		}

		float x=Re_Range(HeliScreenPoint.x,minx,maxx,100,clientwidth-100); //re-range our x,y values, add 100 to keep a nice border
		float y=Re_Range(HeliScreenPoint.y,miny,maxy,100,clientheight-100);
		HeliScreenPoint=Point(x,y); 

		//now re-range our obstacles points
		for(int i=0;i<ObstacleScreenPoints.size();i++)
		{
			Point pnt=ObstacleScreenPoints.at(i);
			float x=Re_Range(pnt.x,minx,maxx,100,clientwidth-100);
			float y=Re_Range(pnt.y,miny,maxy,100,clientheight-100);
			ObstacleScreenPoints.at(i)=Point(x,y);
		}

		for(int i=0;i<DropScreenPoints.size();i++)
		{
			Point pnt=DropScreenPoints.at(i);
			float x=Re_Range(pnt.x,minx,maxx,100,clientwidth-100);
			float y=Re_Range(pnt.y,miny,maxy,100,clientheight-100);
			DropScreenPoints.at(i)=Point(x,y);
		}
	}
	Coordinate GetObstalceAt(int index)
	{
		if(index>ObstacleCoords.size())
			return Coordinate();

		return ObstacleCoords.at(index);
	}
	Point GetObstacleScreenAt(int index)
	{
		if(index>ObstacleScreenPoints.size())
			return Point();

		return ObstacleScreenPoints.at(index);
	}
	Coordinate GetDropat(int index)
	{
		if(index>DropPoints.size())
			return Coordinate();

		return DropPoints.at(index);
	}
	Point GetDropScreenAt(int index)
	{
		if(index>DropScreenPoints.size())
			return Point();

		return DropScreenPoints.at(index);
	}
	int GetObstacleScreenSize()
	{
		return ObstacleScreenPoints.size();
	}
	int GetObstacleSize()
	{
		return ObstacleCoords.size();
	}
	int GetDropPointSize()
	{
		return DropPoints.size();
	}
	int GetDropScreenSize()
	{
		return DropScreenPoints.size();
	}
	Coordinate GetHeli()
	{
		return HeliCoord;
	}
	Point GetHeliScreen()
	{
		return HeliScreenPoint;
	}
	void SetHeli(Coordinate coord)
	{
		HeliCoord=coord;
		CalculateScreens();
	}
	void SetObstacleAt(Coordinate obst,int index)
	{
		if(index>ObstacleCoords.size())
			ObstacleCoords.push_back(obst);

		ObstacleCoords.at(index)=obst;
		CalculateScreens();
	}
	~ObjectsToDraw()
	{
		
	}
	void CalcZoom(Point screen)
	{
		if(screen.x<minx)
			minx=screen.x;

		if(screen.x>maxx)
			maxx=screen.x;

		if(screen.y<miny)
			miny=screen.y;

		if(screen.y>maxy)
			maxy=screen.y;
	}
};