/*********************************************
 * NOR Gate Circuit Optimization
 *********************************************/

// Parameters (read from .dat file)
int nGates = ...;
int nRows = ...;

range Gates = 1..nGates;
range Rows = 1..nRows;
range Inputs = 1..2;

int alpha[Rows][Inputs] = ...;
int desiredOutput[Rows] = ...;
int predecessor[Gates][Inputs] = ...;

// Decision variables
dvar boolean s[Gates];         // gate exists
dvar boolean t[Gates][Inputs]; // external inputs
dvar boolean x[Gates][Rows];   // outputs

// Objective
minimize sum(i in Gates) s[i];

subject to {

  // Gate 1 must exist
  s[1] == 1;

  // Fix outputs of gate 1
  x[1][1] == 0;
  x[1][2] == 1;
  x[1][3] == 1;
  x[1][4] == 0;

  // If gate exists -> can have input
  forall(i in Gates, j in Inputs)
    s[i] - t[i][j] >= 0;

  // If output -> gate exists
  forall(i in Gates, l in Rows)
    s[i] - x[i][l] >= 0;

  // Fan-in constraint (gates 1..3)
  forall(i in 1..3)
    s[predecessor[i][1]] + s[predecessor[i][2]] + t[i][1] + t[i][2] <= 2;

  // Logic constraints (gates 1..3)
  forall(i in 1..3, l in Rows) {
    x[predecessor[i][1]][l] + x[i][l] <= 1;
    x[predecessor[i][2]][l] + x[i][l] <= 1;
  }

  // NOR constraints (all gates)
  forall(i in Gates, l in Rows) {
    alpha[l][1] * t[i][1] + x[i][l] <= 1;
    alpha[l][2] * t[i][2] + x[i][l] <= 1;
  }

  // Gates 4..n (bez poprzedników)
  forall(i in 4..nGates, l in Rows)
    alpha[l][1]*t[i][1] +
    alpha[l][2]*t[i][2] +
    x[i][l] - s[i] >= 0;

  // Gates 1..3 (z poprzednikami)
  forall(i in 1..3, l in Rows)
    alpha[l][1]*t[i][1] +
    alpha[l][2]*t[i][2] +
    x[predecessor[i][1]][l] +
    x[predecessor[i][2]][l] +
    x[i][l] - s[i] >= 0;
}