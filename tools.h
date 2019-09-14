#ifndef SECULAR_TOOL_H
#define SECULAR_TOOL_H

#include <cmath>
#include <tuple>
#include <vector>
#include <sstream>
namespace secular {

#define STD_ACCESSOR(TYPE, NAME, MEMBER)                                                                                 \
  inline TYPE & NAME () {                                                                                                \
    return MEMBER;                                                                                                       \
  };                                                                                                                     \
  inline TYPE const & NAME () const {                                                                                    \
    return MEMBER;                                                                                                       \
  };

#define READ_GETTER(TYPE, NAME, MEMBER)                                                                                  \
  inline TYPE const & NAME () const {                                                                                    \
    return MEMBER;                                                                                                       \
  };

#define OPT_STD_GETTER(COND, TYPE, NAME, MEMBER)                                                                         \
  inline TYPE & NAME () {                                                                                                \
    static_assert(COND, "method is not defined!");                                                                       \
    return MEMBER;                                                                                                       \
  };                                                                                                                     \
  inline TYPE const & NAME () const {                                                                                    \
    static_assert(COND, "method is not defined!");                                                                       \
    return MEMBER;                                                                                                       \
  };

#define OPT_READ_GETTER(COND, TYPE, NAME, MEMBER)                                                                        \
  inline TYPE const & NAME () const {                                                                                    \
    static_assert(COND, "method is not defined!");                                                                       \
    return MEMBER;                                                                                                       \
  };


#define STD_3WAY_SETTER(NAME, X, Y, Z)                                                                                   \
inline void set_##NAME(double x, double y, double z) {                                                                   \
    X = x, Y = y, Z = z;                                                                                                 \
}                                                                                                                        \
inline void add_##NAME(double x, double y, double z) {                                                                   \
    X += x, Y += y, Z += z;                                                                                              \
}                                                                                                                        \
inline void sub_##NAME(double x, double y, double z) {                                                                   \
    X -= x, Y -= y, Z -= z;                                                                                              \
}                                                                                                                        \
inline void set_##NAME(std::tuple<double, double, double> const& t) {                                                    \
    X = std::get<0>(t), Y = std::get<1>(t), Z = std::get<2>(t);                                                          \
}                                                                                                                        \
inline void add_##NAME(std::tuple<double, double, double> const& t) {                                                    \
    X += std::get<0>(t), Y += std::get<1>(t), Z += std::get<2>(t);                                                       \
}                                                                                                                        \
inline void sub_##NAME(std::tuple<double, double, double> const& t) {                                                    \
    X -= std::get<0>(t), Y -= std::get<1>(t), Z -= std::get<2>(t);                                                       \
}

#define OPT_3WAY_SETTER(COND, NAME, X, Y, Z)                                                                             \
inline void set_##NAME(double x, double y, double z) {                                                                   \
    static_assert(COND, "method is not defined!");                                                                       \
    X = x, Y = y, Z = z;                                                                                                 \
}                                                                                                                        \
inline void add_##NAME(double x, double y, double z) {                                                                   \
    static_assert(COND, "method is not defined!");                                                                       \
    X += x, Y += y, Z += z;                                                                                              \
}                                                                                                                        \
inline void sub_##NAME(double x, double y, double z) {                                                                   \
    static_assert(COND, "method is not defined!");                                                                       \
    X -= x, Y -= y, Z -= z;                                                                                              \
}                                                                                                                        \
inline void set_##NAME(std::tuple<double, double, double> const& t) {                                                    \
    static_assert(COND, "method is not defined!");                                                                       \
    X = std::get<0>(t), Y = std::get<1>(t), Z = std::get<2>(t);                                                          \
}                                                                                                                        \
inline void add_##NAME(std::tuple<double, double, double> const& t) {                                                    \
    static_assert(COND, "method is not defined!");                                                                       \
    X += std::get<0>(t), Y += std::get<1>(t), Z += std::get<2>(t);                                                       \
}                                                                                                                        \
inline void sub_##NAME(std::tuple<double, double, double> const& t) {                                                    \
    static_assert(COND, "method is not defined!");                                                                       \
    X -= std::get<0>(t), Y -= std::get<1>(t), Z -= std::get<2>(t);                                                       \
}

template<typename Iter, size_t ...I>
inline auto _unpack_array_(Iter iter, std::index_sequence<I...>) {
    return std::make_tuple(*(iter+I)...);
}

template<size_t num, typename Iter>
inline auto unpack_args(Iter iter) {
    return _unpack_array_(iter, std::make_index_sequence<num>());
}

template<typename Iter, typename Tup, size_t ...I>
inline auto _cast_unpack_(Iter iter, Tup const& t, std::index_sequence<I...>) {
    return std::make_tuple(static_cast<decltype(std::get<I>(t))>( *(iter+I) )...);
}

template<typename Iter, typename ...Args>
inline auto cast_unpack(Iter iter) {
    return _cast_unpack_(iter, std::make_tuple(static_cast<Args>(0)...), std::make_index_sequence<sizeof...(Args)>());
    //return std::make_tuple(static_cast<Args>(*iter++)...);
}

void unpack_args_from_str(std::string const &str, std::vector<double> &vec, bool DA, size_t spin_num) {
    std::stringstream is{str};

    size_t token_num = 20 + static_cast<size_t>(!DA) + spin_num * 3;

    double tmp;

    vec.reserve(token_num);
    for (size_t i = 0; i < token_num; ++i) {
        is >> tmp;
        vec.emplace_back(tmp);
    }
}

#define UNPACK3(x) std::get<0>(x), std::get<1>(x), std::get<2>(x)

namespace consts {
        constexpr double pi = 3.14159265358979323;
        constexpr double G = 4 * pi * pi;
        constexpr double C = 6.32397263e4;
        constexpr double r_G_sqrt = 1.0 / (2 * pi);

        constexpr double year = 1;
}

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

bool is_on(double x) {
    return x > 5e-15;
}

    template<typename Container>
    struct spin_num {
        static constexpr size_t size{Container::s_num};
    };

    using Tup3d = std::tuple<double, double, double>;

    inline double norm2(double x, double y, double z) {
        return x * x + y * y + z * z;
    }

    inline double norm2(Tup3d const &tup) {
        return norm2(UNPACK3(tup));
    }

    inline double norm(double x, double y, double z) {
        return sqrt(norm2(x, y, z));
    }

    inline double norm(Tup3d const &tup) {
        return sqrt(norm2(tup));
    }

    inline double dot(double x1, double y1, double z1, double x2, double y2, double z2) {
        return x1 * x2 + y1 * y2 + z1 * z2;
    }

    inline double dot(Tup3d const &t1, Tup3d const &t2) {
        return dot(UNPACK3(t1), UNPACK3(t2));
    }

    inline auto cross(double x1, double y1, double z1, double x2, double y2, double z2) {
        return std::make_tuple(y1 * z2 - y2 * z1, z1 * x2 - z2 * x1, x1 * y2 - x2 * y1);
    }

    inline auto cross(Tup3d const &t1, Tup3d const &t2) {
        return cross(UNPACK3(t1), UNPACK3(t2));
    }

    inline auto cross_with_coef(double A, double x1, double y1, double z1, double x2, double y2, double z2) {
        return std::make_tuple(A * (y1 * z2 - y2 * z1), A * (z1 * x2 - z2 * x1), A * (x1 * y2 - x2 * y1));
    }

    inline auto cross_with_coef(double A, Tup3d const &t1, Tup3d const &t2) {
        return cross_with_coef(A, UNPACK3(t1), UNPACK3(t2));
    }

    double calc_angular_mom(double m_in, double m_out, double a) {
        double mu = m_in * m_out / (m_in + m_out);
        return mu * sqrt(consts::G * (m_in + m_out) * a);
    }

    auto unit_j(double i, double Omega) {
        double sini = sin(i);
        double cosi = cos(i);
        double sinO = sin(Omega);
        double cosO = cos(Omega);
        return std::make_tuple(sini * sinO, -sini * cosO, cosi);
    }

    auto unit_e(double i, double omega, double Omega) {
        double sini = sin(i);
        double cosi = cos(i);
        double sinO = sin(Omega);
        double cosO = cos(Omega);
        double sino = sin(omega);
        double coso = cos(omega);

        return std::make_tuple(coso * cosO - sino * cosi * sinO, coso * sinO + sino * cosi * cosO, sino * sini);
    }

    auto unit_peri_v(double i, double omega, double Omega) {
        auto[ex, ey, ez] = unit_e(i, omega, Omega);
        auto[jx, jy, jz] = unit_j(i, Omega);
        return cross(jx, jy, jz, ex, ey, ez);
    }


    template<typename ...Args>
    void deg_to_rad(Args &...args) {
        constexpr double rad = consts::pi / 180.0;
        ((args *= rad), ...);
    }

    inline auto calc_orbit_args(double Coef, double lx, double ly, double lz, double ex, double ey, double ez) {
        double e_sqr = norm2(ex, ey, ez);

        double j_sqr = fabs(1 - e_sqr);

        double j = sqrt(j_sqr);

        double L_norm = norm(lx, ly, lz);

        double L = L_norm / j;

        double a = Coef * L * L;

        return std::make_tuple(e_sqr, j_sqr, j, L_norm, L, a);
    }

    inline auto calc_a_eff(double Coef, double lx, double ly, double lz, double ex, double ey, double ez) {
        double e_sqr = norm2(ex, ey, ez);

        double j_sqr = fabs(1 - e_sqr);

        double j = sqrt(j_sqr);

        double L_sqr = norm2(lx, ly, lz);

        return Coef * L_sqr / j;
    }

    inline auto calc_a_j(double Coef, double lx, double ly, double lz, double ex, double ey, double ez) {
        double e_sqr = norm2(ex, ey, ez);

        double j_sqr = fabs(1 - e_sqr);

        double j = sqrt(j_sqr);

        double L_sqr = norm2(lx, ly, lz);

        return std::make_tuple(Coef * L_sqr / j_sqr, j);
    }

    inline auto calc_a(double Coef, double lx, double ly, double lz, double ex, double ey, double ez) {
        double e_sqr = norm2(ex, ey, ez);

        double j_sqr = fabs(1 - e_sqr);

        double L_sqr = norm2(lx, ly, lz);

        return Coef * L_sqr / j_sqr;
    }

    const std::string str_ave[2] = {":SA", ":DA"};
    const std::string str_pole[2] = {"|quad", "| oct"};
    const std::string str_gr[3] = {"", "|GR_{in}", "|GR_{in}|GR_{out}"};
    const std::string str_gw[2] = {"", "|GW"};
    const std::string str_sl[2] = {"", "|S_{in}L_{out}"};
    const std::string str_ll[2] = {"", "|LL"};
    const std::string str_s[4] = {"", "|S_{1}L_{in}", "|S_{1}L_{in}|S_{2}L_{in}", "|S_{1}L_{in}|S_{2}L_{in}|S_{3}L_{out}"};

    template<typename Controler>
    std::string get_log_title(size_t task_id, bool DA, Controler const &ctrl, size_t spin_num) {
        return std::to_string(task_id) + str_ave[DA] + str_pole[ctrl.Oct] + str_gr[ctrl.GR] + str_gw[ctrl.GW] + str_sl[ctrl.SL] + str_ll[ctrl.LL] +
               str_s[spin_num];
    }
}
#endif
