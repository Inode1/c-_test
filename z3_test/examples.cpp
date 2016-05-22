#include <iostream>

#include <z3++.h>

#include "md5.h"

#define MAKE_VECTOR(X) c.bv_val((static_cast<signed>(X)), 32)

using namespace z3;

int main(int argc, char* argv[])
{
/*    std::cout << "bitvector example 1\n";
    context c;
    //if (k==NULL) k= calcKs(kspace);
    expr A = c.bv_const("A", 32);
    expr B = c.bv_const("B", 32);
    expr C = c.bv_const("C", 32);
    expr D = c.bv_const("D", 32);


    //little endian to biendian


    
    expr conjecture2 = conjecture + MAKE_VECTOR(h0[0]) + MAKE_VECTOR(k[0]) + A;
    unsigned int amt = 7;
    expr conjecture3  = to_expr(c, Z3_mk_rotate_left(c, amt, conjecture2));

    solver s(c);
    s.add(MAKE_VECTOR(result[1]) == MAKE_VECTOR(h0[1]) + MAKE_VECTOR(h0[1]) + conjecture3);
    //s.add(A + 5 == 10);
    std::cout << "conjecture:\n" << s << "\n";
    std::cout << s.check() << std::endl;
    if (s.check() == unsat) {
        std::cout << "failed to proved" << "\n";
    }
    else {
        std::cout << "prove" << "\n";
        std::cout << "counterexample:\n" << s.get_model() << "\n";
    }*/

    context c;
    const unsigned N = 16;

    unsigned kspace[64];
    unsigned* k = calcKs(kspace);

    for (int i = 0; i < sizeof result; ++i)
    {
        result[i] = ((result[i] >> 24 )& 0xff) | // move byte 3 to byte 0
                    ((result[i] << 8 ) & 0xff0000) | // move byte 1 to byte 2
                    ((result[i] >> 8)  & 0xff00) | // move byte 2 to byte 1
                    ((result[i] << 24) & 0xff000000); // byte 0 to byte 3
    }

    expr_vector A(c);
    expr_vector B(c);
    expr_vector C(c);
    expr_vector D(c);

    expr_vector M(c);

    for (unsigned i = 0; i < N; i++) 
    { 
        std::stringstream x_name; 
        x_name << "A_" << i;
        A.push_back(c.bv_const(x_name.str().c_str(), 32));
        x_name.str("");
        x_name << "B_" << i;
        B.push_back(c.bv_const(x_name.str().c_str(), 32));
        x_name.str("");
        x_name << "C_" << i;
        C.push_back(c.bv_const(x_name.str().c_str(), 32));
        x_name.str(""); 
        x_name << "D_" << i;
        D.push_back(c.bv_const(x_name.str().c_str(), 32));
        x_name.str(""); 
        x_name << "M_" << i;
        D.push_back(c.bv_const(x_name.str().c_str(), 32));
    }

    solver s(c);

    s.add(A[0] == MAKE_VECTOR(h0[0]));
    s.add(B[0] == MAKE_VECTOR(h0[1]));
    s.add(C[0] == MAKE_VECTOR(h0[2]));
    s.add(D[0] == MAKE_VECTOR(h0[3]));

    s.add(A[15] + A[0] == MAKE_VECTOR(result[0]));
    s.add(B[15] + B[0] == MAKE_VECTOR(result[1]));
    s.add(C[15] + C[0] == MAKE_VECTOR(result[2]));
    s.add(D[15] + D[0] == MAKE_VECTOR(result[3]));

    for (unsigned i = 1; i < N; i++) 
    {
        s.add(A[i] == D[i-1]);
        s.add(C[i] == B[i-1]);
        s.add(D[i] == C[i-1]);
        int rotate = rot0[(i - 1) % 4];

        expr conjecture = (MAKE_VECTOR(B[i-1]) & MAKE_VECTOR(C[i-1])) | 
                          ((~MAKE_VECTOR(B[i-1])) & MAKE_VECTOR(D[i-1]));

        //expr conjecture2 = conjecture + A[i-1] + M[i-1] + MAKE_VECTOR(k[i]);
        s.add(B[i] == B[i-1] + to_expr(c, Z3_mk_rotate_left(c, rotate, conjecture)));
    }
    std::cout << s << "\n" << "solving...\n" << s.check() << "\n";
    model m = s.get_model();
    std::cout << "solution\n" << m;

}