/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "BicrystalICAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "Parser.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh/equation_systems.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/explicit_system.h"
#include "libmesh/libmesh.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/string_to_enum.h"

registerMooseAction("CrowApp", BicrystalICAction, "add_ic");

const Real BicrystalICAction::_abs_zero_tol = 1e-12;

template <> InputParameters validParams<BicrystalICAction>() {
  InputParameters params = validParams<Action>();

  params.addRequiredParam<std::string>(
      "var_name_base", "specifies the base name of the variables");
  params.addRequiredParam<unsigned int>("op_num",
                                        "Number of grains, should be 2");
  params.addRequiredParam<Real>(
      "x1", "The x coordinate of the lower left-hand corner of the box");
  params.addRequiredParam<Real>(
      "y1", "The y coordinate of the lower left-hand corner of the box");
  params.addParam<Real>(
      "z1", 0.0, "The z coordinate of the lower left-hand corner of the box");

  params.addRequiredParam<Real>(
      "x2", "The x coordinate of the upper right-hand corner of the box");
  params.addRequiredParam<Real>(
      "y2", "The y coordinate of the upper right-hand corner of the box");
  params.addParam<Real>(
      "z2", 0.0, "The z coordinate of the upper right-hand corner of the box");

  params.addParam<Real>("int_width", 0.0, "Width of the interface");

  return params;
}

BicrystalICAction::BicrystalICAction(const InputParameters &params)
    : Action(params), _var_name_base(getParam<std::string>("var_name_base")),
      _op_num(getParam<unsigned int>("op_num")), _x1(getParam<Real>("x1")),
      _y1(getParam<Real>("y1")), _z1(getParam<Real>("z1")),
      _x2(getParam<Real>("x2")), _y2(getParam<Real>("y2")),
      _z2(getParam<Real>("z2")), _int(getParam<Real>("int_width")) {
  if (_op_num != 2)
    mooseError("op_num must equal 2 for bicrystal ICs");
}

void BicrystalICAction::act() {
#ifdef DEBUG
  Moose::err << "Inside the BicrystalICAction Object\n";
#endif

  // Loop through the number of order parameters
  for (unsigned int op = 0; op < _op_num; op++) {
    // Create variable names
    std::string var_name = _var_name_base;
    std::stringstream out;
    out << op;
    var_name.append(out.str());

    // Set parameters for IC
    InputParameters poly_params = _factory.getValidParams("BicrystalIC");
    poly_params.set<VariableName>("variable") = var_name;
    poly_params.set<Real>("x1") = _x1;
    poly_params.set<Real>("y1") = _y1;
    poly_params.set<Real>("z1") = _z1;
    poly_params.set<Real>("x2") = _x2;
    poly_params.set<Real>("y2") = _y2;
    poly_params.set<Real>("z2") = _z2;
    poly_params.set<Real>("int_width") = _int;
    if (op == 0) {
      // Values for bounding box
      poly_params.set<Real>("inside") = 1.0;
      poly_params.set<Real>("outside") = 0.0;
    } else {
      // Values for matrix grain
      poly_params.set<Real>("inside") = 0.0;
      poly_params.set<Real>("outside") = 1.0;
    }

    // Add initial condition
    _problem->addInitialCondition("BicrystalIC", "InitialCondition",
                                  poly_params);
  }
}
