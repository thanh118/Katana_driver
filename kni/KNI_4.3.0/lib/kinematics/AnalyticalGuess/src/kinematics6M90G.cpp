/***************************************************************************
 *   Copyright (C) 2008 by Neuronics AG                                    *
 *   support@neuronics.ch                                                  *
 ***************************************************************************/

#include <kinematics6M90G.h>

namespace AnaGuess {

//////////////////////////////////////////////////////////////////////////
Kinematics6M90G::Kinematics6M90G() {
	initialize();
}
//////////////////////////////////////////////////////////////////////////
Kinematics6M90G::~Kinematics6M90G() {

}

//!get link length
std::vector<double> Kinematics6M90G::getLinkLength() {
	std::vector<double> result(mSegmentLength);
	return result;
}
//!get encoders per cycle
std::vector<int> Kinematics6M90G::getEpc() {
	std::vector<int> result(mEncodersPerCycle);
	return result;
}
//!get encoder offset
std::vector<int> Kinematics6M90G::getEncOff() {
	std::vector<int> result(mEncoderOffset);
	return result;
}
//!get direction
std::vector<int> Kinematics6M90G::getDir() {
	std::vector<int> result(mRotationDirection);
	return result;
}
//!get angle offset
std::vector<double> Kinematics6M90G::getAngOff() {
	std::vector<double> result(mAngleOffset);
	return result;
}
//!get angle stop
std::vector<double> Kinematics6M90G::getAngStop() {
	std::vector<double> result(mAngleStop);
	return result;
}
//!get angle range
std::vector<double> Kinematics6M90G::getAngRange() {
	std::vector<double> result;
	double diff;
	for (int i = 0; i < 6; i++) {
		diff = mAngleStop[i] - mAngleOffset[i];
		if (diff < 0) {
			result.push_back(-diff);
		} else {
			result.push_back(diff);
		}
	}
	return result;
}
//!get angle min
std::vector<double> Kinematics6M90G::getAngMin() {
	std::vector<double> result;
	for (int i = 0; i < 6; i++) {
		if (mAngleStop[i] < mAngleOffset[i]) {
			result.push_back(mAngleStop[i]);
		} else {
			result.push_back(mAngleOffset[i]);
		}
	}
	return result;
}
//!get angle max
std::vector<double> Kinematics6M90G::getAngMax() {
	std::vector<double> result;
	for (int i = 0; i < 6; i++) {
		if (mAngleStop[i] < mAngleOffset[i]) {
			result.push_back(mAngleOffset[i]);
		} else {
			result.push_back(mAngleStop[i]);
		}
	}
	return result;
}

//!set link length
bool Kinematics6M90G::setLinkLength(const std::vector<double> aLengths) {
	if ((int) aLengths.size() != mNumberOfSegments) {
		return false;
	}

	for (int i = 0; i < mNumberOfSegments; ++i) {
		mSegmentLength[i] = aLengths.at(i);
	}

	return true;
}
//!set angle offset
bool Kinematics6M90G::setAngOff(const std::vector<double> aAngOff) {
	if ((int) aAngOff.size() != mNumberOfMotors) {
		return false;
	}

	for (int i = 0; i < mNumberOfMotors; ++i) {
		mAngleOffset[i] = aAngOff.at(i);
	}

	return true;
}
//!set angle stop
bool Kinematics6M90G::setAngStop(const std::vector<double> aAngStop) {
	if ((int) aAngStop.size() != mNumberOfMotors) {
		return false;
	}

	for (int i = 0; i < mNumberOfMotors; ++i) {
		mAngleStop[i] = aAngStop.at(i);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Kinematics6M90G::enc2rad(std::vector<double>& aAngles, const std::vector<int> aEncoders) {
	for(int i = 0; i < 6; ++i) {
		aAngles[i] = MHF::enc2rad(aEncoders[i], mAngleOffset[i], mEncodersPerCycle[i], mEncoderOffset[i], mRotationDirection[i]);
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
bool Kinematics6M90G::rad2enc(std::vector<int>& aEncoders, const std::vector<double> aAngles) {
	for(int i = 0; i < 6; ++i ) {
		aEncoders[i] = MHF::rad2enc(aAngles[i], mAngleOffset[i], mEncodersPerCycle[i], mEncoderOffset[i], mRotationDirection[i]);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Kinematics6M90G::directKinematics(std::vector<double>& aPosition, const std::vector<double> aAngles) {
	if(!mIsInitialized) {
		initialize();
	}

	// numering the angles starting by 0-5

	double x0, x1, x2, x3;
	double y0, y1, y2, y3;
	double z0, z1, z2, z3;
	double R13, R23, R33, R31, R32;

	std::vector<double> current_angles(6);
	for(int i = 0; i < 6; ++i) {
		current_angles[i] = aAngles[i];
	}

	// needs refactoring:
	current_angles[1] = current_angles[1] - MHF_PI/2.0;
	current_angles[2] = current_angles[2] - MHF_PI;
	current_angles[3] = MHF_PI - current_angles[3];

	std::vector<double> pose(6);

	std::vector<double> cx(current_angles.size()), sx(current_angles.size());
	std::vector<double>::iterator cx_iter, sx_iter;

	std::vector<double> angle = current_angles;

	angle[2] = angle[1]+angle[2];
	angle[3] = angle[2]+angle[3];

	cx_iter = cx.begin();
	sx_iter = sx.begin();
	std::transform(angle.begin(), angle.end(), sx_iter, MHF::unary_precalc_sin<double>() );
	std::transform(angle.begin(), angle.end(), cx_iter, MHF::unary_precalc_cos<double>() );

	R13 = (-1*cx[0]*cx[3]*cx[4])-(sx[0]*sx[4]);
	R23 = (-1*sx[0]*cx[3]*cx[4])+(cx[0]*sx[4]);
	R33 = sx[3]*cx[4];
	R31 = cx[3];
	R32 =(-1)*sx[3]*sx[4];

	//x
	x0 =  cx[0]*sx[1];
	x1 =  cx[0]*sx[2];
	x2 =  cx[0]*sx[3];
	x3 = -cx[0]*cx[3]*cx[4]-sx[0]*sx[4];
	pose[0] = x0*mSegmentLength[0]+x1*mSegmentLength[1]+x2*mSegmentLength[2]+x3*mSegmentLength[3];

	//y
	y0 = sx[0]*sx[1];
	y1 = sx[0]*sx[2];
	y2 = sx[0]*sx[3];
	y3 = -sx[0]*cx[3]*cx[4]+cx[0]*sx[4];
	pose[1] = y0*mSegmentLength[0]+y1*mSegmentLength[1]+y2*mSegmentLength[2]+y3*mSegmentLength[3];

	//z
	z0 = cx[1];
	z1 = cx[2];
	z2 = cx[3];
	z3 = cx[4]*sx[3];
	pose[2] = z0*mSegmentLength[0]+z1*mSegmentLength[1]+z2*mSegmentLength[2]+z3*mSegmentLength[3];

	// phi, theta, psi
	pose[4] = acos(R33);
	if(pose[4]==0) {
		pose[3] = atan2(pose[1],pose[0]);
		pose[5] = 0;
	} else if(pose[4]==MHF_PI) {
		pose[3] = atan2(pose[1],pose[0])+MHF_PI/2;
		pose[5] = MHF_PI/2;
	} else {
		pose[3] = atan2(R13,-R23);
		pose[5] = atan2(R31,R32);
	}


	std::swap(aPosition, pose);
	return true;
}
//////////////////////////////////////////////////////////////////////////
bool Kinematics6M90G::inverseKinematics(std::vector<double>& aAngles, const std::vector<double> aPosition,
		const std::vector<double> aStartingAngles) {
	if(!mIsInitialized) {
		initialize();
	}

	// aPosition: Winkel Deg->Rad
	// Alle 8 Loeungen werden in einem Array angle, welches aus 8 Structs besteht, gespeichert:
	// 0-3 fr theta1_1
	// 4-7 fr theta1_2

	// Declaration
	position p_gr;
	position p_m;
	angles_container angle(cNrOfPossibleSolutions);

	// calculation of the gripper vector
	p_gr.x = mSegmentLength[3]*sin(aPosition[4])*sin(aPosition[3]);
	p_gr.y = -mSegmentLength[3]*sin(aPosition[4])*cos(aPosition[3]);
	p_gr.z = mSegmentLength[3]*cos(aPosition[4]);

	p_m.x = aPosition[0]-p_gr.x;
	p_m.y = aPosition[1]-p_gr.y;
	p_m.z = aPosition[2]-p_gr.z;

	// calculate theta1_1 and theta1_2
	angle[0].theta1 = MHF::atan1(p_m.x,p_m.y);
	angle[4].theta1 = angle[0].theta1+MHF_PI;


	// check the borders according to the settings
	if(angle[0].theta1>mAngleStop[0])
		angle[0].theta1=angle[0].theta1-2.0*MHF_PI;

	if(angle[0].theta1<mAngleOffset[0])
		angle[0].theta1=angle[0].theta1+2.0*MHF_PI;

	if(angle[4].theta1>mAngleStop[0])
		angle[4].theta1=angle[4].theta1-2.0*MHF_PI;

	if(angle[4].theta1<mAngleOffset[0])
		angle[4].theta1=angle[4].theta1+2.0*MHF_PI;


	//====THETA1_1==================
	//-------THETA234_1-------------
	IK_theta234theta5(angle[0], p_gr);
	IK_b1b2costh3_6MS(angle[0], p_m);

	angle[1]=angle[0];
	angle[0].theta3 =  acos(angle[0].costh3)-MHF_PI;
	thetacomp(angle[0], p_m);
	angle[1].theta3 =  -acos(angle[1].costh3)+MHF_PI;
	thetacomp(angle[1], p_m);

	//-------THETA234_2-------------
	angle[2].theta1=angle[0].theta1;
	angle[2].theta234=angle[0].theta234-MHF_PI;
	angle[2].theta5=MHF_PI-angle[0].theta5;

	IK_b1b2costh3_6MS(angle[2], p_m);
	angle[3]=angle[2];
	angle[2].theta3 =  acos(angle[2].costh3)-MHF_PI;
	thetacomp(angle[2], p_m);
	angle[3].theta3 =  -acos(angle[3].costh3)+MHF_PI;
	thetacomp(angle[3], p_m);


	//====THETA1_2==================
	//-------THETA234_1-------------
	IK_theta234theta5(angle[4], p_gr);
	IK_b1b2costh3_6MS(angle[4], p_m);

	angle[5]=angle[4];
	angle[4].theta3 =  acos(angle[4].costh3)-MHF_PI;
	thetacomp(angle[4], p_m);
	angle[5].theta3 =  -acos(angle[5].costh3)+MHF_PI;
	thetacomp(angle[5], p_m);

	//-------THETA234_2-------------
	angle[6].theta1=angle[4].theta1;
	angle[6].theta234=angle[4].theta234-MHF_PI;
	angle[6].theta5=MHF_PI-angle[4].theta5;
	IK_b1b2costh3_6MS(angle[6], p_m);
	angle[7]=angle[6];
	angle[6].theta3 =  acos(angle[6].costh3)-MHF_PI;
	thetacomp(angle[6], p_m);
	angle[7].theta3 =  -acos(angle[7].costh3)+MHF_PI;
	thetacomp(angle[7], p_m);

	// delete solutions out of range (in joint space)
	for( ::std::vector<angles_calc>::iterator iter = angle.begin(); iter != angle.end(); /* do iter forward in body */ ) {
		if( MHF::pow2(iter->costh3) <= 1.0) {
			if(!angledef(*iter))
				iter = angle.erase(iter);
			else
				++iter;
			continue;
		}
		iter = angle.erase(iter);
	}


	// check if solutions in range left
	if(angle.size() == 0) {
		throw NoSolutionException();
	}

	// store possible solution angles to std::vector<std::vector<double>>
	std::vector< std::vector<double> > PossibleTargets;
	for( std::vector<angles_calc>::iterator i = angle.begin(); i != angle.end(); ++i ) {
		std::vector<double> possangles(5);

		possangles[0] = i->theta1;
		possangles[1] = i->theta2;
		possangles[2] = i->theta3;
		possangles[3] = i->theta4;
		possangles[4] = i->theta5;

		PossibleTargets.push_back(possangles);
	}

	// choose best solution
	std::vector< std::vector<double> >::const_iterator sol = KinematicsDefaultRadMinAlgorithm()(PossibleTargets.begin(), PossibleTargets.end(), aStartingAngles.begin(), aStartingAngles.end());

	if(sol == PossibleTargets.end()) {
		throw NoSolutionException();
	}

	// copy solution to aAngles vector
	for (int i = aAngles.size(); i < 6; ++i)
		aAngles.push_back(0.0);
	std::vector<double>::iterator gripper_iter = std::copy( (*sol).begin(), (*sol).end(), aAngles.begin() );
	*gripper_iter = aStartingAngles[5]; // copy gripper-angle from current

	return true;
}
//////////////////////////////////////////////////////////////////////////
bool Kinematics6M90G::initialize() {
	// NOTE: data for Katana6M90A with angle gripper
	
	mIsInitialized = false;

	//fill in segment data
	mNumberOfSegments = 4;

	mSegmentLength.push_back(190.0);
	mSegmentLength.push_back(139.0);
	mSegmentLength.push_back(147.3);
	mSegmentLength.push_back(150.5);

	//fill in joint data
	mNumberOfMotors = 6;

	mAngleOffset.push_back(0.116064);
	mAngleStop.push_back(6.154904);
	mEncodersPerCycle.push_back(51200);
	mEncoderOffset.push_back(31000);
	mRotationDirection.push_back(1);

	mAngleOffset.push_back(2.168572);
	mAngleStop.push_back(-0.274889);
	mEncodersPerCycle.push_back(94976);
	mEncoderOffset.push_back(-31000);
	mRotationDirection.push_back(1);

	mAngleOffset.push_back(0.919789);
	mAngleStop.push_back(5.283112);
	mEncodersPerCycle.push_back(47488);
	mEncoderOffset.push_back(-31000);
	mRotationDirection.push_back(-1);

	mAngleOffset.push_back(1.108284);
	mAngleStop.push_back(5.122541);
	mEncodersPerCycle.push_back(51200);
	mEncoderOffset.push_back(31000);
	mRotationDirection.push_back(1);

	mAngleOffset.push_back(0.148353); // for 6M90B: -2.99324
	mAngleStop.push_back(6.117379); // for 6M90B: 2.975787
	mEncodersPerCycle.push_back(51200);
	mEncoderOffset.push_back(31000);
	mRotationDirection.push_back(1);

	mAngleOffset.push_back(-2.085668);
	mAngleStop.push_back(3.656465);
	mEncodersPerCycle.push_back(51200);
	mEncoderOffset.push_back(31000);
	mRotationDirection.push_back(1);

	mIsInitialized = true;

	return mIsInitialized;
}
//////////////////////////////////////////////////////////////////////////
void Kinematics6M90G::IK_theta234theta5(angles_calc& angle, const position &p_gr) const {
	angle.theta234 = -MHF::acotan( (  (p_gr.x * p_gr.z * cos(angle.theta1) ) -
	                             sqrt( ( -MHF::pow2(p_gr.z) ) *
	                                   ( -MHF::pow2(mSegmentLength[3]) + MHF::pow2(p_gr.x) + MHF::pow2(p_gr.z) ) * MHF::pow2(sin(angle.theta1))
	                                 )
	                          ) / MHF::pow2(p_gr.z)
	                        );

	angle.theta5   = acos( p_gr.z/(mSegmentLength[3]*sin(angle.theta234)) );

	if(p_gr.z==0) {
		angle.theta234=0;
		angle.theta5=angle.theta1-MHF::atan1(-p_gr.x,-p_gr.y);
	}

	bool griptest;
	griptest = GripperTest(p_gr, angle);
	if(!griptest) {
		angle.theta5=-angle.theta5;
		griptest=GripperTest(p_gr, angle);
		if(!griptest) {
			angle.theta234 = -MHF::acotan( (   ( p_gr.x * p_gr.z * cos(angle.theta1) ) +
			                              sqrt( ( -MHF::pow2(p_gr.z) ) *
			                                    ( -MHF::pow2(mSegmentLength[3]) + MHF::pow2(p_gr.x) + MHF::pow2(p_gr.z) ) * MHF::pow2(sin(angle.theta1))
			                                  )
			                          ) / MHF::pow2(p_gr.z)
			                        );
			angle.theta5 =  acos( p_gr.z / (mSegmentLength[3]*sin(angle.theta234)) );
			if(p_gr.z==0) {
				angle.theta234=-MHF_PI;
				angle.theta5=MHF::atan1(p_gr.x,p_gr.y) - angle.theta1;
			}

			griptest=GripperTest(p_gr, angle);
			if(!griptest) {
				angle.theta5=-angle.theta5;
			}
		}
	}

}
//////////////////////////////////////////////////////////////////////////
bool Kinematics6M90G::GripperTest(const position &p_gr, const angles_calc &angle) const {
	double xgr2, ygr2, zgr2;

	xgr2 = -mSegmentLength[3]*(cos(angle.theta1)*cos(angle.theta234)*cos(angle.theta5)+sin(angle.theta1)*sin(angle.theta5));
	ygr2 = -mSegmentLength[3]*(sin(angle.theta1)*cos(angle.theta234)*cos(angle.theta5)-cos(angle.theta1)*sin(angle.theta5));
	zgr2 =  mSegmentLength[3]*sin(angle.theta234)*cos(angle.theta5);

	if((MHF::pow2(p_gr.x-xgr2)+MHF::pow2(p_gr.y-ygr2)+MHF::pow2(p_gr.z-zgr2))>=cTolerance)
		return false;

	return true;
}
//////////////////////////////////////////////////////////////////////////
void Kinematics6M90G::IK_b1b2costh3_6MS(angles_calc &angle, const position &p) const {
	double xg, yg, zg;
	double d5 = mSegmentLength[2] + mSegmentLength[3];
	xg = p.x + ( mSegmentLength[3] * cos(angle.theta1) * sin(angle.theta234) );
	yg = p.y + ( mSegmentLength[3] * sin(angle.theta1) * sin(angle.theta234) );
	zg = p.z + ( mSegmentLength[3]                     * cos(angle.theta234) );


	angle.b1 = xg*cos(angle.theta1) + yg*sin(angle.theta1) - d5*sin(angle.theta234);
	angle.b2 = zg - d5*cos(angle.theta234);
	angle.costh3 = -( MHF::pow2(angle.b1) + MHF::pow2(angle.b2) - MHF::pow2(mSegmentLength[0]) - MHF::pow2(mSegmentLength[1]) ) / ( 2.0*mSegmentLength[0]*mSegmentLength[1] );

}
//////////////////////////////////////////////////////////////////////////
void Kinematics6M90G::thetacomp(angles_calc &angle, const position &p_m) const {
	angle.theta2 = -MHF_PI/2.0 - ( MHF::atan0(angle.b1,angle.b2)+MHF::atan0(mSegmentLength[0]+mSegmentLength[1]*cos(angle.theta3),mSegmentLength[1]*sin(angle.theta3)) );
	angle.theta4 = angle.theta234-angle.theta2-angle.theta3;

	if(!PositionTest6MS(angle,p_m)) {
		angle.theta2 = angle.theta2+MHF_PI;
		angle.theta4 = angle.theta234-angle.theta2-angle.theta3;
	}

}
//////////////////////////////////////////////////////////////////////////
bool Kinematics6M90G::PositionTest6MS(const angles_calc &a, const position &p) const {
	double temp, xm2, ym2, zm2;

	temp = mSegmentLength[0]*sin(a.theta2)+mSegmentLength[1]*sin(a.theta2+a.theta3)+mSegmentLength[2]*sin(a.theta234);
	xm2 = cos(a.theta1)*temp;
	ym2 = sin(a.theta1)*temp;
	zm2 = mSegmentLength[0]*cos(a.theta2)+mSegmentLength[1]*cos(a.theta2+a.theta3)+mSegmentLength[2]*cos(a.theta234);

	if((MHF::pow2(p.x-xm2)+MHF::pow2(p.y-ym2)+MHF::pow2(p.z-zm2))>=cTolerance)
		return false;

	return true;
}
//////////////////////////////////////////////////////////////////////////
bool Kinematics6M90G::angledef(angles_calc &a) const {
	// constants here. needs refactoring:
	a.theta2=MHF::anglereduce(a.theta2+MHF_PI/2.0);
	a.theta3=MHF::anglereduce(a.theta3+MHF_PI);
	a.theta4=MHF::anglereduce(MHF_PI-a.theta4);
	a.theta5=MHF::anglereduce(a.theta5);

	if(a.theta1>mAngleStop[0]) {
		a.theta1=a.theta1-2.0*MHF_PI;
	}
	if(a.theta2>MHF_PI) {
		a.theta2=a.theta2-2.0*MHF_PI;
	}
	if(a.theta5<mAngleOffset[4]) {
		a.theta5=a.theta5+2.0*MHF_PI;
	}

	return AnglePositionTest(a);

}
//////////////////////////////////////////////////////////////////////////
bool Kinematics6M90G::AnglePositionTest(const angles_calc &a) const {

	if( (a.theta1+0.0087<mAngleOffset[0])||(a.theta1>mAngleStop[0]) )
		return false;

	if( (a.theta2-0.0087>mAngleOffset[1])||(a.theta2<mAngleStop[1]) )
		return false;

	if( (a.theta3<mAngleOffset[2])||(a.theta3>mAngleStop[2]) )
		return false;

	if( (a.theta4<mAngleOffset[3])||(a.theta4>mAngleStop[3]) )
		return false;

	if( (a.theta5<mAngleOffset[4])||(a.theta5>mAngleStop[4]) )
		return false;

	return true;
}
//////////////////////////////////////////////////////////////////////////
} // namespace
