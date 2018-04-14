#include <iostream>
#include "tools.h"

using Eigen::VectorXd;
using Eigen::MatrixXd;
using std::vector;

Tools::Tools() {}

Tools::~Tools() {}

VectorXd Tools::CalculateRMSE(const vector<VectorXd> &estimations,
                              const vector<VectorXd> &ground_truth) {
  /**
  TODO:
    * Calculate the RMSE here.
  */

	VectorXd rmse(4);
		rmse << 0,0,0,0;

	    // TODO: YOUR CODE HERE

		// check the validity of the following inputs:
		//  * the estimation vector size should not be zero
		//  * the estimation vector size should equal ground truth vector size
		// ... your code here

	    VectorXd diff(4);
		diff << 0,0,0,0;
	    VectorXd acc(4);
		acc << 0,0,0,0;
		//accumulate squared residuals
		for(int i=0; i < estimations.size(); ++i){
	        // ... your code here
			diff = (ground_truth[i]-estimations[i]);
			acc = acc.array() + diff.array()*diff.array();
		}

		//calculate the mean
		// ... your code here
	    rmse = acc / estimations.size();

		//calculate the squared root
		// ... your code here
	    rmse = rmse.array().sqrt();
	    return rmse;
}
