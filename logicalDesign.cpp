#include <ilcplex/ilocplex.h>
#include <cmath>

ILOSTLBEGIN

// helper: clean binary output
int clean(double v) {
    return (std::fabs(v) < 1e-6) ? 0 : (std::fabs(v - 1) < 1e-6 ? 1 : (int)std::round(v));
}

int main() {
    IloEnv env;

    try {
        IloModel model(env);

        // -------------------------
        // DATA
        // -------------------------
        int nGates = 7;
        int nRows = 4;

        int alpha[4][2] = {
            {0,0},
            {0,1},
            {1,0},
            {1,1}
        };

        int predecessor[7][2] = {
            {2,3},
            {4,5},
            {6,7},
            {0,0},
            {0,0},
            {0,0},
            {0,0}
        };

        // -------------------------
        // VARIABLES
        // -------------------------
        IloArray<IloBoolVarArray> s(env, nGates);
        IloArray<IloBoolVarArray> t(env, nGates);
        IloArray<IloBoolVarArray> x(env, nGates);

        for (int i = 0; i < nGates; i++) {
            s[i] = IloBoolVarArray(env, 1);
            t[i] = IloBoolVarArray(env, 2);
            x[i] = IloBoolVarArray(env, nRows);
        }

        // -------------------------
        // OBJECTIVE
        // -------------------------
        IloExpr obj(env);
        for (int i = 0; i < nGates; i++)
            obj += s[i][0];

        model.add(IloMinimize(env, obj));
        obj.end();

        // -------------------------
        // CONSTRAINTS
        // -------------------------

        model.add(s[0][0] == 1);

        model.add(x[0][0] == 0);
        model.add(x[0][1] == 1);
        model.add(x[0][2] == 1);
        model.add(x[0][3] == 0);

        for (int i = 0; i < nGates; i++)
            for (int j = 0; j < 2; j++)
                model.add(s[i][0] - t[i][j] >= 0);

        for (int i = 0; i < nGates; i++)
            for (int l = 0; l < nRows; l++)
                model.add(s[i][0] - x[i][l] >= 0);

        for (int i = 0; i < 3; i++) {
            int p1 = predecessor[i][0] - 1;
            int p2 = predecessor[i][1] - 1;

            model.add(s[p1][0] + s[p2][0] + t[i][0] + t[i][1] <= 2);
        }

        for (int i = 0; i < 3; i++) {
            int p1 = predecessor[i][0] - 1;
            int p2 = predecessor[i][1] - 1;

            for (int l = 0; l < nRows; l++) {
                model.add(x[p1][l] + x[i][l] <= 1);
                model.add(x[p2][l] + x[i][l] <= 1);
            }
        }

        for (int i = 0; i < nGates; i++) {
            for (int l = 0; l < nRows; l++) {
                model.add(alpha[l][0] * t[i][0] + x[i][l] <= 1);
                model.add(alpha[l][1] * t[i][1] + x[i][l] <= 1);
            }
        }

        for (int i = 3; i < nGates; i++) {
            for (int l = 0; l < nRows; l++) {
                model.add(
                    alpha[l][0] * t[i][0] +
                    alpha[l][1] * t[i][1] +
                    x[i][l] - s[i][0] >= 0
                );
            }
        }

        for (int i = 0; i < 3; i++) {
            int p1 = predecessor[i][0] - 1;
            int p2 = predecessor[i][1] - 1;

            for (int l = 0; l < nRows; l++) {
                model.add(
                    alpha[l][0] * t[i][0] +
                    alpha[l][1] * t[i][1] +
                    x[p1][l] +
                    x[p2][l] +
                    x[i][l] - s[i][0] >= 0
                );
            }
        }

        // -------------------------
        // SOLVE
        // -------------------------
        IloCplex cplex(model);

        if (cplex.solve()) {

            env.out() << "Objective: " << cplex.getObjValue() << "\n\n";

            // -------------------------
            // s vector
            // -------------------------
            env.out() << "--- s ---\n";
            for (int i = 0; i < nGates; i++) {
                env.out() << "s[" << i + 1 << "] = "
                    << clean(cplex.getValue(s[i][0])) << "\n";
            }

            // -------------------------
            // t vector
            // -------------------------
            env.out() << "\n--- t ---\n";
            for (int i = 0; i < nGates; i++) {
                env.out() << "Gate " << i + 1 << ": ";
                for (int j = 0; j < 2; j++) {
                    env.out() << "t[" << j + 1 << "]="
                        << clean(cplex.getValue(t[i][j])) << " ";
                }
                env.out() << "\n";
            }

            // -------------------------
            // x vector
            // -------------------------
            env.out() << "\n--- x ---\n";
            for (int i = 0; i < nGates; i++) {
                env.out() << "Gate " << i + 1 << ": ";
                for (int l = 0; l < nRows; l++) {
                    env.out() << "x[" << l + 1 << "]="
                        << clean(cplex.getValue(x[i][l])) << " ";
                }
                env.out() << "\n";
            }

        }
        else {
            env.out() << "No solution found\n";
        }

    }
    catch (IloException& e) {
        e.print(env.out());
    }

    env.end();
    return 0;
}