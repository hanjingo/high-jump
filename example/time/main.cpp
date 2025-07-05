#include <iostream>

#include <libcpp/time/time.hpp>

int main(int argc, char* argv[])
{
    std::cout << "now().string() = " << libcpp::date_time::now().string() << std::endl;

    std::cout << "format(now()) = " << libcpp::date_time::format(libcpp::date_time::now()) << std::endl;
    std::cout << "format(now(), \"%Y%m%d %H%M%S\") = "
              << libcpp::date_time::format(libcpp::date_time::now(), "%Y%m%d %H%M%S") << std::endl;

    std::cout << "today() = " << libcpp::date_time::today().string() << std::endl;

    std::cout << "parse(\"2023-03-16 07:41:29\").string(\"%Y%m%d %H%M%S\") = "
              << libcpp::date_time::parse("2023-03-16 07:41:29").string() << std::endl;

    std::cout << "MonthDayCount(2023, 3) = "
              << libcpp::date_time::MonthDayCount(2023, 3) << std::endl;

    std::cout << "date_time(2023, 3, 17).is_null() = " << libcpp::date_time(2023, 3, 17).is_null() << std::endl;

    std::cout << "date_time(libcpp::date_time::now().time()).string() = "
              << libcpp::date_time(libcpp::date_time::now().time()).string() << std::endl;

    std::cout << "now().seconds() = " << libcpp::date_time::now().seconds() << std::endl;

    std::cout << "parse(\"2023-12-13 18:50:00\").seconds_to(parse(\"2023-12-13 18:51:00\")) = " 
              << libcpp::date_time::parse("2023-12-13 18:50:00").seconds_to(libcpp::date_time::parse("2023-12-13 18:51:00")) << std::endl;

    std::cout << "now().minute() = " << libcpp::date_time::now().minute() << std::endl;

    std::cout << "parse(\"2023-12-13 19:50:00\").minutes_to(parse(\"2023-12-13 18:50:00\")) = " 
              << libcpp::date_time::parse("2023-12-13 19:50:00").minutes_to(libcpp::date_time::parse("2023-12-13 18:50:00")) << std::endl;

    std::cout << "now().hour() = " << libcpp::date_time::now().hour() << std::endl;

    std::cout << "parse(\"2023-12-14 18:50:00\").hours_to(parse(\"2023-12-13 18:50:00\")) = " 
              << libcpp::date_time::parse("2023-12-14 18:50:00").hours_to(libcpp::date_time::parse("2023-12-13 18:50:00")) << std::endl;

    std::cout << "now().day() = " << libcpp::date_time::now().day() << std::endl;

    std::cout << "parse(\"2024-01-13 18:50:00\").days_to(parse(\"2023-12-13 18:50:00\")) = " 
              << libcpp::date_time::parse("2024-01-13 18:50:00").days_to(libcpp::date_time::parse("2023-12-13 18:50:00")) << std::endl;

    std::cout << "now().day_of_week() = " << libcpp::date_time::now().day_of_week() << std::endl;

    std::cout << "now().day_of_month() = " << libcpp::date_time::now().day_of_month() << std::endl;

    std::cout << "now().day_of_year() = " << libcpp::date_time::now().day_of_year() << std::endl;

    std::cout << "now().month() = " << libcpp::date_time::now().month() << std::endl;

    std::cout << "now().year() = " << libcpp::date_time::now().year() << std::endl;

    std::cout << "now().start_of_day().string() = " << libcpp::date_time::now().start_of_day().string() << std::endl;

    std::cout << "now().end_of_day().string() = " << libcpp::date_time::now().end_of_day().string() << std::endl;

    std::cout << "now().start_of_week().string() = " << libcpp::date_time::now().start_of_week().string() << std::endl;

    std::cout << "now().end_of_week().string() = " << libcpp::date_time::now().end_of_week().string() << std::endl;

    std::cout << "now().start_of_month().string() = " << libcpp::date_time::now().start_of_month().string() << std::endl;

    std::cout << "now().end_of_month().string() = " << libcpp::date_time::now().end_of_month().string() << std::endl;

    std::cout << "now().start_of_quarter().string() = " << libcpp::date_time::now().start_of_quarter().string() << std::endl;

    std::cout << "now().end_of_quarter().string() = " << libcpp::date_time::now().end_of_quarter().string() << std::endl;

    std::cout << "now().start_of_half_year().string() = " << libcpp::date_time::now().start_of_half_year().string() << std::endl;

    std::cout << "now().end_of_half_year().string() = " << libcpp::date_time::now().end_of_half_year().string() << std::endl;

    std::cout << "now().start_of_year().string() = " << libcpp::date_time::now().start_of_year().string() << std::endl;

    std::cout << "now().end_of_year().string() = " << libcpp::date_time::now().end_of_year().string() << std::endl;

    std::cout << "parse(\"2023-03-16 07:41:29\").next_day().string() = "
              << libcpp::date_time::parse("2023-03-16 07:41:29").next_day().string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").next_day_n(10).string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").next_day_n(10).string() << std::endl;

    std::cout << "parse(\"2023-03-01 00:00:00\").pre_day().string() = "
              << libcpp::date_time::parse("2023-03-01 00:00:00").pre_day().string() << std::endl;

    std::cout << "parse(\"2023-03-01 00:00:00\").pre_day_n(10).string() = "
              << libcpp::date_time::parse("2023-03-01 00:00:00").pre_day_n(10).string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").next_month().string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").next_month().string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").next_month_n(10).string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").next_month_n(10).string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").pre_month().string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").pre_month().string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").pre_month_n(10).string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").pre_month_n(10).string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").next_quarter().string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").next_quarter().string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").next_quarter_n(3).string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").next_quarter_n(3).string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").pre_quarter().string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").pre_quarter().string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").pre_quarter_n(3).string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").pre_quarter_n(3).string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").next_half_year().string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").next_half_year().string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").next_half_year_n(3).string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").next_half_year_n(3).string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").pre_half_year().string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").pre_half_year().string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").pre_half_year_n(3).string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").pre_half_year_n(3).string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").next_year().string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").next_year().string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").next_year_n(3).string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").next_year_n(3).string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").pre_year().string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").pre_year().string() << std::endl;

    std::cout << "parse(\"2023-02-28 00:00:00\").pre_year_n(3).string() = "
              << libcpp::date_time::parse("2023-02-28 00:00:00").pre_year_n(3).string() << std::endl;

    std::cin.get();
    return 0;
}