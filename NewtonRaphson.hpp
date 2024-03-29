#include <iostream>
#include <Teuchos_BLAS.hpp>
#include <Teuchos_LAPACK.hpp>
#include <complex>
#include <vector>
#include <Sacado.hpp>
#include <Teuchos_RCP.hpp>
#include <Teuchos_RCPNode.hpp>
#include <Teuchos_SerialDenseSolver.hpp>
#include "Teuchos_SerialDenseMatrix.hpp"
#include "Teuchos_SerialDenseVector.hpp"
#include "Teuchos_Version.hpp"

namespace NRNameSpace{

typedef Sacado::Fad::DFad<double>  F;  // Forward AD with # of ind. vars given later

class NewtonRaphsonProblem 
{
public:
double myError;

Teuchos::SerialDenseMatrix<int, double> myJacobian;
Teuchos::SerialDenseMatrix<int, double> myConstants;
Teuchos::SerialDenseVector<int, double> myScratchReal;
Teuchos::SerialDenseVector<int, double> myTargetsDesired;
Teuchos::LAPACK<int, double> LAPACK;

NewtonRaphsonProblem(){};

bool reinitializeSameModel(const Teuchos::SerialDenseVector<int, double>& targetsDesired)
{
myTargetsDesired = targetsDesired;
myError = 1.E6;
return true;
};
};

class NRForwardDifference: public NewtonRaphsonProblem 
{
private:
Teuchos::SerialDenseVector<int, double > myCurrentGuesses;
Teuchos::SerialDenseVector<int, double > myTargetsCalculated;

void (*myCalculateDependentVariables)(const Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, double >&, 
Teuchos::SerialDenseVector<int, double >&,
const Teuchos::SerialDenseVector<int, double >&);            

void calculateJacobian(const double PROBELENGTH,
const Teuchos::SerialDenseMatrix<int, double>& constants,
const Teuchos::SerialDenseVector<int, double>& targetsDesired,
Teuchos::SerialDenseMatrix<int,  double>& jacobian, 
Teuchos::SerialDenseVector<int, double >& currentGuesses, 
Teuchos::SerialDenseVector<int, double >& targetsCalculated,
Teuchos::SerialDenseVector<int, double >& unperturbedTargets,
void myCalculateDependentVariables(const Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, double >&, 
Teuchos::SerialDenseVector<int, double >&,
const Teuchos::SerialDenseVector<int, double>&))
{
//evaluate model with no perturbation
myCalculateDependentVariables(constants, currentGuesses, unperturbedTargets, targetsDesired);
double oldGuessValue = 0.0;

for(int column = 0; column < targetsCalculated.length(); column++)
{
//Store old guess vector element value, perturb the current value
oldGuessValue = currentGuesses[column];
currentGuesses[column] += PROBELENGTH;

//Re-evaluate and apply forward difference formula 
myCalculateDependentVariables(constants, currentGuesses, targetsCalculated, targetsDesired);
targetsCalculated -= unperturbedTargets;
targetsCalculated *= pow(PROBELENGTH, -1.0);

for(int row = 0; row < targetsCalculated.length(); row ++)
{
jacobian(row, column) = targetsCalculated[row];
}
currentGuesses[column] = oldGuessValue;
}
//Reset to unperturbed, so we dont waste a function evaluation
targetsCalculated = unperturbedTargets;
}

void updateGuess(const double UPDATEMULTIPLIER,
Teuchos::SerialDenseVector<int, double >& currentGuesses,
Teuchos::SerialDenseVector<int, double >& targetsCalculated,
Teuchos::SerialDenseVector<int, double >& scratch,
Teuchos::SerialDenseMatrix<int, double>& jacobian, 
Teuchos::LAPACK<int, double>& lapack)
{
//v = J(inverse) * (-F(x))
//new guess = v + old guess
scratch = 0.0;
scratch -= targetsCalculated;

//Perform an LU factorization of this matrix. 
int ipiv[targetsCalculated.length()], info;
char TRANS = 'N';
lapack.GETRF( targetsCalculated.length(), targetsCalculated.length(), jacobian.values(), jacobian.stride(), ipiv, &info ); 

// Solve the linear system.
lapack.GETRS( TRANS, targetsCalculated.length(), 1, jacobian.values(), jacobian.stride(),
ipiv, scratch.values(), scratch.stride(), &info );  

//We have overwritten targetsCalculated with guess update values
//Now update current guesses
scratch *= UPDATEMULTIPLIER;
targetsCalculated = currentGuesses;
currentGuesses += scratch;
}

void calculateResidual(const Teuchos::SerialDenseVector<int, double>& currentGuesses, 
const Teuchos::SerialDenseVector<int, double >& targetsCalculated,
Teuchos::SerialDenseVector<int, double>& scratch,
double& error)
{
//targetsCalculated actually contains previous guesses
for(int i = 0; i < targetsCalculated.length(); i++)
{
scratch[i] = targetsCalculated[i] - currentGuesses[i];
}
error = sqrt(scratch.dot(scratch));
}

public:
NRForwardDifference(const Teuchos::SerialDenseVector<int, double >& initialGuess,
const Teuchos::SerialDenseVector<int, double>& targetsDesired,
const Teuchos::SerialDenseMatrix<int, double>& constants,
void yourCalculateDependentVariables(const Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, double >&, 
Teuchos::SerialDenseVector<int, double >&,
const Teuchos::SerialDenseVector<int, double>&))
{
myCurrentGuesses.resize(initialGuess.length());
myTargetsDesired.resize(targetsDesired.length());
myTargetsCalculated.resize(targetsDesired.length());
myScratchReal.resize(targetsDesired.length());
myJacobian.shapeUninitialized(targetsDesired.length(), initialGuess.length());
myConstants.shapeUninitialized(constants.numRows(), constants.numCols());

myCurrentGuesses = initialGuess;
myTargetsDesired = targetsDesired;
myConstants = constants;
myCalculateDependentVariables = yourCalculateDependentVariables;
};

void solve(const double UPDATEMULTIPLIER, const int MAXITERATIONS, const double ERRORTOLLERANCE, const double PROBELENGTH)
{
int count = 0;
double error = 1.0E5;

std::cout << "******************************************" << std::endl;
std::cout << "Forward Difference Example" << std::endl;

while(count < MAXITERATIONS and error > ERRORTOLLERANCE)
{
calculateJacobian(
PROBELENGTH,
myConstants,
myTargetsDesired,
myJacobian,
myCurrentGuesses,
myTargetsCalculated,
myScratchReal,
myCalculateDependentVariables);

updateGuess(UPDATEMULTIPLIER,
myCurrentGuesses,
myTargetsCalculated,
myScratchReal,
myJacobian,
LAPACK);

calculateResidual(myCurrentGuesses,
myTargetsCalculated,
myScratchReal,
error);	  

count ++;
//std::cout << "Residual Error: " << error << std::endl;
}

std::cout << "******************************************" << std::endl;
std::cout << "Number of iterations: " << count << std::endl;
std::cout << "Final Guess:" << std::endl;
for(int i = 0; i < myCurrentGuesses.length(); i++)
{
std::cout << "Dim: " << i << ", Value: " << myCurrentGuesses[i] << std::endl;
}
std::cout << "Error tollerance: " << ERRORTOLLERANCE << std::endl;
std::cout << "Final error: " << error << std::endl;
}

bool fullReinitialize(const Teuchos::SerialDenseVector<int, double>& initialGuess,
const Teuchos::SerialDenseVector<int, double>& targetsDesired,
const Teuchos::SerialDenseMatrix<int, double>& constants,
void yourCalculateDependentVariables(const Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, double >&, 
Teuchos::SerialDenseVector<int, double >&,
const Teuchos::SerialDenseVector<int, double>&))
{
myCurrentGuesses.resize(initialGuess.length());
myTargetsDesired.resize(targetsDesired.length());
myTargetsCalculated.resize(targetsDesired.length());
myScratchReal.resize(targetsDesired.length());
myJacobian.shapeUninitialized(targetsDesired.length(), initialGuess.length());
myConstants.shapeUninitialized(constants.numRows(), constants.numCols());

myCurrentGuesses = initialGuess;
myTargetsDesired = targetsDesired;
myConstants = constants;
myCalculateDependentVariables = yourCalculateDependentVariables;

return true;
};

const Teuchos::SerialDenseVector<int, double>& whatHaveIGuessed()
{
return myCurrentGuesses;
};
};

class NRComplexStep: public NewtonRaphsonProblem 
{
private:
Teuchos::SerialDenseVector<int, std::complex<double> >  myScratchComplex;
Teuchos::SerialDenseVector<int, std::complex<double> > myCurrentGuesses;
Teuchos::SerialDenseVector<int, std::complex<double> > myTargetsCalculated;

void (*myCalculateDependentVariables)(const Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, std::complex<double> >&, 
Teuchos::SerialDenseVector<int, std::complex<double> >&,
const Teuchos::SerialDenseVector<int, double>&);            

void calculateJacobian(const double PROBELENGTH,
const Teuchos::SerialDenseMatrix<int, double>& constants,
const Teuchos::SerialDenseVector<int, double>& targetsDesired,
Teuchos::SerialDenseMatrix<int,  double>& jacobian, 
Teuchos::SerialDenseVector<int, std::complex< double> >& currentGuesses, 
Teuchos::SerialDenseVector<int, std::complex< double> >& targetsCalculated,
Teuchos::SerialDenseVector<int, std::complex< double> >& unperturbedTargets,
void myCalculateDependentVariables(const Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, std::complex< double> >&, 
Teuchos::SerialDenseVector<int, std::complex< double> >&,
const Teuchos::SerialDenseVector<int, double>&))
{
//evaluate the unperturbed model
myCalculateDependentVariables(constants, currentGuesses, unperturbedTargets, targetsDesired);
std::complex< double> oldGuessValue;

for(int column = 0; column < targetsCalculated.length(); column++)
{
//Store old element value, perturb the current value
oldGuessValue = currentGuesses[column];
currentGuesses[column] += std::complex< double>(0.0, PROBELENGTH);

//Evaluate model for perturbed guess
myCalculateDependentVariables(constants, currentGuesses, targetsCalculated, targetsDesired);

//Divide each element by PROBELENGTH
targetsCalculated *= pow(PROBELENGTH, -1.0);

for(int row = 0; row < targetsCalculated.length(); row ++)
{
//Complete expressing the CTSE formula and take the magnitudes of
//the imaginary parts of targetsCalculated as derivatives wrt, the
//row = dof.
jacobian(row, column) = std::imag(targetsCalculated[row]);
}
currentGuesses[column] = oldGuessValue;
}
//Reset to unperturbed, so we dont waste a function evaluation
targetsCalculated = unperturbedTargets;
}

void updateGuess(const double UPDATEMULTIPLIER,
Teuchos::SerialDenseVector<int, std::complex< double> >& currentGuesses,
Teuchos::SerialDenseVector<int, std::complex< double> >& targetsCalculated,
Teuchos::SerialDenseVector<int, double>& scratch,
Teuchos::SerialDenseMatrix<int, double>& jacobian, 
Teuchos::LAPACK<int, double>& lapack)
{
//v = J(inverse) * (-F(x))
//new guess = v + old guess
for(int i = 0; i < targetsCalculated.length(); i++)
{
scratch[i] = -std::real(targetsCalculated[i]);
}

//Perform an LU factorization of this matrix. 
int ipiv[targetsCalculated.length()], info;
char TRANS = 'N';
lapack.GETRF( targetsCalculated.length(), targetsCalculated.length(), jacobian.values(), jacobian.stride(), ipiv, &info ); 

// Solve the linear system.
lapack.GETRS( TRANS, targetsCalculated.length(), 1, jacobian.values(), jacobian.stride(),
ipiv, scratch.values(), scratch.stride(), &info );  

//We have overwritten targetsCalculated with guess update values
for(int i = 0; i < targetsCalculated.length(); i++)
{
targetsCalculated[i] = currentGuesses[i];
currentGuesses[i] += UPDATEMULTIPLIER*scratch[i];
}
}

void calculateResidual(const Teuchos::SerialDenseVector<int, std::complex<double> >& currentGuesses, 
const Teuchos::SerialDenseVector<int, std::complex< double> >& targetsCalculated,
Teuchos::SerialDenseVector<int, double>& scratch,
double& error)
{
//Targets calculated actually contains the previous guess right now
for(int i = 0; i < targetsCalculated.length(); i++)
{
scratch[i] = std::real(targetsCalculated[i]) - std::real(currentGuesses[i]);
}
error = sqrt(scratch.dot(scratch));
}

public:
NRComplexStep(const Teuchos::SerialDenseVector<int, std::complex<double> >& initialGuess,
const Teuchos::SerialDenseVector<int, double>& targetsDesired,
const Teuchos::SerialDenseMatrix<int, double>& constants,
void yourCalculateDependentVariables(const Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, std::complex<double> >&, 
Teuchos::SerialDenseVector<int, std::complex<double> >&,
const Teuchos::SerialDenseVector<int, double>&))
{
myCurrentGuesses.resize(initialGuess.length());
myTargetsDesired.resize(targetsDesired.length());
myTargetsCalculated.resize(targetsDesired.length());
myScratchReal.resize(targetsDesired.length());
myScratchComplex.resize(targetsDesired.length());
myJacobian.shapeUninitialized(targetsDesired.length(), initialGuess.length());

myCurrentGuesses = initialGuess;
myTargetsDesired = targetsDesired;

myConstants.shapeUninitialized(constants.numRows(), constants.numCols());
myConstants = constants;
myCalculateDependentVariables = yourCalculateDependentVariables;

};

void solve(const double UPDATEMULTIPLIER, const int MAXITERATIONS, const double ERRORTOLLERANCE, const double PROBELENGTH)
{
int count = 0;
double error = 1.0E5;


std::cout << "******************************************" << std::endl;
std::cout << "Complex Step Example" << std::endl;
while(count < MAXITERATIONS and error > ERRORTOLLERANCE)
{
calculateJacobian(
PROBELENGTH,
myConstants,
myTargetsDesired,
myJacobian,
myCurrentGuesses,
myTargetsCalculated,
myScratchComplex,
myCalculateDependentVariables);

updateGuess(UPDATEMULTIPLIER,
myCurrentGuesses,
myTargetsCalculated,
myScratchReal,
myJacobian,
LAPACK);

calculateResidual(myCurrentGuesses,
myTargetsCalculated,
myScratchReal,
error);	  

count ++;
//std::cout << "Residual Error: " << error << std::endl;
}

std::cout << "******************************************" << std::endl;
std::cout << "Number of iterations: " << count << std::endl;
std::cout << "Final Guess:" << std::endl;
for(int i = 0; i < myCurrentGuesses.length(); i++)
{
std::cout << "Dim: " << i << ", Value: " << std::real(myCurrentGuesses[i]) << std::endl;
}

std::cout << "Error tollerance: " << ERRORTOLLERANCE << std::endl;
std::cout << "Final error: " << error << std::endl;
}

bool fullReinitialize(const Teuchos::SerialDenseVector<int, std::complex<double> >& initialGuess,
const Teuchos::SerialDenseVector<int, double>& targetsDesired,
const Teuchos::SerialDenseMatrix<int, double>& constants,
void yourCalculateDependentVariables(const Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, std::complex<double> >&, 
Teuchos::SerialDenseVector<int, std::complex<double> >&,
const Teuchos::SerialDenseVector<int, double>&))
{
myCurrentGuesses.resize(initialGuess.length());
myTargetsDesired.resize(targetsDesired.length());
myTargetsCalculated.resize(targetsDesired.length());
myScratchReal.resize(targetsDesired.length());
myScratchComplex.resize(targetsDesired.length());
myJacobian.shapeUninitialized(targetsDesired.length(), initialGuess.length());

myCurrentGuesses = initialGuess;
myTargetsDesired = targetsDesired;

myConstants.shapeUninitialized(constants.numRows(), constants.numCols());
myConstants = constants;
myCalculateDependentVariables = yourCalculateDependentVariables;

};

const Teuchos::SerialDenseVector<int, std::complex<double> >& whatHaveIGuessed()
{
return myCurrentGuesses;
};
};

class NRAutomaticDifferentiation: public NewtonRaphsonProblem 
{

private:

std::vector<F> myCurrentGuesses;
std::vector<F> myTargetsCalculated;

void (*myCalculateDependentVariables)(const Teuchos::SerialDenseMatrix<int, double>&, 
const std::vector<F>&, 
std::vector<F>&i,
const Teuchos::SerialDenseVector<int, double>&);            

void calculateJacobian(
const Teuchos::SerialDenseMatrix<int, double>& constants,
const Teuchos::SerialDenseVector<int, double>& targetsDesired,
Teuchos::SerialDenseMatrix<int,  double>& jacobian, 
std::vector<F>& currentGuesses, 
std::vector<F>& targetsCalculated,
void myCalculateDependentVariables(const Teuchos::SerialDenseMatrix<int, double>&, 
const std::vector<F>&, 
std::vector<F>&,
const Teuchos::SerialDenseVector<int, double>&))
{
myCalculateDependentVariables(constants, currentGuesses, targetsCalculated, targetsDesired);

for(int column = 0; column < targetsCalculated.size(); column++)
{
for(int row = 0; row < targetsCalculated.size(); row ++)
{
jacobian(row, column) = targetsCalculated[row].dx(column);
}
}
}

void updateGuess(const double UPDATEMULTIPLIER,
std::vector<F>& currentGuesses,
std::vector<F>& targetsCalculated,
Teuchos::SerialDenseVector<int, double>& scratch,
Teuchos::SerialDenseMatrix<int, double>& jacobian, 
Teuchos::LAPACK<int, double>& lapack)
{
//v = J(inverse) * (-F(x))
//new guess = v + old guess
for(int i = 0; i < targetsCalculated.size(); i++)
{
scratch[i] = -targetsCalculated[i].val();
}

//Perform an LU factorization of this matrix. 
int ipiv[targetsCalculated.size()], info;
char TRANS = 'N';
lapack.GETRF( targetsCalculated.size(), targetsCalculated.size(), jacobian.values(), jacobian.stride(), ipiv, &info ); 

// Solve the linear system.
lapack.GETRS( TRANS, targetsCalculated.size(), 1, jacobian.values(), jacobian.stride(),
ipiv, scratch.values(), scratch.stride(), &info );  

//We have overwritten scratch 
for(int i = 0; i < targetsCalculated.size(); i++)
{
targetsCalculated[i] = currentGuesses[i];
currentGuesses[i] += UPDATEMULTIPLIER*scratch[i];
}
}

void calculateResidual(std::vector<F>& currentGuesses, 
std::vector<F>& targetsCalculated,
Teuchos::SerialDenseVector<int, double>& scratch,
double& error)
{
//targetsCalculated actually contains the previous guesses right now, or ought to
for(int i = 0; i < targetsCalculated.size(); i++)
{
scratch[i] = targetsCalculated[i].val() - currentGuesses[i].val();
}
error = sqrt(scratch.dot(scratch));
}


public:
NRAutomaticDifferentiation(const Teuchos::SerialDenseVector<int, double >& initialGuess,
const Teuchos::SerialDenseVector<int, double>& targetsDesired,
const Teuchos::SerialDenseMatrix<int, double>& constants,
void yourCalculateDependentVariables(const Teuchos::SerialDenseMatrix<int, double>&, 
const std::vector<F>&, 
std::vector<F>&,
const Teuchos::SerialDenseVector<int, double>&))
{
myTargetsDesired.resize(targetsDesired.length());
myScratchReal.resize(targetsDesired.length());
myJacobian.shapeUninitialized(targetsDesired.length(), initialGuess.length());

myConstants.shapeUninitialized(constants.numRows(), constants.numCols());

myCurrentGuesses.resize(initialGuess.length());
myTargetsCalculated.resize(targetsDesired.length());
for(int i = 0; i < targetsDesired.length(); i++)
{
myCurrentGuesses[i] =initialGuess[i];
myCurrentGuesses[i].diff(i, targetsDesired.length());
}

myTargetsDesired = targetsDesired;
myConstants = constants;
myCalculateDependentVariables = yourCalculateDependentVariables;
};

void solve(const double UPDATEMULTIPLIER, const int MAXITERATIONS, const double ERRORTOLLERANCE)
{
int count = 0;
double error = 1.0E5;

std::cout << "******************************************" << std::endl;
std::cout << "Automatic Differentiation Example" << std::endl;
while(count < MAXITERATIONS and error > ERRORTOLLERANCE)
{
calculateJacobian(
myConstants,
myTargetsDesired,
myJacobian,
myCurrentGuesses,
myTargetsCalculated,
myCalculateDependentVariables);

updateGuess(UPDATEMULTIPLIER,
myCurrentGuesses,
myTargetsCalculated,
myScratchReal,
myJacobian,
LAPACK);

calculateResidual(myCurrentGuesses,
myTargetsCalculated,
myScratchReal,
error);	  

count ++;
//If we have converged, or if we have exceeded our alloted number of iterations, discontinue the loop
//std::cout << "Residual Error: " << error << std::endl;
}
std::cout << "******************************************" << std::endl;
std::cout << "Number of iterations: " << count << std::endl;
std::cout << "Final Guess:" << std::endl;
for(int i = 0; i < myCurrentGuesses.size(); i++)
{
std::cout << "Dim: " << i << ", Value: " << myCurrentGuesses[i].val() << std::endl;
}

std::cout << "Error tollerance: " << ERRORTOLLERANCE << std::endl;
std::cout << "Final error: " << error << std::endl;
}

bool fullReinitialize(const Teuchos::SerialDenseVector<int, double>& initialGuess,
const Teuchos::SerialDenseVector<int, double>& targetsDesired,
const Teuchos::SerialDenseMatrix<int, double>& constants,
void yourCalculateDependentVariables(const Teuchos::SerialDenseMatrix<int, double>&, 
const std::vector<F>&, 
std::vector<F>&,
const Teuchos::SerialDenseVector<int, double>&))

{
myTargetsDesired.resize(targetsDesired.length());
myScratchReal.resize(targetsDesired.length());
myJacobian.shapeUninitialized(targetsDesired.length(), initialGuess.length());

myConstants.shapeUninitialized(constants.numRows(), constants.numCols());

myCurrentGuesses.resize(initialGuess.length());
myTargetsCalculated.resize(targetsDesired.length());
for(int i = 0; i < targetsDesired.length(); i++)
{
myCurrentGuesses[i] =initialGuess[i];
myCurrentGuesses[i].diff(i, targetsDesired.length());
}

myTargetsDesired = targetsDesired;
myConstants = constants;
myCalculateDependentVariables = yourCalculateDependentVariables;

return true;
};

const Teuchos::SerialDenseVector<int, double>& whatHaveIGuessed()
{
for(int i = 0; i < myScratchReal.length(); i++)
{
myScratchReal[i] = myCurrentGuesses[i].val();
}
return myScratchReal;
};
};

class NRUserSupplied: public NewtonRaphsonProblem 
{
private:
Teuchos::SerialDenseVector<int, double > myCurrentGuesses;
Teuchos::SerialDenseVector<int, double > myTargetsCalculated;

void (*myCalculateDependentVariables)(const Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, double >&, 
Teuchos::SerialDenseVector<int, double >&,
const Teuchos::SerialDenseVector<int, double>&);            

void (*myCalculateJacobian)(const Teuchos::SerialDenseMatrix<int, double>&, 
Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, double>&,
const Teuchos::SerialDenseVector<int,double>&);

void updateGuess(const double UPDATEMULTIPLIER,
Teuchos::SerialDenseVector<int, double >& currentGuesses,
Teuchos::SerialDenseVector<int, double >& targetsCalculated,
Teuchos::SerialDenseVector<int, double >& scratch,
Teuchos::SerialDenseMatrix<int, double>& jacobian, 
Teuchos::LAPACK<int, double>& lapack)
{

//currentGuesses[0] += (-targetsCalculated[0]/jacobian(0,0));
//v = J(inverse) * (-F(x))
//new guess = v + old guess
scratch = 0.0;
scratch -= targetsCalculated;

//Perform an LU factorization of this matrix. 
int ipiv[targetsCalculated.length()], info;
char TRANS = 'N';
lapack.GETRF( targetsCalculated.length(), targetsCalculated.length(), jacobian.values(), jacobian.stride(), ipiv, &info ); 

// Solve the linear system.
lapack.GETRS( TRANS, targetsCalculated.length(), 1, jacobian.values(), jacobian.stride(),
ipiv, scratch.values(), scratch.stride(), &info );  

//We have overwritten targetsCalculated with guess update values
//Now update current guesses
scratch *= UPDATEMULTIPLIER;
targetsCalculated = currentGuesses;
currentGuesses += scratch;
}

void calculateResidual(const Teuchos::SerialDenseVector<int, double>& currentGuesses, 
const Teuchos::SerialDenseVector<int, double>& targetsCalculated,
Teuchos::SerialDenseVector<int, double>& scratch,
double& error)
{
//targetsCalculated acutally contains the previous guesses
for(int i = 0; i < targetsCalculated.length(); i++)
{
scratch[i] = targetsCalculated[i] - currentGuesses[i];
}
error = sqrt(scratch.dot(scratch));
}

public:
NRUserSupplied(const Teuchos::SerialDenseVector<int, double >& initialGuess,
const Teuchos::SerialDenseVector<int, double>& targetsDesired,
const Teuchos::SerialDenseMatrix<int, double>& constants,
void yourCalculateDependentVariables(const Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, double >&, 
Teuchos::SerialDenseVector<int, double >&,
const Teuchos::SerialDenseVector<int, double>&),
void yourCalculateJacobian(const Teuchos::SerialDenseMatrix<int, double>&, 
Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, double>&,
const Teuchos::SerialDenseVector<int,double>&))

{
myCurrentGuesses.resize(initialGuess.length());
myTargetsDesired.resize(targetsDesired.length());
myTargetsCalculated.resize(targetsDesired.length());
myScratchReal.resize(targetsDesired.length());
myJacobian.shapeUninitialized(myTargetsDesired.length(), myCurrentGuesses.length());
myConstants.shapeUninitialized(constants.numRows(), constants.numCols());
myConstants = constants;

myCurrentGuesses = initialGuess;
myTargetsDesired = targetsDesired;
myCalculateDependentVariables = yourCalculateDependentVariables;
myCalculateJacobian = yourCalculateJacobian;
};

void solve(const double UPDATEMULTIPLIER, const int MAXITERATIONS, const double ERRORTOLLERANCE)
{
int count = 0;
double error = 1.0E5;

std::cout << "******************************************" << std::endl;
std::cout <<  "User supplied Jacobian" << std::endl;

while(count < MAXITERATIONS and error > ERRORTOLLERANCE)
{

myCalculateDependentVariables(myConstants,
myCurrentGuesses,
myTargetsCalculated,
myTargetsDesired);

myCalculateJacobian(myConstants,
myJacobian,
myCurrentGuesses,
myTargetsDesired);

updateGuess(UPDATEMULTIPLIER,
myCurrentGuesses,
myTargetsCalculated,
myScratchReal,
myJacobian,
LAPACK);

calculateResidual(myCurrentGuesses,
myTargetsCalculated,
myScratchReal,
error);	  

count ++;
//std::cout << "Residual Error: " << error << std::endl;
}

std::cout << "******************************************" << std::endl;
std::cout << "Number of iterations: " << count << std::endl;
std::cout << "Final Guess:" << std::endl;
for(int i = 0; i < myCurrentGuesses.length(); i++)
{
std::cout << "Dim: " << i << ", Value: " << myCurrentGuesses[i] << std::endl;
}

std::cout << "Error tollerance: " << ERRORTOLLERANCE << std::endl;
std::cout << "Final error: " << error << std::endl;
}

bool fullReinitialize(const int MYNUMDIMENSIONS,
const Teuchos::SerialDenseVector<int, double>& initialGuess,
const Teuchos::SerialDenseVector<int, double>& targetsDesired,
const Teuchos::SerialDenseMatrix<int, double>& constants,
void yourCalculateDependentVariables(const Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, double >&, 
Teuchos::SerialDenseVector<int, double >&,
const Teuchos::SerialDenseVector<int, double>&),
void yourCalculateJacobian(const Teuchos::SerialDenseMatrix<int, double>&, 
Teuchos::SerialDenseMatrix<int, double>&, 
const Teuchos::SerialDenseVector<int, double>&,
const Teuchos::SerialDenseVector<int,double>&))
{
myCurrentGuesses.resize(initialGuess.length());
myTargetsDesired.resize(targetsDesired.length());
myTargetsCalculated.resize(targetsDesired.length());
myScratchReal.resize(targetsDesired.length());
myJacobian.shapeUninitialized(myTargetsDesired.length(), myCurrentGuesses.length());
myConstants.shapeUninitialized(constants.numRows(), constants.numCols());
myConstants = constants;

myCurrentGuesses = initialGuess;
myTargetsDesired = targetsDesired;
myCalculateDependentVariables = yourCalculateDependentVariables;
myCalculateJacobian = yourCalculateJacobian;

return true;
};

const Teuchos::SerialDenseVector<int, double>& whatHaveIGuessed()
{
return myCurrentGuesses;
};
};

}

