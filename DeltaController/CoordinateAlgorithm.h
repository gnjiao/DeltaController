#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include "CoordinateStructs.h"
#include "SpaceAlgorithm.h"
#include "SpiralTrajAlgorithm.h"
#include "RocksSpiralBufferManager.h"

#include <rocksapi.h>

uint32_t mix_Boundary = 0;
ROCKS_PLANE mix_plane = ROCKS_PLANE_XY;
ROCKS_POSE mix_pose;
uint32_t mix_moveType = ROCKS_MOVE_TYPE_LINEAR;
double mix_startPos1, mix_endPos1, mix_startPos2, mix_endPos2, mix_center1, mix_center2, mix_angle, mix_endPos_x, mix_endPos_y, mix_endPos_z;

void ConvertCriclePath(const ROCKS_PLANE &plane, const ROCKS_POSE &pose, const double &startPos1, const double &startPos2, const double &center1, const double &center2, const double &angle, const  double &CurrentDistance, const double &CurrentVelocity, double *const pPosition, double *const pVelocity)
{
	double radius = sqrt((center1 - startPos1) * (center1 - startPos1) + (center2 - startPos2) * (center2 - startPos2));
	double beta(0.0);
	CalcRotateAngle(beta, center1, center2, startPos1, startPos2);

	double absoluteAngle(0.0), absoluteVelocity(0.0);
	if (angle>0)
	{
		absoluteAngle = beta - CurrentDistance / radius;
		absoluteVelocity = CurrentVelocity;
	}
	else
	{
		absoluteAngle = beta + CurrentDistance / radius;
		absoluteVelocity = -CurrentVelocity;

	}

	switch(plane)
	{
	case ROCKS_PLANE_XY:
		pPosition[0] = center1 + radius * cos(absoluteAngle);
		pPosition[1] = center2 + radius * sin(absoluteAngle);

		pVelocity[0] =  absoluteVelocity * sin(absoluteAngle);
		pVelocity[1] = -absoluteVelocity * cos(absoluteAngle);
		break;
	case ROCKS_PLANE_YZ:
		pPosition[1] = center1 + radius * cos(absoluteAngle);
		pPosition[2] = center2 + radius * sin(absoluteAngle);

		pVelocity[1] =  absoluteVelocity * sin(absoluteAngle);
		pVelocity[2] = -absoluteVelocity * cos(absoluteAngle);
		break;
	case ROCKS_PLANE_ZX:
		pPosition[0] = center1 + radius * cos(absoluteAngle);
		pPosition[2] = center2 + radius * sin(absoluteAngle);

		pVelocity[0] =  absoluteVelocity * sin(absoluteAngle);
		pVelocity[2] = -absoluteVelocity * cos(absoluteAngle);
		break;
	default:
		break;
	}

	if (pose.r.x)
	{
		Roll(pPosition, startPos1, startPos2, pose.r.x);
		Roll(pVelocity, pose.r.x);
	}

	if (pose.r.y)
	{
		Pitch(pPosition, startPos1, startPos2, pose.r.y);
		Pitch(pVelocity, pose.r.y);
	}

	if (pose.r.z)
	{
		Yaw(pPosition, startPos1, startPos2, pose.r.z);
		Yaw(pVelocity, pose.r.z);
	}

	pPosition[0] += pose.t.x;
	pPosition[1] += pose.t.y;
	pPosition[2] += pose.t.z;
}

void ConvertCriclePath(const double *const pStartPos, const double &totalAngle, const double &CurrentDistance, const double &CurrentVelocity, const ROCKS_PLANE &plane, const double &radius, const double *const pCenter, double *const pPosition, double *const pVelocity)
{
	double angle(0.0);
	double beta(0.0);
	switch(plane)
	{
	case ROCKS_PLANE_XY:
		CalcRotateAngle(beta, pCenter[0], pCenter[1], pStartPos[0], pStartPos[1]);

		if (totalAngle > 0)
		{
			angle = beta - CurrentDistance / radius;

			pVelocity[0] =  CurrentVelocity * sin(angle);
			pVelocity[1] = -CurrentVelocity * cos(angle);
			pVelocity[2] =  0;
		}
		else
		{
			angle = beta + CurrentDistance / radius;

			pVelocity[0] = -CurrentVelocity * sin(angle);
			pVelocity[1] =  CurrentVelocity * cos(angle);
			pVelocity[2] =  0;
		}

		pPosition[0] = pCenter[0] + radius * cos(angle);
		pPosition[1] = pCenter[1] + radius * sin(angle);
		pPosition[2] = pStartPos[2];
		break;
	case ROCKS_PLANE_YZ:
		CalcRotateAngle(beta, pCenter[0], pCenter[1], pStartPos[1], pStartPos[2]);

		if (totalAngle > 0)
		{
			angle = beta - CurrentDistance / radius;

			pVelocity[0] =  0;
			pVelocity[1] =  CurrentVelocity * sin(angle);
			pVelocity[2] = -CurrentVelocity * cos(angle);
		}
		else
		{
			angle = beta + CurrentDistance / radius;

			pVelocity[0] =  0;
			pVelocity[1] = -CurrentVelocity * sin(angle);
			pVelocity[2] =  CurrentVelocity * cos(angle);
		}

		pPosition[0] = pStartPos[0];
		pPosition[1] = pCenter[0] + radius * cos(angle);
		pPosition[2] = pCenter[1] + radius * sin(angle);
		break;
	case ROCKS_PLANE_ZX:
		CalcRotateAngle(beta, pCenter[0], pCenter[1], pStartPos[2], pStartPos[0]);

		if (totalAngle > 0)
		{
			angle = beta - CurrentDistance / radius;

			pVelocity[0] = -CurrentVelocity * cos(angle);
			pVelocity[1] =  0;
			pVelocity[2] =  CurrentVelocity * sin(angle);
		}
		else
		{
			angle = beta + CurrentDistance / radius;

			pVelocity[0] =  CurrentVelocity * cos(angle);
			pVelocity[1] =  0;
			pVelocity[2] = -CurrentVelocity * sin(angle);
		}

		pPosition[0] = pCenter[1] + radius * sin(angle);
		pPosition[1] = pStartPos[1];
		pPosition[2] = pCenter[0] + radius * cos(angle);
		break;
	}

}

void ConverSpiralPath(const ROCKS_PLANE &plane, const ROCKS_POSE &pose, const double &startPos1, const double &startPos2, const double &endPos1, const double &endPos2, const double &center1, const double &center2, const double &CurrentDistance, const double &CurrentVelocity, double *const pPosition, double *const pVelocity)
{
	double stratRadius(0.0), endRadius(0.0), a(0.0), b(0.0);
	CalcArchimedeSpiralPars(startPos1, startPos2, endPos1, endPos2, center1, center2, stratRadius, endRadius, a, b);

	double absoluteAngle(0.0), radius(0.0), absoluteVelocity(0.0);
	switch(plane)
	{
	case ROCKS_PLANE_XY:
		pPosition[0] = center1 + radius * cos(absoluteAngle);
		pPosition[1] = center2 + radius * sin(absoluteAngle);

		pVelocity[0] =  absoluteVelocity * sin(absoluteAngle);
		pVelocity[1] = -absoluteVelocity * cos(absoluteAngle);
		break;
	case ROCKS_PLANE_YZ:
		pPosition[1] = center1 + radius * cos(absoluteAngle);
		pPosition[2] = center2 + radius * sin(absoluteAngle);

		pVelocity[1] =  absoluteVelocity * sin(absoluteAngle);
		pVelocity[2] = -absoluteVelocity * cos(absoluteAngle);
		break;
	case ROCKS_PLANE_ZX:
		pPosition[0] = center1 + radius * cos(absoluteAngle);
		pPosition[2] = center2 + radius * sin(absoluteAngle);

		pVelocity[0] =  absoluteVelocity * sin(absoluteAngle);
		pVelocity[2] = -absoluteVelocity * cos(absoluteAngle);
		break;
	default:
		break;
	}

	if (pose.r.x)
	{
		Roll(pPosition, startPos1, startPos2, pose.r.x);
		Roll(pVelocity, pose.r.x);
	}

	if (pose.r.y)
	{
		Pitch(pPosition, startPos1, startPos2, pose.r.y);
		Pitch(pVelocity, pose.r.y);
	}

	if (pose.r.z)
	{
		Yaw(pPosition, startPos1, startPos2, pose.r.z);
		Yaw(pVelocity, pose.r.z);
	}

	pPosition[0] += pose.t.x;
	pPosition[1] += pose.t.y;
	pPosition[2] += pose.t.z;
}

void ConverLinePath(const ROCKS_PLANE &plane, const ROCKS_POSE &pose, const double &startPos1, const double &endPose1, const double &startPos2, const double &endPose2 , const double &CurrentDistance, const double &CurrentVelocity, double *const pPosition, double *const pVelocity)
{
	double distance = sqrt((endPose1 - startPos1) * (endPose1 - startPos1) + (endPose2 - startPos2) * (endPose2 - startPos2));
	double rate1 = (endPose1 - startPos1) / distance;
	double rate2 = (endPose2 - startPos2) / distance;

	switch(plane)
	{
	case ROCKS_PLANE_XY:

		pPosition[0] = CurrentDistance * rate1 + startPos1;
		pPosition[1] = CurrentDistance * rate2 + startPos2;
		pPosition[2] = 0;

		pVelocity[0] = CurrentVelocity * rate1;
		pVelocity[1] = CurrentVelocity * rate2;
		pVelocity[2] = 0;
		break;
	case ROCKS_PLANE_YZ:

		pPosition[0] = 0;
		pPosition[1] = CurrentDistance * rate1 + startPos1;
		pPosition[2] = CurrentDistance * rate2 + startPos2;

		pVelocity[0] = 0;
		pVelocity[1] = CurrentVelocity * rate1;
		pVelocity[2] = CurrentVelocity * rate2;
		break;
	case ROCKS_PLANE_ZX:

		pPosition[0] = CurrentDistance * rate1 + startPos1;
		pPosition[1] = 0;
		pPosition[2] = CurrentDistance * rate2 + startPos2;

		pVelocity[0] = CurrentVelocity * rate1;
		pVelocity[1] = 0;
		pVelocity[2] = CurrentVelocity * rate2;
		break;
	default:
		break;
	}

	if (pose.r.x)
	{
		Roll(pPosition, startPos1, startPos2, pose.r.x);
		Roll(pVelocity, pose.r.x);
	}

	if (pose.r.y)
	{
		Pitch(pPosition, startPos1, startPos2, pose.r.y);
		Pitch(pVelocity, pose.r.y);
	}

	if (pose.r.z)
	{
		Yaw(pPosition, startPos1, startPos2, pose.r.z);
		Yaw(pVelocity, pose.r.z);
	}

	pPosition[0] += pose.t.x;
	pPosition[1] += pose.t.y;
	pPosition[2] += pose.t.z;
}

void ConverLinePath(const double *const pStartPos, const double *const pEndPos, const double &totalDistance, const double &CurrentDistance, const double &CurrentVelocity, double *const pPosition, double *const pVelocity)
{
	double rate_x = (pEndPos[0] - pStartPos[0]) / totalDistance;
	double rate_y = (pEndPos[1] - pStartPos[1]) / totalDistance;
	double rate_z = (pEndPos[2] - pStartPos[2]) / totalDistance;

	pPosition[0] = CurrentDistance * rate_x + pStartPos[0];
	pPosition[1] = CurrentDistance * rate_y + pStartPos[1];
	pPosition[2] = CurrentDistance * rate_z + pStartPos[2];

	pVelocity[0] = CurrentVelocity * rate_x;
	pVelocity[1] = CurrentVelocity * rate_y;
	pVelocity[2] = CurrentVelocity * rate_z;
}

void ConvertPathToWorldCoordinate(const ROCKS_MECH* const pMech, uint32_t &index, double *const pPosition, double *const pVelocity)
{
	if (pMech->var.moveType == ROCKS_MOVE_TYPE_CIRCULAR)//check the path type
	{
		ConvertCriclePath(pMech->var.startPos, pMech->var.angle, pMech->var.pPositionSplineBuffer[index], pMech->var.pVelocitySplineBuffer[index],pMech->var.plane, pMech->var.radius, pMech->var.center, pPosition, pVelocity);
	} 

	if (pMech->var.moveType == ROCKS_MOVE_TYPE_LINEAR)
	{
		ConverLinePath(pMech->var.startPos, pMech->var.endPos, pMech->var.pPositionSplineBuffer[pMech->var.usedNrOfSplines - 1], pMech->var.pPositionSplineBuffer[index],pMech->var.pVelocitySplineBuffer[index], pPosition, pVelocity);
	}

	if (pMech->var.moveType == ROCKS_MOVE_TYPE_MIX)
	{
		if (index == 0)
		{
			mix_Boundary = 0;
		}
		if (index == mix_Boundary)
		{
			mix_Boundary = (uint32_t)(pMech->var.pVelocitySplineBuffer[index + 3] + index + 7);
			switch ((int)(pMech->var.pPositionSplineBuffer[index + 3]) / 256)
			{
			case ROCKS_PLANE_XY:
				mix_plane = ROCKS_PLANE_XY;
				break;
			case ROCKS_PLANE_YZ:
				mix_plane = ROCKS_PLANE_YZ;
				break;
			case ROCKS_PLANE_ZX:
				mix_plane = ROCKS_PLANE_ZX;
				break;
			default:
				break;
			}
			switch ((int)(pMech->var.pPositionSplineBuffer[index + 3]) % 256)
			{
			case ROCKS_MOVE_TYPE_LINEAR:
				mix_moveType = ROCKS_MOVE_TYPE_LINEAR;
				break;
			case ROCKS_MOVE_TYPE_CIRCULAR:
				mix_moveType = ROCKS_MOVE_TYPE_CIRCULAR;
				break;
			case ROCKS_MOVE_TYPE_SPIRAL:
				mix_moveType = ROCKS_MOVE_TYPE_SPIRAL;
				break;
			case ROCKS_MOVE_TYPE_SPIRAL_EX:
				mix_moveType = ROCKS_MOVE_TYPE_SPIRAL_EX;
				break;
			default:
				break;
			}
			mix_endPos_x  = pMech->var.pVelocitySplineBuffer[index];
			mix_endPos_y  = pMech->var.pPositionSplineBuffer[index];
			mix_endPos_z  = pMech->var.pVelocitySplineBuffer[index + 1];
			mix_pose.r.x  = pMech->var.pPositionSplineBuffer[index + 1];
			mix_pose.r.y  = pMech->var.pPositionSplineBuffer[index + 2];
			mix_pose.r.z  = pMech->var.pVelocitySplineBuffer[index + 2];
			mix_startPos1 = pMech->var.pVelocitySplineBuffer[index + 4];
			mix_startPos2 = pMech->var.pPositionSplineBuffer[index + 4];
			switch (mix_moveType)
			{
			case ROCKS_MOVE_TYPE_LINEAR:
				mix_endPos1 = pMech->var.pVelocitySplineBuffer[index + 5];
				mix_endPos2 = pMech->var.pPositionSplineBuffer[index + 5];
				break;
			case ROCKS_MOVE_TYPE_CIRCULAR:
				mix_angle = pMech->var.pVelocitySplineBuffer[index + 5];
				mix_center1 = pMech->var.pVelocitySplineBuffer[index + 6];
				mix_center2 = pMech->var.pPositionSplineBuffer[index + 6];
				break;
			case ROCKS_MOVE_TYPE_SPIRAL:
				mix_endPos1 = pMech->var.pVelocitySplineBuffer[index + 5];
				mix_endPos2 = pMech->var.pPositionSplineBuffer[index + 5];
				mix_center1 = pMech->var.pVelocitySplineBuffer[index + 6];
				mix_center2 = pMech->var.pPositionSplineBuffer[index + 6];
				break;
			default:
				break;
			}
			index += 7;
		}
		switch (mix_moveType)
		{
		case ROCKS_MOVE_TYPE_LINEAR:
			ConverLinePath(mix_plane, mix_pose, mix_startPos1, mix_endPos1, mix_startPos2, mix_endPos2, pMech->var.pPositionSplineBuffer[index], pMech->var.pVelocitySplineBuffer[index], pPosition, pVelocity);
			break;
		case ROCKS_MOVE_TYPE_CIRCULAR:
			ConvertCriclePath(mix_plane, mix_pose, mix_startPos1, mix_startPos2, mix_center1, mix_center2, mix_angle, pMech->var.pPositionSplineBuffer[index], pMech->var.pVelocitySplineBuffer[index], pPosition, pVelocity);
			break;
		case ROCKS_MOVE_TYPE_SPIRAL:
			//ConverSpiralPath();
			break;
		case ROCKS_MOVE_TYPE_SPIRAL_EX:
			pPosition[0] = pPosSpiralSplineBuffer[index][0];
			pPosition[1] = pPosSpiralSplineBuffer[index][1];
			pPosition[2] = pPosSpiralSplineBuffer[index][2];

			pVelocity[0] = pVelSpiralSplineBuffer[index][0];
			pVelocity[1] = pVelSpiralSplineBuffer[index][1];
			pVelocity[2] = pVelSpiralSplineBuffer[index][2];

			return;

			break;
		default:
			break;
		}
		switch (mix_plane)
		{
		case ROCKS_PLANE_XY://XY-PLANE
			pPosition[2] = mix_endPos_z;
			break;
		case ROCKS_PLANE_YZ://YZ-PLANE
			pPosition[0] = mix_endPos_x;
			break;
		case ROCKS_PLANE_ZX://ZX-PLANE
			pPosition[1] = mix_endPos_y;
			break;
		default:
			break;
		}
	}

	if (pMech->var.refFramePose2.r.x)
	{
		Roll(pPosition, pMech->var.startPos, pMech->var.refFramePose2.r.x);
		Roll(pVelocity, pMech->var.refFramePose2.r.x);
	}

	if (pMech->var.refFramePose2.r.y)
	{
		Pitch(pPosition, pMech->var.startPos, pMech->var.refFramePose2.r.y);
		Pitch(pVelocity, pMech->var.refFramePose2.r.y);
	}

	if (pMech->var.refFramePose2.r.z)
	{
		Yaw(pPosition, pMech->var.startPos, pMech->var.refFramePose2.r.z);
		Yaw(pVelocity, pMech->var.refFramePose2.r.z);
	}

	pPosition[0] += pMech->var.refFramePose2.t.x;
	pPosition[1] += pMech->var.refFramePose2.t.y;
	pPosition[2] += pMech->var.refFramePose2.t.z;
} 

void ConvertTwoCoordinate(const ROCKS_COORD &origin, ROCKS_COORD &target)
{
	ROCKS_COORD buffer;
	buffer.postion.x = origin.postion.x;
	buffer.postion.y = origin.postion.y;
	buffer.postion.z = origin.postion.z;
	buffer.type = WORLD_COORD;

	//转化为世界坐标系
	if (origin.type != WORLD_COORD)
	{
		double buf(0.0);
		uint32_t index(origin.type);
		double angleX(g_pTransfMatrix[index].r.x);
		double angleY(g_pTransfMatrix[index].r.y);
		double angleZ(g_pTransfMatrix[index].r.z);
		//Roll
					 buf =  buffer.postion.y * cos(angleX) + buffer.postion.z * sin(angleX);
		buffer.postion.z = -buffer.postion.y * sin(angleX) + buffer.postion.z * cos(angleX);
		buffer.postion.y =  buf;

		//Pitch
					 buf = buffer.postion.x * cos(angleY) - buffer.postion.z * sin(angleY);
		buffer.postion.z = buffer.postion.x * sin(angleY) + buffer.postion.z * cos(angleY);
		buffer.postion.x = buf;

		//Yaw
					 buf =  buffer.postion.x * cos(angleZ) + buffer.postion.y * sin(angleZ);
		buffer.postion.y = -buffer.postion.x * sin(angleZ) + buffer.postion.y * cos(angleZ);
		buffer.postion.x =  buf;

		buffer.postion.x += g_pTransfMatrix[index].t.x;
		buffer.postion.y += g_pTransfMatrix[index].t.y;
		buffer.postion.z += g_pTransfMatrix[index].t.z;
	}

	target.postion.x = buffer.postion.x;
	target.postion.y = buffer.postion.y;
	target.postion.z = buffer.postion.z;

	//转化为目标坐标系
	if (target.type != WORLD_COORD)
	{
		double buf(0.0);
		uint32_t index(target.type);
		double angleX(-g_pTransfMatrix[index].r.x);
		double angleY(-g_pTransfMatrix[index].r.y);
		double angleZ(-g_pTransfMatrix[index].r.z);

		target.postion.x -= g_pTransfMatrix[index].t.x;
		target.postion.y -= g_pTransfMatrix[index].t.y;
		target.postion.z -= g_pTransfMatrix[index].t.z;

		//Yaw
					 buf =  target.postion.x * cos(angleZ) + target.postion.y * sin(angleZ);
		target.postion.y = -target.postion.x * sin(angleZ) + target.postion.y * cos(angleZ);
		target.postion.x =  buf;

		//Pitch
					 buf = target.postion.x * cos(angleY) - target.postion.z * sin(angleY);
		target.postion.z = target.postion.x * sin(angleY) + target.postion.z * cos(angleY);
		target.postion.x = buf;

		//Roll
					 buf =  target.postion.y * cos(angleX) + target.postion.z * sin(angleX);
		target.postion.z = -target.postion.y * sin(angleX) + target.postion.z * cos(angleX);
		target.postion.y =  buf;
	}
}