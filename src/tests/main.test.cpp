#define main main_real
#include "main.cpp"
#undef main

#include "mocks/ncurses.hpp"
#include "utility.hpp"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class main_test : public ::testing::Test
{
protected:
    std::string tmpdir;

protected:
    void write_config(const std::map<std::string, std::string> &options)
    {
        std::filesystem::path p(tmpdir);
        p /= "config";
        {
            std::ofstream ofs(p.c_str(), std::ios::out);
            for (auto &kv : options) {
                ofs << kv.first << " = " << kv.second << std::endl;
            }
            ofs << std::endl;
            ofs.close();
        }
    }

public:
    void SetUp() override
    {
        cfg::config::new_ref();
        tmpdir = test::make_temp_directory();
    }

    void TearDown() override
    {
        std::filesystem::remove_all(tmpdir);
    }
};

TEST_F(main_test, initscr_fails)
{
    tasker::ext::mock_ncurses ncurses;
    EXPECT_CALL(ncurses, initscr()).WillOnce(Return(nullptr));

    const char *_argv[] = { PROG.c_str(), nullptr };
    auto argv = const_cast<char **>(_argv);

    auto rc = tasker_main(ncurses, 1, argv);
    ASSERT_EQ(rc, ERROR_INITSCR);
}

TEST_F(main_test, getmaxyx_fails)
{
    tasker::ext::mock_ncurses ncurses;

    WINDOW win;
    EXPECT_CALL(ncurses, initscr()).WillOnce(Return(&win));
    EXPECT_CALL(ncurses, get_max_yx(_, _, _))
        .WillOnce(Invoke([](WINDOW *, int &y, int &x) {
            x = y = -1;
        }));
    EXPECT_CALL(ncurses, endwin()).WillOnce(Return(0));

    const char *_argv[] = { PROG.c_str(), nullptr };
    auto argv = const_cast<char **>(_argv);

    auto rc = tasker_main(ncurses, 1, argv);
    ASSERT_EQ(rc, ERROR_GETMAXYX);
}

TEST_F(main_test, keypad_fails)
{
    tasker::ext::mock_ncurses ncurses;
    tasker::ext::ncurses stub;

    WINDOW win;
    EXPECT_CALL(ncurses, initscr()).WillOnce(Return(&win));
    EXPECT_CALL(ncurses, get_max_yx(_, _, _))
        .WillOnce(Invoke([&stub](WINDOW *win, int &y, int &x) {
            stub.get_max_yx(win, y, x);
        }));
    EXPECT_CALL(ncurses, keypad(_, _)).WillOnce(Return(ERR));
    EXPECT_CALL(ncurses, endwin()).WillOnce(Return(0));

    const char *_argv[] = { PROG.c_str(), nullptr };
    auto argv = const_cast<char **>(_argv);

    auto rc = tasker_main(ncurses, 1, argv);
    ASSERT_EQ(rc, ERROR_KEYPAD);
}

TEST_F(main_test, raw_fails)
{
    tasker::ext::mock_ncurses ncurses;
    tasker::ext::ncurses stub;

    WINDOW win;
    EXPECT_CALL(ncurses, initscr()).WillOnce(Return(&win));
    EXPECT_CALL(ncurses, get_max_yx(_, _, _))
        .WillOnce(Invoke([&stub](WINDOW *win, int &y, int &x) {
            stub.get_max_yx(win, y, x);
        }));
    EXPECT_CALL(ncurses, keypad(_, _)).WillOnce(Return(OK));
    EXPECT_CALL(ncurses, raw()).WillOnce(Return(ERR));
    EXPECT_CALL(ncurses, endwin()).WillOnce(Return(0));

    const char *_argv[] = { PROG.c_str(), nullptr };
    auto argv = const_cast<char **>(_argv);

    auto rc = tasker_main(ncurses, 1, argv);
    ASSERT_EQ(rc, ERROR_RAW);
}

TEST_F(main_test, noecho_fails)
{
    tasker::ext::mock_ncurses ncurses;
    tasker::ext::ncurses stub;

    WINDOW win;
    EXPECT_CALL(ncurses, initscr()).WillOnce(Return(&win));
    EXPECT_CALL(ncurses, get_max_yx(_, _, _))
        .WillOnce(Invoke([&stub](WINDOW *win, int &y, int &x) {
            stub.get_max_yx(win, y, x);
        }));
    EXPECT_CALL(ncurses, keypad(_, _)).WillOnce(Return(OK));
    EXPECT_CALL(ncurses, raw()).WillOnce(Return(OK));
    EXPECT_CALL(ncurses, noecho()).WillOnce(Return(ERR));
    EXPECT_CALL(ncurses, endwin()).WillOnce(Return(0));

    const char *_argv[] = { PROG.c_str(), nullptr };
    auto argv = const_cast<char **>(_argv);

    auto rc = tasker_main(ncurses, 1, argv);
    ASSERT_EQ(rc, ERROR_ECHO);
}

TEST_F(main_test, runs)
{
    const char *_argv[] = { PROG.c_str(), nullptr };
    auto argv = const_cast<char **>(_argv);

    auto rc = main_real(1, argv);
    ASSERT_EQ(rc, SUCCESS);
}

TEST_F(main_test, help)
{
    testing::internal::CaptureStdout();

    const char *_argv[] = { PROG.c_str(), "--help", nullptr };
    auto argv = const_cast<char **>(_argv);

    auto rc = main_real(2, argv);
    ASSERT_EQ(rc, SUCCESS);

    auto output = testing::internal::GetCapturedStdout();
    auto lines = split(output, '\n');
    auto &conf = cfg::config::ref();
    ASSERT_EQ(lines[0], "usage: " + conf.usage());
    ASSERT_EQ(lines[1], "");
    ASSERT_EQ(lines[2], "Program options:");
    ASSERT_NE(lines[3].find("-h [ --help ]"), std::string::npos);
    ASSERT_NE(lines[4].find("-v [ --version ]"), std::string::npos);
    ASSERT_NE(lines[5].find("-c [ --config ] arg"), std::string::npos);
    ASSERT_EQ(lines[6], "");
}

TEST_F(main_test, version)
{
    testing::internal::CaptureStdout();

    const char *_argv[] = { PROG.c_str(), "--version", nullptr };
    auto argv = const_cast<char **>(_argv);

    auto rc = main_real(2, argv);
    ASSERT_EQ(rc, SUCCESS);

    auto output = strip(testing::internal::GetCapturedStdout(), '\n');
    ASSERT_EQ(output, VERSION);
}

TEST_F(main_test, custom_config)
{
    std::map<std::string, std::string> options;
    write_config(options);

    std::filesystem::path path(tmpdir);
    path /= "config";

    const char *_argv[] = { PROG.c_str(), "--config", path.c_str(), nullptr };
    auto argv = const_cast<char **>(_argv);
    int argc = 3;

    auto rc = main_real(argc, argv);
    ASSERT_EQ(rc, SUCCESS);
}

TEST_F(main_test, custom_config_unknown_option)
{
    std::cout << "HOME=" << env::variable("HOME") << std::endl;
    std::map<std::string, std::string> options;
    options["fake-option"] = "blahblah";
    write_config(options);

    std::filesystem::path path(tmpdir);
    path /= "config";

    const char *_argv[] = { PROG.c_str(), "--config", path.c_str(), nullptr };
    auto argv = const_cast<char **>(_argv);
    int argc = 3;

    auto rc = main_real(argc, argv);
    ASSERT_EQ(rc, ERR);
}
