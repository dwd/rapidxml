//
// Created by dave on 30/07/2024.
//

#include "gtest/gtest.h"
#ifdef DWD_GTEST_SENTRY
#include <sentry.h>

class EventListener : public ::testing::TestEventListener {
    sentry_transaction_context_t *tx_ctx = nullptr;
    sentry_transaction_t *tx = nullptr;
    sentry_span_t *main_span = nullptr;
    sentry_span_t *suite_span = nullptr;
    sentry_span_t *test_span = nullptr;
    std::string const & m_progname;
public:
    EventListener(std::string const & progname) : m_progname(progname) {}
    ~EventListener() override = default;

    // Override this to define how to set up the environment.
    void OnTestProgramStart(const ::testing::UnitTest & u) override {
        sentry_options_t *options = sentry_options_new();
        sentry_options_set_traces_sample_rate(options, 1.0);
        sentry_init(options);
    }
    void OnTestProgramEnd(const ::testing::UnitTest &) override {
        sentry_shutdown();
    }

    void OnTestStart(::testing::TestInfo const & test_info) override {
        const char * testName = test_info.name();
        std::string tname = test_info.test_suite_name();
        tname += ".";
        tname += testName;
        test_span = sentry_span_start_child(
                suite_span,
                "test",
                tname.c_str()
        );
    }

    // Override this to define how to tear down the environment.
    void OnTestEnd(const ::testing::TestInfo & ti) override {
        if (ti.result()->Failed()) {
            sentry_span_set_status(test_span, sentry_span_status_t::SENTRY_SPAN_STATUS_INTERNAL_ERROR);
        }
        sentry_span_finish(test_span); // Mark the span as finished
    }

    void OnTestIterationStart(const testing::UnitTest &unit_test, int iteration) override {
        tx_ctx = sentry_transaction_context_new(
                m_progname.c_str(),
                "googletest"
        );
        tx = sentry_transaction_start(tx_ctx, sentry_value_new_null());
        main_span = sentry_transaction_start_child(
                tx,
                "googletest",
                m_progname.c_str()
        );
    }

    void OnEnvironmentsSetUpStart(const testing::UnitTest &unit_test) override {

    }

    void OnEnvironmentsSetUpEnd(const testing::UnitTest &unit_test) override {

    }

    void OnTestSuiteStart(const testing::TestSuite &suite) override {
        suite_span = sentry_span_start_child(
                main_span,
                "test.suite",
                suite.name()
        );
        TestEventListener::OnTestSuiteStart(suite);
    }

    void OnTestCaseStart(const testing::TestCase &aCase) override {
        TestEventListener::OnTestCaseStart(aCase);
    }

    void OnTestDisabled(const testing::TestInfo &info) override {
        TestEventListener::OnTestDisabled(info);
    }

    void OnTestPartResult(const testing::TestPartResult &test_part_result) override {
        sentry_set_span(test_span);
        auto val = sentry_value_new_breadcrumb("test", test_part_result.message());
        sentry_add_breadcrumb(val);
        if (test_part_result.failed()) {
            auto ev = sentry_value_new_event();
            auto exc = sentry_value_new_exception("GoogleTest", test_part_result.message());
            sentry_value_set_stacktrace(exc, nullptr, 0);
            sentry_event_add_exception(ev, exc);
            sentry_capture_event(ev);
        }
    }

    void OnTestSuiteEnd(const testing::TestSuite &suite) override {
        TestEventListener::OnTestSuiteEnd(suite);
        if (suite.failed_test_count() > 0) {
            sentry_span_set_status(suite_span, sentry_span_status_t::SENTRY_SPAN_STATUS_INTERNAL_ERROR);
        }
        sentry_span_finish(suite_span); // Mark the span as finished
    }

    void OnTestCaseEnd(const testing::TestCase &aCase) override {
        TestEventListener::OnTestCaseEnd(aCase);
    }

    void OnEnvironmentsTearDownStart(const testing::UnitTest &unit_test) override {

    }

    void OnEnvironmentsTearDownEnd(const testing::UnitTest &unit_test) override {

    }

    void OnTestIterationEnd(const testing::UnitTest &unit_test, int iteration) override {
        if (unit_test.failed_test_count() > 0) {
            sentry_span_set_status(main_span, sentry_span_status_t::SENTRY_SPAN_STATUS_INTERNAL_ERROR);
            sentry_transaction_set_status(tx, sentry_span_status_t::SENTRY_SPAN_STATUS_INTERNAL_ERROR);
        }
        sentry_span_finish(main_span); // Mark the span as finished
        sentry_transaction_finish(tx);
    }
};
#endif

int main(int argc, char ** argv) {
    std::string progname(argv[0]);
    auto slash = progname.find_last_of("/\\");
    if (slash != std::string::npos) {
        progname = progname.substr(slash + 1);
    }
    ::testing::InitGoogleTest(&argc, argv);
    auto & listeners = ::testing::UnitTest::GetInstance()->listeners();
#ifdef DWD_GTEST_SENTRY
    listeners.Append(new EventListener(progname));
#endif
    auto ret =  RUN_ALL_TESTS();
    return ret;
}
