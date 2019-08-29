
#include <iostream>
#include "secular.h"
#include "observer.h"
#include "SpaceHub/src/multi-thread/multi-thread.hpp"
#include "SpaceHub/src/tools/config-reader.hpp"
#include "SpaceHub/src/tools/timer.hpp"
#include "boost/numeric/odeint.hpp"
#include <iomanip>
#include <cstdlib>
#include <algorithm>

using namespace space::multiThread;

double INT_ERROR = 1e-13;

bool get_line(std::fstream &is, std::string &str) {
    std::getline(is, str);
    if (!is)
        return false;
    else
        return true;
}

auto resolve_sim_type(std::string const &line) {
    bool in_space = true;
    size_t token_num = 0;
    for (auto c : line) {
        if (std::isspace(c)) {
            in_space = true;
        } else if (in_space) {
            token_num++;
            in_space = false;
        }
    }
    constexpr static bool single_average{false};
    constexpr static bool double_average{true};

    size_t id = 0;

    if (token_num > 0) {
        id = std::stoi(line);
    } else {
        return std::make_tuple(id, double_average, 0u);
    }

    switch (token_num) {
        case 20 :
            return std::make_tuple(id, double_average, 0u);
        case 21 :
            return std::make_tuple(id, single_average, 0u);
        case 23 :
            return std::make_tuple(id, double_average, 1u);
        case 24 :
            return std::make_tuple(id, single_average, 1u);
        case 26 :
            return std::make_tuple(id, double_average, 2u);
        case 27 :
            return std::make_tuple(id, single_average, 2u);
        case 29 :
            return std::make_tuple(id, double_average, 3u);
        case 30 :
            return std::make_tuple(id, single_average, 3u);
        default :
            return std::make_tuple(static_cast<size_t>(0), single_average, 0u);
    }
}

constexpr size_t CTRL_OFFSET = 3;
constexpr size_t ARGS_OFFSET = 8;

enum class ReturnFlag {
    input_err, max_iter, finish
};

auto args_analy(size_t& start_id, size_t& end_id, std::string const&f){
    if(start_id <=0 || end_id <=0) {
        std::cout << "task id cannot be smaller than 1!\r\n";
        exit(0);
    }

    if(start_id > end_id)
      std::swap(start_id, end_id);

    size_t task_num = end_id - start_id + 1;

    size_t thread_num = std::min(task_num, space::multiThread::auto_thread);

    return std::make_tuple(task_num, thread_num);
}

template<size_t spin_num>
auto call_ode_int(std::string work_dir, ConcurrentFile output, bool DA, secular::Controler const &ctrl, std::vector<double> const &init_args) {
    using namespace boost::numeric::odeint;

    using Container = secular::SecularArray<spin_num>;
    //using stepper_type = boost::numeric::odeint::runge_kutta_fehlberg78<Container>;
    using stepper_type = bulirsch_stoer<Container>;

    auto  [task_id, t_end, out_dt] = secular::cast_unpack<decltype(init_args.begin()), size_t, double, double>(init_args.begin());

    //space::display(std::cout, task_id, t_end, out_dt);

    auto const [m1, m2, m3, a_in_init] = secular::unpack_args<4>(init_args.begin() + ARGS_OFFSET);

    std::fstream f_out;

    if (secular::is_on(out_dt)) {
        f_out.open(work_dir + "output_" + std::to_string(task_id) + ".txt", std::fstream::out);
        f_out << std::setprecision(12);
    }

    Container data;

    initilize_orbit_args(DA, spin_num, data, init_args.begin() + ARGS_OFFSET);

    secular::SecularConst<spin_num> const_parameters{m1, m2, m3};

    double dt = 0.1 * secular::consts::year;

    double time = 0;

    stepper_type stepper{INT_ERROR, INT_ERROR};

    secular::Stream_observer writer{f_out, out_dt};

    secular::SMA_Determinator stop{const_parameters.a_in_coef(), ctrl.stop_a_in()};

    //auto func = secular::Dynamic_dispatch<Container>(ctrl, const_parameters);
    writer(data, time);

    STATIC_DISPATH(ctrl, const_parameters,

    for( ;time <= t_end && !stop(data, time); ) {
        constexpr size_t max_attempts = 500;

        controlled_step_result res = success;
        size_t trials = 0;
        do{
            res = stepper.try_step(func, data, time, dt);
            trials++;
        } while((res == fail) && (trials < max_attempts));

        if(trials == max_attempts){
            return ReturnFlag::max_iter;
        }
        writer(data, time);
    }
    )

    output << PACK(task_id, ' ', time, ' ', data, "\r\n");

    return ReturnFlag::finish;
}

void single_thread_job(std::string work_dir, ConcurrentFile input, size_t start_id, size_t end_id, ConcurrentFile output, ConcurrentFile log) {
    std::string entry;
    for (;input.execute(get_line, entry);) {
            auto[task_id, DA, spin_num] = resolve_sim_type(entry);

            if (start_id <= task_id && task_id <= end_id) {
                std::vector<double> v;

                secular::unpack_args_from_str(entry, v, DA, spin_num);

                secular::Controler ctrl{v.begin() + CTRL_OFFSET, DA};

                log << secular::get_log_title(task_id, DA, ctrl, spin_num) + "\r\n";

                ReturnFlag res;
                if (spin_num == 0) {
                    res = call_ode_int<0>(work_dir, output, DA, ctrl, v);
                } else if (spin_num == 1) {
                    res = call_ode_int<1>(work_dir, output, DA, ctrl, v);
                } else if (spin_num == 2) {
                    res = call_ode_int<2>(work_dir, output, DA, ctrl, v);
                } else if (spin_num == 3) {
                    res = call_ode_int<3>(work_dir, output, DA, ctrl, v);
                }

                if(res == ReturnFlag::max_iter)
                    log << std::to_string(task_id) + ":Max iteration number reaches!\n";
            }
    }
}

int main(int argc, char **argv) {
    std::ios::sync_with_stdio(false);

    std::string input_file_name;
    std::string work_dir;
    size_t start_task_id, end_task_id;
    std::string user_specified_core_num;

    space::tools::read_command_line(argc, argv, user_specified_core_num, input_file_name, start_task_id, end_task_id, work_dir);

    auto [task_num, thread_num] = args_analy(start_task_id, end_task_id, input_file_name);

    if(user_specified_core_num != "auto") {
        if(secular::is_number(user_specified_core_num)){
            thread_num = std::stoi(user_specified_core_num);
        } else {
            std::cout << "wrong format of the first argument(cpu core number)!\n";
            exit(0);
        }
    }

    const int dir_err = system(("mkdir " + work_dir).c_str());
    if (dir_err == -1) {
        std::cout << "Error creating directory!\n";
        return 0;
    }

    work_dir += "/";

    auto input_file = make_thread_safe_fstream(input_file_name, std::fstream::in);

    auto output_file = make_thread_safe_fstream(work_dir + "last_state.txt", std::fstream::out);

    auto log_file = make_thread_safe_fstream(work_dir + "log.txt", std::fstream::out);

    std::cout << task_num << " tasks in total will be processed. \n" << thread_num << " threads will be created for computing!\n";

    space::tools::Timer timer;
    timer.start();
    space::multiThread::multi_thread_run(thread_num, single_thread_job, work_dir, input_file, start_task_id, end_task_id, output_file, log_file);
    std::cout << "\r\n Time:" << timer.get_time() << " s\n";
    return 0;
}
