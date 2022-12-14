/**
 * Real ncurses library calls.
 *
 * The code defined in this file is completely excluded from coverage.
 * During tests, we use stubs found in <project_root>/src/stubs. Those
 * should see and require 100% coverage.
 *
 * Copyright (C) 2022 Kevin Morris <kevr@0cost.org>
 * All Rights Reserved.
 **/
#include "ncurses.hpp"
using namespace tasker;

// LCOV_EXCL_START
WINDOW *ext::ncurses::initscr() noexcept
{
    return (m_root = ::initscr());
}

int ext::ncurses::keypad(WINDOW *win, bool bf) noexcept
{
    return ::keypad(win, bf);
}

int ext::ncurses::raw() noexcept
{
    return ::raw();
}

int ext::ncurses::noecho() noexcept
{
    return ::noecho();
}

int ext::ncurses::getchar() noexcept
{
    return getch();
}

int ext::ncurses::refresh() noexcept
{
    return ::refresh();
}

int ext::ncurses::wrefresh(WINDOW *win) noexcept
{
    return ::wrefresh(win);
}

int ext::ncurses::endwin() noexcept
{
    return ::endwin();
}

WINDOW *ext::ncurses::derwin(WINDOW *parent, int nlines, int ncols,
                             int begin_y, int begin_x) noexcept
{
    return ::derwin(parent, nlines, ncols, begin_y, begin_x);
}

void ext::ncurses::get_max_yx(WINDOW *win, int &y, int &x) noexcept
{
    getmaxyx(win, y, x);
}

int ext::ncurses::delwin(WINDOW *win) noexcept
{
    return ::delwin(win);
}

int ext::ncurses::w_add_str(WINDOW *win, const char *str) noexcept
{
    return waddstr(win, str);
}

int ext::ncurses::wborder(WINDOW *win, chtype ls, chtype rs, chtype ts,
                          chtype bs, chtype tl, chtype tr, chtype bl,
                          chtype br) noexcept
{
    return ::wborder(win, ls, rs, ts, bs, tl, tr, bl, br);
}

int ext::ncurses::werase(WINDOW *win) noexcept
{
    return ::werase(win);
}

int ext::ncurses::wmove(WINDOW *win, int y, int x) noexcept
{
    return ::wmove(win, y, x);
}

int ext::ncurses::curs_set(int visibility) noexcept
{
    return ::curs_set(visibility);
}

int ext::ncurses::start_color() noexcept
{
    return ::start_color();
}

int ext::ncurses::init_pair(short pair, short fg, short bg) noexcept
{
    return ::init_pair(pair, fg, bg);
}

int ext::ncurses::supported_colors() noexcept
{
    return COLORS;
}

bool ext::ncurses::has_colors() noexcept
{
    return ::has_colors();
}

int ext::ncurses::use_default_colors() noexcept
{
    return ::use_default_colors();
}

int ext::ncurses::wattr_enable(WINDOW *win, int attrs) noexcept
{
    return wattron(win, attrs);
}

int ext::ncurses::wattr_disable(WINDOW *win, int attrs) noexcept
{
    return wattroff(win, attrs);
}

WINDOW *ext::ncurses::root() noexcept
{
    return m_root;
}

int ext::ncurses::wbkgd(WINDOW *win, chtype ch) noexcept
{
    return ::wbkgd(win, ch);
}
// LCOV_EXCL_STOP
