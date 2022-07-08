#
TEMPLATE = subdirs

# 定制编译顺序
CONFIG += ordered

# 子项目，一起编译时，是按此顺序编译
SUBDIRS += \
    0BusinessPro/Utilities.pro \
    0BusinessPro/LogBuild.pro \
    0BusinessPro/AtmStartPowerBuild.pro \
    1DriverPlatform/AgentBuild.pro \
    2DriverAndSP/SPBuild_CSCB.pro

