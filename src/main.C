#include "AppFactory.h"
#include "CrowApp.h"
#include "Moose.h"
#include "MooseApp.h"
#include "MooseInit.h"

// Create a performance log
PerfLog Moose::perf_log("Crow");

// Begin the main program.
int main(int argc, char *argv[]) {
  // Initialize MPI, solvers and MOOSE
  MooseInit init(argc, argv);

  // Register this application's MooseApp and any it depends on
  CrowApp::registerApps();


  return 0;
}
