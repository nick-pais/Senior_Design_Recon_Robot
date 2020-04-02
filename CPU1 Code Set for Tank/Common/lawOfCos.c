#include <stdio.h>
#include <math.h>
#include <F28x_Project.h>

#define PI 3.1415926535897932384626433832795
#define DEG2RAD(x) ((PI / 180.0) * x)
#define RAD2DEG(x) ((180.0 / PI) * x)
#define PI_2 PI/2
#define PI_3 PI/3
#define PI_5 PI/5

//Globals
float F[2];
float Jac[2][2];
float Inverse[2][2];
float Result[2];
int angle;

void getFunction(float r1, float theta, float r2, float r3, float r4, float r5, float phi){
	F[0] = pow(r1+r3,2) + pow(r4, 2) - 2*(r1+r3)*r4*cos(theta) - pow(r1+r2,2);
	F[1] = pow(r1+r3,2) + pow(r5, 2) - 2*(r1+r3)*r5*cos(theta-phi) - pow(r1,2);
}

void getJacobian(float r1, float theta, float r2, float r3, float r4, float r5, float phi){
	//Calculate derivative 
	Jac[0][0] = -2*(r4*cos(theta) - r3 + r4);
	Jac[0][1] = 2*r4*(r1+r3)*sin(theta);
	Jac[1][0] = -2*(r5*cos(theta-phi) - r3);
	Jac[1][1] = 2*r5*(r1+r3)*sin(theta-phi);
}

void getInverse2x2(){
	//Get a,b,c,d for ease of use
	float a = Jac[0][0];
	float b = Jac[0][1];
	float c = Jac[1][0];
	float d = Jac[1][1];

	//Calculate determinant
	float det = 1/(a*d - b*c);

	//Invert it
	Inverse[0][0] = d*det;
	Inverse[0][1] = -1*b*det;
	Inverse[1][0] = -1*c*det;
	Inverse[1][1] = a*det;
}

void mult2x2Matrix(){
	//Compute Inverse[2][2]*F[2]
	Result[0] = Inverse[0][0]*F[0] + Inverse[0][1]*F[1];
	Result[1] = Inverse[1][0]*F[0] + Inverse[1][1]*F[1];
}

float calculateError(float xOld, float yOld, float x, float y){
	float temp = pow(x-xOld, 2);
	temp += pow(y-yOld, 2);

	return temp;
}

int soundRange(float r2, float r3, float r4, float r5, float phi, float *ans){

	float r1 = 1.0f;
	float theta = 1.0f; 

	float r1Old;
	float thetaOld;

	float err = 1.0;
	Uint16 count = 0;

	while(err > 0.000001){
		//Calculate function into F
		//F =
		getFunction(r1, theta, r2, r3, r4, r5, phi);
		//printf("F[0]: %f, F[1]: %f\n", F[0], F[1]);

		//Calculate derivatives into Jac
		//Jac =
		getJacobian(r1, theta, r2, r3, r4, r5, phi);
		//printf("Jac[0][0]: %f, Jac[0][1]: %f, Jac[1][0]: %f, Jac[1][1]: %f\n", Jac[0][0], Jac[0][1], Jac[1][0], Jac[1][1]);

		//printf("[%f, %f]\n", F[0], F[1]);
		//printf("[%f, %f, %f, %f]\n", Jac[0][0], Jac[0][1], Jac[1][0], Jac[1][1]);

		//Place inverse of Jac into Inverse
		//Inverse =
		getInverse2x2();
		//printf("Inverse[0][0]: %f, Jac[0][1]: %f, Jac[1][0]: %f, Jac[1][1]: %f\n", Jac[0][0], Jac[0][1], Jac[1][0], Jac[1][1]);

		//Multiply Inverse and F into Result
		//Result = 
		mult2x2Matrix();
		//printf("F[0]: %f, F[1]: %f\n", F[0], F[1]);

		r1 = r1 - Result[0];
		theta = theta - Result[1];

		//Calculate error between new and old values
		err = calculateError(r1Old, thetaOld, r1, theta);

		//printf("xOld: %f, yOld: %f, x: %f, y: %f\n",xOld, yOld, x, y);
		//printf("err: %f\n", err);

		//Update values
		r1Old = r1;
		thetaOld = theta;
		count++;

		if (err > 100000 || count > 100){
		    return 0;
		}
	}
	//Note: this returns the angle from the perspective of the Mic which received the sound first
	//printf("theta: %f, r1: %f\n",theta,r1);
	ans[0] = theta;
	ans[1] = fabs(r1);
	return 1;
}

int calcSoundRange(float t1, float t2, float t3, float* ans){
	float r2 = (t2 - t1)*33000;
	float r3 = (t3 - t1)*33000;
	float r4 = 24;
	float r5 = 24;
	float phi = DEG2RAD(60);

	return soundRange(r2, r3, r4, r5, phi, ans);
}

int16 calibrateResult(float theta, float r, volatile Uint16 * order){
    float y;
    float x;
    float curr_x;
    float curr_y;
    float deg;
    float new_theta;

    //Case 1 [1 2 3]
    if (order[0] == 1 && order[1] == 2 && order[2] == 3){
        //angle = 90 - theta;
        //MIC_3 coords
        curr_x = 12.0f;
        curr_y = -10.4f;

        //Distance
        x = (24.0f + r)*cos(theta);
        y = (24.0f + r)*sin(theta);

        curr_x -= x;
        curr_y += y;

        //deg = atan2(curr_y,curr_x);
        //deg = RAD2DEG(deg);
        //deg -= 90;
    }
    //Case 2 [1 3 2]
    else if (order[0] == 1 && order[1] == 3 && order[2] == 2){
        //Mic_2 coords
        curr_x = -12.0f;
        curr_y = -10.4f;

        x = (24.0f + r)*cos(theta);
        y = (24.0f + r)*sin(theta);

        curr_x += x;
        curr_y += y;


        //deg = atan2(curr_y, curr_x);
        //deg = RAD2DEG(deg);
        //deg = -(90 - deg);

    }
    //Case 3 [2 1 3]
    else if (order[0] == 2 && order[1] == 1 && order[2] == 3){
        //Mic 3 coord
        curr_x = 12.0f;
        curr_y = -10.4f;

        new_theta = fabs(PI_3 - theta);

        x = (24.0f + r)*cos(new_theta);
        y = (24.0f + r)*sin(new_theta);

        curr_x -= x;
        if (theta > PI_3){
            curr_y -= y;
        }
        else{
            curr_y += y;
        }
        //deg = atan2(curr_y, curr_x);
        //deg = RAD2DEG(deg);
       // deg -= 90;

    }
    //Case 4 [2 3 1]
    else if (order[0] == 2 && order[1] == 3 && order[2] == 1){
        //Mic 1
        curr_x = 0.0f;
        curr_y = 10.4f;

        x = (24.0f + r)*cos(theta);
        y = (24.0f + r)*sin(theta);

        //curr_y -= x*sin(PI_3);
        //curr_y -= y*sin(PI - theta - PI_2);
        curr_y -= x*sin(PI_3);
        curr_y -= y*sin(PI_5);

        //curr_x = y*cos(PI - theta - 90);
        //curr_x += x*cos(PI_3);
        if (theta > PI_5){
            curr_x = y*cos(PI_5);
            curr_x -= x*cos(PI_3);
            curr_x = -curr_x;
            //deg = atan2(curr_y, curr_x);
            //deg = RAD2DEG(deg);
            //deg += 270;
        }
        else{
            curr_x = x*cos(PI_3);
            curr_x -= y*cos(PI_5);
            curr_x = -curr_x;
            //deg = atan2(curr_y, curr_x);
            //deg = RAD2DEG(deg);
            //deg -= 90;
        }

    }
    //Case 5 [3 1 2]
    else if (order[0] == 3 && order[1] == 1 && order[2] == 2){
        //Mic_2 coords
        curr_x = -12.0f;
        curr_y = -10.4f;

        new_theta = fabs(PI_3 - theta);

        x = (24.0f + r)*cos(new_theta);
        y = (24.0f + r)*sin(new_theta);

        curr_x += x;

        if (theta > PI_3){
            curr_y -= y;
        }
        else{
            curr_y += y;
        }

        //deg = atan2(curr_y, curr_x);
        //deg = RAD2DEG(deg);
        //deg = -(90 - deg);

    }
    //Case 6 [3 2 1]
    else {
        //Mic 1
        curr_x = 0.0f;
        curr_y = 10.4f;

        x = (24.0f + r)*cos(theta);
        y = (24.0f + r)*sin(theta);

        //curr_y -= x*sin(PI_3);
        //curr_y -= y*sin(PI - theta - PI_2);
        curr_y -= x*sin(PI_3);
        curr_y -= y*sin(PI_5);

        //curr_x = y*cos(PI - theta - 90);
        //curr_x += x*cos(PI_3);
        if (theta > PI_5){
            curr_x = y*cos(PI_5);
            curr_x -= x*cos(PI_3);
            //deg = atan2(curr_y, curr_x);
            //deg = RAD2DEG(deg);
            //deg -= 90;
        }
        else{
            curr_x = x*cos(PI_3);
            curr_x -= y*cos(PI_5);
            //deg = atan2(curr_y, curr_x);
            //deg = RAD2DEG(deg);
            //deg += 270;
        }

    }
    //Shift degrees by 90
    //Formula changes based on lower or upper quad
    //Quad 1
    deg = atan2(curr_y, curr_x);
    deg = RAD2DEG(deg);

    if (curr_y >= 0 && curr_x >= 0){
        deg = -(90 - deg);
    }
    else if (curr_y >= 0 && curr_x <= 0){
        deg = deg - 90;
    }
    else if (curr_y <= 0 && curr_x <= 0){
        deg += 270;
    }
    else{
        deg -= 90;
    }

    return deg;
}

