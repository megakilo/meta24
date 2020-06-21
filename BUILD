cc_library(
  name = "mp11",
  hdrs = glob(["mp11/include/**/*.hpp"]),
  includes = ["mp11/include/"],
)

cc_binary(
  name = "meta24",
  srcs = ["meta24.cc"],
  deps = [
    "//:mp11",
  ],
)
