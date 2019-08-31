#ifndef RELATIVISTIC_H
#define RELATIVISTIC_H

#include "tools.h"

namespace secular {

    class GRConst {
    public:
        GRConst(double M, double Mu) {
            constexpr double C5 = consts::C * consts::C * consts::C * consts::C * consts::C;
            constexpr double G3 = consts::G * consts::G * consts::G;
            GR_coef_ = 3 * consts::G / (consts::C * consts::C) * M / Mu;//3 * pow(consts::G*M, 1.5) / (consts::C * consts::C);
            GW_L_coef_ = -6.4 * G3 * Mu * M * M / C5;
            GW_e_coef_ = -304.0 / 15 * G3 * Mu * M * M / C5;
        }

        GRConst() = default;

        READ_GETTER(double, GR_coef, GR_coef_);

        READ_GETTER(double, GW_L_coef, GW_L_coef_);

        READ_GETTER(double, GW_e_coef, GW_L_coef_);
    private:
        double GR_coef_{0};
        double GW_L_coef_{0};
        double GW_e_coef_{0};
    };

#define GR_PROCESS(ACOEF, COEF, num) {                                                                                                                       \
    double a_eff = calc_a_eff(args.ACOEF, var.L##num##x(), var.L##num##y(), var.L##num##z(), var.e##num##x(), var.e##num##y(), var.e##num##z());\
    double Omega = args.COEF / (a_eff * a_eff * a_eff);                                                                                               \
    dvar.add_e##num(cross_with_coef(Omega, var.L##num##x(), var.L##num##y(), var.L##num##z(), var.e##num##x(), var.e##num##y(), var.e##num##z()));    \
}

    template<typename Args, typename Container, bool DA, size_t Orbit>
    inline void GR_precession(Args const &args, Container const &var, Container &dvar, double t) {
        /*double a_eff = calc_a_eff(args.a_in_coef(), var.L1x(), var.L1y(), var.L1z(), var.e1x(), var.e1y(), var.e1z());

        double Omega = args.GR_in_coef() / (a_eff * a_eff * a_eff);

        dvar.add_e1(cross_with_coef(Omega, var.L1x(), var.L1y(), var.L1z(), var.e1x(), var.e1y(), var.e1z()));

        /*if(DA){
            double a_out_eff = calc_a_eff(args.a_out_coef(), var.L2x(), var.L2y(), var.L2z(), var.e2x(), var.e2y(), var.e2z());

            double Omega_out = args.GR_out_coef() / (a_out_eff * a_out_eff * a_out_eff);

            dvar.add_e2(cross_with_coef(Omega_out, var.L2x(), var.L2y(), var.L2z(), var.e2x(), var.e2y(), var.e2z()));
        } else {

        }*/
        if constexpr(Orbit>0) {
            GR_PROCESS(a_in_coef(), GR_in_coef(), 1);
        }
        
        if constexpr(Orbit>1) {
            if constexpr(DA) {
                GR_PROCESS(a_out_coef(), GR_out_coef(), 2);
            } else{
              
            }
        }
    }

    template<typename Args, typename Container>
    inline void GW_radiation(Args const &args, Container const &var, Container &dvar, double t) {
        auto[e1_sqr, j1_sqr, j1, L1_norm, L_in, a_in] = calc_orbit_args(args.a_in_coef(), var.L1x(), var.L1y(), var.L1z(), var.e1x(), var.e1y(),
                                                                        var.e1z());

        double a_eff = a_in * j1;

        double a_eff2 = a_eff * a_eff;

        double a_eff4 = a_eff2 * a_eff2;

        double GW_L_coef = args.GW_L_in_coef() / a_eff4 / j1 * (1 + 0.875 * e1_sqr);

        double GW_e_coef = args.GW_e_in_coef() / a_eff4 / j1 * (1 + 121.0 / 304 * e1_sqr);

        dvar.add_L1(GW_L_coef * var.L1x(), GW_L_coef * var.L1y(), GW_L_coef * var.L1z());

        dvar.add_e1(GW_e_coef * var.e1x(), GW_e_coef * var.e1y(), GW_e_coef * var.e1z());
    }
} // namespace secular
#endif
