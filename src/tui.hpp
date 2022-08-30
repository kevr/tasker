#ifndef SRC_TUI_HPP
#define SRC_TUI_HPP

#include "config/config.hpp"
#include "errors.hpp"
#include "tui/pane.hpp"
#include "tui/project.hpp"
#include "tui/window.hpp"
#include "utility.hpp"
#include <fmt/format.h>
#include <stdexcept>
#include <tuple>

namespace tasker::tui
{

/**
 * @brief Textual User Interface
 *
 * This class takes the responsibility of running tasker's TUI.
 * It owns the memory location of the root window and children.
 *
 * @tparam CI Curses Interface (e.g. tasker::ext::ncurses)
 **/
template <typename CI>
class tui
{
private:
    // curses library interface, bound on construction
    CI &ncurses;

    std::shared_ptr<root_window<CI>> root;

    std::shared_ptr<pane<CI>> m_pane;
    std::shared_ptr<project<CI>> m_project;

    // `tui` state
    int m_return_code = 0;
    bool m_created = false;
    bool m_ended = true;

public:
    tui(CI &ncurses) noexcept
        : ncurses(ncurses)
        , root(std::make_shared<root_window<CI>>(ncurses))
        , m_pane(std::make_shared<pane<CI>>(ncurses, root))
        , m_project(std::make_shared<project<CI>>(ncurses, m_pane))
    {
    }

    tui(tui &&o) noexcept = default;
    tui &operator=(tui &&o) noexcept = default;
    tui(const tui &o) noexcept = default;
    tui &operator=(const tui &o) noexcept = default;

    ~tui() noexcept
    {
        end();
    }

    tui &init() noexcept
    {
        if (!m_ended || m_created) {
            return *this;
        }
        m_ended = false;
        m_created = true;

        if (auto rc = root->init()) {
            m_return_code = rc;
            return *this;
        }

        if (auto rc = ncurses.keypad(root->handle(), true)) {
            m_return_code = error(ERROR_KEYPAD, "keypad(...) failed: ", rc);
            return *this;
        }

        if (auto rc = ncurses.raw()) {
            m_return_code = error(ERROR_RAW, "raw() failed: ", rc);
            return *this;
        }

        if (auto rc = ncurses.noecho()) {
            m_return_code = error(ERROR_ECHO, "noecho() failed: ", rc);
            return *this;
        }

        // Set a border on `root`.
        ncurses.wborder(root->handle(),
                        ACS_VLINE,
                        ACS_VLINE,
                        ACS_HLINE,
                        ACS_HLINE,
                        ACS_ULCORNER,
                        ACS_URCORNER,
                        ACS_LLCORNER,
                        ACS_LRCORNER);

        m_pane->inherit();  // Update sizes relative to the root
        m_pane->padding(1); // Set a padding of 1
        if (auto rc = m_pane->init()) {
            m_return_code = error(rc, "m_pane->init() failed: ", rc);
            return *this;
        }

        m_project->inherit();
        if (auto rc = m_project->init()) {
            m_return_code = error(rc, "m_project->init() failed: ", rc);
            return *this;
        }

        auto &conf = cfg::config::ref();
        auto key_quit = conf.get<char>("key_quit");

        auto message = fmt::format("Press '{0}' to quit...", key_quit);
        if (auto rc =
                ncurses.w_add_str(m_project->handle(), message.c_str())) {
            m_return_code = error(ERROR_WADDSTR, "waddstr() failed: ", rc);
            return *this;
        }

        return *this;
    }

    int refresh() noexcept
    {
        return root->refresh_all();
    }

    int return_code() const noexcept
    {
        return m_return_code;
    }

    std::shared_ptr<root_window<CI>> window() const
    {
        return root;
    }

    std::tuple<int, int> dimensions() const
    {
        return root->dimensions();
    }

    operator bool() const noexcept
    {
        return m_return_code == 0;
    }

    int end() noexcept
    {
        if (!m_created || m_ended) {
            return m_return_code;
        }
        m_ended = true;
        m_created = false;

        m_pane->end();

        // Just destruct the root window in all cases; in the case where we
        // error out after running initscr(), this will be able to undo what
        // initscr() did if it can.
        auto rc = root->end();

        // Only update m_return_code if it's not already errored out.
        if (m_return_code == 0)
            m_return_code = rc;

        return return_code();
    }
};

}; // namespace tasker::tui

#endif /* SRC_TUI_HPP */
