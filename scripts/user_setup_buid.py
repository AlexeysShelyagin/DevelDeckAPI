Import("env")

env.Append(CPPDEFINES=["USER_SETUP_LOADED"])
env.Append(CCFLAGS=["-include", "../src/UserSetup_override.h"])