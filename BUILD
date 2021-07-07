
load("@rules_cc//cc:defs.bzl", "cc_test")

cc_library(
  name = "mp11",
  hdrs = glob(["mp11/include/**/*.hpp"]),
  includes = ["mp11/include/"],
)

cc_library(
  name = "meta24_lib",
  hdrs = ["meta24.h"],
  deps = [
    "//:mp11"
  ],
)

cc_test(
  name = "meta24_test",
  size = "small",
  srcs = ["meta24_test.cc"],
  deps = ["@googletest//:gtest_main", ":meta24_lib"],
)

cc_binary(
  name = "meta24",
  srcs = ["meta24.cc"],
  deps = [
    ":meta24_lib",
  ],
)
