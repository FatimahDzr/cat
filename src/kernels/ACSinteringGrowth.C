#include "ACSinteringGrowth.h"

registerMooseObject("CrowApp", ACSinteringGrowth);

template <> InputParameters validParams<ACSinteringGrowth>() {
  InputParameters params = ACBulk<Real>::validParams();
  params.addRequiredCoupledVar("v", "Array of coupled variable names");
  params.addCoupledVar("T", "temperature");
  params.addRequiredCoupledVar("rho", "phase field variable");
  params.addParam<Real>("B", 1.0, "Constant value for B");
  return params;
}

ACSinteringGrowth::ACSinteringGrowth(const InputParameters &parameters)
    : ACBulk<Real>(parameters),
      //_mu(getMaterialProperty<Real>("mu")),
      //_gamma(getMaterialProperty<Real>("gamma_asymm")),
      _tgrad_corr_mult(getMaterialProperty<Real>("tgrad_corr_mult")),
      _has_T(isCoupled("T")), _grad_T(_has_T ? &coupledGradient("T") : NULL),
      _rho(coupledValue("rho")), _B(getParam<Real>("B")) {
  // Array of coupled variables is created in the constructor
  _ncrys = coupledComponents("v"); // determine number of grains from the number
                                   // of names passed in.  Note this is the
                                   // actual number -1
  _vals.resize(_ncrys); // Size variable arrays
  _vals_var.resize(_ncrys);

  // _gamma = 1.5;

  // Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _ncrys; ++i) {
    _vals[i] = &coupledValue("v", i);
    _vals_var[i] = coupled("v", i);
  }
}

Real ACSinteringGrowth::computeDFDOP(PFFunctionType type) {
  Real SumEtaj = 0.0;
  for (unsigned int i = 0; i < _ncrys; ++i)
    SumEtaj +=
        (*_vals[i])[_qp] * (*_vals[i])[_qp]; // Sum all other order parameters

  Real tgrad_correction = 0.0;

  // Calcualte either the residual or jacobian of the grain growth free energy
  switch (type) {
  case Residual:
    if (_has_T)
      tgrad_correction = _tgrad_corr_mult[_qp] * _grad_u[_qp] * (*_grad_T)[_qp];
    return 12.0 * _B *
               ((1 - _rho[_qp]) * _u[_qp] -
                (2 - _rho[_qp]) * _u[_qp] * _u[_qp] + _u[_qp] * SumEtaj) +
           tgrad_correction;

  case Jacobian:
    if (_has_T)
      tgrad_correction =
          _tgrad_corr_mult[_qp] * _grad_phi[_j][_qp] * (*_grad_T)[_qp];
    return 12.0 * _B *
               (_phi[_j][_qp] * (-2 * (2 - _rho[_qp]) * _u[_qp] + SumEtaj)) +
           tgrad_correction;
  }

  mooseError("Invalid type passed in");
}

Real ACSinteringGrowth::computeQpOffDiagJacobian(unsigned int jvar) {
  for (unsigned int i = 0; i < _ncrys; ++i) {
    if (jvar == _vals_var[i]) {
      Real dSumEtaj =
          2.0 * _B * (*_vals[i])[_qp] * _phi[_j][_qp]; // Derivative of SumEtaj
      Real dDFDOP = 12.0 * _B * _u[_qp] * dSumEtaj;

      return _L[_qp] * _test[_i][_qp] * dDFDOP;
    }
  }

  return 0.0;
}
