#include "ukf.h"
#include "Eigen/Dense"
#include <iostream>
#include <math.h>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/**
 * Initializes Unscented Kalman filter
 * This is scaffolding, do not modify
 */
UKF::UKF() {

  // State dimension
  n_x_ = 5;

  // Augmented state dimension
  n_aug_ = 7;

  // if this is false, laser measurements will be ignored (except during init)
  use_laser_ = true;

  // if this is false, radar measurements will be ignored (except during init)
  use_radar_ = true;

  // initial state vector
  x_ = VectorXd(n_x_);

  // initial covariance matrix
  P_ = MatrixXd(n_x_, n_x_);

  // Process noise standard deviation longitudinal acceleration in m/s^2
  std_a_ = 1.5;

  // Process noise standard deviation yaw acceleration in rad/s^2
  std_yawdd_ = 0.5;
  
  //DO NOT MODIFY measurement noise values below these are provided by the sensor manufacturer.
  // Laser measurement noise standard deviation position1 in m
  std_laspx_ = 0.15;

  // Laser measurement noise standard deviation position2 in m
  std_laspy_ = 0.15;

  // Radar measurement noise standard deviation radius in m
  std_radr_ = 0.3;

  // Radar measurement noise standard deviation angle in rad
  std_radphi_ = 0.03;

  // Radar measurement noise standard deviation radius change in m/s
  std_radrd_ = 0.3;
  //DO NOT MODIFY measurement noise values above these are provided by the sensor manufacturer.


  //create matrix with predicted sigma points as columns
  Xsig_pred_ = MatrixXd(n_x_, 2 * n_aug_ + 1);

  // Noise matrices
  R_radar = MatrixXd(3,3);
  R_laser = MatrixXd(2,2);

  //Init flag
  is_initialized_=false;

  //Augmented X
  Xsig_aug_ = MatrixXd( n_aug_, 2*n_aug_+1 );

  // Weigths
  weights_= VectorXd(2*n_aug_+1);

  // Lambda
  lambda = 3 - n_aug_;
}

UKF::~UKF() {}

/**
 * @param {MeasurementPackage} meas_package The latest measurement data of
 * either radar or laser.
 */
void UKF::ProcessMeasurement(MeasurementPackage measurement_pack) {
  /**
  TODO:

  Complete this function! Make sure you switch between lidar and radar
  measurements.
  */
  cout<<"ProcessMeasurement..."<<endl;
  /*****************************************************************************
   *  Initialization
   ****************************************************************************/
  if (!is_initialized_)
  {
    //------------------------------------------------
    // Generate weights:
    //------------------------------------------------
    double weight_0 = lambda/(lambda+n_aug_);
    weights_(0) = weight_0;
    for (int i=1; i<2*n_aug_+1; i++)
    {  //2n+1 weights
      double weight = 0.5/(n_aug_+lambda);
      weights_(i) = weight;
    }
    //------------------------------------------------
    previous_timestamp_ = measurement_pack.timestamp_;

    x_.fill(0.0);
    //------------------------------------------------
    //state covariance matrix P
    //------------------------------------------------
    P_ = MatrixXd(5, 5);
    P_ << std_radr_*std_radr_,    0, 0, 0, 0,
        0, std_radr_*std_radr_, 0, 0, 0,
        0,    0, 1, 0, 0,
        0,    0, 0, std_radphi_, 0,
        0,    0, 0, 0, std_radphi_;

    // Create R for update noise
    R_radar << std_radr_*std_radr_, 0, 0,
              0, std_radphi_*std_radphi_, 0,
              0, 0, std_radrd_*std_radrd_;

    // Create R for update noise later
    R_laser << std_laspx_*std_laspx_, 0,
              0, std_laspy_*std_laspy_;

    if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR)
    {
        /*Convert radar from polar to cartesian coordinates and initialize state.*/

        float rho = measurement_pack.raw_measurements_[0];
        float phi = measurement_pack.raw_measurements_[1];
        x_ << rho*cos(phi), rho*sin(phi), 0.0f, 0.0f, 0.0f;

	}
    else if (measurement_pack.sensor_type_ == MeasurementPackage::LASER)
    {
       x_ << measurement_pack.raw_measurements_[0], measurement_pack.raw_measurements_[1], 0., 0., 0.;
    }

    // done initializing, no need to predict or update
    is_initialized_ = true;
    cout<<"Initialized..."<<endl;
    return;
  }

  /*****************************************************************************
   *  Prediction
   ****************************************************************************/

  float dt = (measurement_pack.timestamp_ - previous_timestamp_) / 1000000.0;	//dt - expressed in seconds
  previous_timestamp_ = measurement_pack.timestamp_;


  /*****************************************************************************
   *  Update
   ****************************************************************************/

  Prediction(dt);

  if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR)
  {
    // Radar updates
	UpdateRadar(measurement_pack);
  }
  else
  {
    // Laser updates
    UpdateLidar(measurement_pack);
  }

}
//--------------------------------------------------------------------------------
/**
 * Predicts sigma points, the state, and the state covariance matrix.
 * @param {double} delta_t the change in time (in seconds) between the last
 * measurement and this one.
 */
void UKF::Prediction(double delta_t)
{
  // Augmented Sigma points
  AugmentedSigmaPoints();

  //Prediction
  SigmaPointPrediction(delta_t);

  //Estimate Mean and Covariance
  PredictMeanAndCovariance();

}
//--------------------------------------------------------------------------------

/**
 * Updates the state and the state covariance matrix using a laser measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateLidar(MeasurementPackage measurement_pack)
{
	UpdateLidarMeasurement(measurement_pack);
}
//--------------------------------------------------------------------------------

/**
 * Updates the state and the state covariance matrix using a radar measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateRadar(MeasurementPackage measurement_pack)
{
	UpdateRadarMeasurement(measurement_pack);
}
//--------------------------------------------------------------------------------
// Function not used. This belong to old part to generate the Sigma points
void UKF::GenerateSigmaPoints(MatrixXd* Xsig_out)
{
  //create sigma point matrix
  MatrixXd Xsig = MatrixXd(n_x_, 2 * n_x_ + 1);

  //calculate square root of P
  MatrixXd A = P_.llt().matrixL();

/*******************************************************************************
 * Student part begin
 ******************************************************************************/

  //your code goes here

  //calculate sigma points ...
  //set sigma points as columns of matrix Xsig
  //set first column of sigma point matrix
  Xsig.col(0)  = x_;

  //set remaining sigma points
  for (int i = 0; i < n_x_; i++)
  {
    Xsig.col(i+1)     = x_ + sqrt(lambda+n_x_) * A.col(i);
    Xsig.col(i+1+n_x_) = x_ - sqrt(lambda+n_x_) * A.col(i);
  }

/*******************************************************************************
 * Student part end
 ******************************************************************************/
  //write result
  *Xsig_out = Xsig;

}
//----------------------------------------------------------------------------
void UKF::AugmentedSigmaPoints()
{
    //create augmented mean vector
    VectorXd x_aug = VectorXd(n_aug_);
    x_aug.fill(0.0);

    //create augmented state covariance
    MatrixXd P_aug = MatrixXd(n_aug_, n_aug_);

    Xsig_aug_.fill(0.0);
   /*******************************************************************************
   * Student part begin
   ******************************************************************************/

    //create augmented mean state
    x_aug.head(5) = x_;
    x_aug(5) = 0;
    x_aug(6) = 0;

    //create augmented covariance matrix
    P_aug.fill(0.0);
    /*P_(0,0) = 1.0;
    P_(1,1) = 1.0;
    P_(2,2) = 1.0;
    P_(3,3) = 1.0;
    P_(4,4) = 1.0;*/
    P_aug.topLeftCorner(5,5) = P_;
    P_aug(5,5) = std_a_*std_a_;
    P_aug(6,6) = std_yawdd_*std_yawdd_;


    //create square root matrix
    MatrixXd L = P_aug.llt().matrixL();

    //create augmented sigma points
    Xsig_aug_.col(0)  = x_aug;
    for (int i = 0; i< n_aug_; i++)
    {
    	Xsig_aug_.col(i+1)       = x_aug + std::sqrt(lambda+n_aug_) * L.col(i);
    	Xsig_aug_.col(i+1+n_aug_) = x_aug - std::sqrt(lambda+n_aug_) * L.col(i);
    }

  }
//-------------------------------------------------------------------------------------
void UKF::SigmaPointPrediction(float delta_t)
{

/*******************************************************************************
 * Student part begin
 ******************************************************************************/

  //predict sigma points
  for (int i = 0; i< 2*n_aug_+1; i++)
  {
    //extract values for better readability
    double p_x = Xsig_aug_(0,i);
    double p_y = Xsig_aug_(1,i);
    double v = Xsig_aug_(2,i);
    double yaw = Xsig_aug_(3,i);
    double yawd = Xsig_aug_(4,i);
    double nu_a = Xsig_aug_(5,i);
    double nu_yawdd = Xsig_aug_(6,i);

    //predicted state values
    double px_p, py_p;

    //avoid division by zero
    if (fabs(yawd) > 0.001)
    {
        px_p = p_x + (v/yawd) * ( sin (yaw + yawd*delta_t) - sin(yaw));
        py_p = p_y + (v/yawd) * ( cos(yaw) - cos(yaw+yawd*delta_t) );
    }
    else
    {
        px_p = p_x + v*delta_t*cos(yaw);
        py_p = p_y + v*delta_t*sin(yaw);
    }

    double v_p = v;
    double yaw_p = yaw + yawd*delta_t;
    double yawd_p = yawd;

    //add noise
    px_p = px_p + 0.5*nu_a*delta_t*delta_t * cos(yaw);
    py_p = py_p + 0.5*nu_a*delta_t*delta_t * sin(yaw);
    v_p = v_p + nu_a*delta_t;

    yaw_p = yaw_p + 0.5*nu_yawdd*delta_t*delta_t;
    yawd_p = yawd_p + nu_yawdd*delta_t;

    //write predicted sigma point into right column
    Xsig_pred_(0,i) = px_p;
    Xsig_pred_(1,i) = py_p;
    Xsig_pred_(2,i) = v_p;
    Xsig_pred_(3,i) = yaw_p;
    Xsig_pred_(4,i) = yawd_p;
  }

/*******************************************************************************
 * Student part end
 ******************************************************************************/

}
//-------------------------------------------------------------------------------------
void UKF::PredictMeanAndCovariance()
{

  //predicted state mean
  x_.fill(0.0);
  for (int i = 0; i < 2 * n_aug_ + 1; i++)
  {  //iterate over sigma points
    x_ = x_ + weights_(i) * Xsig_pred_.col(i);
  }
  //predict state covariance matrix

  P_.fill(0.0);
  for (int i = 0; i < 2 * n_aug_ + 1; i++)
  {  //iterate over sigma points

    // state difference
    VectorXd x_diff = Xsig_pred_.col(i) - x_;
    //angle normalization
    if (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
    if (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;

    P_ = P_ + weights_(i) * x_diff * x_diff.transpose() ;
  }

/*******************************************************************************
 * Student part end
 ******************************************************************************/
}
//-------------------------------------------------------------------------------------
void UKF::UpdateLidarMeasurement(MeasurementPackage measurement_pack)
{
	  //Px, Py
	  int n_z = 2;

	  MatrixXd Zsig = MatrixXd(n_z, 2 * n_aug_ + 1);
	  Zsig.fill(0.0);
	  // Transform sigma points into measurement space
	  for( int pt = 0; pt < 2*n_aug_ + 1; pt++ )
	  {
		  Zsig(0,pt) = Xsig_pred_(0,pt);
		  Zsig(1,pt) = Xsig_pred_(1,pt);
	  }

	  //mean predicted measurement
	  VectorXd z_pred = VectorXd(n_z);
	  z_pred.fill(0.0);

	  for (int i=0; i < 2*n_aug_+1; i++)
	  {
	      z_pred = z_pred + weights_(i) * Zsig.col(i);
	  }

	  //innovation covariance matrix S
	  MatrixXd S = MatrixXd(n_z,n_z);
	  S.fill(0.0);
	  for (int i = 0; i < 2 * n_aug_ + 1; i++)
	  {
		  //residual
		  VectorXd z_diff = Zsig.col(i) - z_pred;
		  S = S + weights_(i) * z_diff * z_diff.transpose();
	  }

	  S = S + R_laser;

	  /*******************************************************************************
	  * UKF Update
	  ******************************************************************************/
	  //create matrix for cross correlation Tc
	  MatrixXd Tc = MatrixXd(n_x_, n_z);
	  //calculate cross correlation matrix
	  Tc.fill(0.0);

	  for (int i = 0; i < 2 * n_aug_ + 1; i++)
	  {  //2n+1 simga points

	    //residual
	    VectorXd z_diff = Zsig.col(i) - z_pred;

	    // state difference
	    VectorXd x_diff = Xsig_pred_.col(i) - x_;

	    Tc = Tc + weights_(i) * x_diff * z_diff.transpose();
	   }

	   // Kalman gain K;
	   MatrixXd K = Tc * S.inverse();

	   VectorXd z = measurement_pack.raw_measurements_;

	   // Residual
	   VectorXd z_diff = z - z_pred;

	   //update state mean and covariance matrix
	   x_ = x_ + K * z_diff;
	   P_ = P_- K*S*K.transpose();
}
//-------------------------------------------------------------------------------------
void UKF::UpdateRadarMeasurement(MeasurementPackage measurement_pack)
{

  //set measurement dimension, radar can measure r, phi, and r_dot
  int n_z = 3;

  //create matrix for sigma points in measurement space
  MatrixXd Zsig = MatrixXd(n_z, 2 * n_aug_ + 1);
  Zsig.fill(0.0);
/*******************************************************************************
 * Student part begin
 ******************************************************************************/

  //transform sigma points into measurement space
  for (int i = 0; i < 2 * n_aug_ + 1; i++)
  {


    // extract values for better readibility
    double p_x = Xsig_pred_(0,i);
    double p_y = Xsig_pred_(1,i);
    double v   = Xsig_pred_(2,i);
    double yaw = Xsig_pred_(3,i);

    double v1 = cos(yaw)*v;
    double v2 = sin(yaw)*v;

    // measurement model
    Zsig(0,i) = sqrt(p_x*p_x + p_y*p_y);                        //r
    Zsig(1,i) = atan2(p_y,p_x);                                 //phi
    Zsig(2,i) = (p_x*v1 + p_y*v2 ) / sqrt(p_x*p_x + p_y*p_y);   //r_dot

  }

  //mean predicted measurement
  VectorXd z_pred = VectorXd(n_z);
  z_pred.fill(0.0);
  for (int i=0; i < 2*n_aug_+1; i++)
  {
      z_pred = z_pred + weights_(i) * Zsig.col(i);
  }

  //innovation covariance matrix S
  MatrixXd S = MatrixXd(n_z,n_z);
  S.fill(0.0);
  for (int i = 0; i < 2 * n_aug_ + 1; i++)
  {
    //residual
    VectorXd z_diff = Zsig.col(i) - z_pred;

    //angle normalization
    if (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
    if (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;

    S = S + weights_(i) * z_diff * z_diff.transpose();
  }

  S = S + R_radar;
 /*******************************************************************************
 * UKF Update
 ******************************************************************************/
  //create matrix for cross correlation Tc
  MatrixXd Tc = MatrixXd(n_x_, n_z);
  //calculate cross correlation matrix
  Tc.fill(0.0);

  for (int i = 0; i < 2 * n_aug_ + 1; i++)
  {  //2n+1 simga points

    //residual
    VectorXd z_diff = Zsig.col(i) - z_pred;
    //angle normalization
    if (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
    if (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;

    // state difference
    VectorXd x_diff = Xsig_pred_.col(i) - x_;
    //angle normalization
    if (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
    if (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;

    Tc = Tc + weights_(i) * x_diff * z_diff.transpose();
  }

  // Kalman gain K;
  MatrixXd K = Tc * S.inverse();

  VectorXd z = measurement_pack.raw_measurements_;

  // Residual
  VectorXd z_diff = z - z_pred;

  //angle normalization
  if (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
  if (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;

  //update state mean and covariance matrix
  x_ = x_ + K * z_diff;
  P_ = P_- K*S*K.transpose();

}
