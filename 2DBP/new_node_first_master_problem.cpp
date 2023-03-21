﻿// 2023-03-01
// 列生成求解新生成节点

#include "2DBP.h"
using namespace std;

bool SolveNewNodeFirstMasterProblem(
	All_Values& Values,
	All_Lists& Lists,
	IloEnv& Env_MP,
	IloModel& Model_MP,
	IloObjective& Obj_MP,
	IloRangeArray& Cons_MP,
	IloNumVarArray& Vars_MP,
	Node& this_node,
	Node& parent_node)
{

	int strip_types_num = Values.strip_types_num;
	int item_types_num = Values.item_types_num;

	int J_num = strip_types_num;
	int N_num = item_types_num;

	IloNumArray  con_min(Env_MP);
	IloNumArray  con_max(Env_MP);

	for (int row = 0; row < J_num + N_num; row++)
	{
		if (row < J_num)
		{
			// con >= 0
			con_min.add(IloNum(0)); // con LB
			con_max.add(IloNum(IloInfinity)); // con UB
		}
		if (row >= J_num)
		{
			// con >= item_type demand
			int row_pos = row - J_num;
			double demand_val = Lists.all_item_types_list[row_pos].demand;
			con_min.add(IloNum(demand_val)); // con LB
			con_max.add(IloNum(IloInfinity)); // con UB
		}
	}

	Cons_MP = IloRangeArray(Env_MP, con_min, con_max);
	Model_MP.add(Cons_MP);

	con_min.end();
	con_max.end();

	int K_num = this_node.cutting_stock_cols.size();
	int P_num = this_node.cutting_strip_cols.size();

	int all_cols_num = K_num + P_num;
	int all_rows_num = item_types_num + strip_types_num;

	// Cplex Modeling
	for (int col = 0; col < K_num; col++)
	{
		IloNum obj_para = 1;
		IloNumColumn CplexCol = Obj_MP(obj_para); // Init a col

		for (int row = 0; row < all_rows_num; row++) // set rows in this col
		{
			IloNum row_para = this_node.cutting_stock_cols[col][row];
			CplexCol += Cons_MP[row](row_para); // set para
		}

		string Y_name = "Y_" + to_string(col + 1); // var name

		// Case 1 
		if (col == parent_node.var_to_branch_idx)
		{
			// var of this col is the to be branched - var of Parent Node
			IloNum to_branch_val = this_node.var_to_branch_int_val_final;
			printf("\n\t Y_var_%d is set as %f, to be branched", col + 1, to_branch_val);
			IloNumVar Var_Y(CplexCol, to_branch_val, to_branch_val, ILOFLOAT, Y_name.c_str()); // Init and set var
			Vars_MP.add(Var_Y);
		}

		// Case 2
		else
		{
			// var of this col is not the to be branched - var of Parent Node
			// Case 2.1: var of this col is NOT a branched - var in previous Nodes	
			int branched_num = parent_node.branched_vars_int_val_list.size();
			bool find_flag = 0;

			for (int index = 0; index < branched_num; index++) // loop of all branched-vars in previous Nodes
			{
				int branched_col = parent_node.branched_vars_idx_list[index];
				if (col == branched_col) // var of this col is a branched-var in Parent Node
				{
					IloNum branched_val = parent_node.branched_vars_int_val_list[index];
					printf("\n\t Y_var_%d is set as %f, branched", col + 1, branched_val);
					IloNumVar Var_Y(CplexCol, branched_val, branched_val, ILOFLOAT, Y_name.c_str()); // Init and set var
					Vars_MP.add(Var_Y);

					find_flag = 1;
					break;
				}
			}

			// Case 2.2:
			if (find_flag == 0)
			{
				//  var of this col is NOT a branched-var in previous Nodes
				IloNum var_min = 0;
				IloNum var_max = IloInfinity;
				IloNumVar Var_Y(CplexCol, var_min, var_max, ILOFLOAT, Y_name.c_str()); // Init and set var
				Vars_MP.add(Var_Y);
			}
		}

		CplexCol.end(); // must end this IloNumColumn object
	}

	for (int col = 0; col <  P_num; col++)
	{
		IloNum obj_para = 0;
		IloNumColumn CplexCol = Obj_MP(obj_para); // Init a col

		for (int row = 0; row < all_rows_num; row++) // set rows in this col
		{
			IloNum row_para = this_node.cutting_strip_cols[col][row];
			CplexCol += Cons_MP[row](row_para); // set para
		}

		string X_name = "X_" + to_string(col + 1); // var name

		// Case 1
		if (col+K_num  == parent_node.var_to_branch_idx)
		{
			// var of this col is the to be branched-var of Parent Node
			IloNum to_branch_val = this_node.var_to_branch_int_val_final;
			printf("\n\t X_var_%d is set as %f, to be branched", col + 1, to_branch_val);
			IloNumVar Var_X(CplexCol, to_branch_val, to_branch_val, ILOFLOAT, X_name.c_str()); // Init and set var
			Vars_MP.add(Var_X);
		}

		// Case 2
		else
		{
			// var of this col is not the to be branched - var of Parent Node
			// Case 2.1: var of this col is NOT a branched - var in previous Nodes	
			int branched_num = parent_node.branched_vars_int_val_list.size();
			bool find_flag = 0;

			for (int index = 0; index < branched_num; index++) // loop of all branched-vars in previous Nodes
			{
				int branched_col = parent_node.branched_vars_idx_list[index];
				if (col == branched_col) // var of this col is a branched-var in Parent Node
				{
					IloNum branched_val = parent_node.branched_vars_int_val_list[index];
					printf("\n\t x_var_%d is set as %f, branched", col + 1, branched_val);
					IloNumVar Var_X(CplexCol, branched_val, branched_val, ILOFLOAT, X_name.c_str()); // Init and set var
					Vars_MP.add(Var_X);

					find_flag = 1;
					break;
				}
			}

			// Case 2.2
			if (find_flag == 0)
			{
				// var of this col is NOT a branched-var in previous Nodes
				IloNum var_min = 0;
				IloNum var_max = IloInfinity;
				IloNumVar Var_X(CplexCol, var_min, var_max, ILOFLOAT, X_name.c_str()); // Init and set var
				Vars_MP.add(Var_X);
			}
		}

		CplexCol.end(); // must end this IloNumColumn object
	}

	printf("\n\n################## Node_%d MP-1 CPLEX SOLVING START ##################\n\n", this_node.index);
	IloCplex MP_cplex(Env_MP);
	MP_cplex.extract(Model_MP);
	MP_cplex.exportModel("New Node First Master Problem.lp");
	bool MP_flag = MP_cplex.solve();
	printf("\n################## Node_%d MP-1 CPLEX SOLVING OVER ####################\n\n", this_node.index);

	if (MP_flag == 0)
	{
		this_node.node_pruned_flag = 1;
		printf("\n\t Node_%d MP-1 is NOT FEASIBLE\n", this_node.index);
		printf("\n\t Node_%d MP-1 has to be pruned\n", this_node.index);
	}
	else
	{
		printf("\n\t Node_%d MP-1 is FEASIBLE\n", this_node.index);
		printf("\n\t OBJ of Node_%d MP-1 is %f\n\n", this_node.index, MP_cplex.getValue(Obj_MP));

		int Y_fsb_num = 0;
		int X_fsb_num = 0;
		printf("\n\t Y Solns (stock cutting patterns):\n\n");
		for (int col = 0; col < K_num; col++)
		{
			double soln_val = MP_cplex.getValue(Vars_MP[col]);	
			if (soln_val > 0)
			{
				Y_fsb_num++;
				printf("\t var_Y_%d = %f\n", col + 1, soln_val);
			}
		}

		printf("\n\t X Solns (this_strip cutting patterns):\n\n");
		for (int col = K_num; col < K_num + P_num; col++)
		{
			double soln_val = MP_cplex.getValue(Vars_MP[col]);	
			if (soln_val > 0)
			{
				X_fsb_num++;
				printf("\t var_X_%d = %f\n", col + 1 - K_num, soln_val);
			}
		}

		printf("\n\t BRANCHED VARS: \n\n");
		int branched_num = this_node.branched_vars_int_val_list.size();
		for (int k = 0; k < branched_num; k++)
		{
			printf("\t var_x_%d = %f branched \n",
				this_node.branched_vars_idx_list[k] + 1, this_node.branched_vars_int_val_list[k]);
		}

		this_node.dual_prices_list.clear();

		printf("\n\t strip_type cons dual prices: \n\n");
		for (int row = 0; row < J_num; row++)
		{
			double dual_val = MP_cplex.getDual(Cons_MP[row]);
			printf("\t dual_r_%d = %f\n", row + 1, dual_val);
			this_node.dual_prices_list.push_back(dual_val);
		}

		printf("\n\t item_type cons dual prices: \n\n");
		for (int row = J_num; row < J_num + N_num; row++)
		{
			double dual_val = MP_cplex.getDual(Cons_MP[row]);
			printf("\t dual_r_%d = %f\n", row + 1, dual_val);
			this_node.dual_prices_list.push_back(dual_val);
		}

		printf("\n\t Node_%d MP-1:\n", this_node.index);
		printf("\n\t Lower Bound = %f", MP_cplex.getValue(Obj_MP));
		printf("\n\t NUM of all solns = %d", K_num + P_num);
		printf("\n\t NUM of Y fsb solns = %d", Y_fsb_num);
		printf("\n\t NUM of X fsb solns = %d", X_fsb_num);
		printf("\n\t NUM of all fsb solns = %d", Y_fsb_num + X_fsb_num);
		printf("\n\t NUM of branched-vars = %d\n", branched_num);
	}

	MP_cplex.end();
	return MP_flag;
}