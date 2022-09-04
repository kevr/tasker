/**
 * Description: Main program entry-point.
 *
 * Copyright (C) 2022 Kevin Morris <kevr@0cost.org>
 * All Rights Reserved.
 **/
#include "callback.hpp"
#include "config.hpp"
#include "config/config.hpp"
#include "context.hpp"
#include "env.hpp"
#include "logging.hpp"
#include "ncurses.hpp"
#include "tui.hpp"
#include "utility.hpp"
#include <chrono>
#include <iostream>
#include <thread>
using namespace tasker;

static logger logging;
static std::ofstream ofs;

int tasker_main(ext::ncurses &ncurses, int argc, char *argv[])
{
    // Parse command line arguments and handle them.
    auto &conf = cfg::config::ref();
    namespace po = boost::program_options;
    conf.option("debug,d", "enable debug logging");
    conf.option("logfile,l",
                po::value<std::string>(),
                "designate a log file instead of stderr");

    conf.parse_args(argc, argv);

    if (conf.exists("help")) {
        std::cout << "usage: " << conf.usage() << std::endl
                  << conf << std::endl;
        return OK;
    }

    if (conf.exists("version")) {
        std::cout << VERSION << std::endl;
        return OK;
    }

    std::optional<std::filesystem::path> path;

    if (conf.exists("config")) {
        path = conf["config"];
    } else {
        path = env::search_config_path();
    }

    if (path.has_value()) {
        try {
            conf.parse_config(path.value());
        } catch (boost::program_options::unknown_option &e) {
            std::cerr << "error: " << e.what() << " found in "
                      << path.value().string() << std::endl;
            return ERR;
        }
    }

    if (conf.exists("debug")) {
        logger::set_debug(true);
    }

    if (conf.exists("logfile")) {
        auto logfile = conf["logfile"];
        ofs.open(logfile.c_str(), std::ios::out | std::ios::app);
        logger::stream(ofs);
    } else {
        // Use default log file 'tasker.log'
        ofs.open("tasker.log", std::ios::out | std::ios::app);
        logger::stream(ofs);
    }

    // Construct and initialize the TUI
    logging.info("===== BEGIN SESSION =====");
    logging.debug(LOGTRACE());
    tui::tui term(ncurses);
    if (!term.init())
        return term.end();

    auto message =
        fmt::format("supported colors: {0}", ncurses.supported_colors());
    logging.info(LOG(message));

    // Refresh the TUI
    term.refresh();

    // TODO: We need a real solution for keybinds that can
    // be shared across windows.
    //
    // Perhaps... we can make the root accessible to all windows,
    // and the root could hold a ref/ptr to a keybind handler
    // which we can then call up to from here.
    tasker::context<int> ctx;
    ctx.bind_keys(ctx, conf);

    auto resize = [&term]() {
        term.resize();
        term.refresh();
    };
    ctx.keybinds[KEY_RESIZE] = resize;

    // TUI input logic, wait-state until quit key is pressed
    int ch;
    while (ctx && (ch = ncurses.getchar())) {
        if (ctx.keybind_exists(ch)) {
            ctx.call_keybind(ch);
        }
    }

    // End the TUI
    auto rc = term.end();

    // Restore logger pointer and close any open file stream
    logging.info("===== END SESSION =====");
    logger::reset();
    ofs.close();

    return rc;
}

int main(int argc, char *argv[])
{
    auto ncurses = tasker::ext::ncurses();
    return tasker_main(ncurses, argc, argv);
}
