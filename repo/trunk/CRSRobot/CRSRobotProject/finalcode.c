/* ME446: Robot Dynamics and Control
 * Final Project: Task Space PD Control, Impedance Control
 * By: Ayush Sinha & Michael Alvarez
 * Date: 11 May 2018*/
#include <tistdtypes.h>
#include <coecsl.h>
#include "user_includes.h"
#include "math.h"

// Offsets to make motor angles zero at home position
float offset_Enc2_rad = -0.44;
float offset_Enc3_rad = 0.23;

// Your global variables
long mycount = 0; // counts milliseconds passed (increments on each call of lab())
long timecheck = 0; // in ms for cubic traj

#pragma DATA_SECTION(whattoprint, ".my_vars")
float whattoprint = 0.0;
#pragma DATA_SECTION(testmatlab, ".my_vars") // testmatlab is accessible to MATLAB
float testmatlab = 0;                        // initializing testmatlab to 0

#pragma DATA_SECTION(theta1array, ".my_arrs")
float theta1array[100];
#pragma DATA_SECTION(theta2array, ".my_arrs") // theta2array is accessible to MATLAB
float theta2array[100]; // theta2array made with 100 elements

long arrayindex = 0; // used for storing past theta1 an theta2 values in theta1array and theta2array

// vars used for printing motor angles to Serial
float printtheta1motor = 0;
float printtheta2motor = 0;
float printtheta3motor = 0;

// Trajectory vars
int m = 0; // keeping track of traj points reached
// Points defining trajectory
float x_w[20] = {5.44,5.44,0.78,0.78,0.78,12.33,14.78,15.84,15.83,15.33,12.98,12.41,12.44,14.78,14.55,14.55,14.55,14.55,14.55,5.44};
float y_w[20] = {0,5.45,13.48,13.8,13.8,4.53,4.46,2.97,2.37,2.06,2.46,2.13,1.58,-0.90,-4.99,-4.99,-4.99,-4.99,-4.99,0};
float z_w[20] = {16.78,16.78,8.25,5.25,8.25,9.41,8.21,8.21,8.21,8.21,8.21,8.21,8.21,8.21,17.11,14.11,13.3,13.3,14.11,16.78};
// time intervals between 2 points on the traj
float time_int[19] = {1,1,2,2,2,1,1,0.5,0.5,1,0.5,0.5,1,2,0.5,1,1,1,1};
// Impedance control indices for traj b/w 2 points ; // 0 = all dir stiff,1 = z only stiff,2 = x & z stiff
// Last element = 0 holds final position at home
float stiff_type[20] = {0,0,1,1,0,0,2,2,2,2,2,2,2,0,0,0,0,0,0,0};

// vars for current and older angular vel
float theta1dot = 0; // current values
float theta2dot = 0;
float theta3dot = 0;
float theta1dotOld1 = 0; // values 1ms before
float theta2dotOld1 = 0;
float theta3dotOld1 = 0;
float theta1dotOld2 = 0; // values 2ms before
float theta2dotOld2 = 0;
float theta3dotOld2 = 0;

// old theta values
float theta1Old = 0;
float theta2Old = 0;
float theta3Old = 0;

// temptaux are temporary vars for calculated torques
// temptaux are control input torques before adding friction compensation
float temptau1 = 0;
float temptau2 = 0;
float temptau3 = 0;
// Fx,Fy,Fz are forces at end-effector in x,y,z direction of world frame
float Fx = 0;
float Fy = 0;
float Fz = 0;

// PD Gains in x-y-z directions
float Kpx = 0; // 0.075;
float Kpy = 0; //0.075;
float Kpz = 0; //0.35;
float Kdx = 0; // 0.002;
float Kdy = 0; //0.002;
float Kdz = 0; //0.025;

// friction compensation
// vposx is viscous fric coeff for joint x for positive velocity
// cposx is coulombic fric coeff for jt x for positive velocity
// vnegx is viscous fric coeff for joint x for negative velocity
// cnegx is coulombic fric coeff for jt x for negative velocity
float vpos1 = 0.2513;
float vpos2 = 0.25;
float vpos3 = 0.1922;
float cpos1 = 0.3637;
float cpos2 = 0.4759;
float cpos3 = 0.5339;
float vneg1 = 0.2477;
float vneg2 = 0.287;
float vneg3 = 0.2132;
float cneg1 = -0.2948;
float cneg2 = -0.503;
float cneg3 = -0.5190;
// friction compensation to be added to control input torques
float u_fric1 = 0;
float u_fric2 = 0;
float u_fric3 = 0;

// end-effector position in world frame coordinates in inches
float x = 0;
float y = 0;
float z = 0;
// end-effector velocities in world frame coordinates
float xdot = 0; // current values
float ydot = 0;
float zdot = 0;
float xdotOld1 = 0; // values 1ms before
float ydotOld1 = 0;
float zdotOld1 = 0;
float xdotOld2 = 0; // values 2ms before
float ydotOld2 = 0;
float zdotOld2 = 0;
// old x,y,z values (world frame coordinates)
float xOld = 0;
float yOld = 0;
float zOld = 0;

// xdes,ydes,zdes are desired e.e position every 1 ms to follow desired trajectory
// xdotdes,.. are e.e velocities to follow desired trajectory (world frame)
float xdes = 0;
float ydes = 0;
float zdes = 0;
float xdotdes = 0;
float ydotdes = 0;
float zdotdes = 0;
// xdel,ydel,zdel are changes in x,y,z directions respectively
float xdel = 0;
float ydel = 0;
float zdel = 0;

// Assign these float to the values you would like to plot in Simulink
float Simulink_PlotVar1 = 0;
float Simulink_PlotVar2 = 0;
float Simulink_PlotVar3 = 0;
float Simulink_PlotVar4 = 0;

//sin and cos of motor angles stored every ms to avoid their repetitive computation
float cosq1 = 0;
float sinq1 = 0;
float cosq2 = 0;
float sinq2 = 0;
float cosq3 = 0;
float sinq3 = 0;
// Jacobian transpose elements
float JT_11 = 0;
float JT_12 = 0;
float JT_13 = 0;
float JT_21 = 0;
float JT_22 = 0;
float JT_23 = 0;
float JT_31 = 0;
float JT_32 = 0;
float JT_33 = 0;
// thetax,thetay,thetaz are euler angles to get to a frame in which our desired direction
// is along one of the coordinate axes of the new frame
float thetaz = 0;
float thetax = 0;
float thetay = 0;
//sin and cos of thetax,thetay,thetaz stored every ms to avoid their repetitive computation
float cosz = 0;
float sinz = 0;
float cosx = 0;
float sinx = 0;
float cosy = 0;
float siny = 0;
// Rotation matrix elements
float R11 = 0;
float R12 = 0;
float R13 = 0;
float R21 = 0;
float R22 = 0;
float R23 = 0;
float R31 = 0;
float R32 = 0;
float R33 = 0;
// Rotation transpose elements
float RT11 = 0;
float RT12 = 0;
float RT13 = 0;
float RT21 = 0;
float RT22 = 0;
float RT23 = 0;
float RT31 = 0;
float RT32 = 0;
float RT33 = 0;
// error in e.e position in world frame
float errx = 0;
float erry = 0;
float errz = 0;
// error in e.e position in N space (rotated frame)
float errNx = 0;
float errNy = 0;
float errNz = 0;
// World frame error dots
float errxdot = 0;
float errydot = 0;
float errzdot = 0;
// N space errors dots
float errNxdot = 0;
float errNydot = 0;
float errNzdot = 0;
// forces at e.e in N space (rotated frame) coordinates
float FNx = 0;
float FNy = 0;
float FNz = 0;

// This function is called every 1 ms
void lab(float theta1motor,float theta2motor,float theta3motor,float *tau1,float *tau2,float *tau3, int error) {
    // Set PD gains for trajectory indices
    if (stiff_type[m] == 0){ // all stiff
        Kpx = 0.6;
        Kpy = 0.6;
        Kpz = 0.6;
        Kdx = 0.025;
        Kdy = 0.025;
        Kdz = 0.025;
    }
    else if (stiff_type[m] == 1){ // z stiff
        Kpx = 0.055;
        Kpy = 0.055;
        Kpz = 0.40;
        Kdx = 0.002;
        Kdy = 0.002;
        Kdz = 0.025;
    }
    else{ // x z stiff
        Kpx = 0.5;
        Kpy = 0.055;
        Kpz = 0.5;
        Kdx = 0.025;
        Kdy = 0.002;
        Kdz = 0.025;
    }

    //thetadots
    /*First, angular velocities are calculated numerically for each ms
     * Then, the velocities are filtered through a moving average filter
     * where, thetaxdotOld1 & thetaxdotOld2 are velocities 1ms & 2ms before respectively*/

    theta1dot = (theta1motor - theta1Old)/0.001;
    theta1dot = (theta1dot + theta1dotOld1 + theta1dotOld2)/3.0;

    theta2dot = (theta2motor - theta2Old)/0.001;
    theta2dot = (theta2dot + theta2dotOld1 + theta2dotOld2)/3.0;

    theta3dot = (theta3motor - theta3Old)/0.001;
    theta3dot = (theta3dot + theta3dotOld1 + theta3dotOld2)/3.0;

    // Friction compensation calculation
    /*Friction compensation u_fricx are calculated for each joint using experimentally determined
     *viscous and coulombic friction coefficients in both positive and negative motor directions.
     *They're later added to control torque inputs before sending commands to robot*/
    if(theta1dot >= 0){
        u_fric1 = vpos1*theta1dot + cpos1;
    }
    else {
        u_fric1 = vneg1*theta1dot + cneg1;
    }


    if(theta2dot >= 0){
        u_fric2 = vpos2*theta2dot + cpos2;
    }
    else {
        u_fric2 = vneg2*theta2dot + cneg2;
    }

    if(theta3dot >= 0){
        u_fric3 = vpos3*theta3dot + cpos3;
    }
    else {
        u_fric3 = vneg3*theta3dot + cneg3;
    }

    //Setting trajectory based on end points and time interval chosen for particular index m
    if (m <19){ // 19 paths between 20 points
        if (timecheck < time_int[m]*1000){ // follow traj until time interval has passed
            xdel = x_w[m+1] - x_w[m];
            ydel = y_w[m+1] - y_w[m];
            zdel = z_w[m+1] - z_w[m];
            thetaz = -atan2(ydel, xdel);
            xdes = x_w[m] + xdel*(((float)timecheck*0.001)/time_int[m]);
            ydes = y_w[m] + ydel*(((float)timecheck*0.001)/time_int[m]);
            zdes = z_w[m] + zdel*(((float)timecheck*0.001)/time_int[m]);

            xdotdes = xdel/(time_int[m]);
            ydotdes = ydel/(time_int[m]);
            zdotdes = zdel/(time_int[m]);
        }
        else { // change to next path
            xdes = x_w[m+1];
            ydes = y_w[m+1];
            zdes = z_w[m+1];

            xdotdes = 0;
            ydotdes = 0;
            zdotdes = 0;
            m++;
            timecheck = 0; // reset time keeper to zero for next path
        }
    }
    else { // last 'path' is holding position 20
        xdes = x_w[m];
        ydes = y_w[m];
        zdes = z_w[m];
        xdotdes = 0;
        ydotdes = 0;
        zdotdes = 0;
    }

    // Sines and Cosines of Motor angles for repetitive use
    cosq1 = cos(theta1motor);
    sinq1 = sin(theta1motor);
    cosq2 = cos(theta2motor);
    sinq2 = sin(theta2motor);
    cosq3 = cos(theta3motor);
    sinq3 = sin(theta3motor);
    // Sines and Cosines of frame rotation for repetitive use
    cosz = cos(thetaz);
    sinz = sin(thetaz);
    cosx = cos(thetax);
    sinx = sin(thetax);
    cosy = cos(thetay);
    siny = sin(thetay);

    // Rotation matrix elements
    RT11 = R11 = cosz*cosy-sinz*sinx*siny;
    RT21 = R12 = -sinz*cosx;
    RT31 = R13 = cosz*siny+sinz*sinx*cosy;
    RT12 = R21 = sinz*cosy+cosz*sinx*siny;
    RT22 = R22 = cosz*cosx;
    RT32 = R23 = sinz*siny-cosz*sinx*cosy;
    RT13 = R31 = -cosx*siny;
    RT23 = R32 = sinx;
    RT33 = R33 = cosx*cosy;
    // Jacobian Transpose elements
    JT_11 = -10*sinq1*(cosq3 + sinq2);
    JT_12 = 10*cosq1*(cosq3 + sinq2);
    JT_13 = 0;
    JT_21 = 10*cosq1*(cosq2 - sinq3);
    JT_22 = 10*sinq1*(cosq2 - sinq3);
    JT_23 = -10*(cosq3 + sinq2);
    JT_31 = -10*cosq1*sinq3;
    JT_32 = -10*sinq1*sinq3;
    JT_33 = -10*cosq3;

    // forward kinematics to get e.e position
    x = cosq1*(cosq3 + sinq2)*10;//*10 for feet to inches
    y = sinq1*(cosq3 + sinq2)*10;
    z = (1 + cosq2 - sinq3)*10;

    // e.e velocities in task space (average filtered)
    xdot = (x - xOld)/0.001;
    xdot = (xdot + xdotOld1 + xdotOld2)/3.0;
    ydot = (y - yOld)/0.001;
    ydot = (ydot + ydotOld1 + ydotOld2)/3.0;
    zdot = (z - zOld)/0.001;
    zdot = (zdot + zdotOld1 + zdotOld2)/3.0;

    // Storing old values of e.e velocities
    xdotOld2 = xdotOld1;
    xdotOld1 = xdot;
    ydotOld2 = ydotOld1;
    ydotOld1 = ydot;
    zdotOld2 = zdotOld1;
    zdotOld1 = zdot;
    // old position of e.e
    xOld = x;
    yOld = y;
    zOld = z;

    /* errors in e.e position are calculated in world frame and stored in errx,erry,errz.
     * Then, the errors are transformed to rotated frame (or N space) using the relation,
     * (error in N space) = R matrix*(error in world frame). They're stored in errNx,errNy,errNz.
     * Time derivatives of errors are handled similarly*/
    // World frame errors
    errx = xdes - x;
    erry = ydes - y;
    errz = zdes - z;

    // N space errors
    errNx = R11*errx + R12*erry + R13*errz;
    errNy = R21*errx + R22*erry + R23*errz;
    errNz = R31*errx + R32*erry + R33*errz;

    // World frame error dots
    errxdot = xdotdes - xdot;
    errydot = ydotdes - ydot;
    errzdot = zdotdes - zdot;

    // N space errors
    errNxdot = R11*errxdot + R12*errydot + R13*errzdot;
    errNydot = R21*errxdot + R22*errydot + R23*errzdot;
    errNzdot = R31*errxdot + R32*errydot + R33*errzdot;
    /* Now, control input forces at e.e are computed in N space using a PD controller.
     * Since, the PD controller is using errors in N space and hence providing forces in Nspace,
     * changing PD gains will affect forces FNx,FNy,FNz. Since we're trying to get impedance control along our chosen
     * trajectory which lies only in x-y plane, our frame rotation is only about the z-axis. Effectively,
     * FNx is force along direction of motion from start (xa,ya,za) to end (xb,yb,zb), FNy is force perpendicular
     * to direction of motion in xy plane and FNz is along the z-axis. Hence, lowering PD gains for FNy (Kpy,Kdy) will allow slight movement in
     * direction perpendicular to desired direction of motion.*/
    // N space forces
    FNx = Kpx*errNx + Kdx*errNxdot;
    FNy = Kpy*errNy + Kdy*errNydot;
    FNz = Kpz*errNz + Kdz*errNzdot;

    /* FNx,FNy,FNz are converted into world frame using transpose of Rotation matrix used earlier.
     * This transformation is required as relation between world frame e.e forces and joint torques is available*/
    // World forces
    Fx = RT11*FNx + RT12*FNy + RT13*FNz;
    Fy = RT21*FNx + RT22*FNy + RT23*FNz;
    Fz = RT31*FNx + RT32*FNy + RT33*FNz;

    /* e.e forces are converted to joint torques using Jacobian Transpose
     * u = JT*F */
    // control torque inputs
    temptau1 = JT_11*Fx + JT_12*Fy + JT_13*Fz;
    temptau2 = JT_21*Fx + JT_22*Fy + JT_23*Fz;
    temptau3 = JT_31*Fx + JT_32*Fy + JT_33*Fz;

    // adding friction compensation
    // only 35% of friction compenstion used
    *tau1 = 0.35*u_fric1 + temptau1;
    *tau2 = 0.35*u_fric2 + temptau2;
    *tau3 = 0.35*u_fric3 + temptau3;

    // torque saturation to avoid very high values
    if(*tau1>5)
        *tau1 = 5;
    if(*tau1<-5)
        *tau1 = -5;
    if(*tau2>5)
        *tau2 = 5;
    if(*tau2<-5)
        *tau2 = -5;
    if(*tau3>5)
        *tau3 = 5;
    if(*tau3<-5)
        *tau3 = -5;

    //updating thetadot olds
    theta1dotOld2 = theta1dotOld1;
    theta1dotOld1 = theta1dot;

    theta2dotOld2 = theta2dotOld1;
    theta2dotOld1 = theta2dot;

    theta3dotOld2 = theta3dotOld1;
    theta3dotOld1 = theta3dot;

    //theta olds
    theta1Old = theta1motor;
    theta2Old = theta2motor;
    theta3Old = theta3motor;

    // save past states
    if ((mycount%50)==0) {
        // executed every 50 ms
        theta1array[arrayindex] = theta1motor; // save motor1 angles
        theta2array[arrayindex] = theta2motor; // save motor2 angles

        if (arrayindex >= 100) { // when array is full
            arrayindex = 0; // move to first array element
        } else {
            arrayindex++; // move to next array element
        }

    }

    if ((mycount%500)==0) { // executed every 500 ms

        // testmatlab & whattoprint can be changed from MATLAB
        if (testmatlab > 0){ // print when variable value suitable changed from MATLAB
            serial_printf(&SerialA, "We changed testmatlab");
        }
        if (whattoprint > 0.5) { // print when variable value suitably changed from MATLAB
            serial_printf(&SerialA, "I love robotics\n\r");
        } else { // print motor angles
            printtheta1motor = theta1motor;
            printtheta2motor = theta2motor;
            printtheta3motor = theta3motor;

            SWI_post(&SWI_printf); //Using a SWI to fix SPI issue from sending too many floats.
        }
        GpioDataRegs.GPBTOGGLE.bit.GPIO34 = 1; // Blink LED on Control Card
        //GpioDataRegs.GPBTOGGLE.bit.GPIO60 = 1;
    }

    // Motor velocities to Simulink
    Simulink_PlotVar1 = theta1motor;
    Simulink_PlotVar2 = theta2motor;
    Simulink_PlotVar3 = theta3motor;
    Simulink_PlotVar4 = 0;

    // Increment counters to record t in ms
    mycount++;
    timecheck++;
}

void printing(void){
    // print actual motor angles
    serial_printf(&SerialA, "Theta1 = %.2f, Theta2 = %.2f, Theta3 = %.2f   \n\r",printtheta1motor*180/PI,printtheta2motor*180/PI,printtheta3motor*180/PI);
    // print end effector position calculated using forward kinematics
    serial_printf(&SerialA, "x = %.2f, y = %.2f, z = %.2f   \n\r",x,y,z);
}
