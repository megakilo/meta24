load("@rules_cc//cc:defs.bzl", "cc_test")

cc_library(
  name = "common",
  hdrs = ["common.h"],
)

cc_library(
  name = "mp11",
  hdrs = glob(["mp11/include/**/*.hpp"]),
  includes = ["mp11/include/"],
)

cc_library(
  name = "meta24_lib",
  hdrs = ["meta24.h"],
  deps = [
    "//:mp11",
    ":common",
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
  deps = [":meta24_lib"],
)

cc_library(
  name = "metal_lib",
  hdrs = glob(["metal/include/**/*.hpp"]),
  includes = ["metal/include/"],
)

cc_library(
  name = "meta24_metal_lib",
  hdrs = ["meta24_metal.h"],
  deps = [
    "common",
    ":metal_lib",
  ],
)

cc_test(
  name = "meta24_metal_test",
  size = "small",
  srcs = ["meta24_metal_test.cc"],
  deps = [
    "@googletest//:gtest_main",
    ":meta24_metal_lib"
  ],
)

cc_binary(
  name = "meta24_metal",
  srcs = ["meta24_metal.cc"],
  deps = [
    ":meta24_metal_lib",
  ],
)
